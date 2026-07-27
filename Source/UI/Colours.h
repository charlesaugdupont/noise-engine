#pragma once
#include <JuceHeader.h>

// Shared dark/cyan palette, promoted from the constants that used to live
// at the top of PluginEditor.cpp. Named "Palette" (not "Colours") to avoid
// any ambiguity with juce::Colours.
namespace Palette
{
    inline const juce::Colour bgDark     { 0xFF1A1A1A };
    inline const juce::Colour panelDark  { 0xFF242424 };
    inline const juce::Colour accentCyan { 0xFF00E5CC };
    inline const juce::Colour textWhite  { 0xFFE0E0E0 };
    inline const juce::Colour knobTrack  { 0xFF3A3A3A };
}
