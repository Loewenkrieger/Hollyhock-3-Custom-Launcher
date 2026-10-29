// SPDX-License-Identifier: GPL-3.0-or-later
#include "app_list.hpp"
#include "loaders/elf/loader.hpp"
#include <cstring>

std::forward_list<std::unique_ptr<Executable>> discover_apps() {
  std::forward_list<std::unique_ptr<Executable>> list;
  discover<ELFLoader>::run(list);
  list.sort([](const auto &a, const auto &b) {
    return a->pathView() < b->pathView();
  });
  return list;
}
