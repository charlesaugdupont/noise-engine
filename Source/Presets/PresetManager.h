#pragma once
#include <JuceHeader.h>
#include "FactoryPresets.h"

class NoiseEngineAudioProcessor;

// Browses/loads/saves/deletes presets. Factory presets are built-in C++
// data (see FactoryPresets.h), always available regardless of install
// location. User presets are plain XML files under a per-user directory,
// using the same combined macro-params + pattern ValueTree that DAW session
// state uses (NoiseEngineAudioProcessor::captureFullState/applyFullState) —
// one serialization path, two different outer wrappers (binary blob for
// host state, plain XML file for presets).
class PresetManager
{
public:
    explicit PresetManager(NoiseEngineAudioProcessor& processorToUse);

    juce::StringArray getFactoryPresetNames() const;
    juce::StringArray getUserPresetNames() const;

    // Searches factory presets first, then user presets.
    bool loadPreset(const juce::String& name);
    void loadNext();
    void loadPrevious();

    // User presets only. Fails (returns false) if `name` is empty or
    // collides with a factory preset name — a user preset can't shadow or
    // replace a factory one, it just wouldn't be clear which one loads.
    bool saveAsNewPreset(const juce::String& name);

    // False for factory presets or a name that doesn't exist as a user preset.
    bool deletePreset(const juce::String& name);

    juce::String getCurrentPresetName() const { return currentPresetName; }
    bool isFactoryPresetName(const juce::String& name) const;

private:
    NoiseEngineAudioProcessor& processor;
    std::vector<FactoryPresetDefinition> factoryPresets;
    juce::String currentPresetName;

    juce::File getUserPresetDirectory() const;
    juce::File getUserPresetFile(const juce::String& name) const;

    void resetParametersToDefault();
    void applyFactoryDefinition(const FactoryPresetDefinition& def);
    bool applyUserPresetFile(const juce::File& file, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
