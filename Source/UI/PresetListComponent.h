#pragma once
#include <JuceHeader.h>

// Scrollable, FACTORY/USER-grouped preset list, shown inside a
// juce::CallOutBox by PresetBarComponent. A CallOutBox constrains its own
// position to whatever parent component it's given (see updatePosition in
// JUCE's source) — launching it with the plugin editor as parent means this
// list can never be clipped off the top/bottom of the screen the way the
// old ComboBox's native popup could be.
class PresetListComponent : public juce::Component
{
public:
    PresetListComponent(const juce::StringArray& factoryNames,
                         const juce::StringArray& userNames,
                         const juce::String& currentName);

    void resized() override;

    static constexpr int width            = 240;
    static constexpr int maxVisibleHeight = 320;

    // Full unclamped height of the list content. The caller sizes this
    // component to preferredHeight() (already clamped to maxVisibleHeight)
    // and gets scrolling for free from the internal Viewport when the full
    // list is taller than that.
    int preferredHeight() const { return juce::jmin(maxVisibleHeight, contentHeight); }

    // Fired with the clicked preset's name. The caller (PresetBarComponent)
    // handles loading it and dismissing the CallOutBox that hosts this list.
    std::function<void(juce::String)> onSelect;

private:
    juce::Viewport  viewport;
    juce::Component content;
    juce::OwnedArray<juce::Component> items;
    int contentHeight = 0;

    void addHeading(const juce::String& text, int& y);
    void addRow(const juce::String& name, bool isCurrent, int& y);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetListComponent)
};
