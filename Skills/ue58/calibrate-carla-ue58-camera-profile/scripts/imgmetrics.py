#!/usr/bin/env python3
"""Look metrics of images, shared by the calibration and usable on its own.

    python3 imgmetrics.py IMG_OR_DIR ...            per-image table + median row
    python3 imgmetrics.py --json out.json DIR       also write the medians as JSON

Every image loses CROP of each side (frames, captions) and is resized to ANALYSIS_WIDTH, so references and CARLA frames
of any resolution are compared at the same scale (grain and sharpness depend
on it). Metrics are computed on the display-referred image (sRGB as stored),
because that is what a camera look is judged on.

  L_p05 L_p50 L_p95   CIE L* percentiles (0-100): black level, mid-tone, white level
  a_mid b_mid         mean a* / b* of the mid-tones (25 < L* < 85): colour cast
                      (b* > 0 warm/yellow, < 0 cool/blue; a* > 0 magenta/red, < 0 green)
  chroma_sd           colour left once the toning is removed: spread of a*/b* around their
                      mean at each brightness (0 = monochrome, toned or not, e.g. sepia;
                      colour photos 6-20)
  grain               fine noise in flat areas, in L* units
  sharpness           mean gradient energy (report only; not set by a profile)
  vignette            L* drop from the centre to the bottom corners (report only;
                      depends on the scene)
"""
import argparse
import glob
import json
import os
import statistics
import sys

import numpy as np
from PIL import Image, ImageFilter

ANALYSIS_WIDTH = 960
CROP = 0.06  # fraction trimmed from every side: print borders, plate edges, captions, scanner margins
KEYS = ['L_p05', 'L_p50', 'L_p95', 'a_mid', 'b_mid', 'chroma_sd', 'grain', 'sharpness', 'vignette']
IMG_EXT = ('.jpg', '.jpeg', '.png', '.webp', '.tif', '.tiff', '.bmp')


def load(path_or_image):
    im = path_or_image if isinstance(path_or_image, Image.Image) else Image.open(path_or_image)
    im = im.convert('RGB')
    dx, dy = round(im.width * CROP), round(im.height * CROP)
    im = im.crop((dx, dy, im.width - dx, im.height - dy))
    if im.width != ANALYSIS_WIDTH:
        im = im.resize((ANALYSIS_WIDTH, max(1, round(im.height * ANALYSIS_WIDTH / im.width))), Image.LANCZOS)
    return im


def lab(rgb):
    c = rgb.astype(np.float32) / 255.0
    lin = np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)
    xyz = lin @ np.array([[0.4124, 0.3576, 0.1805], [0.2126, 0.7152, 0.0722], [0.0193, 0.1192, 0.9505]]).T
    xyz /= np.array([0.95047, 1.0, 1.08883])
    f = np.where(xyz > 0.008856, np.cbrt(xyz), 7.787 * xyz + 16 / 116)
    return 116 * f[..., 1] - 16, 500 * (f[..., 0] - f[..., 1]), 200 * (f[..., 1] - f[..., 2])


def metrics(path_or_image):
    im = load(path_or_image)
    L, a, b = lab(np.asarray(im))
    p05, p50, p95 = np.percentile(L, [5, 50, 95])
    mid = (L > 25) & (L < 85)
    if mid.sum() < 100:
        mid = np.ones_like(L, bool)
    am, bm = float(a[mid].mean()), float(b[mid].mean())
    # remove the brightness-dependent toning (sepia, split toning): mean a*/b* per 5-L* band
    band = np.clip((L / 5).astype(int), 0, 19)
    ra, rb = a.copy(), b.copy()
    for i in np.unique(band):
        m = band == i
        ra[m] -= a[m].mean()
        rb[m] -= b[m].mean()
    chroma_sd = float(np.sqrt(ra[mid].var() + rb[mid].var()))
    # grain: high-pass of L*, then the 20th percentile of the local std over 16x16 blocks (flat areas)
    Li = Image.fromarray(np.clip(L * 2.55, 0, 255).astype(np.uint8))
    hp = L - np.asarray(Li.filter(ImageFilter.GaussianBlur(1.5)), np.float32) / 2.55
    h, w = hp.shape
    bs = 16
    blocks = hp[:h - h % bs, :w - w % bs].reshape(h // bs, bs, w // bs, bs).std(axis=(1, 3))
    grain = float(np.percentile(blocks, 20))
    gy, gx = np.gradient(L)
    sharp = float(np.sqrt(gx ** 2 + gy ** 2).mean())
    ch, cw = h // 5, w // 5
    centre = L[2 * ch:3 * ch, 2 * cw:3 * cw].mean()
    corners = (L[-ch:, :cw].mean() + L[-ch:, -cw:].mean()) / 2
    return dict(L_p05=float(p05), L_p50=float(p50), L_p95=float(p95), a_mid=am, b_mid=bm, chroma_sd=chroma_sd,
                grain=grain, sharpness=sharp, vignette=float(centre - corners))


def expand(paths):
    out = []
    for p in paths:
        if os.path.isdir(p):
            out += sorted(f for f in glob.glob(os.path.join(p, '*')) if f.lower().endswith(IMG_EXT)
                          and not os.path.basename(f).startswith('_'))
        else:
            out += sorted(glob.glob(p))
    return out


def median(rows):
    return {k: statistics.median(r[k] for r in rows) for k in KEYS}


def table(rows, names):
    s = '%-28s' % 'image' + ''.join('%10s' % k for k in KEYS) + '\n'
    for n, r in zip(names, rows):
        s += '%-28s' % os.path.basename(n)[:27] + ''.join('%10.2f' % r[k] for k in KEYS) + '\n'
    return s


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('paths', nargs='+')
    ap.add_argument('--json')
    a = ap.parse_args()
    files = expand(a.paths)
    if not files:
        sys.exit('no images found')
    rows = [metrics(f) for f in files]
    med = median(rows)
    sys.stdout.write(table(rows + [med], files + ['MEDIAN']))
    if a.json:
        json.dump(dict(median=med, images=dict(zip(files, rows))), open(a.json, 'w'), indent=1)
