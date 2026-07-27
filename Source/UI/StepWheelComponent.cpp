#include "StepWheelComponent.h"
#include "Colours.h"

namespace
{
    constexpr float dragThreshold = 4.0f;
    constexpr float gapFraction   = 0.10f; // fraction of each wedge's angle left as a visual gap
    constexpr float maxGapAngle   = juce::degreesToRadians(5.0f); // caps the gap in absolute terms —
                                                                    // a pure fraction blows up at low step
                                                                    // counts (e.g. 18° at length=2)
}

// ---------------------------------------------------------------------------
// Construction / layout
// ---------------------------------------------------------------------------
StepWheelComponent::StepWheelComponent(NoiseEngineAudioProcessor& processorToUse)
    : processor(processorToUse), pattern(processor.getPatternSnapshot())
{
}

void StepWheelComponent::resized()
{
    const auto bounds = getLocalBounds().toFloat();
    center = bounds.getCentre();

    const float maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 4.0f;

    // The main ring no longer reaches all the way to the edge — that leaves
    // a dedicated outer band for the playhead indicator, which used to be a
    // translucent tint drawn behind the wedges and got lost against
    // similarly coloured fills, especially at fast rates.
    ringInnerR     = maxRadius * 0.30f;
    ringOuterR     = maxRadius * 0.84f;
    playheadInnerR = ringOuterR + 4.0f;
    playheadOuterR = maxRadius;
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------
void StepWheelComponent::paint(juce::Graphics& g)
{
    g.setColour(Palette::panelDark);
    g.fillEllipse(center.x - ringOuterR, center.y - ringOuterR, ringOuterR * 2.0f, ringOuterR * 2.0f);

    if (pattern.length > 0)
    {
        const float anglePerStep = juce::MathConstants<float>::twoPi / (float) pattern.length;
        const float gapAngle     = juce::jmin(anglePerStep * gapFraction, maxGapAngle);

        for (int i = 0; i < pattern.length; ++i)
        {
            const float wedgeStart = anglePerStep * (float) i;
            const float wedgeEnd   = anglePerStep * (float) (i + 1);
            const float trimStart  = wedgeStart + gapAngle * 0.5f;
            const float trimEnd    = wedgeEnd   - gapAngle * 0.5f;

            const auto& step = pattern.steps[(size_t) i];

            // Background track
            juce::Path track;
            track.addPieSegment(center.x - ringOuterR, center.y - ringOuterR,
                                 ringOuterR * 2.0f, ringOuterR * 2.0f,
                                 trimStart, trimEnd, ringInnerR / ringOuterR);
            g.setColour(Palette::knobTrack);
            g.fillPath(track);

            // Filled portion, growing outward from ringInnerR with level.
            const float filledOuterR = ringInnerR + (ringOuterR - ringInnerR) * juce::jlimit(0.0f, 1.0f, step.level);
            if (filledOuterR > ringInnerR + 0.5f)
            {
                juce::Path fill;
                fill.addPieSegment(center.x - filledOuterR, center.y - filledOuterR,
                                    filledOuterR * 2.0f, filledOuterR * 2.0f,
                                    trimStart, trimEnd, ringInnerR / filledOuterR);
                g.setColour(step.enabled ? Palette::accentCyan : Palette::knobTrack.brighter(0.2f));
                g.fillPath(fill);
            }
        }

        // Playhead: a solid, fully-opaque wedge in its own band just outside
        // the ring, rather than a translucent overlay competing with wedge
        // fill colours for attention.
        const int uiCurrentStep = processor.getCurrentStepForUI();
        if (uiCurrentStep >= 0 && uiCurrentStep < pattern.length)
        {
            const float wedgeStart = anglePerStep * (float) uiCurrentStep;
            const float wedgeEnd   = anglePerStep * (float) (uiCurrentStep + 1);

            juce::Path playhead;
            playhead.addPieSegment(center.x - playheadOuterR, center.y - playheadOuterR,
                                    playheadOuterR * 2.0f, playheadOuterR * 2.0f,
                                    wedgeStart, wedgeEnd, playheadInnerR / playheadOuterR);
            g.setColour(Palette::accentCyan);
            g.fillPath(playhead);
        }
    }

    // Centre power button (Bypass) — the ring's hollow middle was otherwise
    // empty space, and Bypass used to be a plain toggle lost in the flat
    // macro-knob grid.
    {
        const bool  bypassed  = processor.isBypassed();
        const auto  colour    = bypassed ? Palette::knobTrack.brighter(0.3f) : Palette::accentCyan;
        const float btnRadius = ringInnerR * 0.65f;

        g.setColour(Palette::bgDark);
        g.fillEllipse(center.x - btnRadius, center.y - btnRadius, btnRadius * 2.0f, btnRadius * 2.0f);

        const float iconRadius = btnRadius * 0.5f;
        juce::Path powerPath;
        powerPath.addCentredArc(center.x, center.y, iconRadius, iconRadius,
                                 0.0f,
                                 juce::MathConstants<float>::pi * 0.25f,
                                 juce::MathConstants<float>::pi * 1.75f,
                                 true);
        g.setColour(colour);
        g.strokePath(powerPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.drawLine(center.x, center.y - iconRadius * 1.3f, center.x, center.y - iconRadius * 0.15f,
                   2.5f);
    }
}

// ---------------------------------------------------------------------------
// Hit-testing — angle 0 = 12 o'clock, clockwise, matching addPieSegment's
// convention (Point::getPointOnCircumference: x = cx + r*sin(a), y = cy - r*cos(a),
// inverted here as angle = atan2(dx, -dy)).
// ---------------------------------------------------------------------------
StepWheelComponent::HitResult StepWheelComponent::resolveHit(juce::Point<float> pos) const
{
    HitResult result;

    if (pattern.length <= 0)
        return result;

    const float dx     = pos.x - center.x;
    const float dy     = pos.y - center.y;
    const float radius = std::sqrt(dx * dx + dy * dy);

    float angle = std::atan2(dx, -dy);
    if (angle < 0.0f)
        angle += juce::MathConstants<float>::twoPi;

    const float anglePerStep = juce::MathConstants<float>::twoPi / (float) pattern.length;
    result.stepIndex = juce::jlimit(0, pattern.length - 1, (int) (angle / anglePerStep));

    if (radius >= ringInnerR && radius <= ringOuterR)
    {
        result.inRing         = true;
        result.radialFraction = juce::jlimit(0.0f, 1.0f, (radius - ringInnerR) / (ringOuterR - ringInnerR));
    }
    else if (radius < ringInnerR)
    {
        result.inCenter = true;
    }

    return result;
}

void StepWheelComponent::mouseDown(const juce::MouseEvent& e)
{
    dragStartPos  = e.position;
    isDragging    = false;

    const auto hit = resolveHit(e.position);
    dragStepIndex       = hit.stepIndex;
    dragStartedInCenter = hit.inCenter;
}

void StepWheelComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (! isDragging && e.position.getDistanceFrom(dragStartPos) > dragThreshold)
        isDragging = true;

    if (isDragging)
        applyEdit(resolveHit(e.position));
}

void StepWheelComponent::mouseUp(const juce::MouseEvent&)
{
    // A double-click delivers its own mouseUp before mouseDoubleClick fires,
    // which would otherwise toggle `enabled` right back off immediately
    // after the reset.
    if (! isDragging && ! suppressNextToggle)
    {
        if (dragStartedInCenter)
            processor.setBypassed(! processor.isBypassed());
        else if (dragStepIndex >= 0)
            toggleStep(dragStepIndex);
    }

    suppressNextToggle  = false;
    isDragging          = false;
    dragStepIndex        = -1;
    dragStartedInCenter  = false;
}

juce::String StepWheelComponent::getTooltip()
{
    const auto hit = resolveHit(getMouseXYRelative().toFloat());

    if (hit.inCenter)
        return "Bypass the gate.";

    if (hit.inRing)
        return "Click to toggle a step. Drag up/down to set its level. Double-click to reset to full.";

    return {};
}

void StepWheelComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    const auto hit = resolveHit(e.position);
    if (hit.stepIndex < 0 || ! hit.inRing)
        return;

    resetStepToFull(hit.stepIndex);
    suppressNextToggle = true;
}

void StepWheelComponent::applyEdit(const HitResult& hit)
{
    if (hit.stepIndex < 0 || ! hit.inRing)
        return;

    auto& step   = pattern.steps[(size_t) hit.stepIndex];
    step.level   = hit.radialFraction;
    step.enabled = true; // dragging a level implicitly turns the step on

    pushPatternToProcessor();
    repaint();
}

void StepWheelComponent::toggleStep(int stepIndex)
{
    if (stepIndex < 0 || stepIndex >= pattern.length)
        return;

    auto& step = pattern.steps[(size_t) stepIndex];

    if (! step.enabled)
    {
        // Turning a step on: restore its existing level if it has one worth
        // restoring, otherwise default to full. A bare toggle that leaves
        // level at 0 is a dead click — silent, and the wedge doesn't even
        // render a fill at level 0, so it looks like nothing happened.
        if (step.level <= 0.001f)
            step.level = 1.0f;
        step.enabled = true;
    }
    else
    {
        step.enabled = false;
    }

    pushPatternToProcessor();
    repaint();
}

void StepWheelComponent::resetStepToFull(int stepIndex)
{
    if (stepIndex < 0 || stepIndex >= pattern.length)
        return;

    auto& step   = pattern.steps[(size_t) stepIndex];
    step.level   = 1.0f;
    step.enabled = true;

    pushPatternToProcessor();
    repaint();
}

void StepWheelComponent::pushPatternToProcessor()
{
    pattern.length = processor.getPatternLength();
    processor.setPatternSnapshot(pattern);
}

// ---------------------------------------------------------------------------
// Full pattern sync — the wheel's pixel size never changes with Length,
// only the wedge angles do, so this is just a data refresh, no layout work.
// ---------------------------------------------------------------------------
void StepWheelComponent::syncFromProcessor()
{
    pattern = processor.getPatternSnapshot();
    pattern.length = processor.getPatternLength();
}
