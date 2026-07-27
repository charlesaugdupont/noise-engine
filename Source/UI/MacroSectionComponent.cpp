#include "MacroSectionComponent.h"
#include "Colours.h"

namespace
{
    constexpr int padding    = 8;
    constexpr int headerH    = 24;
    constexpr int rowGap     = 14; // breathing room between the combo row and the knob grid below it

    // 104, not the box's own 100px, so three comboboxes actually fit across
    // a section's ~312px content width (312/104 = 3 exactly) instead of
    // wrapping the third one onto its own mostly-empty row.
    constexpr int comboCellW = 104;
    constexpr int comboW     = 100;
    constexpr int comboH     = 22;
    constexpr int comboLabelH = 14;
    constexpr int comboCellH = comboLabelH + 2 + comboH;

    constexpr int knobCellW  = 96;
    constexpr int knobSize   = 60;
    constexpr int knobLabelH = 14;
    constexpr int knobCellH  = knobSize + 2 + knobLabelH;

    int rowsNeeded(size_t itemCount, int cellW, int availableWidth)
    {
        if (itemCount == 0)
            return 0;

        const int itemsPerRow = juce::jmax(1, availableWidth / cellW);
        return (int) ((itemCount + (size_t) itemsPerRow - 1) / (size_t) itemsPerRow);
    }
}

MacroSectionComponent::MacroSectionComponent(NoiseEngineAudioProcessor& processorToUse, juce::String titleText)
    : processor(processorToUse), title(std::move(titleText))
{
    titleLabel.setText(title, juce::dontSendNotification);
    titleLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Palette::accentCyan);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
}

void MacroSectionComponent::addChoice(const juce::String& paramID, const juce::String& labelText, const juce::String& tooltip)
{
    auto control = std::make_unique<ChoiceControl>();

    control->box.setJustificationType(juce::Justification::centred);
    control->box.setTooltip(tooltip);
    addAndMakeVisible(control->box);

    control->label.setText(labelText, juce::dontSendNotification);
    control->label.setFont(juce::Font(11.0f, juce::Font::bold));
    control->label.setColour(juce::Label::textColourId, Palette::accentCyan);
    control->label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(control->label);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(processor.apvts.getParameter(paramID)))
        control->box.addItemList(param->choices, 1);

    control->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.apvts, paramID, control->box);

    choices.push_back(std::move(control));
}

void MacroSectionComponent::addKnob(const juce::String& paramID, const juce::String& labelText, const juce::String& tooltip)
{
    auto control = std::make_unique<KnobControl>();

    control->slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    control->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    control->slider.setTooltip(tooltip);
    // Display precision is controlled by the parameter's own
    // stringFromValue function (see createParameterLayout/displayAttributes)
    // — SliderAttachment installs a textFromValueFunction that calls
    // param.getText(...), which ignores setNumDecimalPlacesToDisplay entirely.
    control->slider.setColour(juce::Slider::textBoxTextColourId,       Palette::textWhite);
    control->slider.setColour(juce::Slider::textBoxBackgroundColourId, Palette::panelDark);
    control->slider.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    addAndMakeVisible(control->slider);

    control->label.setText(labelText, juce::dontSendNotification);
    control->label.setFont(juce::Font(11.0f, juce::Font::bold));
    control->label.setColour(juce::Label::textColourId, Palette::accentCyan);
    control->label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(control->label);

    control->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, paramID, control->slider);

    knobs.push_back(std::move(control));
}

int MacroSectionComponent::computePreferredHeight(int width) const
{
    if (compactSingleRow)
    {
        // Assumes everything fits on one row at the given width — true for
        // the one section this is used on (5 controls at full panel width).
        // Not worth a full wrapping simulation for a case that doesn't wrap.
        const int rowH = knobs.empty() ? comboCellH : knobCellH;
        return headerH + padding * 2 + rowH;
    }

    const int contentWidth = juce::jmax(1, width - padding * 2);

    int height = headerH + padding;
    height += rowsNeeded(choices.size(), comboCellW, contentWidth) * comboCellH;

    if (! choices.empty() && ! knobs.empty())
        height += rowGap;

    height += rowsNeeded(knobs.size(), knobCellW, contentWidth) * knobCellH;
    height += padding;
    return height;
}

void MacroSectionComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(Palette::panelDark);
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(Palette::knobTrack);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void MacroSectionComponent::resized()
{
    auto bounds = getLocalBounds().reduced(padding);

    titleLabel.setBounds(bounds.removeFromTop(headerH));

    if (compactSingleRow)
    {
        layoutSingleRow(bounds);
        return;
    }

    const int contentWidth = bounds.getWidth();

    // When this section's actual assigned height (matched to a taller
    // row-mate, e.g. TIMING next to SHAPE) exceeds what its own content
    // needs, centre the content block in the space below the title instead
    // of leaving all the slack as a dead zone at the bottom.
    const int comboRows    = rowsNeeded(choices.size(), comboCellW, contentWidth);
    const int knobRows     = rowsNeeded(knobs.size(), knobCellW, contentWidth);
    const int neededHeight = comboRows * comboCellH
                            + ((! choices.empty() && ! knobs.empty()) ? rowGap : 0)
                            + knobRows * knobCellH;
    const int extraSpace   = juce::jmax(0, bounds.getHeight() - neededHeight);
    bounds.removeFromTop(extraSpace / 2);

    if (! choices.empty())
    {
        const int itemsPerRow = juce::jmax(1, contentWidth / comboCellW);
        int col = 0;
        int x   = bounds.getX();
        int y   = bounds.getY();

        for (auto& c : choices)
        {
            if (col == itemsPerRow)
            {
                col = 0;
                x = bounds.getX();
                y += comboCellH;
            }

            c->label.setBounds(x, y, comboW, comboLabelH);
            c->box.setBounds(x, y + comboLabelH + 2, comboW - 8, comboH);

            x += comboCellW;
            ++col;
        }

        bounds.removeFromTop(rowsNeeded(choices.size(), comboCellW, contentWidth) * comboCellH);

        if (! knobs.empty())
            bounds.removeFromTop(rowGap);
    }

    if (! knobs.empty())
    {
        const int itemsPerRow = juce::jmax(1, contentWidth / knobCellW);
        int col = 0;
        int x   = bounds.getX();
        int y   = bounds.getY();

        for (auto& k : knobs)
        {
            if (col == itemsPerRow)
            {
                col = 0;
                x = bounds.getX();
                y += knobCellH;
            }

            k->slider.setBounds(x, y, knobSize, knobSize);
            k->label.setBounds(x, y + knobSize + 2, knobSize, knobLabelH);

            x += knobCellW;
            ++col;
        }
    }
}

// Choices then knobs, left to right, all on one row. Combos are shorter
// than knobs (label+box vs. knob+label), so their content block is
// vertically centred within the shared row height rather than top-aligned,
// to keep both control types looking like they belong to the same row.
void MacroSectionComponent::layoutSingleRow(juce::Rectangle<int> bounds)
{
    const int rowH = knobs.empty() ? comboCellH : knobCellH;
    const int y    = bounds.getY();
    int       x    = bounds.getX();

    for (auto& c : choices)
    {
        const int comboContentH = comboLabelH + 2 + comboH;
        const int comboY        = y + (rowH - comboContentH) / 2;

        c->label.setBounds(x, comboY, comboW, comboLabelH);
        c->box.setBounds(x, comboY + comboLabelH + 2, comboW - 8, comboH);

        x += comboCellW;
    }

    for (auto& k : knobs)
    {
        k->slider.setBounds(x, y, knobSize, knobSize);
        k->label.setBounds(x, y + knobSize + 2, knobSize, knobLabelH);

        x += knobCellW;
    }
}
