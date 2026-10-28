#!/usr/bin/env python3
"""Generate DejaVu Sans bitmap subset. Ordinary builds use the included header."""
from pathlib import Path
import argparse
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser()
parser.add_argument('font', type=Path)
parser.add_argument('output', type=Path)
args = parser.parse_args()
font = ImageFont.truetype(str(args.font), 16)
codepoints = list(range(32, 127)) + list(range(160, 256))
advances, rows = [], []
for codepoint in codepoints:
    char = chr(codepoint)
    bitmap = Image.new('1', (16, 20))
    ImageDraw.Draw(bitmap).text((0, -1), char, font=font, fill=1)
    rows.append([sum((0x8000 >> x) for x in range(16) if bitmap.getpixel((x, y))) for y in range(20)])
    advances.append(min(16, max(1, round(font.getlength(char)))))
lines = ['// Generated from DejaVu Sans; see FONT-LICENSE.txt.', '#pragma once',
         '#include <cstdint>', 'namespace classic::font {',
         'inline constexpr int width = 16, height = 20, line = 20;',
         'inline constexpr std::uint8_t advance[] = {' + ','.join(map(str, advances)) + '};',
         'inline constexpr std::uint16_t bits[][20] = {']
lines += ['  {' + ','.join(f'0x{v:04x}' for v in glyph) + '},' for glyph in rows]
lines += ['};', '} // namespace classic::font', '']
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text('\n'.join(lines))
