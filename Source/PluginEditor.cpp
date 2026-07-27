#include "PluginEditor.h"
#include "DSP/ParameterIDs.h"
#include "UI/Colours.h"
#include "UI/NoiseEngineLookAndFeel.h"

// Single shared instance — must outlive the editor
static NoiseEngineLookAndFeel neLookAndFeel;

namespace
{
    constexpr int margin              = 16;
    constexpr int comboW              = 100;
    constexpr int comboH              = 24;
    constexpr int knobSize            = 72;
    constexpr int labelH              = 14;
    constexpr int cellW               = 100;
    constexpr int rowGap              = 16;
    constexpr int colsPerRow          = 6;
    constexpr int presetBarHeight      = PresetBarComponent::preferredHeight;
    constexpr int stepSequencerHeight  = StepSequencerComponent::preferredHeight;
    constexpr int generatorPanelHeight = GeneratorPanelComponent::preferredHeight;
    constexpr int windowWidth          = margin * 2 + colsPerRow * cellW + 60; // extra room for the generator panel row
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
NoiseEngineAudioEditor::NoiseEngineAudioEditor(NoiseEngineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), presetBar(p), stepSequencer(p), generatorPanel(p)
{
    setLookAndFeel(&neLookAndFeel);

    addAndMakeVisible(presetBar);
    addAndMakeVisible(stepSequencer);
    addAndMakeVisible(generatorPanel);

    addChoice(ParamIDs::syncMode,     "SYNC");
    addChoice(ParamIDs::rate,         "RATE");
    addChoice(ParamIDs::rateModifier, "MODIFIER");
    addChoice(ParamIDs::stereoMode,   "STEREO");

    addKnob(ParamIDs::freeBpm,       "FREE BPM");
    addKnob(ParamIDs::swing,         "SWING");
    addKnob(ParamIDs::attack,        "ATTACK");
    addKnob(ParamIDs::hold,          "HOLD");
    addKnob(ParamIDs::release,       "RELEASE");
    addKnob(ParamIDs::stereoOffset,  "ST OFFSET");
    addKnob(ParamIDs::depth,         "DEPTH");
    addKnob(ParamIDs::mix,           "MIX");
    addKnob(ParamIDs::outputGain,    "OUTPUT");
    addKnob(ParamIDs::stepGlide,     "GLIDE");
    addKnob(ParamIDs::probability,   "PROBABILITY");

    // Bypass now lives as a power button in the wheel's centre; Prob On was
    // removed entirely since it was fully redundant with the Probability
    // knob (100% already means "no randomness", no separate flag needed).
    setSize(windowWidth, presetBarHeight + rowGap + stepSequencerHeight + rowGap + generatorPanelHeight + rowGap + 300);
}

NoiseEngineAudioEditor::~NoiseEngineAudioEditor()
{
    setLookAndFeel(nullptr);
}

// ---------------------------------------------------------------------------
// Control factories
// ---------------------------------------------------------------------------
void NoiseEngineAudioEditor::addKnob(const juce::String& paramID, const juce::String& labelText)
{
    auto control = std::make_unique<KnobControl>();

    control->slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    control->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
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
        audioProcessor.apvts, paramID, control->slider);

    knobs.push_back(std::move(control));
}

void NoiseEngineAudioEditor::addChoice(const juce::String& paramID, const juce::String& labelText)
{
    auto control = std::make_unique<ChoiceControl>();

    control->box.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(control->box);

    control->label.setText(labelText, juce::dontSendNotification);
    control->label.setFont(juce::Font(11.0f, juce::Font::bold));
    control->label.setColour(juce::Label::textColourId, Palette::accentCyan);
    control->label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(control->label);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter(paramID)))
        control->box.addItemList(param->choices, 1);

    control->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, paramID, control->box);

    choices.push_back(std::move(control));
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------
void NoiseEngineAudioEditor::paint(juce::Graphics& g)
{
    g.fillAll(Palette::bgDark);
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------
void NoiseEngineAudioEditor::resized()
{
    presetBar.setBounds(margin, margin, getWidth() - margin * 2, presetBarHeight);

    stepSequencer.setBounds(margin, margin + presetBarHeight + rowGap,
                             getWidth() - margin * 2, stepSequencerHeight);

    generatorPanel.setBounds(margin, margin + presetBarHeight + rowGap + stepSequencerHeight + rowGap,
                              getWidth() - margin * 2, generatorPanelHeight);

    int x = margin;
    int y = margin + presetBarHeight + rowGap + stepSequencerHeight + rowGap + generatorPanelHeight + rowGap;

    for (auto& c : choices)
    {
        c->label.setBounds(x, y, comboW, labelH);
        c->box.setBounds(x, y + labelH + 2, comboW - 8, comboH);
        x += cellW;
    }

    x = margin;
    y += labelH + 2 + comboH + rowGap;

    const int knobCellH = knobSize + 4 + labelH;
    int col = 0;

    for (auto& k : knobs)
    {
        if (col == colsPerRow)
        {
            col = 0;
            x = margin;
            y += knobCellH + rowGap;
        }

        k->slider.setBounds(x, y, knobSize, knobSize);
        k->label.setBounds(x, y + knobSize + 2, knobSize, labelH);

        x += cellW;
        ++col;
    }
}
