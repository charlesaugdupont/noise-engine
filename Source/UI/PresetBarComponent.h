#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

// Preset browser: prev/next, a name button that opens a scrollable FACTORY/
// USER popup (PresetListComponent, hosted in a juce::CallOutBox — not a
// plain ComboBox, whose native popup could get clipped/mis-scrolled near
// screen edges with a long list and a selection near the end), Save As, and
// Delete (disabled for factory presets).
class PresetBarComponent : public juce::Component
{
public:
    explicit PresetBarComponent(NoiseEngineAudioProcessor& processorToUse);

    void resized() override;

    static constexpr int preferredHeight = 30;

private:
    NoiseEngineAudioProcessor& processor;

    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };
    juce::TextButton presetNameButton;
    juce::TextButton saveAsButton { "SAVE AS" };
    juce::TextButton deleteButton { "DELETE" };

    void refreshPresetDisplay();
    void showPresetPopup();
    void showSaveAsDialog();
    void showDeleteConfirm();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBarComponent)
};
