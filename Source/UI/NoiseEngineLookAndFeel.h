#pragma once
#include <JuceHeader.h>

// Custom LookAndFeel for a clean, modern rotary knob. Promoted from the
// class that used to be defined locally at the top of PluginEditor.cpp.
class NoiseEngineLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;
};
