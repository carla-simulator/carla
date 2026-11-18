"""Labelled grid: grid.py OUT.webp COLS name1.png:Label1 name2.png:Label2 ... [--tw 640]"""
import sys
from PIL import Image, ImageDraw, ImageFont
out, cols = sys.argv[1], int(sys.argv[2]); items = [a.split(":", 1) for a in sys.argv[3:] if not a.startswith("--")]
tw = 640; th = 360; pad = 8
F = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24)
rows = (len(items) + cols - 1) // cols
g = Image.new("RGB", (cols * tw + (cols + 1) * pad, rows * th + (rows + 1) * pad), (255, 255, 255))
for i, (f, lab) in enumerate(items):
    t = Image.open(f).convert("RGB").resize((tw, th), Image.LANCZOS); d = ImageDraw.Draw(t, "RGBA")
    w = d.textlength(lab, font=F); d.rectangle((0, 0, w + 22, 38), fill=(0, 0, 0, 165)); d.text((11, 6), lab, font=F, fill="white")
    g.paste(t, (pad + (i % cols) * (tw + pad), pad + (i // cols) * (th + pad)))
g.save(out, quality=86); print(out)
