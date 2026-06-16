#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class NoiseEngineAudioEditor : public juce::AudioProcessorEditor
{
public:
    NoiseEngineAudioEditor(NoiseEngineAudioProcessor&);
    ~NoiseEngineAudioEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    NoiseEngineAudioProcessor& audioProcessor;

    // --- Knobs ---
    juce::Slider bitDepthKnob;
    juce::Slider downsampleKnob;
    juce::Slider mixKnob;

    // --- Labels ---
    juce::Label bitDepthLabel;
    juce::Label downsampleLabel;
    juce::Label mixLabel;

    // --- APVTS Attachments ---
    // These keep the knobs in sync with the processor parameters automatically
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> downsampleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseEngineAudioEditor)
};
