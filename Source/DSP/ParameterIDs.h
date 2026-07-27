#pragma once
#include <JuceHeader.h>

// Parameter ID strings, shared between the processor (layout + reads)
// and the editor (attachments).
namespace ParamIDs
{
    inline const juce::String rate                   { "rate" };
    inline const juce::String rateModifier            { "rateModifier" };
    inline const juce::String syncMode                { "syncMode" };
    inline const juce::String freeBpm                 { "freeBpm" };
    inline const juce::String patternLength           { "patternLength" };
    inline const juce::String attack                  { "attack" };
    inline const juce::String hold                    { "hold" };
    inline const juce::String release                 { "release" };
    inline const juce::String swing                   { "swing" };
    inline const juce::String stereoMode              { "stereoMode" };
    inline const juce::String stereoOffset            { "stereoOffset" };
    inline const juce::String depth                   { "depth" };
    inline const juce::String mix                     { "mix" };
    inline const juce::String probability             { "probability" };
    inline const juce::String outputGain              { "outputGain" };
    inline const juce::String stepGlide               { "stepGlide" };
    inline const juce::String bypass                  { "bypass" };
}

namespace ParamChoices
{
    inline const juce::StringArray rate       { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64" };
    inline const juce::StringArray modifier   { "Straight", "Dotted", "Triplet" };
    inline const juce::StringArray syncMode   { "DAW Sync", "Free" };
    inline const juce::StringArray stereoMode { "Mono", "L-R Offset", "Mid-Side" };
}
