#include "StepPattern.h"

const juce::Identifier StepPattern::patternType { "Pattern" };
const juce::Identifier StepPattern::stepType    { "Step" };

namespace
{
    const juce::Identifier lengthProp  { "length" };
    const juce::Identifier indexProp   { "index" };
    const juce::Identifier levelProp   { "level" };
    const juce::Identifier enabledProp { "enabled" };
}

StepPattern::StepPattern()
{
    // Placeholder default until the step-grid UI (Phase 2) lets a user
    // author real patterns: every step on. This isolates the shape/timing
    // macros (Attack/Hold/Release/Swing/Glide/Rate) for testing, since any
    // fixed on/off arrangement chosen here would bias what's audible for
    // those controls (e.g. swing only shifts the onset of odd-indexed
    // "offbeat" steps, so it's only clearly audible when offbeats are
    // actually sounding). Pattern Length has no audible effect against this
    // uniform placeholder — that only becomes meaningful once real pattern
    // content exists.
    for (int i = 0; i < maxSteps; ++i)
    {
        auto& step = steps[(size_t) i];
        step.level   = 1.0f;
        step.enabled = true;
    }
}

juce::ValueTree StepPattern::toValueTree() const
{
    juce::ValueTree tree { patternType };
    tree.setProperty(lengthProp, length, nullptr);

    for (int i = 0; i < maxSteps; ++i)
    {
        const auto& step = steps[(size_t) i];

        juce::ValueTree stepTree { stepType };
        stepTree.setProperty(indexProp, i, nullptr);
        stepTree.setProperty(levelProp, step.level, nullptr);
        stepTree.setProperty(enabledProp, step.enabled, nullptr);
        tree.appendChild(stepTree, nullptr);
    }

    return tree;
}

StepPattern StepPattern::fromValueTree(const juce::ValueTree& tree)
{
    StepPattern pattern;

    if (! tree.isValid() || tree.getType() != patternType)
        return pattern;

    pattern.length = juce::jlimit(1, maxSteps, (int) tree.getProperty(lengthProp, pattern.length));

    for (const auto& stepTree : tree)
    {
        if (stepTree.getType() != stepType)
            continue;

        const int index = (int) stepTree.getProperty(indexProp, -1);
        if (index < 0 || index >= maxSteps)
            continue;

        auto& step = pattern.steps[(size_t) index];
        step.level   = (float) stepTree.getProperty(levelProp, step.level);
        step.enabled = (bool) stepTree.getProperty(enabledProp, step.enabled);
    }

    return pattern;
}
