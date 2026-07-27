#pragma once
#include <JuceHeader.h>
#include "DSP/StepPattern.h"
#include "DSP/TranceGateEngine.h"
#include "Presets/PresetManager.h"

class NoiseEngineAudioProcessor : public juce::AudioProcessor
{
public:
    NoiseEngineAudioProcessor();
    ~NoiseEngineAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // APVTS holds the host-automatable macro parameters (rate, depth, mix,
    // attack/hold/release, etc). Per-step pattern data is NOT parameter-backed
    // (see StepPattern) — nobody automates "step 37's level" from a DAW, and
    // APVTS's layout is fixed at construction time while pattern length is
    // variable at runtime.
    juce::AudioProcessorValueTreeState apvts;

    // Polled by the editor to paint a playhead indicator.
    int getCurrentStepForUI() const noexcept { return engine.getCurrentStepForUI(); }

    // Thread-safe pattern editing API for the step-grid UI (message thread
    // only). Reads/writes go through `patternLock`, same as the audio
    // thread's opportunistic cache refresh in processBlock.
    StepPattern getPatternSnapshot() const;
    void setPatternSnapshot(const StepPattern& newPattern);

    // Pattern Length is a real APVTS parameter (host-automatable), separate
    // from the non-parameter StepPattern content — see class comment above.
    int getPatternLength() const;
    void setPatternLength(int newLength);

    // Bypass, read/set by the wheel's centre power button.
    bool isBypassed() const;
    void setBypassed(bool shouldBypass);

    // Combined macro-params + pattern snapshot, as one ValueTree. This is
    // the single serialization path shared by DAW session state
    // (getStateInformation/setStateInformation, below) and preset files
    // (PresetManager) — one place that knows how to build/apply the full
    // state, two different outer wrappers (binary blob vs. plain XML file).
    juce::ValueTree captureFullState();
    void applyFullState(juce::ValueTree state);

    PresetManager& getPresetManager() noexcept { return presetManager; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    TransportInfo   readTransportInfo() const;
    GateMacroParams readMacroParams() const;

    TranceGateEngine engine;

    // Pattern data lives outside the APVTS. `uiPattern` is authoritative and
    // only ever touched on the message thread; `audioPatternCache` is the
    // audio thread's working copy, refreshed opportunistically via a
    // per-block try-lock so the audio thread never blocks on the UI.
    StepPattern uiPattern;
    StepPattern audioPatternCache;
    mutable juce::SpinLock patternLock;

    PresetManager presetManager { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseEngineAudioProcessor)
};
