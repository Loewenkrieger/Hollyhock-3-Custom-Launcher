#!/usr/bin/env python3
"""Host tests for the exact legacy parser used in run.bin. No SH code is run."""
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest
import os

ROOT = Path(__file__).resolve().parents[1]
MAP = (ROOT / 'src/legacy/sdk_maps.hpp').read_text()
# Independently pinned LCD_Refresh addresses from each original SDK version.
OLD, NEW = 0x8003733e, 0x800373ac


def fixture(*, symbol=OLD, literal=OLD, address=0x8cff0000, entry=None,
            symbols=True, bss_address=0x8cff0020, metadata=b'Fixture\0', data=True):
    strings = b'\0_LCD_Refresh\0'
    sym = bytes(16) + struct.pack('>IIIBBH', 1, symbol, 0, 0x12, 0, 0xfff1)
    code = bytes.fromhex('d101000b00090009') + struct.pack('>I', literal) + struct.pack('>I', OLD)
    names = ['', '.text', '.bss', '.hollyhock_name', '.symtab', '.strtab', '.shstrtab', '.data']
    name_data = b''
    name_offsets = []
    for name in names:
        name_offsets.append(len(name_data))
        name_data += name.encode() + b'\0'
    specs = [(0,0,0,b'',0,0,0), (1,6,address,code,len(code),0,0),
             (8,3,bss_address,b'',8,0,0), (1,0,0,metadata,len(metadata),0,0),
             (2 if symbols else 0,0,0,sym,len(sym),5,16),
             (3,0,0,strings,len(strings),0,0), (3,0,0,name_data,len(name_data),0,0),
             (1,3,0x8cff0030,struct.pack('>I',literal) if data else b'',4 if data else 0,0,0)]
    out = bytearray(52)
    headers = []
    for i,(kind,flags,addr,content,size,link,entsize) in enumerate(specs):
        while len(out) % 4: out.append(0)
        off = len(out)
        out += content
        headers.append(struct.pack('>IIIIIIIIII',name_offsets[i],kind,flags,addr,off,size,link,0,4,entsize))
    while len(out) % 4: out.append(0)
    table = len(out)
    out += b''.join(headers)
    ident = b'\x7fELF\x01\x02\x01' + bytes(9)
    out[:52] = struct.pack('>16sHHIIIIIHHHHHH',ident,2,42,1,address if entry is None else entry,0,table,0,52,0,0,40,len(specs),6)
    return bytes(out)


class ParserTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.probe = Path(cls.temp.name) / 'legacy-probe'
        subprocess.run(['g++','-std=c++23','-O1','-g','-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-Wall','-Wextra','-Werror','-I'+str(ROOT/'src'),
                        str(ROOT/'tests/legacy_probe.cpp'),str(ROOT/'src/legacy/image.cpp'),'-o',str(cls.probe)],check=True)

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def run_image(self, image, target='7002'):
        path = Path(self.temp.name) / 'fixture.hhk'
        path.write_bytes(image)
        return subprocess.run([str(self.probe),target,str(path)],capture_output=True,text=True)

    def reject(self, image, phrase):
        result = self.run_image(image)
        self.assertEqual(result.returncode,1,result.stdout+result.stderr)
        self.assertIn(phrase,result.stderr)

    def test_translates_only_referenced_literals_and_data(self):
        r = self.run_image(fixture())
        self.assertEqual(r.returncode,0,r.stderr)
        self.assertIn('PATCHES 2 SECTIONS 3',r.stdout)
        self.assertIn(f'8cff0000:d101000b00090009{NEW:08x}{OLD:08x}',r.stdout)
        self.assertIn(f'8cff0030:{NEW:08x}',r.stdout)

    def test_os2000_keeps_original_addresses(self):
        r=self.run_image(fixture(),'2000')
        self.assertEqual(r.returncode,0,r.stderr)
        self.assertIn('PATCHES 0',r.stdout)

    def test_absent_symbol_table(self): self.reject(fixture(symbols=False),'symbol table required')
    def test_unknown_absolute_symbol(self): self.reject(fixture(symbol=0x800dead0),'unknown absolute OS symbol')
    def test_unknown_rom_literal(self): self.reject(fixture(literal=0x80123456),'unknown ROM literal')
    def test_bitmask_is_not_treated_as_rom(self):
        self.assertEqual(self.run_image(fixture(literal=0x80000000)).returncode,0)
    def test_loader_overlap(self): self.reject(fixture(address=0x8cfd0000),'must use RAM')
    def test_section_overflow(self): self.reject(fixture(address=0x8cfffffc),'must use RAM')
    def test_overlapping_sections(self): self.reject(fixture(bss_address=0x8cff0008),'overlapping')
    def test_entry_not_in_code(self): self.reject(fixture(entry=0x8cff0020),'outside executable')
    def test_entry_alignment(self): self.reject(fixture(entry=0x8cff0001),'misaligned entry')
    def test_unterminated_metadata(self): self.reject(fixture(metadata=b'bad'),'unterminated')
    def test_truncation(self): self.reject(fixture()[:-5],'truncated')
    def test_header_endian(self):
        b=bytearray(fixture());b[5]=1
        self.reject(b,'ELF32 big endian')
    def test_processor(self):
        b=bytearray(fixture());struct.pack_into('>H',b,18,3)
        self.reject(b,'SuperH')
    def test_header_offset_overflow(self):
        b=bytearray(fixture());struct.pack_into('>I',b,32,0xfffffff0)
        self.reject(b,'truncated')
    def test_generated_maps_have_no_missing_or_duplicate_entries(self):
        for block in MAP.split('inline constexpr')[1:]:
            values=re.findall(r'\{(0x[0-9a-f]+), (0x[0-9a-f]+)\}',block)
            self.assertEqual(len(values),72)
            self.assertEqual(len({a for a,b in values}),72)


if __name__=='__main__': unittest.main(verbosity=2)
