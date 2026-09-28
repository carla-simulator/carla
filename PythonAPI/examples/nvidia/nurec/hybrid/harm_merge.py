"""Detail-preserving, temporally smoothed Harmonizer merge.
The Harmonizer's useful work on an inserted car is low-frequency (tone, shading, edge softening); its high-frequency
output jitters from frame to frame. Keep the composite's detail and apply only the low-frequency delta, smoothed
over time:  out_k = comp_k + up( EMA_k( down(harm_k) - down(comp_k) ) ).
usage: harm_merge.py RUN_DIR [--scale 4] [--ema 0.5] [--sigma 1.5]   -> frames/hm_NNNN.png
"""
import argparse, glob, os
import numpy as np
from PIL import Image
from scipy.ndimage import gaussian_filter, zoom

ap = argparse.ArgumentParser(); ap.add_argument("run"); ap.add_argument("--scale", type=int, default=4); ap.add_argument("--ema", type=float, default=0.5)
ap.add_argument("--sigma", type=float, default=1.5); ap.add_argument("--stats", action="store_true")
ap.add_argument("--harm-dir", default=None, help="directory of harmonized frames named comp_NNNN.png (default: frames/harm_NNNN.png)"); ap.add_argument("--prefix", default="hm")
a = ap.parse_args()
F = f"{a.run}/frames"; comps = sorted(glob.glob(f"{F}/comp_*.png")); ema = None; prev = {}
rows = []
for p in comps:
    k = os.path.basename(p)[5:9]; hp = f"{a.harm_dir}/comp_{k}.png" if a.harm_dir else f"{F}/harm_{k}.png"
    if not os.path.exists(hp): continue
    C = np.asarray(Image.open(p)).astype(np.float32); Hh = np.asarray(Image.open(hp)).astype(np.float32)
    s = a.scale
    lo = lambda X: gaussian_filter(X, (a.sigma * s, a.sigma * s, 0))[::s, ::s]
    delta = lo(Hh) - lo(C)
    ema = delta if ema is None else a.ema * ema + (1 - a.ema) * delta
    up = zoom(ema, (s, s, 1), order=1)[:C.shape[0], :C.shape[1]]
    if up.shape[:2] != C.shape[:2]:
        pad = np.zeros_like(C); pad[:up.shape[0], :up.shape[1]] = up; up = pad
    out = (C + up).clip(0, 255)
    Image.fromarray(out.astype(np.uint8)).save(f"{F}/{a.prefix}_{k}.png")
    if a.stats and prev:
        rows.append((k, float(np.abs(Hh - prev["H"]).mean()), float(np.abs(out - prev["O"]).mean()), float(np.abs(C - prev["C"]).mean())))
    prev = dict(H=Hh, O=out, C=C)
if rows:
    r = np.array([x[1:] for x in rows]); print("mean |frame diff| whole image: harmonizer %.2f  merged %.2f  composite %.2f" % tuple(r.mean(0)))
print("done", len(comps))
