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

    timingSection.addChoice(ParamIDs::syncMode,     "SYNC",     "DAW: locked to host tempo. Free: runs at its own tempo, set by Free BPM, ignoring the host.");
    timingSection.addChoice(ParamIDs::rate,         "RATE",     "Step duration, as a note division of the tempo.");
    timingSection.addChoice(ParamIDs::rateModifier, "MODIFIER", "Straight, dotted, or triplet feel applied to the rate.");
    timingSection.addKnob(ParamIDs::freeBpm,        "FREE BPM", "Tempo used when Sync is set to Free.");
    timingSection.addKnob(ParamIDs::swing,          "SWING",    "Delays every other step for a shuffled feel.");
    addAndMakeVisible(timingSection);

    shapeSection.addKnob(ParamIDs::attack,      "ATTACK",  "How quickly each step fades in.");
    shapeSection.addKnob(ParamIDs::hold,        "HOLD",    "How long each step stays at full level before releasing.");
    shapeSection.addKnob(ParamIDs::release,     "RELEASE", "How quickly each step fades out.");
    shapeSection.addKnob(ParamIDs::stepGlide,   "GLIDE",   "Blends the gate into a smooth slide between step levels instead of a hard on/off.");
    shapeSection.addKnob(ParamIDs::probability, "PROB.",   "Chance that an enabled step actually fires each time the pattern loops.");
    addAndMakeVisible(shapeSection);

    // One full-width row rather than Stereo/Output as separate half-width
    // panels — Stereo (Mode + Offset) alone looked sparse/empty at the same
    // width Output (3 knobs) needed, and both fit comfortably in a single
    // row at full panel width anyway.
    stereoOutputSection.addChoice(ParamIDs::stereoMode,   "MODE",   "Mono: identical both channels. L-R Offset: shifts the right channel's timing. Mid-Side: shifts the stereo width instead of left/right.");
    stereoOutputSection.addKnob(ParamIDs::stereoOffset,   "OFFSET", "How far the second channel's timing shifts, as a fraction of one step. Only audible when Mode isn't Mono.");
    stereoOutputSection.addKnob(ParamIDs::depth,          "DEPTH",  "How strongly the gate affects the signal — 0% leaves it untouched, 100% is a full gate.");
    stereoOutputSection.addKnob(ParamIDs::mix,            "MIX",    "Blends between the dry and gated signal.");
    stereoOutputSection.addKnob(ParamIDs::outputGain,     "OUTPUT", "Output level trim.");
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
