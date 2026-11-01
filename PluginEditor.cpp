#include "PluginEditor.h"
using namespace juce;

namespace
{
const Colour kPurple (0xffa855f7), kLilac (0xffd8b4fe), kDeep (0xff7e22ce), kInk (0xff0b0713), kPanel (0xbf140a23);
const char* kCatNames[3] = { "TRAP PIANO", "CANDY LEAD", "BASS" };

void heart (Graphics& g, float x, float y, float size, float alpha)
{
    Path p;
    p.startNewSubPath (12.0f, 21.5f);
    p.cubicTo (5.5f, 17.0f, 2.0f, 13.2f, 2.0f, 9.2f);
    p.cubicTo (2.0f, 6.2f, 4.3f, 4.0f, 7.0f, 4.0f);
    p.cubicTo (9.0f, 4.0f, 10.8f, 5.0f, 12.0f, 7.0f);
    p.cubicTo (13.2f, 5.0f, 15.0f, 4.0f, 17.0f, 4.0f);
    p.cubicTo (19.7f, 4.0f, 22.0f, 6.2f, 22.0f, 9.2f);
    p.cubicTo (22.0f, 13.2f, 18.5f, 17.0f, 12.0f, 21.5f);
    p.closeSubPath();
    g.setColour (kPurple.withAlpha (alpha));
    g.fillPath (p, AffineTransform::scale (size / 24.0f).translated (x, y));
}

void panel (Graphics& g, Rectangle<int> r)
{
    g.setColour (kPanel);        g.fillRoundedRectangle (r.toFloat(), 18.0f);
    g.setColour (kDeep);         g.drawRoundedRectangle (r.toFloat().reduced (1.0f), 18.0f, 2.0f);
}

void label (Graphics& g, const String& t, Rectangle<int> r, float size, Colour c, bool bold = false)
{
    g.setColour (c); g.setFont (Font (size, bold ? Font::bold : Font::plain));
    g.drawText (t, r, Justification::centred);
}
}

DCLook::DCLook()
{
    setColour (Slider::thumbColourId, kLilac);
    setColour (Slider::trackColourId, kPurple);
    setColour (Slider::backgroundColourId, Colour (0xff1a0f2e));
}

void DCLook::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, Slider&)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float r = jmin (b.getWidth(), b.getHeight()) / 2.0f;
    const auto c = b.getCentre();
    g.setColour (Colour (0xff1a0f2e)); g.fillEllipse (c.x - r, c.y - r, 2 * r, 2 * r);
    g.setColour (kPurple);             g.drawEllipse (c.x - r, c.y - r, 2 * r, 2 * r, 3.0f);
    const float ang = a0 + (a1 - a0) * pos;
    Path arc; arc.addCentredArc (c.x, c.y, r - 8.0f, r - 8.0f, 0.0f, a0, ang, true);
    g.setColour (Colour (0xffc084fc)); g.strokePath (arc, PathStrokeType (3.0f));
    g.setColour (kLilac);
    g.drawLine (c.x + std::sin (ang) * r * 0.15f, c.y - std::cos (ang) * r * 0.15f,
                c.x + std::sin (ang) * r * 0.62f, c.y - std::cos (ang) * r * 0.62f, 4.0f);
}

DCWorldEditor::DCWorldEditor (DCWorldProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      keys (p.keyboardState, MidiKeyboardComponent::horizontalKeyboard)
{
    setSize (1000, 620);

    auto setup = [this] (Slider& s, const char* id, std::unique_ptr<SA>& att, bool rotary)
    {
        s.setSliderStyle (rotary ? Slider::RotaryHorizontalVerticalDrag : Slider::LinearHorizontal);
        s.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        if (rotary) s.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
        s.setLookAndFeel (&look);
        addAndMakeVisible (s);
        att = std::make_unique<SA> (proc.apvts, id, s);
    };
    setup (cutoff, "cutoff", aCut, true);  setup (reso, "reso", aRes, true);   setup (glide, "glide", aGli, true);
    setup (attack, "attack", aAtt, true);  setup (release, "release", aRel, true); setup (reverb, "reverb", aRev, true);
    setup (level, "level", aLev, false);

    for (int i = 0; i < 3; ++i)
    {
        auto& b = tab[i];
        b.setButtonText (kCatNames[i]);
        b.setColour (TextButton::buttonColourId, Colours::transparentBlack);
        b.setColour (TextButton::buttonOnColourId, kPurple);
        b.setColour (TextButton::textColourOffId, kLilac);
        b.setColour (TextButton::textColourOnId, kInk);
        b.onClick = [this, i] { selectFirstOf (i); };
        addAndMakeVisible (b);
    }
    for (auto* b : { &prev, &next })
    {
        b->setColour (TextButton::buttonColourId, Colour (0xff2a1248));
        b->setColour (TextButton::textColourOffId, kLilac);
        addAndMakeVisible (*b);
    }
    prev.onClick = [this] { step (-1); };
    next.onClick = [this] { step (1); };

    keys.setWantsKeyboardFocus (false);
    keys.setScrollButtonsVisible (false);
    keys.setAvailableRange (48, 83);
    keys.setKeyWidth (44.19f);
    keys.setColour (MidiKeyboardComponent::whiteNoteColourId, Colour (0xfff3e8ff));
    keys.setColour (MidiKeyboardComponent::blackNoteColourId, Colour (0xff12091f));
    keys.setColour (MidiKeyboardComponent::keySeparatorLineColourId, kPurple);
    keys.setColour (MidiKeyboardComponent::keyDownOverlayColourId, kPurple.withAlpha (0.8f));
    keys.setColour (MidiKeyboardComponent::mouseOverKeyOverlayColourId, Colour (0xffc084fc).withAlpha (0.4f));
    addAndMakeVisible (keys);

    shownPreset = proc.getPreset();
    setCategory (tsh::kPresets[shownPreset].cat);
    startTimerHz (15);
}

DCWorldEditor::~DCWorldEditor()
{
    stopTimer();
    for (auto* s : { &cutoff, &reso, &glide, &attack, &release, &reverb, &level }) s->setLookAndFeel (nullptr);
}

void DCWorldEditor::timerCallback()
{
    const int p = proc.getPreset();
    if (p != shownPreset) { shownPreset = p; setCategory (tsh::kPresets[p].cat); }
}

void DCWorldEditor::setCategory (int c)
{
    cat = c;
    for (int i = 0; i < 3; ++i) tab[i].setToggleState (i == c, dontSendNotification);
    nRows = 0;
    for (int i = 0; i < 16 && nRows < 6; ++i)
        if (tsh::kPresets[i].cat == c)
        {
            rowPreset[nRows] = i;
            rows[nRows] = Rectangle<int> (48, 214 + nRows * 36, 206, 32);
            ++nRows;
        }
    repaint();
}

void DCWorldEditor::selectFirstOf (int c)
{
    for (int i = 0; i < 16; ++i) if (tsh::kPresets[i].cat == c) { select (i); return; }
}

void DCWorldEditor::select (int i)
{
    proc.loadPreset (i);
    shownPreset = proc.getPreset();
    setCategory (tsh::kPresets[shownPreset].cat);
}

void DCWorldEditor::step (int d) { select ((proc.getPreset() + d + 16) % 16); }

void DCWorldEditor::mouseDown (const MouseEvent& e)
{
    for (int i = 0; i < nRows; ++i) if (rows[i].contains (e.getPosition())) { select (rowPreset[i]); return; }
}

void DCWorldEditor::resized()
{
    tab[0].setBounds (36, 120, 140, 40);
    tab[1].setBounds (186, 120, 140, 40);
    tab[2].setBounds (336, 120, 100, 40);
    prev.setBounds (568, 32, 36, 40);
    next.setBounds (920, 32, 36, 40);
    auto knob = [] (Slider& s, int cx, int cy) { s.setBounds (cx - 38, cy - 38, 76, 76); };
    knob (cutoff, 374, 268); knob (reso, 514, 268); knob (glide, 654, 268);
    knob (attack, 374, 372); knob (release, 514, 372); knob (reverb, 654, 372);
    level.setBounds (758, 358, 190, 22);
    keys.setBounds (36, 462, 928, 130);
}

void DCWorldEditor::paint (Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (ColourGradient (Colour (0xff3b0f66), 200.0f, 0.0f, Colour (0xff07040d), 800.0f, 620.0f, true));
    g.fillRoundedRectangle (bounds, 28.0f);

    heart (g, 900, 100, 60, 0.35f);  heart (g, 14, 300, 40, 0.3f);   heart (g, 610, 140, 30, 0.5f);
    heart (g, 930, 430, 70, 0.25f);  heart (g, 270, 92, 26, 0.6f);   heart (g, 470, 588, 30, 0.4f);
    heart (g, 8, 556, 50, 0.3f);

    g.setColour (kPurple); g.drawRoundedRectangle (bounds.reduced (2.0f), 28.0f, 4.0f);

    // titre
    heart (g, 36, 22, 52, 1.0f);
    {
        GlyphArrangement ga; ga.addLineOfText (Font (40.0f, Font::bold), "16 DC WORLD", 100.0f, 62.0f);
        Path tp; ga.createPath (tp);
        g.setColour (kPurple.withAlpha (0.35f)); g.fillPath (tp, AffineTransform::translation (0.0f, 3.0f));
        g.setGradientFill (ColourGradient (Colour (0xffe9d5ff), 0.0f, 30.0f, Colour (0xff6d28d9), 0.0f, 66.0f, false));
        g.fillPath (tp);
    }
    label (g, "SYNTH  -  TRAP x CANDY x BASS", Rectangle<int> (100, 68, 380, 18), 12.0f, Colour (0xffc084fc));

    // afficheur de preset
    const int cur = proc.getPreset();
    const auto& pr = tsh::kPresets[cur];
    g.setColour (kInk); g.fillRoundedRectangle (560.0f, 24.0f, 404.0f, 56.0f, 14.0f);
    g.setColour (kPurple); g.drawRoundedRectangle (561.0f, 25.0f, 402.0f, 54.0f, 14.0f, 2.0f);
    label (g, String (pr.name).toUpperCase(), Rectangle<int> (610, 30, 300, 28), 20.0f, kLilac, true);
    label (g, String (kCatNames[pr.cat]) + "  -  " + String (cur + 1).paddedLeft ('0', 2) + "/16",
           Rectangle<int> (610, 56, 300, 16), 10.0f, Colour (0xff9333ea));

    // autocollant
    {
        Graphics::ScopedSaveState ss (g);
        g.addTransform (AffineTransform::rotation (-0.07f, 860.0f, 139.0f));
        Path r, d; r.addRectangle (770.0f, 124.0f, 180.0f, 30.0f);
        const float dash[] = { 5.0f, 4.0f };
        PathStrokeType (2.0f).createDashedStroke (d, r, dash, 2);
        g.setColour (kLilac); g.fillPath (d);
        label (g, "Y2K EDITION  -  V1.0", Rectangle<int> (770, 124, 180, 30), 11.0f, kLilac);
    }

    // panneaux
    panel (g, Rectangle<int> (36, 176, 250, 264));
    panel (g, Rectangle<int> (304, 176, 420, 264));
    panel (g, Rectangle<int> (742, 176, 222, 264));
    label (g, "PRESETS", Rectangle<int> (48, 184, 100, 20), 11.0f, Colour (0xff9333ea));
    label (g, "SOUND SHAPING", Rectangle<int> (316, 184, 140, 20), 11.0f, Colour (0xff9333ea));
    label (g, "WAVE", Rectangle<int> (754, 184, 60, 20), 11.0f, Colour (0xff9333ea));

    for (int i = 0; i < nRows; ++i)
    {
        const bool on = rowPreset[i] == cur;
        g.setColour (on ? kLilac : Colour (0xffe9d5ff).withAlpha (0.8f));
        g.setFont (Font (14.0f, on ? Font::bold : Font::plain));
        if (on) heart (g, (float) rows[i].getX(), (float) rows[i].getY() + 8.0f, 16.0f, 1.0f);
        g.drawText (String (tsh::kPresets[rowPreset[i]].name), rows[i].withTrimmedLeft (24), Justification::centredLeft);
    }

    const char* names[6] = { "CUTOFF", "RESO", "GLIDE", "ATTACK", "RELEASE", "REVERB" };
    const int cx[3] = { 374, 514, 654 };
    for (int i = 0; i < 6; ++i)
        label (g, names[i], Rectangle<int> (cx[i % 3] - 50, (i < 3 ? 268 : 372) + 44, 100, 16), 11.0f, Colour (0xffc084fc));

    // forme d'onde
    const Rectangle<float> wr (758.0f, 214.0f, 186.0f, 110.0f);
    g.setColour (kInk); g.fillRoundedRectangle (wr, 10.0f);
    g.setColour (kDeep); g.drawRoundedRectangle (wr, 10.0f, 1.0f);
    Path wp;
    for (int x = 0; x <= 170; ++x)
    {
        const float ph = std::fmod ((float) x / 56.0f, 1.0f);
        float y;
        switch (pr.a)
        {
            case tsh::T: y = 1.0f - 4.0f * std::fabs (ph - 0.5f); break;
            case tsh::W: y = 2.0f * ph - 1.0f; break;
            case tsh::Q: y = ph < 0.5f ? 1.0f : -1.0f; break;
            default:     y = std::sin (ph * tsh::kTwoPi); break;
        }
        const float px = wr.getX() + 8.0f + (float) x, py = wr.getCentreY() - y * 36.0f;
        if (x == 0) wp.startNewSubPath (px, py); else wp.lineTo (px, py);
    }
    g.setColour (Colour (0xffc084fc)); g.strokePath (wp, PathStrokeType (3.0f));
    label (g, "LEVEL", Rectangle<int> (754, 332, 60, 18), 11.0f, Colour (0xffc084fc));
}
