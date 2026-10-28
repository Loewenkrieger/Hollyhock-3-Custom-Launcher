#!/bin/sh
set -eu
cd "$(dirname "$0")"
: "${SDK_DIR:=/sdk}"
export SDK_DIR
python3 generate_legacy_map.py hhk2-sdk meta/addresses.7002.json src/legacy/sdk_maps.hpp
make -j2 dist/run.7002.bin
