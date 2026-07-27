#include "NoiseEngineLookAndFeel.h"
#include "Colours.h"

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
