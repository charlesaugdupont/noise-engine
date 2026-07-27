#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/PresetBarComponent.h"
#include "UI/StepSequencerComponent.h"
#include "UI/GeneratorPanelComponent.h"

// The macro-knob grid is still bare-bones/data-driven (real grouped layout
// is Phase 5 polish); the step grid above it is the real Phase 2 deliverable.
class NoiseEngineAudioEditor : public juce::AudioProcessorEditor
{
public:
    explicit NoiseEngineAudioEditor(NoiseEngineAudioProcessor&);
    ~NoiseEngineAudioEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct KnobControl
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct ChoiceControl
    {
        juce::ComboBox box;
        juce::Label    label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    NoiseEngineAudioProcessor& audioProcessor;
    PresetBarComponent         presetBar;
    StepSequencerComponent     stepSequencer;
    GeneratorPanelComponent    generatorPanel;

    std::vector<std::unique_ptr<KnobControl>>   knobs;
    std::vector<std::unique_ptr<ChoiceControl>> choices;

    void addKnob(const juce::String& paramID, const juce::String& labelText);
    void addChoice(const juce::String& paramID, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseEngineAudioEditor)
};
