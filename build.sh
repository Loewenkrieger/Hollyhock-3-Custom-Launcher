#!/bin/sh
set -eu
cd "$(dirname "$0")"
: "${SDK_DIR:=/sdk}"
export SDK_DIR
make -j2 dist/run.7002.bin
