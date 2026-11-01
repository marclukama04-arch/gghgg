#pragma once
#include "PluginProcessor.h"

class DCLook : public juce::LookAndFeel_V4
{
public:
    DCLook();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
};

class DCWorldEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DCWorldEditor (DCWorldProcessor&);
    ~DCWorldEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    void timerCallback() override;
    void setCategory (int c);
    void selectFirstOf (int c);
    void select (int presetIndex);
    void step (int d);

    DCWorldProcessor& proc;
    DCLook look;
    juce::Slider cutoff, reso, glide, attack, release, reverb, level;
    std::unique_ptr<SA> aCut, aRes, aGli, aAtt, aRel, aRev, aLev;
    juce::TextButton tab[3], prev { "<" }, next { ">" };
    juce::MidiKeyboardComponent keys;
    int cat = 0, shownPreset = -1, nRows = 0;
    juce::Rectangle<int> rows[6];
    int rowPreset[6] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DCWorldEditor)
};
