#!/usr/bin/env python3
"""Generate exact SDK maps from the pinned HHK2 sources and HHK3 metadata."""
import argparse
import json
from pathlib import Path
import re

p = argparse.ArgumentParser()
p.add_argument('hhk2_sdk', type=Path)
p.add_argument('addresses_7002', type=Path)
p.add_argument('output', type=Path)
args = p.parse_args()
old = {}
for source in sorted((args.hhk2_sdk / 'os/functions').glob('*.s')):
    for name, address in re.findall(r'DEFINE_OS_FUNC\s+(\w+)\s+(0x[0-9A-Fa-f]+)', source.read_text()):
        old[name] = int(address, 16)
new = {x['FunctionName']: int(x['startAddress'], 16) for x in json.loads(args.addresses_7002.read_text())['addresses']}
aliases = {'free': 'Mem_Free', 'malloc': 'Mem_Malloc', 'memcpy': 'Mem_Memcpy', 'memset': 'Mem_Memset'}
aliases.update({s: 'String_' + s.capitalize() for s in ['strcat','strchr','strcmp','strcpy','strlen']})
aliases.update({s: 'File_' + s.capitalize() for s in ['close','fstat','getAddr','lseek','mkdir','open','read','remove','rename','stat','write']})
aliases.update({'getAddr':'File_GetAddr', 'findFirst':'File_FindFirst', 'findNext':'File_FindNext', 'findClose':'File_FindClose'})
assert len(old) == 72
lines = ['#pragma once', '#include "image.hpp"', '#include <array>', 'namespace legacy {']
for target in ['2000','7002']:
    lines.append('inline constexpr std::array<AddressMap, 72> map_' + target + ' = {{')
    for name, value in sorted(old.items(), key=lambda pair: pair[1]):
        mapped = value if target == '2000' else new[aliases.get(name,name)]
        lines.append(f'  {{0x{value:08x}, 0x{mapped:08x}}}, // {name}')
    lines.append('}};')
lines.append('}')
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text('\n'.join(lines) + '\n')
print('Generated 72 exact SDK address pairs per OS')
