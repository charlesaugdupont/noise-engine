#include "StepSequencerComponent.h"
#include "Colours.h"
#include "../DSP/ParameterIDs.h"

namespace
{
    constexpr int rotateButtonSize = 44;
    constexpr int rotateGap        = 20;
}

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
    // background/track colours come from NoiseEngineLookAndFeel's Slider defaults.
    lengthSlider.setColour(juce::Slider::textBoxTextColourId, Palette::textWhite);
    lengthSlider.setTooltip("Number of steps in the pattern (1-64).");
    lengthSlider.setColour(juce::Slider::textBoxBackgroundColourId, Palette::panelDark);
    lengthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(lengthSlider);

    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, ParamIDs::patternLength, lengthSlider);

    addAndMakeVisible(wheel);

    rotateLeftButton.setTooltip("Rotate the pattern left.");
    rotateRightButton.setTooltip("Rotate the pattern right.");

    // Colours come from NoiseEngineLookAndFeel's TextButton defaults.
    for (auto* button : { &rotateLeftButton, &rotateRightButton })
        addAndMakeVisible(*button);

    rotateLeftButton.onClick  = [this] { rotatePattern([](StepPattern& p) { PatternGenerators::rotateLeft(p); }); };
    rotateRightButton.onClick = [this] { rotatePattern([](StepPattern& p) { PatternGenerators::rotateRight(p); }); };

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

    // Centre the (square) wheel within the available area, then flank it
    // with rotate-left/right — their spatial position either side of the
    // wheel doubles as a hint for what they do.
    const int size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const auto wheelBounds = bounds.withSizeKeepingCentre(size, size);
    wheel.setBounds(wheelBounds);

    rotateLeftButton.setBounds(wheelBounds.getX() - rotateGap - rotateButtonSize,
                                wheelBounds.getCentreY() - rotateButtonSize / 2,
                                rotateButtonSize, rotateButtonSize);

    rotateRightButton.setBounds(wheelBounds.getRight() + rotateGap,
                                 wheelBounds.getCentreY() - rotateButtonSize / 2,
                                 rotateButtonSize, rotateButtonSize);
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

// ---------------------------------------------------------------------------
// Rotate — reads/mutates/writes the pattern directly; rotate never changes
// length, so no need for the length-resync GeneratorPanelComponent's
// generic mutator does for generators that might.
// ---------------------------------------------------------------------------
void StepSequencerComponent::rotatePattern(const std::function<void(StepPattern&)>& mutator)
{
    auto pattern = processor.getPatternSnapshot();
    pattern.length = processor.getPatternLength();
    mutator(pattern);
    processor.setPatternSnapshot(pattern);
}
