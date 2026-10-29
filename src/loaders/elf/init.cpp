#include "loader.hpp"
#include <bit>
#include <cstdio>
#include <cstring>
#include <elf.h>
#include <memory>
#include <stdexcept>
#include <unistd.h>

void ELFLoader::init_ehdr() {
  if (ehdr)
    return;

  const auto shared = file.acquire();
  const auto file = shared.get();

  auto header = std::make_unique<Elf32_Ehdr>();
  if (std::fseek(file, 0, SEEK_SET) != 0 ||
      std::fread(header.get(), sizeof(Elf32_Ehdr), 1, file) != 1)
    throw std::runtime_error("Failed to read ehdr");

  if (std::memcmp(header->e_ident, ELFMAG, SELFMAG) != 0)
    throw std::runtime_error("Invalid ELF magic");

  if (header->e_ident[EI_CLASS] != ELFCLASS32)
    throw std::runtime_error("Invalid ELF class");

  if (header->e_ident[EI_DATA] !=
      (std::endian::native == std::endian::little ? ELFDATA2LSB : ELFDATA2MSB))
    throw std::runtime_error("Invalid ELF data encoding");

  if (header->e_ident[EI_VERSION] != EV_CURRENT)
    throw std::runtime_error("Invalid ELF version");

  // dont care about OS ABI
  ehdr = std::move(header);
}

void ELFLoader::init_phdrs() {
  if (phdrs)
    return;

  const auto shared = file.acquire();
  const auto file = shared.get();

  init_ehdr();

  if (ehdr->e_phnum == 0) {
    phdrs = std::make_unique<Elf32_Phdr[]>(0);
    return;
  }

  if (ehdr->e_phentsize != sizeof(Elf32_Phdr))
    throw std::runtime_error("Invalid ELF program header size");

  auto headers = std::make_unique<Elf32_Phdr[]>(ehdr->e_phnum);
  if (std::fseek(file, ehdr->e_phoff, SEEK_SET) != 0 ||
      std::fread(headers.get(), sizeof(Elf32_Phdr), ehdr->e_phnum, file) !=
          ehdr->e_phnum)
    throw std::runtime_error("Failed to read program headers");

  // sanity check
  for (const auto &phdr : std::span(headers.get(), ehdr->e_phnum))
    switch (phdr.p_type) {
    case PT_NULL:
    case PT_LOAD:
    // case PT_DYNAMIC:
    // case PT_INTERP:
    case PT_NOTE:
    case PT_PHDR:
      break;
    default:
      throw std::runtime_error("Invalid program header type");
    }
  phdrs = std::move(headers);
}

void ELFLoader::init_notes() {
  if (notes)
    return;

  const auto shared = file.acquire();
  const auto file = shared.get();

  init_phdrs();

  auto parsed = std::make_unique<decltype(notes)::element_type>();
  std::vector<std::unique_ptr<std::uint8_t[]>> segments;
  if (std::fseek(file, 0, SEEK_END) != 0)
    throw std::runtime_error("Failed to seek to ELF end");
  const auto file_size = std::ftell(file);
  if (file_size < 0) throw std::runtime_error("Failed to read ELF size");

  for (const auto &phdr : std::span(phdrs.get(), ehdr->e_phnum)) {
    if (phdr.p_type != PT_NOTE)
      continue;
    if (phdr.p_filesz != phdr.p_memsz)
      throw std::runtime_error("Mismatched note size");

    if (phdr.p_filesz == 0)
      continue;

    if (phdr.p_offset > static_cast<unsigned long>(file_size) ||
        phdr.p_filesz > static_cast<unsigned long>(file_size) - phdr.p_offset)
      throw std::runtime_error("Invalid note segment size");

    if (std::fseek(file, phdr.p_offset, SEEK_SET) != 0)
      throw std::runtime_error("Failed to seek to note header");

    auto bytes = std::make_unique<std::uint8_t[]>(phdr.p_filesz);
    if (std::fread(bytes.get(), phdr.p_filesz, 1, file) != 1)
      throw std::runtime_error("Failed to read note segment");
    const auto *cursor = bytes.get();
    std::size_t remaining_size = phdr.p_filesz;
    while (remaining_size) {
      if (remaining_size < sizeof(Elf32_Nhdr))
        throw std::runtime_error("Invalid note header size");

      Elf32_Nhdr header;
      std::memcpy(&header, cursor, sizeof(header));
      cursor += sizeof(header);
      remaining_size -= sizeof(Elf32_Nhdr);

      // Widen before rounding: malformed 32-bit lengths must not wrap on SH.
      const auto name_size = (std::uint64_t{header.n_namesz} + 3) & ~std::uint64_t{3};
      const auto desc_size = (std::uint64_t{header.n_descsz} + 3) & ~std::uint64_t{3};
      if (name_size > remaining_size || desc_size > remaining_size - name_size)
        throw std::runtime_error("Invalid note content size");
      const auto *name = reinterpret_cast<const char *>(cursor);
      const auto *desc = cursor + static_cast<std::size_t>(name_size);
      parsed->emplace_front(std::span(name, header.n_namesz),
                            std::span(desc, header.n_descsz), header.n_type);
      const auto consumed = static_cast<std::size_t>(name_size + desc_size);
      cursor += consumed;
      remaining_size -= consumed;
    }
    segments.push_back(std::move(bytes));
  }
  note_segments = std::move(segments);
  notes = std::move(parsed);
}
