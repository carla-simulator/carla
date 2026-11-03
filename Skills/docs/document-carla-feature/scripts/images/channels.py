#!/usr/bin/env python3
"""channels.py DIFFUSE.png ORME.png OUT_DIFFUSE.webp OUT_ORME.webp: labelled channel strips of a building's textures."""
import sys
from PIL import Image, ImageDraw, ImageFont
F = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 22)
Fs = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 17)
def strip(tiles, out, T=440):
    pad, top = 14, 64
    W = len(tiles) * (T + pad) + pad
    im = Image.new('RGB', (W, T + top + pad), 'white'); d = ImageDraw.Draw(im)
    for i, (img, lab, sub) in enumerate(tiles):
        x = pad + i * (T + pad)
        im.paste(img.resize((T, T), Image.LANCZOS), (x, top))
        d.text((x, 8), lab, fill=(43, 43, 43), font=F); d.text((x, 36), sub, fill=(95, 107, 115), font=Fs)
    im.save(out, quality=88)
dif = Image.open(sys.argv[1]).convert('RGBA'); orm = Image.open(sys.argv[2]).convert('RGBA')
r, g, b, a = dif.split()
strip([(Image.merge('RGB', (r, g, b)), 'Diffuse RGB', 'base colours'),
       (a.convert('RGB'), 'Diffuse Alpha', 'white = area the shader recolours')], sys.argv[3])
R, G, B, A = orm.split()
strip([(R.convert('RGB'), 'ORME R', 'ambient occlusion'), (G.convert('RGB'), 'ORME G', 'roughness'),
       (B.convert('RGB'), 'ORME B', 'metallic'), (A.convert('RGB'), 'ORME Alpha', 'emissive mask')], sys.argv[4])
