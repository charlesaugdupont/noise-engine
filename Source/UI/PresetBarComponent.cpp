#include "PresetBarComponent.h"
#include "Colours.h"

PresetBarComponent::PresetBarComponent(NoiseEngineAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    for (auto* button : { &prevButton, &nextButton, &saveAsButton, &deleteButton })
    {
        button->setColour(juce::TextButton::buttonColourId, Palette::panelDark);
        button->setColour(juce::TextButton::textColourOffId, Palette::accentCyan);
        addAndMakeVisible(*button);
    }

    prevButton.onClick = [this]
    {
        processor.getPresetManager().loadPrevious();
        refreshPresetBox();
    };

    nextButton.onClick = [this]
    {
        processor.getPresetManager().loadNext();
        refreshPresetBox();
    };

    presetBox.setColour(juce::ComboBox::backgroundColourId, Palette::panelDark);
    presetBox.setColour(juce::ComboBox::textColourId, Palette::textWhite);
    presetBox.setColour(juce::ComboBox::outlineColourId, Palette::knobTrack);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id <= 0 || id > itemNamesById.size())
            return;

        processor.getPresetManager().loadPreset(itemNamesById[id - 1]);
        refreshPresetBox();
    };
    addAndMakeVisible(presetBox);

    saveAsButton.onClick   = [this] { showSaveAsDialog(); };
    deleteButton.onClick   = [this] { showDeleteConfirm(); };

    refreshPresetBox();
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

    presetBox.setBounds(bounds);
}

void PresetBarComponent::refreshPresetBox()
{
    auto& pm = processor.getPresetManager();

    presetBox.clear(juce::dontSendNotification);
    itemNamesById.clear();

    presetBox.addSectionHeading("FACTORY");
    for (auto& name : pm.getFactoryPresetNames())
    {
        itemNamesById.add(name);
        presetBox.addItem(name, itemNamesById.size());
    }

    auto userNames = pm.getUserPresetNames();
    if (! userNames.isEmpty())
    {
        presetBox.addSectionHeading("USER");
        for (auto& name : userNames)
        {
            itemNamesById.add(name);
            presetBox.addItem(name, itemNamesById.size());
        }
    }

    const auto currentName = pm.getCurrentPresetName();
    const int  idx         = itemNamesById.indexOf(currentName);

    if (idx >= 0)
        presetBox.setSelectedId(idx + 1, juce::dontSendNotification);
    else
        presetBox.setText(currentName, juce::dontSendNotification);

    deleteButton.setEnabled(! pm.isFactoryPresetName(currentName));
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
                refreshPresetBox();
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
            refreshPresetBox();
        }
    });
}
