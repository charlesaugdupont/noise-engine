#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/ParameterIDs.h"

namespace
{
    // SliderAttachment installs its own textFromValueFunction that calls
    // param.getText(...) — it ignores a Slider's own
    // setNumDecimalPlacesToDisplay() entirely. The only place that actually
    // controls what gets displayed (in our UI *and* in any host's generic
    // parameter view / automation lane) is the parameter's own
    // stringFromValue function, set here.
    juce::AudioParameterFloatAttributes displayAttributes(const juce::String& label, int decimalPlaces)
    {
        return juce::AudioParameterFloatAttributes()
            .withLabel(label)
            .withStringFromValueFunction([decimalPlaces](float value, int) -> juce::String
            {
                // juce::String(value, 0) does NOT mean "zero decimal places" —
                // 0 is JUCE's internal sentinel for "auto/natural" formatting
                // (the same path the argument-less String(float) constructor
                // uses), so it was showing full natural precision for any
                // non-round dragged value. Rounding to an int explicitly is
                // the only way to actually force a whole-number display.
                if (decimalPlaces <= 0)
                    return juce::String(juce::roundToInt(value));

                return juce::String(value, decimalPlaces);
            });
    }
}

// ---------------------------------------------------------------------------
// Parameter layout
// ---------------------------------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout
NoiseEngineAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParamIDs::rate, 1 }, "Rate", ParamChoices::rate, 4));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParamIDs::rateModifier, 1 }, "Rate Modifier", ParamChoices::modifier, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParamIDs::syncMode, 1 }, "Sync Mode", ParamChoices::syncMode, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::freeBpm, 1 }, "Free BPM",
        juce::NormalisableRange<float>(20.0f, 999.0f, 0.01f), 120.0f,
        displayAttributes("", 0)));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { ParamIDs::patternLength, 1 }, "Pattern Length",
        1, StepPattern::maxSteps, 16));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::attack, 1 }, "Attack",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 5.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::hold, 1 }, "Hold",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 55.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::release, 1 }, "Release",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 35.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::swing, 1 }, "Swing",
        juce::NormalisableRange<float>(0.0f, 75.0f, 0.01f), 0.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParamIDs::stereoMode, 1 }, "Stereo Mode", ParamChoices::stereoMode, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::stereoOffset, 1 }, "Stereo Offset",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.01f), 0.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::depth, 1 }, "Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 100.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::mix, 1 }, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 100.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::probability, 1 }, "Probability",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 100.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::outputGain, 1 }, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f,
        displayAttributes("dB", 1)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::stepGlide, 1 }, "Step Glide",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 0.0f,
        displayAttributes("%", 0)));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { ParamIDs::bypass, 1 }, "Bypass", false));

    return { params.begin(), params.end() };
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
NoiseEngineAudioProcessor::NoiseEngineAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    audioPatternCache = uiPattern;
}

NoiseEngineAudioProcessor::~NoiseEngineAudioProcessor() {}

// ---------------------------------------------------------------------------
// Playback lifecycle
// ---------------------------------------------------------------------------
void NoiseEngineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
}

void NoiseEngineAudioProcessor::releaseResources() {}

// ---------------------------------------------------------------------------
// Transport / parameter reads
// ---------------------------------------------------------------------------
TransportInfo NoiseEngineAudioProcessor::readTransportInfo() const
{
    TransportInfo info;

    if (auto* playHead = getPlayHead())
    {
        if (const auto position = playHead->getPosition())
        {
            info.isPlaying = position->getIsPlaying();

            if (const auto bpm = position->getBpm())
            {
                info.bpm = *bpm;
                info.hasValidBpm = true;
            }

            if (const auto ppq = position->getPpqPosition())
            {
                info.ppqPosition = *ppq;
                info.hasValidPpq = true;
            }
        }
    }

    return info;
}

GateMacroParams NoiseEngineAudioProcessor::readMacroParams() const
{
    GateMacroParams macros;

    macros.rateIndex               = (int) apvts.getRawParameterValue(ParamIDs::rate)->load();
    macros.rateModifierIndex       = (int) apvts.getRawParameterValue(ParamIDs::rateModifier)->load();
    macros.syncModeIndex           = (int) apvts.getRawParameterValue(ParamIDs::syncMode)->load();
    macros.freeBpm                 = apvts.getRawParameterValue(ParamIDs::freeBpm)->load();
    macros.patternLength           = (int) apvts.getRawParameterValue(ParamIDs::patternLength)->load();
    macros.attackPct               = apvts.getRawParameterValue(ParamIDs::attack)->load();
    macros.holdPct                 = apvts.getRawParameterValue(ParamIDs::hold)->load();
    macros.releasePct              = apvts.getRawParameterValue(ParamIDs::release)->load();
    macros.swingPct                = apvts.getRawParameterValue(ParamIDs::swing)->load();
    macros.stereoModeIndex         = (int) apvts.getRawParameterValue(ParamIDs::stereoMode)->load();
    macros.stereoOffsetPct         = apvts.getRawParameterValue(ParamIDs::stereoOffset)->load();
    macros.depth                   = apvts.getRawParameterValue(ParamIDs::depth)->load() / 100.0f;
    macros.mix                     = apvts.getRawParameterValue(ParamIDs::mix)->load() / 100.0f;
    macros.probability             = apvts.getRawParameterValue(ParamIDs::probability)->load() / 100.0f;
    macros.outputGainDb            = apvts.getRawParameterValue(ParamIDs::outputGain)->load();
    macros.stepGlidePct            = apvts.getRawParameterValue(ParamIDs::stepGlide)->load();
    macros.bypass                  = apvts.getRawParameterValue(ParamIDs::bypass)->load() > 0.5f;

    return macros;
}

// ---------------------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------------------
void NoiseEngineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    const auto transportInfo = readTransportInfo();
    const auto macros        = readMacroParams();

    // Opportunistic, never-blocking refresh of the audio thread's pattern copy.
    {
        const juce::SpinLock::ScopedTryLockType tryLock(patternLock);
        if (tryLock.isLocked())
            audioPatternCache = uiPattern;
    }

    engine.processBlock(buffer, transportInfo, audioPatternCache, macros);
}

// ---------------------------------------------------------------------------
// Pattern editing API (message thread)
// ---------------------------------------------------------------------------
StepPattern NoiseEngineAudioProcessor::getPatternSnapshot() const
{
    const juce::SpinLock::ScopedLockType lock(patternLock);
    return uiPattern;
}

void NoiseEngineAudioProcessor::setPatternSnapshot(const StepPattern& newPattern)
{
    const juce::SpinLock::ScopedLockType lock(patternLock);
    uiPattern = newPattern;
}

int NoiseEngineAudioProcessor::getPatternLength() const
{
    return (int) apvts.getRawParameterValue(ParamIDs::patternLength)->load();
}

void NoiseEngineAudioProcessor::setPatternLength(int newLength)
{
    if (auto* param = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter(ParamIDs::patternLength)))
        *param = newLength;
}

bool NoiseEngineAudioProcessor::isBypassed() const
{
    return apvts.getRawParameterValue(ParamIDs::bypass)->load() > 0.5f;
}

void NoiseEngineAudioProcessor::setBypassed(bool shouldBypass)
{
    if (auto* param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(ParamIDs::bypass)))
        *param = shouldBypass;
}

// ---------------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------------
juce::AudioProcessorEditor* NoiseEngineAudioProcessor::createEditor()
{
    return new NoiseEngineAudioEditor(*this);
}

bool NoiseEngineAudioProcessor::hasEditor() const { return true; }

// ---------------------------------------------------------------------------
// Boilerplate
// ---------------------------------------------------------------------------
const juce::String NoiseEngineAudioProcessor::getName() const { return JucePlugin_Name; }
bool NoiseEngineAudioProcessor::acceptsMidi()  const { return false; }
bool NoiseEngineAudioProcessor::producesMidi() const { return false; }
double NoiseEngineAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int  NoiseEngineAudioProcessor::getNumPrograms()                          { return 1; }
int  NoiseEngineAudioProcessor::getCurrentProgram()                       { return 0; }
void NoiseEngineAudioProcessor::setCurrentProgram(int)                    {}
const juce::String NoiseEngineAudioProcessor::getProgramName(int)         { return {}; }
void NoiseEngineAudioProcessor::changeProgramName(int, const juce::String&) {}

juce::ValueTree NoiseEngineAudioProcessor::captureFullState()
{
    auto state = apvts.copyState();
    state.appendChild(getPatternSnapshot().toValueTree(), nullptr);
    return state;
}

void NoiseEngineAudioProcessor::applyFullState(juce::ValueTree state)
{
    if (! state.isValid())
        return;

    auto patternTree = state.getChildWithName(StepPattern::patternType);
    if (patternTree.isValid())
    {
        setPatternSnapshot(StepPattern::fromValueTree(patternTree));
        state.removeChild(patternTree, nullptr);
    }

    apvts.replaceState(state);
}

void NoiseEngineAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Serialise macro params + pattern to binary so the DAW can save/recall
    // full session state (see captureFullState).
    auto state = captureFullState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NoiseEngineAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        applyFullState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseEngineAudioProcessor();
}
