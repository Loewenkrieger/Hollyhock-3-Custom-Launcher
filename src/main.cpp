// for setenv from <stdlib.h>
#include <functional>
#define _POSIX_C_SOURCE 200112L
#include "addresses.h"
#include "app_list.hpp"
#include "gui.hpp"
#include "impl.hpp"
#include "strconv.hpp"
#include <alloca.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sdk/calc/calc.h>
#include <sdk/os/gui.h>
#include <stdexcept>
#include <string>

using namespace std::string_literals;

void do_override() {
  const size_t guard_len = static_cast<std::decay_t<decltype(addresses)>>(
                               std::memchr(addresses, 0, addresses_size)) -
                           addresses;

  const auto path = "\\fls0\\addresses."s +
                    reinterpret_cast<const char *>(addresses) + ".override";

  const auto file = std::unique_ptr<std::FILE, void (*)(std::FILE *)>(
      std::fopen(path.c_str(), "rb"), [](std::FILE *const file) {
        if (!file)
          return;
        if (std::fclose(file) != 0)
          throw std::runtime_error("Failed to close override");
      });
  if (!file)
    return;

  if (std::fseek(file.get(), 0, SEEK_END) != 0)
    throw std::runtime_error("Failed to seek to end of override");
  const auto file_size = std::ftell(file.get());
  if (std::fseek(file.get(), 0, SEEK_SET) != 0)
    throw std::runtime_error("Failed to seek to start of override");

  const auto guard = static_cast<char *>(alloca(guard_len + 1));
  if (std::fread(guard, sizeof(char) * (guard_len + 1), 1, file.get()) != 1)
    throw std::runtime_error("Failed to read guard");
  if (!check_safe_guard(guard))
    throw std::runtime_error("Invalid guard");

  const auto size = file_size - (guard_len + 1) + addresses_size;
  const auto buf = new uint8_t[size];
  if (!buf)
    throw std::runtime_error("Failed to allocate override");
  std::memcpy(buf, addresses, addresses_size);

  if (std::fread(buf + addresses_size, size - addresses_size, 1, file.get()) !=
      1)
    throw std::runtime_error("Failed to read override");

  if (!relink_sdk(buf, size))
    throw std::runtime_error("Failed to relink sdk");

  if (!relink_libc(buf, size))
    throw std::runtime_error("Failed to relink libc");

  {
    char cbuf[sizeof(std::uintptr_t) * 2 + 1];
    ultohexstr(reinterpret_cast<std::uintptr_t>(buf), cbuf);
    cbuf[sizeof(cbuf) - 1] = '\0';
    setenv("HHK_SYMBOL_TABLE", cbuf, 1);
  }
  {
    char cbuf[sizeof(std::size_t) * 2 + 1];
    ultohexstr(size, cbuf);
    cbuf[sizeof(cbuf) - 1] = '\0';
    setenv("HHK_SYMBOL_TABLE_LEN", cbuf, 1);
  }
}

int main() {
  do_override();

  std::unique_ptr<Executable> chosen;
  {
    auto list = discover_apps();
    chosen = do_gui(list);
  }

  if (!chosen)
    return 0;

  calcExit();
  try {
    chosen->load();
    const auto ret = chosen->execute();
    chosen->unload();
    calcInit();
    return ret;
  } catch (const std::exception &error) {
    chosen->unload();
    calcInit();
    GUI_DisplayMessageBox_Internal(0, "App could not start", "", error.what(), BUTTON_OK, false);
    return 1;
  }
}
