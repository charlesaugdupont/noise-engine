#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "StepWheelComponent.h"

// Outer shell around the step wheel: just the Length slider header — the
// wheel itself shows the whole pattern at once, so no scrolling/paging is
// needed here (unlike the earlier linear-grid design).
class StepSequencerComponent : public juce::Component,
                                private juce::Timer
{
public:
    explicit StepSequencerComponent(NoiseEngineAudioProcessor& processorToUse);
    ~StepSequencerComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int headerHeight    = 26;
    static constexpr int headerGridGap   = 4;
    static constexpr int wheelSize       = 260;
    static constexpr int preferredHeight = headerHeight + headerGridGap + wheelSize;

private:
    NoiseEngineAudioProcessor& processor;

    juce::Slider   lengthSlider;
    juce::Label    lengthCaption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lengthAttachment;

    StepWheelComponent wheel;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepSequencerComponent)
};
