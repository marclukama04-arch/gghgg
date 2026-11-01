#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout DCWorldProcessor::createLayout()
{
    using F = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<F> (juce::ParameterID { "cutoff", 1 },  "Cutoff", R (40.0f, 18000.0f, 0.0f, 0.3f), 2000.0f));
    l.add (std::make_unique<F> (juce::ParameterID { "reso", 1 },    "Reso",   R (0.0f, 0.95f, 0.001f), 0.1f));
    l.add (std::make_unique<F> (juce::ParameterID { "glide", 1 },   "Glide",  R (0.0f, 1.0f, 0.001f, 0.5f), 0.0f));
    l.add (std::make_unique<F> (juce::ParameterID { "attack", 1 },  "Attack", R (0.001f, 2.0f, 0.0f, 0.4f), 0.002f));
    l.add (std::make_unique<F> (juce::ParameterID { "release", 1 }, "Release", R (0.01f, 4.0f, 0.0f, 0.4f), 0.5f));
    l.add (std::make_unique<F> (juce::ParameterID { "reverb", 1 },  "Reverb", R (0.0f, 1.0f, 0.001f), 0.3f));
    l.add (std::make_unique<F> (juce::ParameterID { "level", 1 },   "Level",  R (0.0f, 1.0f, 0.001f), 0.8f));
    l.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "preset", 1 }, "Preset", 0, 15, 0));
    return l;
}

DCWorldProcessor::DCWorldProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

int DCWorldProcessor::getPreset() const
{
    return juce::jlimit (0, 15, (int) apvts.getRawParameterValue ("preset")->load());
}

void DCWorldProcessor::loadPreset (int index)
{
    index = juce::jlimit (0, 15, index);
    const auto& p = tsh::kPresets[index];
    auto set = [this] (const char* id, float v)
    {
        if (auto* prm = apvts.getParameter (id)) prm->setValueNotifyingHost (prm->convertTo0to1 (v));
    };
    set ("preset", (float) index);
    set ("cutoff", p.cutoff);   set ("reso", p.reso);       set ("glide", p.glide);
    set ("attack", p.attack);   set ("release", p.release); set ("reverb", p.reverb);
}

void DCWorldProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
    reverb.setSampleRate (sampleRate);
    reverb.reset();
    lastPreset = -1;
}

void DCWorldProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    const int idx = getPreset();
    if (idx != lastPreset) { engine.setPreset (idx); lastPreset = idx; }

    auto val = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
    engine.setParams ({ val ("cutoff"), val ("reso"), val ("glide"), val ("attack"), val ("release") });

    juce::Reverb::Parameters rp;
    rp.roomSize = 0.65f; rp.damping = 0.45f; rp.width = 1.0f; rp.freezeMode = 0.0f;
    rp.wetLevel = val ("reverb") * 0.45f; rp.dryLevel = 1.0f - 0.3f * val ("reverb");
    reverb.setParameters (rp);

    float* left = buffer.getWritePointer (0);
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : left;
    int pos = 0;
    auto renderTo = [&] (int end)
    {
        for (; pos < end; ++pos) { const float s = engine.tick(); left[pos] = s; right[pos] = s; }
    };

    for (const auto meta : midi)
    {
        renderTo (juce::jlimit (0, n, meta.samplePosition));
        const auto m = meta.getMessage();
        if (m.isNoteOn())                          engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())                    engine.noteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) engine.allOff();
    }
    renderTo (n);

    buffer.applyGain (val ("level"));
    if (buffer.getNumChannels() > 1) reverb.processStereo (left, right, n);
}

juce::AudioProcessorEditor* DCWorldProcessor::createEditor() { return new DCWorldEditor (*this); }

void DCWorldProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void DCWorldProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DCWorldProcessor(); }
