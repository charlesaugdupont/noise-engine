#include "PresetManager.h"
#include "../PluginProcessor.h"

PresetManager::PresetManager(NoiseEngineAudioProcessor& processorToUse)
    : processor(processorToUse), factoryPresets(FactoryPresets::createAll())
{
    if (! factoryPresets.empty())
        currentPresetName = factoryPresets.front().name;
}

// ---------------------------------------------------------------------------
// Listing
// ---------------------------------------------------------------------------
juce::StringArray PresetManager::getFactoryPresetNames() const
{
    juce::StringArray names;
    for (auto& def : factoryPresets)
        names.add(def.name);
    return names;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    juce::StringArray names;
    auto dir = getUserPresetDirectory();

    if (! dir.isDirectory())
        return names;

    for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*.nepreset", juce::File::findFiles))
        names.add(entry.getFile().getFileNameWithoutExtension());

    names.sort(true);
    return names;
}

bool PresetManager::isFactoryPresetName(const juce::String& name) const
{
    for (auto& def : factoryPresets)
        if (def.name == name)
            return true;
    return false;
}

// ---------------------------------------------------------------------------
// Paths
// ---------------------------------------------------------------------------
juce::File PresetManager::getUserPresetDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("NoiseEngine")
        .getChildFile("Presets");
}

juce::File PresetManager::getUserPresetFile(const juce::String& name) const
{
    return getUserPresetDirectory().getChildFile(name + ".nepreset");
}

// ---------------------------------------------------------------------------
// Applying
// ---------------------------------------------------------------------------
void PresetManager::resetParametersToDefault()
{
    for (auto* param : processor.getParameters())
        param->setValueNotifyingHost(param->getDefaultValue());
}

void PresetManager::applyFactoryDefinition(const FactoryPresetDefinition& def)
{
    resetParametersToDefault();

    for (auto& [paramID, value] : def.paramValues)
        if (auto* param = processor.apvts.getParameter(paramID))
            param->setValueNotifyingHost(param->convertTo0to1(value));

    processor.setPatternSnapshot(def.pattern);
    processor.setPatternLength(def.pattern.length); // keep the Length parameter in sync — see header comment

    currentPresetName = def.name;
}

bool PresetManager::applyUserPresetFile(const juce::File& file, const juce::String& name)
{
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
        return false;

    processor.applyFullState(juce::ValueTree::fromXml(*xml));
    currentPresetName = name;
    return true;
}

// ---------------------------------------------------------------------------
// Load / navigate
// ---------------------------------------------------------------------------
bool PresetManager::loadPreset(const juce::String& name)
{
    for (auto& def : factoryPresets)
    {
        if (def.name == name)
        {
            applyFactoryDefinition(def);
            return true;
        }
    }

    auto file = getUserPresetFile(name);
    if (file.existsAsFile())
        return applyUserPresetFile(file, name);

    return false;
}

void PresetManager::loadNext()
{
    auto all = getFactoryPresetNames();
    all.addArray(getUserPresetNames());

    if (all.isEmpty())
        return;

    const int index = all.indexOf(currentPresetName);
    loadPreset(all[(index < 0) ? 0 : (index + 1) % all.size()]);
}

void PresetManager::loadPrevious()
{
    auto all = getFactoryPresetNames();
    all.addArray(getUserPresetNames());

    if (all.isEmpty())
        return;

    const int index = all.indexOf(currentPresetName);
    loadPreset(all[(index < 0) ? 0 : (index - 1 + all.size()) % all.size()]);
}

// ---------------------------------------------------------------------------
// Save / delete
// ---------------------------------------------------------------------------
bool PresetManager::saveAsNewPreset(const juce::String& name)
{
    if (name.isEmpty() || isFactoryPresetName(name))
        return false;

    auto dir = getUserPresetDirectory();
    if (! dir.isDirectory() && dir.createDirectory().failed())
        return false;

    auto state = processor.captureFullState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml == nullptr || ! xml->writeTo(getUserPresetFile(name)))
        return false;

    currentPresetName = name;
    return true;
}

bool PresetManager::deletePreset(const juce::String& name)
{
    if (isFactoryPresetName(name))
        return false;

    auto file = getUserPresetFile(name);
    if (! file.existsAsFile())
        return false;

    if (! file.deleteFile())
        return false;

    // Deleting the preset you're currently on would otherwise leave the
    // dropdown showing a name that's no longer in the list at all — fall
    // back to the first factory preset (Default) instead.
    if (currentPresetName == name && ! factoryPresets.empty())
        loadPreset(factoryPresets.front().name);

    return true;
}
