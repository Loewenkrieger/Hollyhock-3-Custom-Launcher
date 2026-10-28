// SPDX-License-Identifier: GPL-3.0-or-later
#include "exit_protocol.hpp"

extern "C" __attribute__((noinline)) int hhk_completed_dialog(void *) noexcept {
  // No allocation, rendering, timers or runtime services after finalization.
  return os_return::cancelled;
}

namespace {
// Static trivial objects survive __libc_fini_array and remain valid until
// the OS has consumed the result. The OS does not own or delete them.
constinit const os_return::ReturnTable table{{}, 0, 0, hhk_completed_dialog};
constinit const os_return::ReturnObject object{{}, &table};
}

namespace os_return {
bool matches_caller(const volatile std::uint8_t *bytes,
                    std::uint16_t result_constant) noexcept {
  if (result_constant != accepted) return false;
  for (std::size_t i = 0; i < sizeof(caller_signature); ++i)
    if (bytes[i] != caller_signature[i]) return false;
  return true;
}
void *completed_dialog() noexcept {
  return const_cast<ReturnObject *>(&object);
}
}
