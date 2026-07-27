#pragma once
#include <JuceHeader.h>
#include "StepPattern.h"

// Transport snapshot read from the host playhead once per block. Fields are
// individually flagged as valid/invalid because a host may provide some
// position info but not all of it (see AudioPlayHead::PositionInfo).
struct TransportInfo
{
    bool   isPlaying   = false;
    bool   hasValidBpm = false;
    double bpm         = 120.0;
    bool   hasValidPpq = false;
    double ppqPosition = 0.0;
};

// The small set of host-automatable macro controls, read from the APVTS
// each block. Pattern data (per-step level/probability/enabled) travels
// separately as a StepPattern snapshot.
struct GateMacroParams
{
    int   rateIndex               = 4;     // index into ParamChoices::rate (4 == "1/16")
    int   rateModifierIndex       = 0;     // 0 = Straight, 1 = Dotted, 2 = Triplet
    int   syncModeIndex           = 0;     // 0 = DAW, 1 = Free
    float freeBpm                 = 120.0f;
    int   patternLength           = 16;
    float attackPct               = 5.0f;
    float holdPct                 = 55.0f;
    float releasePct              = 35.0f;
    float swingPct                = 0.0f;
    int   stereoModeIndex         = 0;     // 0 = Mono, 1 = L-R Offset, 2 = Mid-Side
    float stereoOffsetPct         = 0.0f;
    float depth                   = 1.0f;  // 0..1
    float mix                     = 1.0f;  // 0..1
    float probability             = 1.0f;  // 0..1 — chance an ON step actually fires each pass
    float outputGainDb            = 0.0f;
    float stepGlidePct            = 0.0f;
    bool  bypass                  = false;
};

// Pure DSP core: tempo-synced step sequencing, attack/hold/release step
// shaping, and stereo-mode gain computation. No JUCE Component/APVTS
// dependency, so it can be exercised/tested in isolation.
class TranceGateEngine
{
public:
    TranceGateEngine();

    void prepare(double newSampleRate, int maximumBlockSize);
    void reset();

    void processBlock(juce::AudioBuffer<float>& buffer,
                       const TransportInfo& transport,
                       const StepPattern& pattern,
                       const GateMacroParams& macros);

    // Polled by the UI (StepSequencerComponent, added in a later phase) to
    // paint a playhead indicator.
    int getCurrentStepForUI() const noexcept { return currentStepForUI.load(); }

private:
    struct GateSample
    {
        float gain;
        int   stepIndex;
    };

    struct SwungStep
    {
        long long stepFloor;
        float     phase;
    };

    static double     getBeatsPerStep(int rateIndex, int rateModifierIndex);
    static SwungStep  resolveSwing(double rawStepUnits, double swingFrac);
    static float      raisedCosine(float x);
    static float      shapeEnvelope(float phase, float attack, float hold, float release, float level);
    static long long  floorDiv(long long a, long long b);
    static int         computeStepIndex(double ppq, double beatsPerStep, double swingFrac, int patternLength);

    GateSample computeGateGain(double ppq, double beatsPerStep, double swingFrac,
                                int patternLength, const StepPattern& pattern,
                                float attackPct, float holdPct, float releasePct,
                                float glideAmount) const;

    // Re-rolls `triggerMask` for every step, once per pattern cycle, at the
    // instant step 0 begins — never mid-step, so it can't cause a glitch.
    // At probability=1.0 every roll always wins (rng.nextFloat() < 1.0f is
    // always true), so this naturally degenerates to fully deterministic
    // behaviour without needing a separate enable flag.
    void updateTriggerMask(double ppq, double beatsPerStep, double swingFrac,
                            int patternLength, float probability);

    double sampleRate          = 44100.0;
    double freeRunPhaseSeconds = 0.0;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGainSmoothed;

    // Live per-step probability re-roll — audio-thread-only state, never
    // serialized (it's runtime randomness, not pattern data or a parameter).
    juce::Random rng;
    std::array<bool, StepPattern::maxSteps> triggerMask;
    long long lastCycleCount = std::numeric_limits<long long>::min();

    std::atomic<int> currentStepForUI { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TranceGateEngine)
};
