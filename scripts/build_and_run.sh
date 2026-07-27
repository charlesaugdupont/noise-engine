#!/usr/bin/env bash
# Builds the plugin and relaunches the Standalone app. The Standalone app has
# no host-side plugin caching (unlike a DAW's VST3 hosting), so this is the
# fastest reliable way to iterate on DSP/UI changes without fighting a DAW's
# plugin cache. VST3/AU builds land alongside it and are symlinked into the
# system VST3 folders for Ableton/etc. to pick up separately.
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Building (Release)..."
cmake --build build --config Release --parallel

APP="build/NoiseEngine_artefacts/Release/Standalone/Noise Engine.app"

if pgrep -f "Noise Engine" > /dev/null 2>&1; then
    echo "==> Quitting running Standalone instance..."
    osascript -e 'tell application "Noise Engine" to quit' > /dev/null 2>&1 || pkill -f "Noise Engine" || true
    sleep 1
fi

echo "==> Launching Standalone..."
open "$APP"
