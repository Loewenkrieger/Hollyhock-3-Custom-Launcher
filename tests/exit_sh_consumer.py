"""Narrow instruction-level check of the OS 7002 result consumer.

Runs the exact 32-byte consumer sequence and the callback instructions from
the freshly linked SH image. It stops at the normal OS continuation; it does
not simulate the rest of the operating system or physical hardware.
"""
import struct
import subprocess

CALLER = 0x8015c898
CONTINUE = 0x8015cf94
CODE = bytes.fromhex('6203e04c0b2eea634a083bac64b255b2450b342c960a620f32608901a36e0009')


def signed(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


class Consumer:
    def __init__(self, binary, result):
        self.binary = binary
        self.r = [0] * 16
        self.r[0] = result
        self.r[8], self.r[9], self.r[15] = 0x12345678, 0x76543210, 0x8c0ff000
        self.pc, self.pr, self.t = CALLER, 0, False
        self.calls = []

    def read(self, address, size):
        if CALLER <= address and address + size <= CALLER + len(CODE):
            return int.from_bytes(CODE[address - CALLER:address - CALLER + size], 'big')
        if address == 0x8015c8c4 and size == 2:
            return 0x03ea
        offset = address - 0x8cfd0000
        if 0 <= offset and offset + size <= len(self.binary):
            return int.from_bytes(self.binary[offset:offset + size], 'big')
        raise MemoryError(f'unmapped consumer read at {address:08x}')

    def step(self, delay=False):
        pc = self.pc
        insn = self.read(pc, 2)
        n, m = (insn >> 8) & 15, (insn >> 4) & 15
        self.pc += 2
        if insn == 0x0009:  # nop
            return
        if insn & 0xf000 == 0xe000:  # mov #imm,Rn
            self.r[n] = signed(insn & 255, 8) & 0xffffffff
        elif insn & 0xf00f == 0x6003:  # mov Rm,Rn
            self.r[n] = self.r[m]
        elif insn & 0xf00f == 0x000e:  # mov.l @(R0,Rm),Rn
            self.r[n] = self.read((self.r[0] + self.r[m]) & 0xffffffff, 4)
        elif insn & 0xf0ff == 0x4008:  # shll2 Rn
            self.r[n] = (self.r[n] << 2) & 0xffffffff
        elif insn & 0xf00f == 0x300c:  # add Rm,Rn
            self.r[n] = (self.r[n] + self.r[m]) & 0xffffffff
        elif insn & 0xf00f == 0x6002:  # mov.l @Rm,Rn
            self.r[n] = self.read(self.r[m], 4)
        elif insn & 0xf000 == 0x5000:  # mov.l @(disp,Rm),Rn
            self.r[n] = self.read(self.r[m] + (insn & 15) * 4, 4)
        elif insn & 0xf0ff == 0x400b:  # jsr @Rn; delay slot
            assert not delay
            target = self.r[n]
            self.pr = pc + 4
            self.calls.append(target)
            self.step(delay=True)
            self.pc = target
        elif insn & 0xf000 == 0x9000:  # mov.w @(disp,PC),Rn
            self.r[n] = signed(self.read(pc + 4 + (insn & 255) * 2, 2), 16) & 0xffffffff
        elif insn == 0x000b:  # rts; delay slot
            assert not delay
            target = self.pr
            self.step(delay=True)
            self.pc = target
        elif insn & 0xf00f == 0x600f:  # exts.w Rm,Rn
            self.r[n] = signed(self.r[m] & 0xffff, 16) & 0xffffffff
        elif insn & 0xf00f == 0x3000:  # cmp/eq Rm,Rn
            self.t = self.r[n] == self.r[m]
        elif insn & 0xff00 == 0x8900:  # bt disp
            if self.t:
                self.pc = pc + 4 + signed(insn & 255, 8) * 2
        elif insn & 0xf000 == 0xa000:  # bra disp; delay slot
            assert not delay
            target = pc + 4 + signed(insn & 0xfff, 12) * 2
            self.step(delay=True)
            self.pc = target
        else:
            raise AssertionError(f'Unexpected SH instruction {insn:04x} at {pc:08x}')

    def run(self):
        for _ in range(40):
            if self.pc == CONTINUE:
                return
            self.step()
        raise AssertionError('Consumer did not reach normal OS continuation')


def test(elf, binary_path, nm):
    symbols = subprocess.check_output([nm, '-C', str(elf)], text=True)

    def symbol(name):
        return int(next(line.split(maxsplit=2)[0] for line in symbols.splitlines()
                        if len(line.split(maxsplit=2)) == 3 and line.split(maxsplit=2)[2] == name), 16)

    image = binary_path.read_bytes()
    assert image[:2] != b'\x7fE', 'Expected raw binary, not ELF'
    result = symbol('(anonymous namespace)::object')
    callback = symbol('hhk_completed_dialog')
    machine = Consumer(image, result)
    machine.run()
    assert machine.calls == [callback]
    assert machine.r[0] == 0x03eb
    assert machine.r[8] == 0x12345678 and machine.r[9] == 0x76543210
    assert machine.r[15] == 0x8c0ff000
    print('PASS target SH image: actual return object/callback reaches normal OS continuation with Cancel')

    try:
        Consumer(image, 0).run()
    except MemoryError as error:
        assert '0000004c' in str(error)
    else:
        raise AssertionError('Old null return unexpectedly survived the OS dereference')
    print('PASS regression: old null return faults at OS object dereference +0x4c')
    print('2/2 target-consumer checks passed (narrow SH interpreter; not a hardware test)')
