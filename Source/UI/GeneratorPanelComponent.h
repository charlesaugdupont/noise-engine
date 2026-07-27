#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "../DSP/PatternGenerators.h"

// One-shot pattern-writing tools: a Euclidean rhythm generator, a
// density-random generator, and rotate/duplicate/halve utilities. All of
// them mutate pattern data directly (through the processor's thread-safe
// pattern API) rather than being host-automatable parameters — see
// StepPattern.h for why pattern data isn't parameter-backed.
//
// No standalone reverse — removed to keep the toolset small; rotate covers
// most of what people reached for it for.
//
// No rotation control on the Euclidean generator itself: that would just
// duplicate the general-purpose rotate-left/right buttons (generate at
// rotation 0, then nudge with < / > — works on any pattern, not just a
// freshly-generated Euclidean one, and it's one less control to learn).
//
// Pulses/Density/Jitter are plain UI-local state, not persisted anywhere
// yet — that lands with the preset system (Phase 4).
class GeneratorPanelComponent : public juce::Component,
                                 private juce::Timer
{
public:
    explicit GeneratorPanelComponent(NoiseEngineAudioProcessor& processorToUse);
    ~GeneratorPanelComponent() override;

    void resized() override;

    static constexpr int preferredHeight = 68;

private:
    NoiseEngineAudioProcessor& processor;
    juce::Random rng;
    int lastKnownLength = -1;

    juce::Label euclideanCaption, pulsesCaption;
    juce::Slider pulsesSlider;
    juce::TextButton applyEuclideanButton { "APPLY" };

    juce::Label randomCaption, densityCaption, jitterCaption;
    juce::Slider densitySlider, jitterSlider;
    juce::TextButton randomizeButton { "RANDOMIZE" };

    juce::TextButton rotateLeftButton  { "<" };
    juce::TextButton rotateRightButton { ">" };
    juce::TextButton duplicateButton   { "x2" };
    juce::TextButton halveButton       { "/2" };

    void timerCallback() override;
    void setupSlider(juce::Slider& slider, double minV, double maxV, double step, double defaultV);
    void mutatePattern(const std::function<void(StepPattern&)>& mutator);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorPanelComponent)
};
