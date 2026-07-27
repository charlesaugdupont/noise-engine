#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

// Preset browser: prev/next, a dropdown grouped into FACTORY/USER sections,
// Save As, and Delete (disabled for factory presets). Deliberately a plain
// ComboBox rather than a searchable popup — with ~16-20 presets a dropdown
// is plenty, and it matches the rest of the plugin's plain-control style.
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
    juce::ComboBox   presetBox;
    juce::TextButton saveAsButton { "SAVE AS" };
    juce::TextButton deleteButton { "DELETE" };

    // Maps ComboBox item id (1-based) back to the preset name it represents
    // — id-1 indexes this array. Rebuilt each time refreshPresetBox() runs.
    juce::StringArray itemNamesById;

    void refreshPresetBox();
    void showSaveAsDialog();
    void showDeleteConfirm();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBarComponent)
};
