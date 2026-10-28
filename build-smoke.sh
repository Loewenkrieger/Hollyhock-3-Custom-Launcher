#!/bin/sh
set -eu
cd "$(dirname "$0")"
: "${SDK_DIR:=/sdk}"
export SDK_DIR
make -C smoke/hh3 -j2 APP_NAME=dual-test-hh3
make -C hhk2-sdk -j2 AS=sh4a_nofpueb-elf-as CC=sh4a_nofpueb-elf-g++ LD=sh4a_nofpueb-elf-ld \
  'CC_FLAGS=-ffreestanding -fno-exceptions -fno-rtti -fshort-wchar -Wall -Wextra -O2 -I include/'
sh4a_nofpueb-elf-g++ -c smoke/hhk2/main.cpp -o smoke/hhk2/main.o \
  -ffreestanding -fno-exceptions -fno-rtti -fshort-wchar -Wall -Wextra -O2 -I hhk2-sdk/include
sh4a_nofpueb-elf-ld -T smoke/hhk2/linker.ld -o smoke/hhk2/dual-test-hhk2.hhk \
  -nostdlib --no-undefined smoke/hhk2/main.o hhk2-sdk/sdk.o
sh4a_nofpueb-elf-objcopy \
  --set-section-flags .hollyhock_name=contents,strings,readonly \
  --set-section-flags .hollyhock_description=contents,strings,readonly \
  --set-section-flags .hollyhock_author=contents,strings,readonly \
  --set-section-flags .hollyhock_version=contents,strings,readonly smoke/hhk2/dual-test-hhk2.hhk
