#include "PluginEditor.h"
#include "DSP/ParameterIDs.h"
#include "UI/Colours.h"
#include "UI/NoiseEngineLookAndFeel.h"

// Single shared instance — must outlive the editor
static NoiseEngineLookAndFeel neLookAndFeel;

namespace
{
    constexpr int margin              = 16;
    constexpr int rowGap              = 16;
    constexpr int sectionGap          = 12;
    constexpr int presetBarHeight     = PresetBarComponent::preferredHeight;
    constexpr int stepSequencerHeight = StepSequencerComponent::preferredHeight;
    constexpr int generatorPanelHeight = GeneratorPanelComponent::preferredHeight;
    constexpr int windowWidth         = 700;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
NoiseEngineAudioEditor::NoiseEngineAudioEditor(NoiseEngineAudioProcessor& p)
    : AudioProcessorEditor(&p),
      presetBar(p), stepSequencer(p), generatorPanel(p),
      timingSection(p, "TIMING"), shapeSection(p, "SHAPE"),
      stereoOutputSection(p, "STEREO & OUTPUT")
{
    setLookAndFeel(&neLookAndFeel);

    addAndMakeVisible(presetBar);
    addAndMakeVisible(stepSequencer);
    addAndMakeVisible(generatorPanel);

    timingSection.addChoice(ParamIDs::syncMode,     "SYNC");
    timingSection.addChoice(ParamIDs::rate,         "RATE");
    timingSection.addChoice(ParamIDs::rateModifier, "MODIFIER");
    timingSection.addKnob(ParamIDs::freeBpm,        "FREE BPM");
    timingSection.addKnob(ParamIDs::swing,          "SWING");
    addAndMakeVisible(timingSection);

    shapeSection.addKnob(ParamIDs::attack,      "ATTACK");
    shapeSection.addKnob(ParamIDs::hold,        "HOLD");
    shapeSection.addKnob(ParamIDs::release,     "RELEASE");
    shapeSection.addKnob(ParamIDs::stepGlide,   "GLIDE");
    shapeSection.addKnob(ParamIDs::probability, "PROB.");
    addAndMakeVisible(shapeSection);

    // One full-width row rather than Stereo/Output as separate half-width
    // panels — Stereo (Mode + Offset) alone looked sparse/empty at the same
    // width Output (3 knobs) needed, and both fit comfortably in a single
    // row at full panel width anyway.
    stereoOutputSection.addChoice(ParamIDs::stereoMode,   "MODE");
    stereoOutputSection.addKnob(ParamIDs::stereoOffset,   "OFFSET");
    stereoOutputSection.addKnob(ParamIDs::depth,          "DEPTH");
    stereoOutputSection.addKnob(ParamIDs::mix,            "MIX");
    stereoOutputSection.addKnob(ParamIDs::outputGain,     "OUTPUT");
    stereoOutputSection.setCompactSingleRow(true);
    addAndMakeVisible(stereoOutputSection);

    const int contentWidth = windowWidth - margin * 2;
    const int sectionWidth = (contentWidth - sectionGap) / 2;
    const int topRowHeight = juce::jmax(timingSection.computePreferredHeight(sectionWidth),
                                         shapeSection.computePreferredHeight(sectionWidth));
    const int bottomRowHeight = stereoOutputSection.computePreferredHeight(contentWidth);

    setSize(windowWidth,
             margin + presetBarHeight + rowGap
             + stepSequencerHeight + rowGap
             + generatorPanelHeight + rowGap
             + topRowHeight + sectionGap
             + bottomRowHeight + margin);
}

NoiseEngineAudioEditor::~NoiseEngineAudioEditor()
{
    setLookAndFeel(nullptr);
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
    const int contentWidth = getWidth() - margin * 2;

    presetBar.setBounds(margin, margin, contentWidth, presetBarHeight);

    int y = margin + presetBarHeight + rowGap;
    stepSequencer.setBounds(margin, y, contentWidth, stepSequencerHeight);

    y += stepSequencerHeight + rowGap;
    generatorPanel.setBounds(margin, y, contentWidth, generatorPanelHeight);

    y += generatorPanelHeight + rowGap;

    const int sectionWidth = (contentWidth - sectionGap) / 2;
    const int topRowHeight = juce::jmax(timingSection.computePreferredHeight(sectionWidth),
                                         shapeSection.computePreferredHeight(sectionWidth));

    timingSection.setBounds(margin, y, sectionWidth, topRowHeight);
    shapeSection.setBounds(margin + sectionWidth + sectionGap, y, sectionWidth, topRowHeight);

    y += topRowHeight + sectionGap;

    const int bottomRowHeight = stereoOutputSection.computePreferredHeight(contentWidth);
    stereoOutputSection.setBounds(margin, y, contentWidth, bottomRowHeight);
}
