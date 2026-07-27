#include "NoiseEngineLookAndFeel.h"
#include "Colours.h"

NoiseEngineLookAndFeel::NoiseEngineLookAndFeel()
{
    // ComboBox (macro dropdowns + preset browser)
    setColour(juce::ComboBox::backgroundColourId, Palette::panelDark);
    setColour(juce::ComboBox::textColourId, Palette::textWhite);
    setColour(juce::ComboBox::outlineColourId, Palette::knobTrack);
    setColour(juce::ComboBox::buttonColourId, Palette::panelDark);
    setColour(juce::ComboBox::arrowColourId, Palette::accentCyan);
    setColour(juce::ComboBox::focusedOutlineColourId, Palette::accentCyan);

    // PopupMenu (every dropdown's opened list, including FACTORY/USER
    // section headers in the preset browser)
    setColour(juce::PopupMenu::backgroundColourId, Palette::panelDark);
    setColour(juce::PopupMenu::textColourId, Palette::textWhite);
    setColour(juce::PopupMenu::headerTextColourId, Palette::accentCyan);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Palette::accentCyan.withAlpha(0.20f));
    setColour(juce::PopupMenu::highlightedTextColourId, Palette::textWhite);

    // TextButton / ToggleButton
    setColour(juce::TextButton::buttonColourId, Palette::panelDark);
    setColour(juce::TextButton::buttonOnColourId, Palette::accentCyan.withAlpha(0.35f));
    setColour(juce::TextButton::textColourOffId, Palette::accentCyan);
    setColour(juce::TextButton::textColourOnId, Palette::bgDark);
    setColour(juce::ToggleButton::textColourId, Palette::textWhite);
    setColour(juce::ToggleButton::tickColourId, Palette::accentCyan);
    setColour(juce::ToggleButton::tickDisabledColourId, Palette::knobTrack);

    // TextEditor (the Save Preset name field)
    setColour(juce::TextEditor::backgroundColourId, Palette::panelDark);
    setColour(juce::TextEditor::textColourId, Palette::textWhite);
    setColour(juce::TextEditor::highlightColourId, Palette::accentCyan.withAlpha(0.35f));
    setColour(juce::TextEditor::highlightedTextColourId, Palette::textWhite);
    setColour(juce::TextEditor::outlineColourId, Palette::knobTrack);
    setColour(juce::TextEditor::focusedOutlineColourId, Palette::accentCyan);

    // Slider defaults (LinearHorizontal sliders — Length/Pulses/Rotation/
    // Density/Jitter — already set track/background explicitly per-slider,
    // but not thumbColourId, so it was falling back to JUCE's stock colour)
    setColour(juce::Slider::thumbColourId, Palette::accentCyan);
    setColour(juce::Slider::trackColourId, Palette::accentCyan);
    setColour(juce::Slider::backgroundColourId, Palette::knobTrack);

    // Label (fallback for any label that doesn't set its own colour, e.g.
    // ComboBox's internal "nothing selected" placeholder text)
    setColour(juce::Label::textColourId, Palette::textWhite);

    // AlertWindow (Save/Delete confirmation dialogs) — its buttons and text
    // field are regular TextButton/TextEditor instances, already themed above
    setColour(juce::AlertWindow::backgroundColourId, Palette::panelDark);
    setColour(juce::AlertWindow::textColourId, Palette::textWhite);
    setColour(juce::AlertWindow::outlineColourId, Palette::knobTrack);
}

void NoiseEngineLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                               int x, int y, int width, int height,
                                               float sliderPosProportional,
                                               float rotaryStartAngle, float rotaryEndAngle,
                                               juce::Slider&)
{
    const float radius  = (float) juce::jmin(width, height) * 0.5f - 4.0f;
    const float centreX = (float) x + (float) width  * 0.5f;
    const float centreY = (float) y + (float) height * 0.5f;
    const float angle   = rotaryStartAngle
                        + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Background circle
    g.setColour(Palette::knobTrack);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Full arc track
    juce::Path arcTrack;
    arcTrack.addArc(centreX - radius, centreY - radius,
                    radius * 2.0f, radius * 2.0f,
                    rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(Palette::knobTrack.brighter(0.3f));
    g.strokePath(arcTrack, juce::PathStrokeType(3.0f));

    // Value arc (cyan fill up to current position)
    juce::Path valueArc;
    valueArc.addArc(centreX - radius, centreY - radius,
                    radius * 2.0f, radius * 2.0f,
                    rotaryStartAngle, angle, true);
    g.setColour(Palette::accentCyan);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f));

    // Pointer line
    const float pointerLength = radius * 0.55f;
    const float pointerX = centreX + std::sin(angle) * pointerLength;
    const float pointerY = centreY - std::cos(angle) * pointerLength;
    g.setColour(Palette::textWhite);
    g.drawLine(centreX, centreY, pointerX, pointerY, 2.5f);

    // Centre dot
    g.setColour(Palette::accentCyan);
    g.fillEllipse(centreX - 3.0f, centreY - 3.0f, 6.0f, 6.0f);
}
