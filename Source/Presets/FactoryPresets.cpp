#include "FactoryPresets.h"
#include "../DSP/ParameterIDs.h"
#include "../DSP/PatternGenerators.h"

namespace
{
    // Small fluent builder so each preset below reads as "what's different
    // from the plugin's own defaults" rather than a wall of positional
    // arguments. PresetManager resets every parameter to its default before
    // applying these, so a preset only needs to list what it actually cares
    // about.
    class Builder
    {
    public:
        explicit Builder(juce::String presetName) { def.name = std::move(presetName); }

        Builder& rate(int idx)                  { return set(ParamIDs::rate, (float) idx); }
        Builder& rateMod(int idx)                { return set(ParamIDs::rateModifier, (float) idx); }
        Builder& ahr(float a, float h, float r)  { set(ParamIDs::attack, a); set(ParamIDs::hold, h); return set(ParamIDs::release, r); }
        Builder& swing(float v)                  { return set(ParamIDs::swing, v); }
        Builder& stereoMode(int idx)             { return set(ParamIDs::stereoMode, (float) idx); }
        Builder& stereoOffset(float v)            { return set(ParamIDs::stereoOffset, v); }
        Builder& depth(float v)                  { return set(ParamIDs::depth, v); }
        Builder& mix(float v)                    { return set(ParamIDs::mix, v); }
        Builder& probability(float v)             { return set(ParamIDs::probability, v); }
        Builder& glide(float v)                  { return set(ParamIDs::stepGlide, v); }
        Builder& pattern(StepPattern p)           { def.pattern = std::move(p); return *this; }

        FactoryPresetDefinition build() { return std::move(def); }

    private:
        Builder& set(const juce::String& id, float v) { def.paramValues.push_back({ id, v }); return *this; }
        FactoryPresetDefinition def;
    };

    StepPattern patternFromSteps(int length, std::initializer_list<int> onSteps, float level = 1.0f)
    {
        StepPattern p;
        p.length = length;
        for (auto& s : p.steps) { s.enabled = false; s.level = 0.0f; }
        for (int idx : onSteps)
            if (idx >= 0 && idx < StepPattern::maxSteps)
            {
                p.steps[(size_t) idx].enabled = true;
                p.steps[(size_t) idx].level   = level;
            }
        return p;
    }

    StepPattern patternFromLevels(std::initializer_list<float> levels)
    {
        StepPattern p;
        p.length = (int) levels.size();
        int i = 0;
        for (float lvl : levels)
        {
            auto& s = p.steps[(size_t) i++];
            s.enabled = lvl > 0.0f;
            s.level   = lvl;
        }
        return p;
    }

    StepPattern patternEuclidean(int length, int pulses)
    {
        StepPattern p;
        PatternGenerators::applyEuclidean(p, length, pulses);
        return p;
    }

    StepPattern patternRandom(int length, float density, float jitter, juce::int64 seed)
    {
        StepPattern p;
        p.length = length;
        juce::Random rng(seed);
        PatternGenerators::randomize(p, density, jitter, rng);
        return p;
    }

    StepPattern patternAllOn(int length)
    {
        StepPattern p; // default-constructed pattern is already all steps on
        p.length = length;
        return p;
    }
}

namespace FactoryPresets
{

std::vector<FactoryPresetDefinition> createAll()
{
    std::vector<FactoryPresetDefinition> presets;

    presets.push_back(Builder("Default")
        .pattern(patternFromSteps(16, { 0, 4, 8, 12 }))
        .build());

    presets.push_back(Builder("Classic 16th Chop")
        .rate(4).ahr(5, 55, 35)
        .pattern(patternFromSteps(16, { 0, 2, 4, 6, 8, 10, 12, 14 }))
        .build());

    presets.push_back(Builder("8th Note Pump")
        .rate(3).ahr(25, 50, 25).depth(85)
        .pattern(patternAllOn(8))
        .build());

    presets.push_back(Builder("Trance Stab Triplets")
        .rate(4).rateMod(2).ahr(8, 40, 25)
        .pattern(patternEuclidean(12, 5))
        .build());

    presets.push_back(Builder("Slow Pad Swell")
        .rate(1).ahr(45, 10, 45).depth(60).glide(40)
        .pattern(patternAllOn(4))
        .build());

    presets.push_back(Builder("Euclidean 3/8 Groove")
        .rate(4).ahr(5, 55, 35)
        .pattern(patternEuclidean(8, 3))
        .build());

    presets.push_back(Builder("Euclidean 5/8 Polyrhythm")
        .rate(4).ahr(5, 50, 35)
        .pattern(patternEuclidean(8, 5))
        .build());

    presets.push_back(Builder("Stereo Ping-Pong")
        .rate(4).ahr(5, 45, 45).stereoMode(1).stereoOffset(50)
        .pattern(patternFromSteps(16, { 0, 2, 4, 6, 8, 10, 12, 14 }))
        .build());

    presets.push_back(Builder("Mid-Side Widener")
        .rate(3).ahr(20, 60, 20).stereoMode(2).stereoOffset(25).depth(40)
        .pattern(patternAllOn(8))
        .build());

    presets.push_back(Builder("Evolving Probability")
        .rate(4).ahr(5, 55, 35).probability(55)
        .pattern(patternRandom(16, 0.7f, 0.2f, 1))
        .build());

    presets.push_back(Builder("Hard Gate 1/32")
        .rate(5).ahr(1, 90, 1)
        .pattern(patternFromSteps(16, { 0, 2, 4, 6, 8, 10, 12, 14 }))
        .build());

    presets.push_back(Builder("Swung 16ths")
        .rate(4).ahr(5, 55, 35).swing(55)
        .pattern(patternAllOn(16))
        .build());

    presets.push_back(Builder("Glide Wave")
        .rate(3).ahr(30, 20, 30).glide(100)
        .pattern(patternFromLevels({ 1.0f, 0.3f, 1.0f, 0.3f, 1.0f, 0.3f, 1.0f, 0.3f }))
        .build());

    presets.push_back(Builder("Triplet Gallop")
        .rate(3).rateMod(2).ahr(8, 50, 30)
        .pattern(patternFromSteps(6, { 0, 1, 3 }))
        .build());

    presets.push_back(Builder("Sparse Random")
        .rate(4).ahr(5, 55, 35)
        .pattern(patternRandom(32, 0.35f, 0.15f, 2))
        .build());

    presets.push_back(Builder("Full Send Strobe")
        .rate(6).ahr(1, 95, 1)
        .pattern(patternAllOn(16))
        .build());

    return presets;
}

} // namespace FactoryPresets
