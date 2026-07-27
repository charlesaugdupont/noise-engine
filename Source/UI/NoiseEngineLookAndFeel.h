#pragma once
#include <JuceHeader.h>

// Custom LookAndFeel for the whole plugin. drawRotarySlider is a genuine
// custom paint (promoted from the class that used to sit locally at the
// top of PluginEditor.cpp). Everything else — ComboBox, PopupMenu,
// TextButton, ToggleButton, TextEditor, AlertWindow — is themed by setting
// colour IDs in the constructor rather than overriding their paint methods:
// LookAndFeel_V4's default rendering for those already reads from exactly
// these colour IDs, so this gets consistent theming (including things like
// hover/press states and disabled dimming, which V4 already handles well)
// without reimplementing paint code that can't be visually verified here.
class NoiseEngineLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NoiseEngineLookAndFeel();

    void drawRotarySlider(juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;
};
