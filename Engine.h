// 16 DC WORLD - moteur de son (C++ pur, sans JUCE, donc testable seul)
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace tsh
{
constexpr float kPi = 3.14159265358979f;
constexpr float kTwoPi = 6.28318530717959f;

enum Wave { S, T, W, Q, F };   // sinus, triangle, scie, carre, FM

struct Preset
{
    const char* name; int cat; bool mono;           // cat: 0 piano, 1 lead, 2 basse
    Wave a, b; float bRatio, detune, mixB, sub, click;
    float fmRatio, fmIndex, fmDecay;
    float pitchDrop, dropTime, vib;
    float envAmt, fDecay, fLfo;
    float decay, sus;
    float cutoff, reso, glide, attack, release, reverb;   // valeurs des boutons
};

inline const Preset kPresets[16] = {
 {"Midnight Keys",0,false, T,S,2.f,3.f,.35f,.25f,.12f, 0,0,1.f, 0,.05f,0, 1.5f,.5f,0, 1.1f,0.f, 2200,.1f,0,.002f,.6f,.25f},
 {"Lil Glass",0,false, F,S,4.f,0,.15f,.1f,.05f, 2.f,2.5f,.6f, 0,.05f,0, .8f,.4f,0, 1.6f,0.f, 9000,.05f,0,.002f,.9f,.35f},
 {"Rain Piano",0,false, S,T,2.f,5.f,.3f,.15f,.15f, 0,0,1.f, 0,.05f,0, 1.f,.6f,0, 1.8f,0.f, 3500,.1f,0,.002f,1.f,.5f},
 {"Cold Chords",0,false, W,T,1.f,8.f,.6f,.2f,.08f, 0,0,1.f, 0,.05f,0, 2.f,.35f,0, .9f,0.f, 1600,.15f,0,.002f,.5f,.3f},
 {"Haze Keys",0,false, T,S,1.f,12.f,.8f,.3f,.05f, 0,0,1.f, 0,.05f,0, .8f,.8f,0, 2.2f,.08f, 1800,.1f,0,.004f,1.2f,.55f},
 {"Dusty Trap",0,false, S,T,3.f,2.f,.12f,.1f,.3f, 0,0,1.f, 0,.05f,0, 1.2f,.3f,0, .8f,0.f, 1400,.05f,0,.002f,.4f,.2f},
 {"Bubblegum",1,false, W,W,1.f,9.f,1.f,0,0, 0,0,1.f, 0,.05f,.12f, .6f,.3f,0, .3f,.8f, 6500,.15f,0,.005f,.35f,.3f},
 {"Sugar Rush",1,false, Q,W,1.f,7.f,.7f,0,0, 0,0,1.f, 0,.05f,.2f, .5f,.3f,0, .3f,.85f, 9000,.1f,0,.005f,.35f,.35f},
 {"Cotton Candy",1,false, F,S,2.f,0,.3f,0,0, 1.f,1.2f,1.5f, 0,.05f,.15f, .3f,.4f,0, .4f,.9f, 12000,.05f,0,.02f,.5f,.45f},
 {"Heartbeat",1,true, W,Q,2.f,3.f,.4f,0,0, 0,0,1.f, 0,.05f,.1f, 1.2f,.4f,0, .3f,.8f, 5000,.25f,.08f,.005f,.3f,.3f},
 {"Lollipop",1,false, T,S,2.f,0,.5f,0,0, 0,0,1.f, 0,.05f,.25f, .2f,.4f,0, .3f,.9f, 14000,.05f,0,.01f,.4f,.4f},
 {"808 Sub",2,true, S,S,1.f,0,0,0,.1f, 0,0,1.f, 7.f,.06f,0, 0,.3f,0, 1.8f,.15f, 3000,.05f,.06f,.002f,.25f,0},
 {"808 Dirty",2,true, S,W,1.f,0,.25f,0,.1f, 0,0,1.f, 5.f,.05f,0, .6f,.3f,0, 1.5f,.2f, 1800,.1f,.06f,.002f,.25f,0},
 {"Purple Sub",2,true, S,S,1.f,0,0,.5f,0, 0,0,1.f, 0,.05f,0, 0,.3f,0, .3f,1.f, 900,.05f,.1f,.005f,.2f,.05f},
 {"Reese",2,true, W,W,1.f,18.f,1.f,.4f,0, 0,0,1.f, 0,.05f,0, .8f,.5f,0, .3f,1.f, 700,.2f,.04f,.01f,.3f,.1f},
 {"Wobble",2,true, Q,W,1.f,6.f,.6f,.4f,0, 0,0,1.f, 0,.05f,0, 1.f,.4f,1.5f, .3f,.9f, 600,.35f,.03f,.005f,.25f,.1f}
};

struct Params { float cutoff = 2000.f, reso = .1f, glide = 0.f, attack = .002f, release = .5f; };

inline float blep (float t, float dt)
{
    if (t < dt)          { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt)   { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

inline float osc (Wave w, float& ph, float inc)
{
    inc = std::min (inc, 0.45f);
    float y = 0.0f;
    switch (w)
    {
        case S: y = std::sin (kTwoPi * ph); break;
        case T: y = 1.0f - 4.0f * std::fabs (ph - 0.5f); break;
        case W: y = 2.0f * ph - 1.0f - blep (ph, inc); break;
        case Q: { float p2 = ph + 0.5f; if (p2 >= 1.0f) p2 -= 1.0f;
                  y = (ph < 0.5f ? 1.0f : -1.0f) + blep (ph, inc) - blep (p2, inc); break; }
        default: break;
    }
    ph += inc; if (ph >= 1.0f) ph -= 1.0f;
    return y;
}

class Voice
{
public:
    bool active = false, held = false; int note = -1; float vel = 0.f; uint64_t order = 0;

    void noteOn (int n, float v, bool legato, bool fromCurrent)
    {
        note = n; vel = v; target = 440.0f * std::exp2 ((n - 69) / 12.0f);
        if (! legato) { t = 0; env = 0; stage = Attack; phA = phB = phS = phM = 0; ic1 = ic2 = 0; }
        if (! fromCurrent || ! active) freq = target;
        active = true; held = true;
    }
    void noteOff() { held = false; if (active) stage = Release; }
    void kill() { active = false; held = false; stage = Off; env = 0; }

    float tick (const Preset& p, const Params& pr, float sr)
    {
        if (! active) return 0.0f;
        const float dt = 1.0f / sr; t += dt;
        if (p.mono && pr.glide > 0.0005f) freq += (target - freq) * (1.0f - std::exp (-dt / pr.glide));
        else freq = target;
        lfo += 5.5f * dt;  if (lfo  >= 1.f) lfo  -= 1.f;
        lfo2 += 2.5f * dt; if (lfo2 >= 1.f) lfo2 -= 1.f;

        const float semi = p.pitchDrop * std::exp (-t / p.dropTime)
                         + p.vib * std::sin (kTwoPi * lfo) * std::min (1.0f, t * 4.0f);
        const float inc = std::min (0.45f, freq * std::exp2 (semi / 12.0f) * dt);

        float a;
        if (p.a == F)
        {
            const float idx = p.fmIndex * std::exp (-t / p.fmDecay);
            const float m = std::sin (kTwoPi * phM);
            phM += inc * p.fmRatio; phM -= std::floor (phM);
            a = std::sin (kTwoPi * phA + idx * m);
            phA += inc; phA -= std::floor (phA);
        }
        else a = osc (p.a, phA, inc);

        float b = 0.0f;
        if (p.mixB > 0.0f) b = osc (p.b, phB, inc * p.bRatio * std::exp2 (p.detune / 1200.0f));

        const float sub = std::sin (kTwoPi * phS);
        phS += inc * 0.5f; if (phS >= 1.f) phS -= 1.f;

        rng = rng * 1664525u + 1013904223u;
        const float noise = (float) (rng >> 8) * (1.0f / 8388608.0f) - 1.0f;
        const float s = 0.6f * (a + p.mixB * b + p.sub * sub) + noise * p.click * std::exp (-t / 0.012f);

        float fc = pr.cutoff * std::exp2 (p.envAmt * std::exp (-t / p.fDecay) * (0.5f + 0.5f * vel)
                                          + p.fLfo * std::sin (kTwoPi * lfo2));
        fc = std::clamp (fc, 30.0f, std::min (18000.0f, sr * 0.45f));
        const float g = std::tan (kPi * fc / sr), k = 2.0f - 1.9f * std::min (pr.reso, 0.95f);
        const float a1 = 1.0f / (1.0f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = s - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1; ic2 = 2.0f * v2 - ic2;

        switch (stage)
        {
            case Attack:  env += dt / std::max (0.0005f, pr.attack); if (env >= 1.0f) { env = 1.0f; stage = Decay; } break;
            case Decay:
                env = p.sus + (env - p.sus) * std::exp (-4.6f * dt / p.decay);
                if (p.sus <= 0.0f && env < 1e-4f) kill();
                break;
            case Release: env *= std::exp (-4.6f * dt / std::max (0.01f, pr.release)); if (env < 1e-4f) kill(); break;
            default: break;
        }
        return v2 * env * (0.3f + 0.7f * vel);
    }

private:
    enum Stage { Off, Attack, Decay, Release };
    Stage stage = Off; float freq = 440.f, target = 440.f, t = 0, env = 0, lfo = 0, lfo2 = 0;
    float phA = 0, phB = 0, phS = 0, phM = 0, ic1 = 0, ic2 = 0; uint32_t rng = 12345u;
};

class Engine
{
public:
    void prepare (double sampleRate) { sr = (float) sampleRate; allOff(); }
    void setPreset (int i) { i = std::clamp (i, 0, 15); if (i != cur) { cur = i; allOff(); } }
    void setParams (const Params& p) { pr = p; }
    int activeVoices() const { int n = 0; for (auto& v : voices) if (v.active) ++n; return n; }

    void allOff() { for (auto& v : voices) v.kill(); nHeld = 0; }

    void noteOn (int n, float vel)
    {
        if (kPresets[cur].mono)
        {
            removeHeld (n); if (nHeld < 16) held[nHeld++] = n;
            Voice& v = voices[0];
            v.noteOn (n, vel, v.active && v.held, true);
        }
        else
        {
            Voice* pick = nullptr;
            for (auto& v : voices) if (! v.active) { pick = &v; break; }
            if (! pick) { pick = &voices[0]; for (auto& v : voices) if (v.order < pick->order) pick = &v; }
            pick->noteOn (n, vel, false, false); pick->order = ++counter;
        }
    }

    void noteOff (int n)
    {
        if (kPresets[cur].mono)
        {
            removeHeld (n);
            if (nHeld > 0) voices[0].noteOn (held[nHeld - 1], voices[0].vel, true, true);
            else voices[0].noteOff();
        }
        else for (auto& v : voices) if (v.active && v.held && v.note == n) v.noteOff();
    }

    float tick()
    {
        float s = 0.0f;
        for (auto& v : voices) s += v.tick (kPresets[cur], pr, sr);
        return std::tanh (s * 0.6f);
    }

private:
    void removeHeld (int n)
    {
        int j = 0; for (int i = 0; i < nHeld; ++i) if (held[i] != n) held[j++] = held[i];
        nHeld = j;
    }
    Voice voices[8]; Params pr; float sr = 44100.f; int cur = 0; uint64_t counter = 0;
    int held[16] = {}; int nHeld = 0;
};
}
