#include "GeneratorPanelComponent.h"
#include "Colours.h"

namespace
{
    constexpr int padding    = 8;
    constexpr int headerH    = 24;
    constexpr int rowH       = 30;
    constexpr int rowGap     = 6;
    constexpr int gap        = 6;
    constexpr int buttonColW = 100; // reserved on the right in both generator rows, so
                                     // APPLY and RANDOMIZE always land in the same column

    // EUCLID/RANDOM are group labels (one per generator, bigger + cyan, like
    // a mini section title); PULSES/DENSITY/JITTER are field labels for the
    // individual sliders (smaller + plain white). Same size/colour as each
    // other was reading as no hierarchy at all.
    void setupGroupLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(13.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, Palette::accentCyan);
        label.setJustificationType(juce::Justification::centredLeft);
    }

    void setupFieldLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(11.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, Palette::textWhite);
        label.setJustificationType(juce::Justification::centredLeft);
    }
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
GeneratorPanelComponent::GeneratorPanelComponent(NoiseEngineAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    titleLabel.setText("PATTERN GENERATOR", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Palette::accentCyan);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    setupGroupLabel(euclideanCaption, "EUCLID");
    setupFieldLabel(pulsesCaption,    "PULSES");
    setupGroupLabel(randomCaption,    "RANDOM");
    setupFieldLabel(densityCaption,   "DENSITY");
    setupFieldLabel(jitterCaption,    "JITTER");

    for (auto* label : { &euclideanCaption, &pulsesCaption, &randomCaption, &densityCaption, &jitterCaption })
        addAndMakeVisible(*label);

    const int initialLength = processor.getPatternLength();
    lastKnownLength = initialLength;

    setupSlider(pulsesSlider,  0.0, (double) initialLength, 1.0, (double) juce::jmin(4, initialLength));
    setupSlider(densitySlider, 0.0, 100.0, 1.0, 50.0);
    setupSlider(jitterSlider,  0.0, 100.0, 1.0, 0.0);

    for (auto* button : { &applyEuclideanButton, &randomizeButton, &duplicateButton, &halveButton })
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

    duplicateButton.onClick = [this] { mutatePattern([](StepPattern& p) { PatternGenerators::duplicateToDouble(p); }); };
    halveButton.onClick     = [this] { mutatePattern([](StepPattern& p) { PatternGenerators::halveLength(p); }); };

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
// Paint — bordered/titled panel, matching MacroSectionComponent's look, plus
// a thin divider between the Euclidean and Random rows so they read as two
// distinct tools rather than one undifferentiated block.
// ---------------------------------------------------------------------------
void GeneratorPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(Palette::panelDark);
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(Palette::knobTrack);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    const float dividerY = (float) (padding + headerH + rowH + rowGap / 2);
    g.drawLine((float) padding, dividerY, bounds.getWidth() - (float) padding, dividerY, 1.0f);
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------
void GeneratorPanelComponent::resized()
{
    auto bounds = getLocalBounds().reduced(padding);

    titleLabel.setBounds(bounds.removeFromTop(headerH));

    // Row 1 — Euclidean. Button column reserved first (same width as row 2's)
    // so APPLY and RANDOMIZE always line up in the same column regardless of
    // how much space the preceding captions/sliders in each row take up.
    auto row1        = bounds.removeFromTop(rowH);
    auto row1Buttons = row1.removeFromRight(buttonColW);
    applyEuclideanButton.setBounds(row1Buttons.withSizeKeepingCentre(90, 24));

    euclideanCaption.setBounds(row1.removeFromLeft(58));
    row1.removeFromLeft(gap);
    pulsesCaption.setBounds(row1.removeFromLeft(52));
    pulsesSlider.setBounds(row1);

    bounds.removeFromTop(rowGap);

    // Row 2 — density-random.
    auto row2        = bounds.removeFromTop(rowH);
    auto row2Buttons = row2.removeFromRight(buttonColW);
    randomizeButton.setBounds(row2Buttons.withSizeKeepingCentre(90, 24));

    randomCaption.setBounds(row2.removeFromLeft(58));
    row2.removeFromLeft(gap);
    densityCaption.setBounds(row2.removeFromLeft(52));
    densitySlider.setBounds(row2.removeFromLeft(150));
    row2.removeFromLeft(gap);
    jitterCaption.setBounds(row2.removeFromLeft(42));
    jitterSlider.setBounds(row2);

    bounds.removeFromTop(rowGap);

    // Row 3 — pattern utilities (rotate lives on the wheel now).
    auto row3 = bounds.removeFromTop(26);
    duplicateButton.setBounds(row3.removeFromLeft(40));
    row3.removeFromLeft(gap);
    halveButton.setBounds(row3.removeFromLeft(52));
}
