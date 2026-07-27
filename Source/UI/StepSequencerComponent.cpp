#include "StepSequencerComponent.h"
#include "Colours.h"
#include "../DSP/ParameterIDs.h"

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
StepSequencerComponent::StepSequencerComponent(NoiseEngineAudioProcessor& processorToUse)
    : processor(processorToUse), wheel(processorToUse)
{
    lengthCaption.setText("LENGTH", juce::dontSendNotification);
    lengthCaption.setFont(juce::Font(11.0f, juce::Font::bold));
    lengthCaption.setColour(juce::Label::textColourId, Palette::accentCyan);
    addAndMakeVisible(lengthCaption);

    // A slider (rather than +/- buttons) so jumping across the 1..64 range
    // is one drag instead of dozens of clicks; the text box still allows
    // exact entry.
    lengthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, headerHeight);
    lengthSlider.setColour(juce::Slider::backgroundColourId, Palette::knobTrack);
    lengthSlider.setColour(juce::Slider::trackColourId, Palette::accentCyan);
    lengthSlider.setColour(juce::Slider::textBoxTextColourId, Palette::textWhite);
    lengthSlider.setColour(juce::Slider::textBoxBackgroundColourId, Palette::panelDark);
    lengthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(lengthSlider);

    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, ParamIDs::patternLength, lengthSlider);

    addAndMakeVisible(wheel);

    startTimerHz(30);
}

StepSequencerComponent::~StepSequencerComponent()
{
    stopTimer();
}

// ---------------------------------------------------------------------------
// Paint / layout
// ---------------------------------------------------------------------------
void StepSequencerComponent::paint(juce::Graphics& g)
{
    g.fillAll(Palette::bgDark);
}

void StepSequencerComponent::resized()
{
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(headerHeight);

    lengthCaption.setBounds(header.removeFromLeft(56));
    lengthSlider.setBounds(header.removeFromLeft(240).reduced(2, 0));

    bounds.removeFromTop(headerGridGap);

    // Centre the (square) wheel within the available area.
    const int size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    wheel.setBounds(bounds.withSizeKeepingCentre(size, size));
}

// ---------------------------------------------------------------------------
// Timer: pull the live pattern (steps + length) so the wheel reflects edits
// made anywhere else (Length slider, generator panel, host automation), and
// animate the playhead.
// ---------------------------------------------------------------------------
void StepSequencerComponent::timerCallback()
{
    wheel.syncFromProcessor();
    wheel.repaint();
}
