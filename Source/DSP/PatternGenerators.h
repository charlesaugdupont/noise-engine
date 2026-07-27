#pragma once
#include <JuceHeader.h>
#include "StepPattern.h"

// One-shot pattern-writing tools (as opposed to TranceGateEngine's live,
// continuously-running probability re-roll): each of these mutates a
// StepPattern once, when called, and leaves it alone afterward. Pure
// functions with no state of their own — the caller supplies an RNG for
// randomize() so repeated clicks keep advancing rather than re-seeding.
namespace PatternGenerators
{
    // Distributes `pulses` as evenly as possible across `steps` positions
    // (bucket/error-accumulation algorithm, equivalent to Bjorklund's).
    // Writes pattern.steps[0..steps) and sets pattern.length = steps. No
    // rotation parameter — use rotateLeft/rotateRight afterward instead of
    // duplicating that control here (see GeneratorPanelComponent's header
    // comment for why the Euclidean rotation slider was removed).
    void applyEuclidean(StepPattern& pattern, int steps, int pulses);

    // Independently rolls each step within the pattern's current length
    // on/off weighted by `density` (0..1); steps that land on also get their
    // level randomly reduced by up to `jitter` (0..1) below full — e.g. at
    // jitter=0.3, triggered steps land somewhere in [0.7, 1.0]. Steps that
    // land off are reset to level 0 (unlike the wheel's manual toggle,
    // which preserves level — this is a full regenerate, not a mute).
    void randomize(StepPattern& pattern, float density, float jitter, juce::Random& rng);

    void rotateLeft(StepPattern& pattern);
    void rotateRight(StepPattern& pattern);

    // Doubles the active pattern by copying steps [0, length) into
    // [length, 2*length), clamped to StepPattern::maxSteps.
    void duplicateToDouble(StepPattern& pattern);

    // Halves the active length (floor, minimum 1). Non-destructive, like
    // dragging the Length slider down: steps beyond the new length keep
    // their data, just become inactive.
    void halveLength(StepPattern& pattern);
}
