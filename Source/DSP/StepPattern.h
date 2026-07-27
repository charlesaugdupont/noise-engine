#pragma once
#include <JuceHeader.h>
#include <array>

// One step's data. `enabled` is kept distinct from `level == 0` so toggling
// a step off doesn't destroy a previously dialed-in level.
struct StepData
{
    float level   = 1.0f; // 0..1
    bool  enabled = true;
};

// Fixed-capacity pattern data. Not a parameter — pattern data is stored as
// custom ValueTree state alongside the APVTS state, not as individual
// AudioProcessorParameters (see plan doc, Section 3, for reasoning).
struct StepPattern
{
    static constexpr int maxSteps = 64;

    std::array<StepData, maxSteps> steps;
    int length = 16;

    StepPattern();

    juce::ValueTree toValueTree() const;
    static StepPattern fromValueTree(const juce::ValueTree& tree);

    static const juce::Identifier patternType;
    static const juce::Identifier stepType;
};
