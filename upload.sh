#!/usr/bin/env bash

set -e
set -u

# random ssid:
ssid="$(shuf -n 2 <(echo -e "elephant\ngumdrop\neat\nstinkbug\nfunky\ndonuts\nwise\nfickle\ncandycorn\nunicorn\nfairy\ndust") | tr '\n' '-' | sed 's/-$//')"
ssid="${1:-$ssid}"

echo "Compiling with SSID=$ssid"

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property "build.extra_flags=-DSSID_NAME=\"${ssid}\""
arduino-cli upload --fqbn esp32:esp32:esp32 -p /dev/ttyUSB0
