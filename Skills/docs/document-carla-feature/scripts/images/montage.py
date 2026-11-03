"""montage.py OUT.webp W H MS IMG[:LABEL[:croptop]] ... -> animated webp, one still per frame."""
import sys
from PIL import Image, ImageDraw, ImageFont
F = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 30)
out, W, H, ms = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]); frames = []
for a in sys.argv[5:]:
    p, lab, ct = (a.split(':') + ['', '0'])[:3]
    im = Image.open(p).convert('RGB'); ct = float(ct or 0)
    if ct: im = im.crop((0, int(im.height * ct), im.width, im.height))
    r = max(W / im.width, H / im.height); im = im.resize((round(im.width * r), round(im.height * r)), Image.LANCZOS)
    x0, y0 = (im.width - W) // 2, (im.height - H) // 2; im = im.crop((x0, y0, x0 + W, y0 + H))
    if lab:
        d = ImageDraw.Draw(im); tw = d.textlength(lab, font=F); d.rectangle((0, H - 56, tw + 32, H), fill=(0, 0, 0)); d.text((16, H - 48), lab, fill='white', font=F)
    frames.append(im)
frames[0].save(out, save_all=True, append_images=frames[1:], duration=ms, loop=0, quality=80, method=4)
print(out, len(frames))
