#include "PatternGenerators.h"

namespace PatternGenerators
{

void applyEuclidean(StepPattern& pattern, int steps, int pulses)
{
    steps  = juce::jlimit(1, StepPattern::maxSteps, steps);
    pulses = juce::jlimit(0, steps, pulses);

    int bucket = 0;
    for (int i = 0; i < steps; ++i)
    {
        bucket += pulses;
        auto& step = pattern.steps[(size_t) i];

        if (bucket >= steps)
        {
            bucket -= steps;
            step.enabled = true;
            step.level   = 1.0f;
        }
        else
        {
            step.enabled = false;
            step.level   = 0.0f;
        }
    }

    pattern.length = steps;
}

void randomize(StepPattern& pattern, float density, float jitter, juce::Random& rng)
{
    density = juce::jlimit(0.0f, 1.0f, density);
    jitter  = juce::jlimit(0.0f, 1.0f, jitter);

    const int length = pattern.length;

    // Exact-count selection rather than an independent coin flip per step:
    // a per-step Bernoulli trial only matches `density` in expectation, and
    // the actual on-step count has real variance around that (e.g. a
    // 16-step pattern at 25% has a standard deviation of ~1.7 steps — often
    // landing visibly off from what the knob says, especially at shorter
    // lengths). Shuffling indices and taking the first N instead guarantees
    // the resulting density always matches, while still being random about
    // *which* steps land on.
    const int targetOnCount = juce::jlimit(0, length, juce::roundToInt((float) length * density));

    std::array<int, StepPattern::maxSteps> indices {};
    for (int i = 0; i < length; ++i)
        indices[(size_t) i] = i;

    for (int i = length - 1; i > 0; --i)
    {
        const int j = rng.nextInt(i + 1);
        std::swap(indices[(size_t) i], indices[(size_t) j]);
    }

    std::array<bool, StepPattern::maxSteps> shouldBeOn {};
    for (int i = 0; i < targetOnCount; ++i)
        shouldBeOn[(size_t) indices[(size_t) i]] = true;

    for (int i = 0; i < length; ++i)
    {
        auto& step   = pattern.steps[(size_t) i];
        step.enabled = shouldBeOn[(size_t) i];

        // Unlike the wheel's manual toggle (which preserves level so a
        // muted step can come back at the same value), a full regenerate
        // resets off steps to 0 — otherwise leftover levels from whatever
        // was there before show up as a stray partial wedge on an "off"
        // step, which reads as a rendering glitch rather than "off".
        step.level = step.enabled ? (1.0f - rng.nextFloat() * jitter) : 0.0f;
    }
}

void rotateLeft(StepPattern& pattern)
{
    const int length = pattern.length;
    if (length <= 1)
        return;

    const StepData first = pattern.steps[0];
    for (int i = 0; i < length - 1; ++i)
        pattern.steps[(size_t) i] = pattern.steps[(size_t) (i + 1)];
    pattern.steps[(size_t) (length - 1)] = first;
}

void rotateRight(StepPattern& pattern)
{
    const int length = pattern.length;
    if (length <= 1)
        return;

    const StepData last = pattern.steps[(size_t) (length - 1)];
    for (int i = length - 1; i > 0; --i)
        pattern.steps[(size_t) i] = pattern.steps[(size_t) (i - 1)];
    pattern.steps[0] = last;
}

void duplicateToDouble(StepPattern& pattern)
{
    const int oldLength = pattern.length;
    const int newLength = juce::jmin(StepPattern::maxSteps, oldLength * 2);
    const int copyCount  = newLength - oldLength;

    for (int i = 0; i < copyCount; ++i)
        pattern.steps[(size_t) (oldLength + i)] = pattern.steps[(size_t) i];

    pattern.length = newLength;
}

void halveLength(StepPattern& pattern)
{
    pattern.length = juce::jmax(1, pattern.length / 2);
}

} // namespace PatternGenerators
