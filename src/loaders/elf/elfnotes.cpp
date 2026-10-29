#include "elfnotes.hpp"
#include <cstddef>
#include <cstdint>
#include <elf.h>
#include <memory>
#include <span>

ELFNotes::ELFNotes(std::span<const char> name,
                   std::span<const std::uint8_t> desc, const Elf32_Word type)
    : type(type), data_name(name), data_desc(desc) {};

std::span<const char> ELFNotes::getName() const {
  return data_name;
}

std::span<const std::uint8_t> ELFNotes::getDesc() const {
  return data_desc;
}
