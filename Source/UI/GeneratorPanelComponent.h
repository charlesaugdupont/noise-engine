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
// Rotate-left/right live on StepSequencerComponent now, flanking the wheel
// — they're about repositioning the pattern that's already there, which
// reads more as "a wheel control" than "a generator", unlike Euclid/Random/
// duplicate/halve which all write new pattern content.
//
// Pulses/Density/Jitter are plain UI-local state, not persisted anywhere
// yet — that lands with the preset system (Phase 4).
//
// Bordered/titled like MacroSectionComponent's panels (own paint(), not a
// shared base — the two components' layout logic differs enough that
// factoring out the ~10 lines of chrome wasn't worth the coupling risk to
// MacroSectionComponent, which was already visually verified working).
class GeneratorPanelComponent : public juce::Component,
                                 private juce::Timer
{
public:
    explicit GeneratorPanelComponent(NoiseEngineAudioProcessor& processorToUse);
    ~GeneratorPanelComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int preferredHeight = 138;

private:
    NoiseEngineAudioProcessor& processor;
    juce::Random rng;
    int lastKnownLength = -1;

    juce::Label titleLabel;

    juce::Label euclideanCaption, pulsesCaption;
    juce::Slider pulsesSlider;
    juce::TextButton applyEuclideanButton { "APPLY" };

    juce::Label randomCaption, densityCaption, jitterCaption;
    juce::Slider densitySlider, jitterSlider;

    // Custom-painted die-face icon rather than a Unicode glyph in a
    // TextButton — glyph size is capped by LookAndFeel::getTextButtonFont
    // (min(16px, height*0.6)), which reads as small; drawing the die
    // ourselves sizes it purely from the button's bounds instead.
    class DiceButton : public juce::Button
    {
    public:
        DiceButton() : juce::Button("Randomize") {}
        void paintButton(juce::Graphics&, bool isMouseOverButton, bool isButtonDown) override;
    };
    DiceButton randomizeButton;

    juce::TextButton halveButton     { "0.5x" };
    juce::TextButton duplicateButton { "2x" };

    void timerCallback() override;
    void setupSlider(juce::Slider& slider, double minV, double maxV, double step, double defaultV);
    void mutatePattern(const std::function<void(StepPattern&)>& mutator);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorPanelComponent)
};
