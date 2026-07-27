#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/PresetBarComponent.h"
#include "UI/StepSequencerComponent.h"
#include "UI/GeneratorPanelComponent.h"
#include "UI/MacroSectionComponent.h"

// Top-level layout: PresetBar -> StepSequencer (wheel) -> GeneratorPanel ->
// TIMING/SHAPE side by side -> one full-width STEREO & OUTPUT section.
// (Stereo and Output used to be separate half-width panels, but Stereo only
// has 2 controls and looked sparse/empty at the same width as Output's 3 —
// merging them into one full-width row fits both comfortably in a single
// row of controls instead.)
class NoiseEngineAudioEditor : public juce::AudioProcessorEditor
{
public:
    explicit NoiseEngineAudioEditor(NoiseEngineAudioProcessor&);
    ~NoiseEngineAudioEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PresetBarComponent         presetBar;
    StepSequencerComponent     stepSequencer;
    GeneratorPanelComponent    generatorPanel;

    MacroSectionComponent timingSection;
    MacroSectionComponent shapeSection;
    MacroSectionComponent stereoOutputSection;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseEngineAudioEditor)
};
