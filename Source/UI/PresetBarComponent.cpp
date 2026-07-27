#include "PresetBarComponent.h"
#include "Colours.h"
#include "PresetListComponent.h"

PresetBarComponent::PresetBarComponent(NoiseEngineAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    // Colours come from NoiseEngineLookAndFeel's TextButton defaults.
    for (auto* button : { &prevButton, &nextButton, &presetNameButton, &saveAsButton, &deleteButton })
        addAndMakeVisible(*button);

    prevButton.setTooltip("Load the previous preset.");
    nextButton.setTooltip("Load the next preset.");
    presetNameButton.setTooltip("Browse presets.");
    saveAsButton.setTooltip("Save the current settings as a new preset.");
    deleteButton.setTooltip("Delete this preset. Only available for your own saved presets.");

    prevButton.onClick = [this]
    {
        processor.getPresetManager().loadPrevious();
        refreshPresetDisplay();
    };

    nextButton.onClick = [this]
    {
        processor.getPresetManager().loadNext();
        refreshPresetDisplay();
    };

    presetNameButton.onClick = [this] { showPresetPopup(); };
    saveAsButton.onClick     = [this] { showSaveAsDialog(); };
    deleteButton.onClick     = [this] { showDeleteConfirm(); };

    refreshPresetDisplay();
}

void PresetBarComponent::resized()
{
    constexpr int gap = 6;

    auto bounds = getLocalBounds();

    prevButton.setBounds(bounds.removeFromLeft(26));
    bounds.removeFromLeft(gap);
    nextButton.setBounds(bounds.removeFromLeft(26));
    bounds.removeFromLeft(gap);

    saveAsButton.setBounds(bounds.removeFromRight(70));
    bounds.removeFromRight(gap);
    deleteButton.setBounds(bounds.removeFromRight(70));
    bounds.removeFromRight(gap);

    presetNameButton.setBounds(bounds);
}

void PresetBarComponent::refreshPresetDisplay()
{
    auto& pm = processor.getPresetManager();
    const auto currentName = pm.getCurrentPresetName();

    presetNameButton.setButtonText(currentName + "  \xE2\x96\xBE"); // U+25BE small down triangle
    deleteButton.setEnabled(! pm.isFactoryPresetName(currentName));
}

// ---------------------------------------------------------------------------
// Preset popup — a PresetListComponent hosted in a CallOutBox, launched with
// the top-level editor as its parent. CallOutBox constrains its own position
// to whatever parent it's given, so anchoring it to the (fixed-size) editor
// window rather than the screen means it can never get clipped or scrolled
// oddly near a screen edge the way the old ComboBox's native popup could.
// ---------------------------------------------------------------------------
void PresetBarComponent::showPresetPopup()
{
    auto& pm = processor.getPresetManager();

    auto listComponent = std::make_unique<PresetListComponent>(
        pm.getFactoryPresetNames(), pm.getUserPresetNames(), pm.getCurrentPresetName());

    auto* listPtr = listComponent.get();
    listComponent->setSize(PresetListComponent::width, listComponent->preferredHeight());

    auto* target = presetNameButton.getTopLevelComponent();
    const auto areaInTarget = target->getLocalArea(&presetNameButton, presetNameButton.getLocalBounds());

    auto& callout = juce::CallOutBox::launchAsynchronously(std::move(listComponent), areaInTarget, target);

    listPtr->onSelect = [this, &callout](const juce::String& name)
    {
        processor.getPresetManager().loadPreset(name);
        refreshPresetDisplay();
        callout.dismiss();
    };
}

void PresetBarComponent::showSaveAsDialog()
{
    auto* window = new juce::AlertWindow("Save Preset",
                                          "Enter a name for this preset:",
                                          juce::MessageBoxIconType::NoIcon);
    window->addTextEditor("name", processor.getPresetManager().getCurrentPresetName(), "Name:");
    window->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
    {
        if (result == 1)
        {
            const auto name = window->getTextEditorContents("name").trim();
            auto& pm        = processor.getPresetManager();

            if (name.isEmpty() || pm.isFactoryPresetName(name))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon, "Can't Save Preset",
                    name.isEmpty() ? "Preset name can't be empty."
                                   : "\"" + name + "\" is a factory preset name — pick a different name.");
            }
            else
            {
                pm.saveAsNewPreset(name);
                refreshPresetDisplay();
            }
        }

        delete window;
    }), false);
}

void PresetBarComponent::showDeleteConfirm()
{
    const auto name = processor.getPresetManager().getCurrentPresetName();

    if (processor.getPresetManager().isFactoryPresetName(name))
        return;

    auto options = juce::MessageBoxOptions::makeOptionsYesNo(
        juce::MessageBoxIconType::WarningIcon, "Delete Preset",
        "Delete \"" + name + "\"? This can't be undone.");

    juce::AlertWindow::showAsync(options, [this, name](int result)
    {
        if (result == 1)
        {
            processor.getPresetManager().deletePreset(name);
            refreshPresetDisplay();
        }
    });
}
