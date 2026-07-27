#include "PresetListComponent.h"
#include "Colours.h"

namespace
{
    constexpr int rowHeight     = 24;
    constexpr int headingHeight = 20;

    class Heading : public juce::Label
    {
    public:
        explicit Heading(const juce::String& text)
        {
            setText(text, juce::dontSendNotification);
            setFont(juce::Font(11.0f, juce::Font::bold));
            setColour(juce::Label::textColourId, Palette::accentCyan);
            setJustificationType(juce::Justification::centredLeft);
            setInterceptsMouseClicks(false, false);
        }
    };

    // A plain Component rather than a TextButton — TextButton always centres
    // its text (no left-justify option), which reads worse for a scrollable
    // list of names than a simple custom-painted row does.
    class Row : public juce::Component
    {
    public:
        Row(juce::String presetName, bool isCurrentPreset, std::function<void(juce::String)>& onSelectRef)
            : name(std::move(presetName)), isCurrent(isCurrentPreset), onSelect(onSelectRef)
        {
            setRepaintsOnMouseActivity(true);
        }

        void paint(juce::Graphics& g) override
        {
            if (isMouseOver())
            {
                g.setColour(Palette::accentCyan.withAlpha(0.18f));
                g.fillAll();
            }

            g.setColour(isCurrent ? Palette::accentCyan : Palette::textWhite);
            g.setFont(juce::Font(13.0f, isCurrent ? juce::Font::bold : juce::Font::plain));
            g.drawText(name, getLocalBounds().reduced(10, 0), juce::Justification::centredLeft);
        }

        void mouseUp(const juce::MouseEvent& e) override
        {
            if (contains(e.getPosition()) && onSelect)
                onSelect(name);
        }

    private:
        juce::String name;
        bool isCurrent;
        std::function<void(juce::String)>& onSelect;
    };
}

PresetListComponent::PresetListComponent(const juce::StringArray& factoryNames,
                                          const juce::StringArray& userNames,
                                          const juce::String& currentName)
{
    int y = 0;

    addHeading("FACTORY", y);
    for (auto& name : factoryNames)
        addRow(name, name == currentName, y);

    if (! userNames.isEmpty())
    {
        addHeading("USER", y);
        for (auto& name : userNames)
            addRow(name, name == currentName, y);
    }

    contentHeight = y;
    content.setSize(width, juce::jmax(1, contentHeight));

    viewport.setViewedComponent(&content, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);
}

void PresetListComponent::resized()
{
    viewport.setBounds(getLocalBounds());
}

void PresetListComponent::addHeading(const juce::String& text, int& y)
{
    auto* heading = new Heading(text);
    items.add(heading);
    heading->setBounds(0, y, width, headingHeight);
    content.addAndMakeVisible(heading);
    y += headingHeight;
}

void PresetListComponent::addRow(const juce::String& name, bool isCurrent, int& y)
{
    auto* row = new Row(name, isCurrent, onSelect);
    items.add(row);
    row->setBounds(0, y, width, rowHeight);
    content.addAndMakeVisible(row);
    y += rowHeight;
}
