#include "image.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <set>
#include <stdexcept>

namespace legacy {
namespace {
constexpr std::uint32_t ram_begin = 0x8cff0000, ram_end = 0x8d000000;
struct Header {
  std::uint32_t name, type, flags, address, offset, size, link, entry_size;
};
void require(bool ok, const char *message) {
  if (!ok) throw std::runtime_error(message);
}
void bounds(std::span<const std::uint8_t> b, std::size_t off, std::size_t size) {
  require(off <= b.size() && size <= b.size() - off, "HHK2: truncated ELF data");
}
std::uint16_t u16(std::span<const std::uint8_t> b, std::size_t o) {
  bounds(b, o, 2);
  return (std::uint16_t(b[o]) << 8) | b[o + 1];
}
std::uint32_t u32(std::span<const std::uint8_t> b, std::size_t o) {
  bounds(b, o, 4);
  return (std::uint32_t(b[o]) << 24) | (std::uint32_t(b[o + 1]) << 16) |
         (std::uint32_t(b[o + 2]) << 8) | b[o + 3];
}
void put32(std::vector<std::uint8_t> &b, std::size_t o, std::uint32_t v) {
  for (unsigned i = 0; i < 4; ++i) b[o + i] = (v >> (24 - i * 8)) & 255;
}
std::string text(std::span<const std::uint8_t> b, std::size_t offset) {
  bounds(b, offset, 1);
  auto end = offset;
  while (end < b.size() && b[end] && end - offset < 4096) ++end;
  require(end < b.size() && b[end] == 0, "HHK2: unterminated string");
  return std::string(reinterpret_cast<const char *>(b.data() + offset), end - offset);
}
bool translate(std::span<const AddressMap> map, std::uint32_t v, std::uint32_t &out) {
  for (auto &entry : map) if (entry.old_address == v) {
    out = entry.new_address;
    return true;
  }
  return false;
}
}

Image parse(std::span<const std::uint8_t> b, std::span<const AddressMap> map) {
  require(b.size() >= 52 && b.size() <= 2 * 1024 * 1024, "HHK2: invalid ELF size");
  constexpr std::array<std::uint8_t, 7> magic = {127, 'E', 'L', 'F', 1, 2, 1};
  require(std::equal(magic.begin(), magic.end(), b.begin()), "HHK2: expected ELF32 big endian");
  require(u16(b, 16) == 2 && u16(b, 18) == 42 && u32(b, 20) == 1,
          "HHK2: expected SuperH ET_EXEC");
  require(u16(b, 40) == 52 && u16(b, 46) == 40, "HHK2: invalid ELF header sizes");
  const auto count = u16(b, 48), names = u16(b, 50);
  require(count > 0 && count <= 1024 && names < count, "HHK2: invalid section count");
  const auto table = u32(b, 32);
  bounds(b, table, std::size_t(count) * 40);
  std::vector<Header> headers;
  for (std::size_t i = 0; i < count; ++i) {
    auto o = table + i * 40;
    Header h{u32(b,o),u32(b,o+4),u32(b,o+8),u32(b,o+12),u32(b,o+16),
             u32(b,o+20),u32(b,o+24),u32(b,o+36)};
    if (h.type != 8) bounds(b, h.offset, h.size);
    headers.push_back(h);
  }
  const auto &name_header = headers[names];
  require(name_header.type == 3, "HHK2: missing section-name table");
  const auto name_data = b.subspan(name_header.offset, name_header.size);
  Image image;
  image.entry = u32(b, 24);
  bool has_symbols = false;
  for (auto &h : headers) {
    const auto name = text(name_data, h.name);
    if (h.type == 2) {
      has_symbols = true;
      require(h.entry_size == 16 && h.size % 16 == 0 && h.link < count &&
              headers[h.link].type == 3, "HHK2: invalid symbol table");
      for (std::size_t off = h.offset; off < std::size_t(h.offset) + h.size; off += 16) {
        auto value = u32(b, off + 4);
        if (u16(b, off + 14) == 0xfff1 && value >= 0x80000000 && value < 0x82000000) {
          std::uint32_t translated;
          require(translate(map, value, translated), "HHK2: unknown absolute OS symbol; port this app to HH3");
        }
      }
    }
    if (name.starts_with(".hollyhock_")) {
      require(h.type == 1 && h.size > 0 && h.size <= 4096, "HHK2: invalid app metadata");
      auto value = text(b.subspan(h.offset, h.size), 0);
      if (name == ".hollyhock_name") image.name = value;
      if (name == ".hollyhock_author") image.author = value;
      if (name == ".hollyhock_version") image.version = value;
      if (name == ".hollyhock_description") image.description = value;
    }
    if (!(h.flags & 2) || h.size == 0) continue;
    require(h.type == 1 || h.type == 8, "HHK2: unsupported allocated section");
    require(h.address >= ram_begin && h.address < ram_end && h.size <= ram_end - h.address,
            "HHK2: app must use RAM 8CFF0000-8CFFFFFF");
    require((h.address & 1) == 0, "HHK2: misaligned section");
    for (auto &s : image.sections)
      require(std::uint64_t(h.address) + h.size <= s.address ||
              std::uint64_t(s.address) + s.size <= h.address, "HHK2: overlapping sections");
    Section s{h.address, h.size, h.flags, {}};
    if (h.type == 1) s.data.assign(b.begin() + h.offset, b.begin() + h.offset + h.size);
    image.sections.push_back(std::move(s));
  }
  require(has_symbols, "HHK2: unstripped .hhk with symbol table required");
  require((image.entry & 1) == 0, "HHK2: misaligned entry point");
  require(std::any_of(image.sections.begin(), image.sections.end(), [&](auto &s) {
    return (s.flags & 4) && image.entry >= s.address && image.entry < s.address + s.size;
  }), "HHK2: entry point outside executable RAM");

  // Only patch literal pools addressed by SH mov.l @(disp,PC),Rn in code.
  // In non-code data, aligned function pointers may also need translation.
  // Never rescan patched values: some new addresses also occur in the old map.
  std::set<std::uint32_t> literals;
  for (const auto &s : image.sections) {
    if (s.flags & 4) {
      for (std::size_t o = 0; o + 2 <= s.data.size(); o += 2) {
        auto op = u16(s.data, o);
        if ((op & 0xf000) == 0xd000)
          literals.insert(((s.address + o + 4) & ~std::uint32_t(3)) + (op & 255) * 4);
      }
    } else {
      for (std::size_t o = (4 - (s.address & 3)) & 3; o + 4 <= s.data.size(); o += 4)
        literals.insert(s.address + o);
    }
  }
  for (auto address : literals) {
    for (auto &s : image.sections) {
      if (address < s.address || std::uint64_t(address) + 4 > std::uint64_t(s.address) + s.data.size()) continue;
      const auto o = address - s.address;
      std::uint32_t mapped;
      const auto value = u32(s.data, o);
      const bool known = translate(map, value, mapped);
      require(known || value < 0x80020000 || value >= 0x82000000 || std::has_single_bit(value),
              "HHK2: unknown ROM literal or constant; app cannot be translated");
      if (known && mapped != value) {
        put32(s.data, o, mapped);
        ++image.patched_words;
      }
      break;
    }
  }
  return image;
}
}
