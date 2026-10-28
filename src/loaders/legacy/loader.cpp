#include "loader.hpp"
#include "addresses.h"
#include "legacy/sdk_maps.hpp"
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
std::unique_ptr<char[]> copy(const std::string &text) {
  auto out = std::make_unique<char[]>(text.size() + 1);
  std::memcpy(out.get(), text.c_str(), text.size() + 1);
  return out;
}
}
legacy::Image LegacyLoader::read_image() {
  auto opened = file.acquire();
  auto f = opened.get();
  if (std::fseek(f, 0, SEEK_END) != 0) throw std::runtime_error("HHK2: seek failed");
  const auto size = std::ftell(f);
  if (size < 52 || size > 2 * 1024 * 1024) throw std::runtime_error("HHK2: invalid file size");
  std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
  if (std::fseek(f, 0, SEEK_SET) != 0 || std::fread(bytes.data(), 1, bytes.size(), f) != bytes.size())
    throw std::runtime_error("HHK2: file read failed");
  const char *guard = reinterpret_cast<const char *>(addresses);
  if (std::strcmp(guard, "2023.0209.1500") == 0) return legacy::parse(bytes, legacy::map_7002);
  if (std::strcmp(guard, "2017.0512.1515") == 0) return legacy::parse(bytes, legacy::map_2000);
  throw std::runtime_error("HHK2: unsupported target OS");
}
void LegacyLoader::inspect() {
  if (inspected) return;
  inspected = true;
  try {
    auto candidate = read_image();
    name = candidate.name.empty() ? path.get() : candidate.name;
    author = candidate.author;
    version = candidate.version;
    description = candidate.description;
  } catch (const std::exception &e) {
    name = path.get();
    description = std::string("Cannot start this HHK2 app:\n") + e.what();
  }
}
std::unique_ptr<char[]> LegacyLoader::getName() { inspect(); return copy(name); }
std::unique_ptr<char[]> LegacyLoader::getAuthor() { inspect(); return copy(author); }
std::unique_ptr<char[]> LegacyLoader::getVersion() { inspect(); return copy(version); }
std::unique_ptr<char[]> LegacyLoader::getDescription() { inspect(); return copy(description); }
void LegacyLoader::load() {
  image = read_image();
  // The parser has validated every section, symbol and write range first.
  for (const auto &s : image->sections) {
    auto out = reinterpret_cast<std::uint8_t *>(s.address);
    if (!s.data.empty()) std::memcpy(out, s.data.data(), s.data.size());
    if (s.data.size() < s.size) std::memset(out + s.data.size(), 0, s.size - s.data.size());
    const auto end = (s.address + s.size + 31) & ~std::uint32_t(31);
    for (auto p = s.address & ~std::uint32_t(31); p < end; p += 32) {
      __asm__ volatile("ocbwb @%0" : : "r"(p) : "memory");
      if (s.flags & 4) __asm__ volatile("icbi @%0" : : "r"(p) : "memory");
    }
  }
}
int LegacyLoader::execute() {
  if (!image) throw std::runtime_error("HHK2: app not loaded");
  reinterpret_cast<void (*)()>(image->entry)();
  return 0;
}
