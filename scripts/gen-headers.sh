#!/bin/sh
# Generates the component position and asset headers from the panels and res/.
#
#   scripts/gen-headers.sh          write them to src/
#   scripts/gen-headers.sh <dir>    write them to <dir>, for make -C tests generated to compare
#                                   with the ones in src/
set -e

if [ -n "$1" ]; then
        mkdir -p "$1"
        out=$(cd "$1" && pwd)
fi
cd "$(dirname "$0")/.."
out=${out:-src}

python3 scripts/panel.py eugene res/eugene-dark-panel.svg "$out/EugeneComponentPositions.h"
python3 scripts/panel.py polygene res/polygene-dark-panel.svg "$out/PolygeneComponentPositions.h"
python3 scripts/assets.py --output "$out/OrbitsAssets.h" res
