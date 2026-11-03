"""Animated WebP from frame dirs. anim.py OUT.webp DIR[,DIR2] [--w 800] [--step 2] [--fps 10] [--labels a,b]"""
import argparse, glob, os
from PIL import Image, ImageDraw, ImageFont
ap = argparse.ArgumentParser(); ap.add_argument("out"); ap.add_argument("dirs")
ap.add_argument("--w", type=int, default=800); ap.add_argument("--step", type=int, default=2)
ap.add_argument("--fps", type=float, default=10); ap.add_argument("--q", type=int, default=72)
ap.add_argument("--labels"); ap.add_argument("--start", type=int, default=0); ap.add_argument("--end", type=int, default=10**9)
a = ap.parse_args()
dirs = a.dirs.split(","); labels = a.labels.split(",") if a.labels else None
font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 18)
files = [sorted(glob.glob(os.path.join(d, "*.png")))[a.start:a.end:a.step] for d in dirs]
frames = []
for group in zip(*files):
    ims = [Image.open(f).convert("RGB") for f in group]
    w = a.w // len(ims); ims = [im.resize((w, round(im.height * w / im.width)), Image.LANCZOS) for im in ims]
    fr = Image.new("RGB", (w * len(ims) + 4 * (len(ims) - 1), ims[0].height), (255, 255, 255))
    for i, im in enumerate(ims):
        if labels:
            d = ImageDraw.Draw(im, "RGBA"); tw = d.textlength(labels[i], font=font)
            d.rectangle((0, 0, tw + 16, 28), fill=(0, 0, 0, 160)); d.text((8, 4), labels[i], font=font, fill="white")
        fr.paste(im, (i * (w + 4), 0))
    frames.append(fr)
frames[0].save(a.out, save_all=True, append_images=frames[1:], duration=int(1000 / a.fps), loop=0, quality=a.q, method=4)
print(a.out, len(frames), "frames", os.path.getsize(a.out) // 1024, "KB")
