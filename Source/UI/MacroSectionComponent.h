#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

// A titled, bordered panel grouping a related set of macro controls (e.g.
// "TIMING": Sync/Rate/Modifier/Free BPM/Swing). Replaces the old flat,
// undifferentiated knob/combo grid in the editor with visually distinct
// sections. Comboboxes lay out in their own row (label above box, matching
// the plugin's existing convention), knobs wrap in a grid below (label
// below knob) — same conventions the flat grid already used, just now
// scoped per-section instead of globally.
class MacroSectionComponent : public juce::Component
{
public:
    MacroSectionComponent(NoiseEngineAudioProcessor& processorToUse, juce::String titleText);

    void addChoice(const juce::String& paramID, const juce::String& labelText);
    void addKnob(const juce::String& paramID, const juce::String& labelText);

    // Opt-in: lays every control (choices then knobs, in add order) out on
    // a single row instead of choices-row-then-knob-grid. Off by default —
    // only used where a section has few enough controls to comfortably fit
    // one row at full panel width (e.g. STEREO & OUTPUT); TIMING/SHAPE keep
    // the default wrapping layout, which is what they were designed for.
    void setCompactSingleRow(bool shouldBeCompact) { compactSingleRow = shouldBeCompact; }

    // Pure calculation (no layout side effects) — the parent uses this to
    // size sections before committing to a grid of bounds.
    int computePreferredHeight(int width) const;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct KnobControl
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct ChoiceControl
    {
        juce::ComboBox box;
        juce::Label    label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    NoiseEngineAudioProcessor& processor;
    juce::String title;
    juce::Label  titleLabel;
    bool         compactSingleRow = false;

    std::vector<std::unique_ptr<ChoiceControl>> choices;
    std::vector<std::unique_ptr<KnobControl>>   knobs;

    void layoutSingleRow(juce::Rectangle<int> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroSectionComponent)
};
