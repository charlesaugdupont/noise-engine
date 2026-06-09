#include "PluginEditor.h"

NoiseEngineAudioEditor::NoiseEngineAudioEditor(NoiseEngineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(400, 300);
}

NoiseEngineAudioEditor::~NoiseEngineAudioEditor() {}

void NoiseEngineAudioEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("Noise Engine", getLocalBounds(), juce::Justification::centred, 1);
}

void NoiseEngineAudioEditor::resized() {}