#include "GeneratorPanelComponent.h"
#include "Colours.h"

namespace
{
    void setupCaption(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(10.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, Palette::accentCyan);
        label.setJustificationType(juce::Justification::centredLeft);
    }
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
GeneratorPanelComponent::GeneratorPanelComponent(NoiseEngineAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    setupCaption(euclideanCaption, "EUCLID");
    setupCaption(pulsesCaption,    "PULSES");
    setupCaption(randomCaption,    "RANDOM");
    setupCaption(densityCaption,   "DENSITY");
    setupCaption(jitterCaption,    "JITTER");

    for (auto* label : { &euclideanCaption, &pulsesCaption, &randomCaption, &densityCaption, &jitterCaption })
        addAndMakeVisible(*label);

    const int initialLength = processor.getPatternLength();
    lastKnownLength = initialLength;

    setupSlider(pulsesSlider,  0.0, (double) initialLength, 1.0, (double) juce::jmin(4, initialLength));
    setupSlider(densitySlider, 0.0, 100.0, 1.0, 50.0);
    setupSlider(jitterSlider,  0.0, 100.0, 1.0, 0.0);

    for (auto* button : { &applyEuclideanButton, &randomizeButton, &rotateLeftButton,
                           &rotateRightButton, &duplicateButton, &halveButton })
    {
        button->setColour(juce::TextButton::buttonColourId, Palette::panelDark);
        button->setColour(juce::TextButton::textColourOffId, Palette::accentCyan);
        addAndMakeVisible(*button);
    }

    applyEuclideanButton.onClick = [this]
    {
        const int steps  = processor.getPatternLength();
        const int pulses = (int) pulsesSlider.getValue();

        mutatePattern([steps, pulses](StepPattern& p)
        {
            PatternGenerators::applyEuclidean(p, steps, pulses);
        });
    };

    randomizeButton.onClick = [this]
    {
        const float density = (float) densitySlider.getValue() / 100.0f;
        const float jitter   = (float) jitterSlider.getValue() / 100.0f;

        mutatePattern([this, density, jitter](StepPattern& p)
        {
            PatternGenerators::randomize(p, density, jitter, rng);
        });
    };

    rotateLeftButton.onClick  = [this] { mutatePattern([](StepPattern& p) { PatternGenerators::rotateLeft(p); }); };
    rotateRightButton.onClick = [this] { mutatePattern([](StepPattern& p) { PatternGenerators::rotateRight(p); }); };
    duplicateButton.onClick   = [this] { mutatePattern([](StepPattern& p) { PatternGenerators::duplicateToDouble(p); }); };
    halveButton.onClick       = [this] { mutatePattern([](StepPattern& p) { PatternGenerators::halveLength(p); }); };

    startTimerHz(10);
}

GeneratorPanelComponent::~GeneratorPanelComponent()
{
    stopTimer();
}

void GeneratorPanelComponent::setupSlider(juce::Slider& slider, double minV, double maxV, double step, double defaultV)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setRange(minV, maxV, step);
    slider.setValue(defaultV, juce::dontSendNotification);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 34, 20);
    slider.setColour(juce::Slider::backgroundColourId, Palette::knobTrack);
    slider.setColour(juce::Slider::trackColourId, Palette::accentCyan);
    slider.setColour(juce::Slider::textBoxTextColourId, Palette::textWhite);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, Palette::panelDark);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);
}

// ---------------------------------------------------------------------------
// Pattern mutation glue: read the live pattern + length, apply a generator,
// write the length back only if the generator changed it, then push the
// pattern. Shared by every button so each one is a one-line lambda.
// ---------------------------------------------------------------------------
void GeneratorPanelComponent::mutatePattern(const std::function<void(StepPattern&)>& mutator)
{
    StepPattern pattern = processor.getPatternSnapshot();
    pattern.length = processor.getPatternLength();

    mutator(pattern);

    if (pattern.length != processor.getPatternLength())
        processor.setPatternLength(pattern.length);

    processor.setPatternSnapshot(pattern);
}

// ---------------------------------------------------------------------------
// Timer: keep the Pulses range matched to the live Pattern Length
// ---------------------------------------------------------------------------
void GeneratorPanelComponent::timerCallback()
{
    const int length = processor.getPatternLength();
    if (length == lastKnownLength)
        return;

    lastKnownLength = length;
    pulsesSlider.setRange(0.0, (double) length, 1.0);
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------
void GeneratorPanelComponent::resized()
{
    constexpr int rowH    = 30;
    constexpr int sliderW = 100;
    constexpr int gap     = 6;

    auto bounds = getLocalBounds();
    auto row1   = bounds.removeFromTop(rowH);

    euclideanCaption.setBounds(row1.removeFromLeft(46));
    row1.removeFromLeft(gap);
    pulsesCaption.setBounds(row1.removeFromLeft(40));
    pulsesSlider.setBounds(row1.removeFromLeft(220));
    row1.removeFromLeft(gap);
    applyEuclideanButton.setBounds(row1.removeFromLeft(70).reduced(0, 2));

    bounds.removeFromTop(4);
    auto row2 = bounds.removeFromTop(rowH);

    randomCaption.setBounds(row2.removeFromLeft(46));
    row2.removeFromLeft(gap);
    densityCaption.setBounds(row2.removeFromLeft(48));
    densitySlider.setBounds(row2.removeFromLeft(sliderW));
    row2.removeFromLeft(gap);
    jitterCaption.setBounds(row2.removeFromLeft(38));
    jitterSlider.setBounds(row2.removeFromLeft(sliderW));
    row2.removeFromLeft(gap);
    randomizeButton.setBounds(row2.removeFromLeft(86).reduced(0, 2));

    row2.removeFromLeft(gap * 2);
    rotateLeftButton.setBounds(row2.removeFromLeft(26).reduced(0, 2));
    rotateRightButton.setBounds(row2.removeFromLeft(26).reduced(0, 2));
    row2.removeFromLeft(gap);
    duplicateButton.setBounds(row2.removeFromLeft(36).reduced(0, 2));
    row2.removeFromLeft(gap);
    halveButton.setBounds(row2.removeFromLeft(36).reduced(0, 2));
}
