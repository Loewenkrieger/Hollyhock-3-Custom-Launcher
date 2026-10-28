#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace legacy {
struct AddressMap { std::uint32_t old_address, new_address; };
struct Section {
  std::uint32_t address, size, flags;
  std::vector<std::uint8_t> data;
};
struct Image {
  std::uint32_t entry = 0;
  std::size_t patched_words = 0;
  std::string name, author, version, description;
  std::vector<Section> sections;
};
// Parses and prepares a complete image before any calculator memory is written.
// This is deliberately independent of the ClassPad SDK for host regression tests.
Image parse(std::span<const std::uint8_t> file, std::span<const AddressMap> map);
}
