// SPDX-License-Identifier: GPL-3.0-or-later
#include "exit_protocol.hpp"
#include "strconv.hpp"
#include <cstring>
#include <sdk/os/gui.h>

namespace os_return {
void *finish(int exit_status, const char *os_guard) noexcept {
  // This firmware hook is a dialog factory, not a void callback. Its caller
  // immediately dereferences the result, so nullptr resets the calculator.
  if (exit_status == 0 && std::strcmp(os_guard, "2023.0209.1500") == 0 &&
      matches_caller(reinterpret_cast<const volatile std::uint8_t *>(caller_address),
                     *reinterpret_cast<const volatile std::uint16_t *>(result_constant_address)))
    return completed_dialog();

  // Other OS/hook revisions retain the original native-dialog return path.
  if (exit_status == 0)
    return GUI_DisplayMessageBox_Internal(0, "Hollyhock", "", "Launcher beendet.", BUTTON_OK, false);

  static char code[sizeof(unsigned long) * 2 + 1];
  ultohexstr(static_cast<unsigned long>(exit_status), code);
  code[sizeof(code) - 1] = '\0';
  return GUI_DisplayMessageBox_Internal(0, "Hollyhock", "Fehlercode: ", code, BUTTON_OK, false);
}
}
