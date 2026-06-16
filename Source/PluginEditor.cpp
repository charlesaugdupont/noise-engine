#include "PluginEditor.h"

// ---------------------------------------------------------------------------
// Colour palette (Neural DSP-inspired dark theme)
// ---------------------------------------------------------------------------
static const juce::Colour BG_DARK     { 0xFF1A1A1A };
static const juce::Colour PANEL_DARK  { 0xFF242424 };
static const juce::Colour ACCENT_CYAN { 0xFF00E5CC };
static const juce::Colour TEXT_WHITE  { 0xFFE0E0E0 };
static const juce::Colour KNOB_TRACK  { 0xFF3A3A3A };

// ---------------------------------------------------------------------------
// Custom LookAndFeel for a clean, modern rotary knob
// ---------------------------------------------------------------------------
class NoiseEngineLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override
    {
        const float radius  = (float)juce::jmin(width, height) * 0.5f - 4.0f;
        const float centreX = (float)x + (float)width  * 0.5f;
        const float centreY = (float)y + (float)height * 0.5f;
        const float angle   = rotaryStartAngle
                            + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background circle
        g.setColour(KNOB_TRACK);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        // Full arc track
        juce::Path arcTrack;
        arcTrack.addArc(centreX - radius, centreY - radius,
                        radius * 2.0f, radius * 2.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(KNOB_TRACK.brighter(0.3f));
        g.strokePath(arcTrack, juce::PathStrokeType(3.0f));

        // Value arc (cyan fill up to current position)
        juce::Path valueArc;
        valueArc.addArc(centreX - radius, centreY - radius,
                        radius * 2.0f, radius * 2.0f,
                        rotaryStartAngle, angle, true);
        g.setColour(ACCENT_CYAN);
        g.strokePath(valueArc, juce::PathStrokeType(3.0f));

        // Pointer line
        const float pointerLength = radius * 0.55f;
        const float pointerX = centreX + std::sin(angle) * pointerLength;
        const float pointerY = centreY - std::cos(angle) * pointerLength;
        g.setColour(TEXT_WHITE);
        g.drawLine(centreX, centreY, pointerX, pointerY, 2.5f);

        // Centre dot
        g.setColour(ACCENT_CYAN);
        g.fillEllipse(centreX - 3.0f, centreY - 3.0f, 6.0f, 6.0f);
    }
};

// Single shared instance — must outlive the editor
static NoiseEngineLookAndFeel neLookAndFeel;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
NoiseEngineAudioEditor::NoiseEngineAudioEditor(NoiseEngineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&neLookAndFeel);

    setupKnob(bitDepthKnob,   bitDepthLabel,   "BIT DEPTH");
    setupKnob(downsampleKnob, downsampleLabel, "DOWNSAMPLE");
    setupKnob(mixKnob,        mixLabel,        "MIX");

    // Wire each knob to its APVTS parameter — keeps UI and DSP in sync automatically
    bitDepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "bitDepth",   bitDepthKnob);
    downsampleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "downsample", downsampleKnob);
    mixAttachment        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "mix",        mixKnob);

    setSize(480, 280);
}

NoiseEngineAudioEditor::~NoiseEngineAudioEditor()
{
    setLookAndFeel(nullptr); // must clear before destruction
}

// ---------------------------------------------------------------------------
// Helper: configure a rotary knob + label
// ---------------------------------------------------------------------------
void NoiseEngineAudioEditor::setupKnob(juce::Slider& knob,
                                        juce::Label&  label,
                                        const juce::String& labelText)
{
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    knob.setColour(juce::Slider::textBoxTextColourId,       TEXT_WHITE);
    knob.setColour(juce::Slider::textBoxBackgroundColourId, PANEL_DARK);
    knob.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    addAndMakeVisible(knob);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(11.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, ACCENT_CYAN);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------
void NoiseEngineAudioEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black); // bright red — impossible to miss
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------
void NoiseEngineAudioEditor::resized()
{
    const int knobSize = 110;
    const int labelH   = 18;
    const int topY     = 75;
    const int spacing  = getWidth() / 3;

    auto placeKnob = [&](juce::Slider& knob, juce::Label& label, int col)
    {
        const int cx = spacing * col + spacing / 2;
        knob.setBounds (cx - knobSize / 2, topY,               knobSize, knobSize);
        label.setBounds(cx - knobSize / 2, topY + knobSize + 4, knobSize, labelH);
    };

    placeKnob(bitDepthKnob,   bitDepthLabel,   0);
    placeKnob(downsampleKnob, downsampleLabel, 1);
    placeKnob(mixKnob,        mixLabel,        2);
}
