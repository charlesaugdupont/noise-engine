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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseEngineAudioEditor)
};