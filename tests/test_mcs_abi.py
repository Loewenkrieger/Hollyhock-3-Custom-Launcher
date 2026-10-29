#!/usr/bin/env python3
"""Check the pinned SDK's type representation in actual SH compiler output."""
from pathlib import Path
from sdk_path import sdk_include
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
prefix = 'sh4a_nofpueb-elf-'
with tempfile.TemporaryDirectory() as temporary:
    obj = Path(temporary) / 'mcs.o'
    binary = Path(temporary) / 'mcs.bin'
    subprocess.run([prefix + 'g++', '-std=c++23', '-Os', '-fno-builtin',
                    '-Wall', '-Wextra', '-Werror', '-Wno-attributes',
                    '-I' + str(sdk_include(root)), '-c',
                    str(root / 'tests/mcs_abi_probe.cpp'), '-o', str(obj)], check=True)
    subprocess.run([prefix + 'objcopy', '-O', 'binary', '-j', '.text',
                    str(obj), str(binary)], check=True)
    symbols = subprocess.check_output([prefix + 'nm', '--defined-only', str(obj)], text=True)
    image = binary.read_bytes()

    def result(name):
        pc = next(int(line.split()[0], 16) for line in symbols.splitlines()
                  if line.split()[-1] == '_' + name)
        value = None
        returning = False
        # These leaf probes only return constants. Reject unexpected instructions.
        for _ in range(8):
            instruction = struct.unpack_from('>H', image, pc)[0]
            if instruction & 0xff00 == 0xe000:  # mov #imm,r0
                value = instruction & 255
                if value & 128:
                    value -= 256
            elif instruction & 0xff00 == 0xd000:  # mov.l @(disp,PC),r0
                address = ((pc + 4) & ~3) + 4 * (instruction & 255)
                value = struct.unpack_from('>I', image, address)[0]
            elif instruction == 0x000b:  # rts, followed by one delay-slot instruction
                returning = True
                pc += 2
                continue
            elif instruction != 0x0009:
                raise AssertionError(f'Unsupported probe instruction {instruction:04x}')
            if returning:
                assert value is not None
                return value & 0xffffffff
            pc += 2
        raise AssertionError('Probe did not return')

    assert result('enum_after_byte_write') == 0x05000000
    print('PASS SH enum receives 0x05000000 from a one-byte VARTYPE_STR write')
    assert result('byte_after_byte_write') == 5
    print('PASS SH byte receives VARTYPE_STR unchanged')
