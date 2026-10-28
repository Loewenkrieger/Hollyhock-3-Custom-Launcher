// SPDX-License-Identifier: GPL-3.0-or-later
#include "exit_protocol.hpp"
#include <sdk/os/gui.h>
#include <sys/mman.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

namespace {
int native_calls = 0;
char native_object;
std::string last_prefix, last_content;
void *native_message(int unknown, const char *, const char *prefix, const char *content,
                     int buttons, bool disable_close) {
  assert(unknown == 0 && buttons == BUTTON_OK && !disable_close);
  ++native_calls; last_prefix = prefix; last_content = content;
  return &native_object;
}
int consume_as_os(void *result) {
  assert(result);
  // Exactly the ABI fields read by the actual System-menu caller.
  const auto *object = static_cast<const os_return::ReturnObject *>(result);
  const auto *table = object->table;
  assert(table && table->self_adjustment == 0 && table->reserved == 0 && table->finish);
  return table->finish(static_cast<char *>(result) + table->self_adjustment);
}
}

extern "C" {
void *(*GUI_DisplayMessageBox_Internal)(int, const char *, const char *, const char *, int, bool) = native_message;
}

int main() {
  // Independently captured from the SHA-pinned original OS image. These are
  // instructions in its consumer, not a copy of our ReturnObject definition.
  constexpr unsigned char caller[] = {
    0x62,0x03,0xe0,0x4c,0x0b,0x2e,0xea,0x63,0x4a,0x08,0x3b,0xac,
    0x64,0xb2,0x55,0xb2,0x45,0x0b,0x34,0x2c,0x96,0x0a,0x62,0x0f,
    0x32,0x60,0x89,0x01,0xa3,0x6e,0x00,0x09
  };
  constexpr std::uintptr_t page_address = 0x8015c000;
  void *page = mmap(reinterpret_cast<void *>(page_address), 4096, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  assert(page != MAP_FAILED);
  auto *rom = reinterpret_cast<unsigned char *>(os_return::caller_address);
  auto *constant = reinterpret_cast<std::uint16_t *>(os_return::result_constant_address);
  std::memcpy(rom, caller, sizeof(caller));
  *constant = 0x03ea; // Host-endian storage for the real function's word load.

  auto *result = os_return::finish(0, "2023.0209.1500");
  assert(native_calls == 0 && consume_as_os(result) == 0x03eb);
  std::cout << "PASS matched OS 7002 returns a non-null result consumed as Cancel, without a dialog\n";

  for (std::size_t i = 0; i < sizeof(caller); ++i) {
    rom[i] ^= 1;
    assert(os_return::finish(0, "2023.0209.1500") == &native_object);
    rom[i] ^= 1;
  }
  assert(native_calls == static_cast<int>(sizeof(caller)));
  std::cout << "PASS each of 32 mismatching caller bytes falls back to the native completion path\n";

  *constant = 0;
  assert(os_return::finish(0, "2023.0209.1500") == &native_object);
  *constant = 0x03ea;
  std::cout << "PASS unexpected settings-result constant disables the silent return\n";

  const auto calls = native_calls;
  for (int i = 0; i < 1000; ++i) {
    auto *again = os_return::finish(0, "2023.0209.1500");
    assert(again == result && consume_as_os(again) == 0x03eb);
  }
  assert(native_calls == calls);
  std::cout << "PASS repeated completions reuse a static result without opening native windows\n";

  assert(munmap(page, 4096) == 0);
  assert(os_return::finish(0, "2017.0512.1515") == &native_object);
  assert(last_content == "Launcher beendet.");
  std::cout << "PASS other OS versions use the native path without reading the OS 7002 address\n";

  assert(os_return::finish(0x123, "2023.0209.1500") == &native_object);
  assert(last_prefix == "Fehlercode: " && last_content.ends_with("123"));
  std::cout << "PASS nonzero exits retain an explicit error dialog and avoid the silent return\n";
  std::cout << "6/6 return-protocol checks passed\n";
}
