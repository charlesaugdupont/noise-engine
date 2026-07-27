#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

// Circular "wheel" step editor (kilohearts Trance Gate-style): steps are
// wedges arranged around a ring instead of bars in a row. The whole pattern
// is always visible at once regardless of length — no scrolling/paging —
// since the ring just subdivides into more/thinner wedges as Length grows,
// and fewer/wider wedges as it shrinks.
//
// Per-step probability was tried as a second inner ring and dropped — at
// any interesting step count the wedges were too small to use. Probability
// is now a single global "Probability" macro knob instead (how likely any
// ON step is to actually fire each pass).
//
// The ring's hollow centre hosts a power button bound to Bypass — this used
// to be a plain toggle in the macro grid, but that duplicated the wheel's
// otherwise-empty middle for no reason. The current step is shown as a
// bright arc in a dedicated band just outside the main ring, rather than a
// translucent tint behind the wedges — the tint got lost against similarly
// coloured wedge fills, especially at fast rates.
class StepWheelComponent : public juce::Component,
                            public juce::TooltipClient
{
public:
    explicit StepWheelComponent(NoiseEngineAudioProcessor& processorToUse);

    void paint(juce::Graphics&) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;

    // Dynamic rather than a single setTooltip() string — this one component
    // covers both the ring (per-step editing) and the centre power button
    // (bypass), which want completely different explanations. Reuses
    // resolveHit() against the current mouse position rather than tracking
    // hover state separately.
    juce::String getTooltip() override;

    int getPatternLength() const noexcept { return pattern.length; }

    // Pulls the full current pattern (not just length) from the processor.
    // Called every timer tick from the parent, so it picks up edits from
    // anywhere else — the generator panel's Euclidean/Randomize/rotate/
    // reverse/duplicate buttons all write through the processor, not
    // through this component, so without this the wheel would keep
    // displaying its own stale copy (and worse, the next drag on the wheel
    // would push that stale copy back out, silently reverting whatever the
    // generator panel just did).
    void syncFromProcessor();

private:
    struct HitResult
    {
        int   stepIndex      = -1;
        bool  inRing         = false;
        bool  inCenter       = false;
        float radialFraction = 0.0f;
    };

    NoiseEngineAudioProcessor& processor;
    StepPattern pattern;

    juce::Point<float> center;
    float ringInnerR = 0.0f, ringOuterR = 0.0f;
    float playheadInnerR = 0.0f, playheadOuterR = 0.0f;

    juce::Point<float> dragStartPos;
    bool isDragging          = false;
    int  dragStepIndex       = -1;
    bool dragStartedInCenter = false;
    bool suppressNextToggle  = false; // consumes the mouseUp that follows a double-click

    HitResult resolveHit(juce::Point<float> pos) const;
    void applyEdit(const HitResult& hit);
    void toggleStep(int stepIndex);
    void resetStepToFull(int stepIndex);
    void pushPatternToProcessor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepWheelComponent)
};
