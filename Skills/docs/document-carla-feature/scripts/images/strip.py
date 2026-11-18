#!/usr/bin/env python3
"""strip.py OUT.webp CROP(x0,y0,x1,y1|none) W  IMG:LABEL ... : labelled horizontal strip of renders."""
import sys
from PIL import Image, ImageDraw, ImageFont
F = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 22)
out, crop, w = sys.argv[1], sys.argv[2], int(sys.argv[3])
items = [a.split(':', 1) for a in sys.argv[4:]]
tiles = []
for p, lab in items:
    im = Image.open(p).convert('RGB')
    if crop != 'none': im = im.crop(tuple(int(v) for v in crop.split(',')))
    im = im.resize((w, int(im.height * w / im.width)), Image.LANCZOS); tiles.append((im, lab))
h, pad = tiles[0][0].height, 8
S = Image.new('RGB', (len(tiles) * w + (len(tiles) - 1) * pad, h), 'white'); d = ImageDraw.Draw(S)
for i, (im, lab) in enumerate(tiles):
    x = i * (w + pad); S.paste(im, (x, 0))
    tw = d.textlength(lab, font=F); d.rectangle((x, 0, x + tw + 20, 36), fill=(0, 0, 0)); d.text((x + 10, 6), lab, fill='white', font=F)
S.save(out, quality=85); print(out, S.size)
