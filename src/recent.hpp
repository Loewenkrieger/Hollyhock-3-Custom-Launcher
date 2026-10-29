// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "loaders/interface.hpp"
#include <array>
#include <string>
#include <string_view>

namespace recent {
inline constexpr std::size_t capacity = 5;
struct History {
  std::array<std::string, capacity> paths;
  std::size_t size = 0;
};

History load() noexcept;
// Moves existing entries to the front, retaining the order of every other app.
// Missing paths are removed from the in-memory history without writing MCS.
std::size_t prioritize(std::forward_list<std::unique_ptr<Executable>> &apps,
                       History &history);
// Called only after load() succeeds, immediately before handing off to an app.
void remember(std::string_view path, History &history) noexcept;
} // namespace recent
