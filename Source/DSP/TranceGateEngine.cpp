#include "TranceGateEngine.h"

namespace
{
    constexpr float kMinEnvelopeSegment = 0.0005f; // fraction of a step; guards against div-by-zero
}

// ---------------------------------------------------------------------------
// Construction / lifecycle
// ---------------------------------------------------------------------------
TranceGateEngine::TranceGateEngine() {}

void TranceGateEngine::prepare(double newSampleRate, int /*maximumBlockSize*/)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;

    constexpr double smoothingSeconds = 0.02;
    depthSmoothed.reset(sampleRate, smoothingSeconds);
    mixSmoothed.reset(sampleRate, smoothingSeconds);
    outputGainSmoothed.reset(sampleRate, smoothingSeconds);

    depthSmoothed.setCurrentAndTargetValue(1.0f);
    mixSmoothed.setCurrentAndTargetValue(1.0f);
    outputGainSmoothed.setCurrentAndTargetValue(1.0f);

    reset();
}

void TranceGateEngine::reset()
{
    freeRunPhaseSeconds = 0.0;
    currentStepForUI.store(0);
    lastCycleCount = std::numeric_limits<long long>::min(); // forces a re-roll on the next sample processed
    triggerMask.fill(true);
}

// ---------------------------------------------------------------------------
// Timing helpers
// ---------------------------------------------------------------------------
double TranceGateEngine::getBeatsPerStep(int rateIndex, int rateModifierIndex)
{
    static constexpr int denominators[] = { 1, 2, 4, 8, 16, 32, 64 };
    const int index = juce::jlimit(0, 6, rateIndex);
    double beats = 4.0 / (double) denominators[index];

    switch (rateModifierIndex)
    {
        case 1:  beats *= 1.5;        break; // Dotted
        case 2:  beats *= 2.0 / 3.0;  break; // Triplet
        default: break;                      // Straight
    }

    return beats;
}

// Resolves a raw (unswung) step-unit position to a swung step index + local
// phase. Every second step in a pair ("the offbeat") is delayed later by
// `swingFrac` (0..~0.75) of a step; the preceding step is correspondingly
// lengthened and the offbeat step's own duration is shortened so the pair
// still totals 2 steps. Each step's local phase always runs 0..1 across
// *that step's own* (possibly stretched/compressed) duration, so the AHR
// envelope never jumps mid-step — it only resets at a step boundary, same
// as the unswung case.
TranceGateEngine::SwungStep TranceGateEngine::resolveSwing(double rawStepUnits, double swingFrac)
{
    const double pairIndex = std::floor(rawStepUnits / 2.0);
    const double posInPair = rawStepUnits - pairIndex * 2.0; // [0, 2)

    const double downbeatDur = 1.0 + swingFrac; // (1 .. 1.75]

    if (posInPair < downbeatDur)
    {
        const long long stepFloorInt = (long long) pairIndex * 2;
        return { stepFloorInt, (float) (posInPair / downbeatDur) };
    }

    const double offbeatDur      = juce::jmax(0.01, 2.0 - downbeatDur); // [0.25 .. 1)
    const long long stepFloorInt = (long long) pairIndex * 2 + 1;
    return { stepFloorInt, (float) ((posInPair - downbeatDur) / offbeatDur) };
}

// Floor division (rounds toward -infinity), unlike C++'s `/` which truncates
// toward zero — matters here because `ppq` (and so the swung step-floor) can
// be negative before the host's playback position reaches zero.
long long TranceGateEngine::floorDiv(long long a, long long b)
{
    const long long q = a / b;
    const long long r = a % b;
    return (r != 0 && ((r < 0) != (b < 0))) ? q - 1 : q;
}

// Re-rolls the trigger mask once per pattern cycle, at the instant step 0
// begins (never mid-step). Uses the *primary* (non-stereo-offset) timeline
// only, so both stereo channels always read the same mask for a given step
// index — otherwise a stereo offset could cause L/R to re-roll at different
// moments and decorrelate in a way that has nothing to do with the intended
// stereo effect.
void TranceGateEngine::updateTriggerMask(double ppq, double beatsPerStep, double swingFrac,
                                          int patternLength, float probability)
{
    const double rawUnits    = ppq / beatsPerStep;
    const auto    swung      = resolveSwing(rawUnits, swingFrac);
    const long long cycleCount = floorDiv(swung.stepFloor, (long long) patternLength);

    if (cycleCount == lastCycleCount)
        return;

    lastCycleCount = cycleCount;

    for (int i = 0; i < patternLength; ++i)
        triggerMask[(size_t) i] = rng.nextFloat() < probability;
}

// ---------------------------------------------------------------------------
// Envelope shaping
// ---------------------------------------------------------------------------
float TranceGateEngine::raisedCosine(float x)
{
    x = juce::jlimit(0.0f, 1.0f, x);
    return 0.5f - 0.5f * std::cos(juce::MathConstants<float>::pi * x);
}

// Zero-slope-at-the-joins envelope, expressed purely in terms of the step's
// local phase (0..1) so it auto-scales with tempo/rate. `attack`/`hold`/
// `release` are fractions of the step (0..1); if their sum exceeds 1 they
// are scaled down proportionally so they never overrun the step.
float TranceGateEngine::shapeEnvelope(float phase, float attack, float hold, float release, float level)
{
    float atk = attack, hld = hold, rel = release;
    const float sum = atk + hld + rel;
    if (sum > 1.0f && sum > 0.0f)
    {
        const float scale = 1.0f / sum;
        atk *= scale; hld *= scale; rel *= scale;
    }

    atk = juce::jmax(atk, kMinEnvelopeSegment);
    rel = juce::jmax(rel, kMinEnvelopeSegment);

    if (phase < atk)
        return level * raisedCosine(phase / atk);

    if (phase < atk + hld)
        return level;

    if (phase < atk + hld + rel)
        return level * (1.0f - raisedCosine((phase - atk - hld) / rel));

    return 0.0f;
}

int TranceGateEngine::computeStepIndex(double ppq, double beatsPerStep, double swingFrac, int patternLength)
{
    const auto swung = resolveSwing(ppq / beatsPerStep, swingFrac);
    return (int) (((swung.stepFloor % patternLength) + patternLength) % patternLength);
}

TranceGateEngine::GateSample TranceGateEngine::computeGateGain(double ppq, double beatsPerStep, double swingFrac,
                                                                 int patternLength, const StepPattern& pattern,
                                                                 float attackPct, float holdPct, float releasePct,
                                                                 float glideAmount) const
{
    const auto  swung      = resolveSwing(ppq / beatsPerStep, swingFrac);
    const int   stepIndex  = (int) (((swung.stepFloor % patternLength) + patternLength) % patternLength);
    const float stepPhase  = swung.phase;

    const auto& step        = pattern.steps[(size_t) stepIndex];
    const bool  triggered    = step.enabled && triggerMask[(size_t) stepIndex];
    const float targetLevel = triggered ? step.level : 0.0f;

    const float sharpGain = shapeEnvelope(stepPhase, attackPct / 100.0f, holdPct / 100.0f, releasePct / 100.0f, targetLevel);

    float gain = sharpGain;

    if (glideAmount > 0.0f)
    {
        // Glide blends the percussive AHR-shaped gate towards a smooth
        // cosine slide from the *previous* step's level to this step's
        // level across the step's full duration — at 100% the plugin
        // behaves more like a smoothly stepped tremolo than a hard gate.
        // Uses what the previous step actually sounded like (post-probability),
        // not just its nominal enabled/level, so glide stays consistent with
        // what was just heard.
        const int   prevIndex     = (stepIndex - 1 + patternLength) % patternLength;
        const auto& prevStep      = pattern.steps[(size_t) prevIndex];
        const bool  prevTriggered = prevStep.enabled && triggerMask[(size_t) prevIndex];
        const float prevLevel     = prevTriggered ? prevStep.level : 0.0f;
        const float glideGain     = prevLevel + (targetLevel - prevLevel) * raisedCosine(stepPhase);

        gain = sharpGain + (glideGain - sharpGain) * glideAmount;
    }

    return { gain, stepIndex };
}

// ---------------------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------------------
void TranceGateEngine::processBlock(juce::AudioBuffer<float>& buffer,
                                     const TransportInfo& transport,
                                     const StepPattern& pattern,
                                     const GateMacroParams& macros)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const bool wantsHostSync     = (macros.syncModeIndex == 0);
    const bool hostSyncAvailable = wantsHostSync && transport.hasValidBpm && transport.hasValidPpq;

    double bpm;
    double ppqStart;
    bool   playing;

    if (hostSyncAvailable)
    {
        bpm      = juce::jmax(1.0, transport.bpm);
        ppqStart = transport.ppqPosition;
        playing  = transport.isPlaying;
    }
    else
    {
        // Free-running fallback: no host playhead, host doesn't report
        // BPM/PPQ, or the user explicitly chose Free mode.
        bpm      = juce::jmax(1.0, (double) macros.freeBpm);
        playing  = true;
        ppqStart = freeRunPhaseSeconds * (bpm / 60.0);
    }

    const double beatsPerStep      = getBeatsPerStep(macros.rateIndex, macros.rateModifierIndex);
    const double swingFrac         = (double) juce::jlimit(0.0f, 0.75f, macros.swingPct / 100.0f);
    const int    patternLength     = juce::jlimit(1, StepPattern::maxSteps, macros.patternLength);
    const double stereoOffsetSteps = (double) juce::jlimit(-1.0f, 1.0f, macros.stereoOffsetPct / 100.0f);
    const int    stereoMode        = (numChannels >= 2) ? macros.stereoModeIndex : 0;
    const double ppqPerSample      = playing ? (bpm / 60.0) / sampleRate : 0.0;
    const float  glideAmount       = juce::jlimit(0.0f, 1.0f, macros.stepGlidePct / 100.0f);

    depthSmoothed.setTargetValue(macros.depth);
    mixSmoothed.setTargetValue(macros.mix);
    outputGainSmoothed.setTargetValue(juce::Decibels::decibelsToGain(macros.outputGainDb));

    int uiStepIndex = currentStepForUI.load();

    if (! macros.bypass)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const double ppq = ppqStart + ppqPerSample * (double) sample;

            // Re-roll happens on the primary (non-offset) timeline only —
            // see updateTriggerMask's comment for why.
            updateTriggerMask(ppq, beatsPerStep, swingFrac, patternLength, macros.probability);

            const auto primary = computeGateGain(ppq, beatsPerStep, swingFrac, patternLength, pattern,
                                                  macros.attackPct, macros.holdPct, macros.releasePct, glideAmount);
            uiStepIndex = primary.stepIndex;

            float gainA = primary.gain;
            float gainB = primary.gain;

            if (stereoMode != 0)
            {
                const double offsetPpq = ppq + stereoOffsetSteps * beatsPerStep;
                gainB = computeGateGain(offsetPpq, beatsPerStep, swingFrac, patternLength, pattern,
                                         macros.attackPct, macros.holdPct, macros.releasePct, glideAmount).gain;
            }

            const float depthNow = depthSmoothed.getNextValue();
            const float mixNow   = mixSmoothed.getNextValue();
            const float outGain  = outputGainSmoothed.getNextValue();

            auto blend = [depthNow, mixNow](float dry, float gateGain)
            {
                const float applied = 1.0f - depthNow * (1.0f - gateGain);
                return dry * (1.0f - mixNow) + (dry * applied) * mixNow;
            };

            if (stereoMode == 2 && numChannels >= 2) // Mid-Side
            {
                auto* left  = buffer.getWritePointer(0);
                auto* right = buffer.getWritePointer(1);

                const float l = left[sample];
                const float r = right[sample];
                const float mid  = (l + r) * 0.5f;
                const float side = (l - r) * 0.5f;

                const float newMid  = blend(mid,  gainA) * outGain;
                const float newSide = blend(side, gainB) * outGain;

                left[sample]  = newMid + newSide;
                right[sample] = newMid - newSide;
            }
            else
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    auto* data = buffer.getWritePointer(ch);
                    const float g = (stereoMode == 1 && ch == 1) ? gainB : gainA;
                    data[sample] = blend(data[sample], g) * outGain;
                }
            }
        }
    }
    else
    {
        // Bypassed: skip gain computation entirely, but still track which
        // step the host transport (or free-run clock) is *currently* over.
        // Host/free-run position keeps advancing during bypass regardless —
        // freezing this would make un-bypassing look like the playhead
        // randomly jumped, when really it was just catching up to where
        // playback already was.
        uiStepIndex = computeStepIndex(ppqStart, beatsPerStep, swingFrac, patternLength);
    }

    currentStepForUI.store(uiStepIndex);

    if (! hostSyncAvailable)
        freeRunPhaseSeconds += (double) numSamples / sampleRate;
}
