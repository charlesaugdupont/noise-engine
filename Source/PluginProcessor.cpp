#include "PluginProcessor.h"
#include "PluginEditor.h"

NoiseEngineAudioProcessor::NoiseEngineAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{}

NoiseEngineAudioProcessor::~NoiseEngineAudioProcessor() {}

void NoiseEngineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {}
void NoiseEngineAudioProcessor::releaseResources() {}

void NoiseEngineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Audio passes through unchanged for now
}

juce::AudioProcessorEditor* NoiseEngineAudioProcessor::createEditor()
{
    return new NoiseEngineAudioEditor(*this);
}

bool NoiseEngineAudioProcessor::hasEditor() const { return true; }
const juce::String NoiseEngineAudioProcessor::getName() const { return JucePlugin_Name; }
bool NoiseEngineAudioProcessor::acceptsMidi() const { return false; }
bool NoiseEngineAudioProcessor::producesMidi() const { return false; }
double NoiseEngineAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NoiseEngineAudioProcessor::getNumPrograms() { return 1; }
int NoiseEngineAudioProcessor::getCurrentProgram() { return 0; }
void NoiseEngineAudioProcessor::setCurrentProgram(int index) {}
const juce::String NoiseEngineAudioProcessor::getProgramName(int index) { return {}; }
void NoiseEngineAudioProcessor::changeProgramName(int index, const juce::String& newName) {}
void NoiseEngineAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {}
void NoiseEngineAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseEngineAudioProcessor();
}