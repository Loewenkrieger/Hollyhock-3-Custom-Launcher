#!/usr/bin/env python3
"""Create the install bundle with complete pinned SDK sources and checksums."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import zipfile
from source import ROOT, inventory, test_input_hash

parser = argparse.ArgumentParser()
parser.add_argument('--outdir', type=Path, default=ROOT / 'dist')
args = parser.parse_args()
out = args.outdir.resolve()
config = json.loads((ROOT / 'tools/release.json').read_text())
info = inventory()
assert info['sdk_commit'] == config['upstream']['hhk3_sdk_commit'], 'Unexpected SDK revision'
inputs = test_input_hash()
reports = {}
for suite in ['host', 'target']:
    report = json.loads((out / ('tests-' + suite + '.json')).read_text())
    assert report['passed'] and report['inputs_sha256'] == inputs, 'Missing or stale ' + suite + ' tests'
    reports[suite] = report
run = (out / 'run.7002.bin').read_bytes()
run_hash = hashlib.sha256(run).hexdigest()
assert reports['target']['run_bin_sha256'] == run_hash, 'Target tests belong to a different binary'
elf = (out / 'run.7002.elf').read_bytes()
assert elf[:6] == b'\x7fELF\x01\x02', 'Expected big-endian ELF32'
assert struct.unpack_from('>H', elf, 18)[0] == 42, 'Expected SH ELF'
assert struct.unpack_from('>I', elf, 24)[0] == 0x8cfd0000, 'Unexpected launcher entry point'
assert 0 < len(run) < 0x20000, 'Launcher exceeds reserved RAM'
compiler = subprocess.check_output(['sh4a_nofpueb-elf-g++', '--version'], text=True).splitlines()[0]
metadata = dict(config, source_commit=info['commit'], source_dirty=info['dirty'],
                compiler=compiler, inputs_sha256=inputs, run_bin_size=len(run),
                run_bin_sha256=run_hash, tests={'host': 'passed', 'target': 'passed'})
release = out / 'release'
release.mkdir(parents=True, exist_ok=True)
bundle_name = 'hhk3-launcher-' + config['version'] + '-os7002'
entries = {'run.bin': run, 'BUILD.json': (json.dumps(metadata, indent=2) + '\n').encode(),
           'debug/run.7002.elf': elf,
           'smoke/launcher-test.hh3': (ROOT / 'smoke/hh3/launcher-test.hh3').read_bytes()}
for name in ['README_DE.md', 'TEST-REPORT.txt', 'PERFORMANCE.md']:
    entries[name] = (ROOT / 'docs' / name).read_bytes()
entries['LICENSE'] = (ROOT / 'LICENSE').read_bytes()
for path in (ROOT / 'docs/preview').glob('*.png'):
    entries['preview/' + path.name] = path.read_bytes()
for name in info['files']:
    entries['source/' + name] = (ROOT / name).read_bytes()
entries['source/SOURCE-INFO.json'] = (json.dumps(info, indent=2) + '\n').encode()
for suite in reports:
    entries['tests-' + suite + '.json'] = (out / ('tests-' + suite + '.json')).read_bytes()


def checksums(files):
    return ''.join(hashlib.sha256(data).hexdigest() + '  ' + name + '\n'
                   for name, data in sorted(files.items())).encode()


entries['SHA256SUMS.txt'] = checksums(entries)
archive = release / (bundle_name + '.zip')
with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as package:
    for name, data in sorted(entries.items()):
        item = zipfile.ZipInfo(bundle_name + '/' + name, date_time=(2026, 1, 1, 0, 0, 0))
        item.compress_type = zipfile.ZIP_DEFLATED
        item.external_attr = 0o100644 << 16
        package.writestr(item, data)
with zipfile.ZipFile(archive) as package:
    assert package.testzip() is None, 'ZIP CRC check failed'
    assert len(package.namelist()) == len(entries), 'Duplicate ZIP entries'
shutil.copyfile(out / 'run.7002.bin', release / 'run.bin')
(release / 'BUILD.json').write_bytes(entries['BUILD.json'])
assets = {name: (release / name).read_bytes()
          for name in ['run.bin', 'BUILD.json', archive.name]}
(release / 'SHA256SUMS.txt').write_bytes(checksums(assets))
print(f'Package verified: {archive} ({len(entries)} files, complete SDK source included)')
