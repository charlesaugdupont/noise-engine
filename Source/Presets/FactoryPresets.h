#pragma once
#include <JuceHeader.h>
#include "../DSP/StepPattern.h"

// One factory preset: a set of macro-parameter raw values (keyed by
// ParamIDs string, in that parameter's natural range — e.g. rate as an
// index 0-6, attack as 0-100) plus a step pattern. Applied programmatically
// (PresetManager sets each parameter directly) rather than stored as
// hand-written XML — that sidesteps needing to match APVTS's internal
// ValueTree schema by hand, which isn't something worth risking a silent
// mismatch on for content that can just be plain data instead.
struct FactoryPresetDefinition
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> paramValues;
    StepPattern pattern;
};

namespace FactoryPresets
{
    std::vector<FactoryPresetDefinition> createAll();
}
