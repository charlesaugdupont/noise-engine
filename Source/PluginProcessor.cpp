#include "PluginProcessor.h"
#include "PluginEditor.h"

// --- Parameter IDs (string constants to avoid typos) ---
static const juce::String BITDEPTH_ID   = "bitDepth";
static const juce::String DOWNSAMPLE_ID = "downsample";
static const juce::String MIX_ID        = "mix";

// ---------------------------------------------------------------------------
// Parameter layout
// ---------------------------------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout
NoiseEngineAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID { BITDEPTH_ID, 1 },   // <-- add ParameterID with version hint
    "Bit Depth",
    juce::NormalisableRange<float>(1.0f, 16.0f, 0.01f),
    16.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { DOWNSAMPLE_ID, 1 },
        "Downsample",
        juce::NormalisableRange<float>(1.0f, 32.0f, 0.01f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { MIX_ID, 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        1.0f));

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
{}

NoiseEngineAudioProcessor::~NoiseEngineAudioProcessor() {}

// ---------------------------------------------------------------------------
// Playback lifecycle
// ---------------------------------------------------------------------------
void NoiseEngineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Reset held-sample state when playback starts or settings change
    sampleCounter  = 0;
    heldSample[0]  = 0.0f;
    heldSample[1]  = 0.0f;
}

void NoiseEngineAudioProcessor::releaseResources() {}

// ---------------------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------------------
void NoiseEngineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals; // prevents CPU spikes from denormal floats

    // Read current parameter values (thread-safe atomic reads via APVTS)
    const float bitDepth   = apvts.getRawParameterValue(BITDEPTH_ID)->load();
    const float downsample = apvts.getRawParameterValue(DOWNSAMPLE_ID)->load();
    const float mix        = apvts.getRawParameterValue(MIX_ID)->load();

    // Number of quantisation levels for the chosen bit depth: 2^bitDepth
    // e.g. 8-bit → 256 levels, 4-bit → 16 levels
    const float levels = std::pow(2.0f, bitDepth);

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // --- Sample Rate Reduction ---
        // Every `downsample` input samples we latch a new held value.
        // Between latches we output the same held sample (zero-order hold).
        bool latchNewSample = (sampleCounter == 0);
        sampleCounter = (sampleCounter + 1) % static_cast<int>(downsample);

        for (int ch = 0; ch < numChannels && ch < 2; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            const float dry = channelData[sample];

            if (latchNewSample)
                heldSample[ch] = dry;

            // --- Bit Depth Reduction ---
            // Quantise to `levels` steps in the range [-1, 1]:
            //   1. Scale up into [0, levels]
            //   2. Round to nearest integer (quantise)
            //   3. Scale back to [-1, 1]
            float wet = std::round(heldSample[ch] * (levels * 0.5f)) / (levels * 0.5f);
            wet = juce::jlimit(-1.0f, 1.0f, wet); // hard clip to prevent overflow

            // Dry/wet blend
            channelData[sample] = dry * (1.0f - mix) + wet * mix;
        }
    }
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

void NoiseEngineAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Serialise APVTS state to binary so the DAW can save/recall presets
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NoiseEngineAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore APVTS state from binary
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseEngineAudioProcessor();
}
