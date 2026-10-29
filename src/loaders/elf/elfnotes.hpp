#pragma once

#include <cstddef>
#include <cstdint>
#include <elf.h>
#include <memory>
#include <span>

class ELFNotes {
public:
  ELFNotes(std::span<const char> name, std::span<const std::uint8_t> desc,
           Elf32_Word type);

  [[nodiscard]] std::span<const char> getName() const;
  [[nodiscard]] std::span<const std::uint8_t> getDesc() const;
  const Elf32_Word type;

private:
  std::span<const char> data_name;
  std::span<const std::uint8_t> data_desc;
};
