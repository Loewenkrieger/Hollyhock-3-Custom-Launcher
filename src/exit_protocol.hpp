// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>

namespace os_return {
// OS 02.01.7002.0001, System-menu handler:
//   0x8015c898: result->vtable_at_4c->method_at_18c(result + adjustment)
//   0x8015c8ac: result == 0x03ea commits the Imaginary Unit setting;
//               any other result resumes at 0x8015cf94 without that change.
// The handler neither retains nor frees this object. No OS widget is opened.
inline constexpr std::uintptr_t caller_address = 0x8015c898;
inline constexpr std::uintptr_t result_constant_address = 0x8015c8c4;
inline constexpr std::uint16_t accepted = 0x03ea, cancelled = 0x03eb;
inline constexpr std::uint8_t caller_signature[] = {
  0x62,0x03,0xe0,0x4c,0x0b,0x2e,0xea,0x63,0x4a,0x08,0x3b,0xac,
  0x64,0xb2,0x55,0xb2,0x45,0x0b,0x34,0x2c,0x96,0x0a,0x62,0x0f,
  0x32,0x60,0x89,0x01,0xa3,0x6e,0x00,0x09
};

bool matches_caller(const volatile std::uint8_t *bytes,
                    std::uint16_t result_constant) noexcept;
void *completed_dialog() noexcept;
void *finish(int exit_status, const char *os_guard) noexcept;

// These are deliberately narrow return-protocol structures, not general
// Casio widget wrappers. Packing at 4 keeps field offsets identical in the
// SH build and host ABI tests even when host pointers are 64 bit wide.
#pragma pack(push, 4)
struct ReturnTable {
  std::uint8_t unused[0x18c];
  std::int32_t self_adjustment;
  std::uint32_t reserved;
  int (*finish)(void *);
};
struct ReturnObject {
  std::uint8_t unused[0x4c];
  const ReturnTable *table;
};
#pragma pack(pop)
static_assert(offsetof(ReturnObject, table) == 0x4c);
static_assert(offsetof(ReturnTable, self_adjustment) == 0x18c);
static_assert(offsetof(ReturnTable, finish) == 0x194);
} // namespace os_return

extern "C" int hhk_completed_dialog(void *self) noexcept;
