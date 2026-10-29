#!/bin/sh
set -eu
cd "$(dirname "$0")"
: "${SDK_DIR:=/sdk}"
export SDK_DIR
make -C smoke/hh3 -j2 APP_NAME=launcher-test
