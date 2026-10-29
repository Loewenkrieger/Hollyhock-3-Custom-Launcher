// SPDX-License-Identifier: GPL-3.0-or-later
#include <bit>
#include <cstdint>
#include <sdk/os/mcs.h>

static_assert(std::endian::native == std::endian::big);
static_assert(sizeof(MCS_VariableType) == 4);

// Reproduce the OS byte write, without copying or executing firmware.
extern "C" std::uint32_t enum_after_byte_write() {
    auto type = static_cast<MCS_VariableType>(0);
    *reinterpret_cast<unsigned char *>(&type) = VARTYPE_STR;
    return type;
}

extern "C" std::uint32_t byte_after_byte_write() {
    std::uint8_t type = 0;
    *reinterpret_cast<unsigned char *>(&type) = VARTYPE_STR;
    return type;
}
