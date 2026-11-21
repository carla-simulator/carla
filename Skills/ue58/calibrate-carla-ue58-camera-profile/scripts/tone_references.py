#!/usr/bin/env python3
"""Make more references for a toned look: give black-and-white photos the colour of an example.

    python3 tone_references.py EXAMPLE.jpg OUT_DIR BW_1.jpg [BW_2.jpg ...]

Measures the mean a*/b* of the example per brightness band (20 bands of L*) and
paints each black-and-white photo with it: a sepia example gives sepia copies whose
shadows, mid-tones and highlights carry the same cast as the example. Brightness and
contrast of each photo stay its own. Writes OUT_DIR/toned_<name>.jpg.
"""
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import imgmetrics  # noqa: E402

M = np.array([[0.4124, 0.3576, 0.1805], [0.2126, 0.7152, 0.0722], [0.0193, 0.1192, 0.9505]])


def lab_to_rgb(L, a, b):
    fy = (L + 16) / 116
    fx, fz = fy + a / 500, fy - b / 200
    f = lambda t: np.where(t ** 3 > 0.008856, t ** 3, (t - 16 / 116) / 7.787)  # noqa: E731
    xyz = np.stack([f(fx) * 0.95047, f(fy), f(fz) * 1.08883], -1)
    lin = xyz @ np.linalg.inv(M).T
    s = np.where(lin <= 0.0031308, 12.92 * lin, 1.055 * np.clip(lin, 0, None) ** (1 / 2.4) - 0.055)
    return (np.clip(s, 0, 1) * 255 + 0.5).astype(np.uint8)


if len(sys.argv) < 4:
    sys.exit(__doc__)
example, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
L, a, b = imgmetrics.lab(np.asarray(Image.open(example).convert('RGB')))
edges = np.linspace(0, 100, 21)
centres, A, B = [], [], []
for i in range(20):
    m = (L >= edges[i]) & (L < edges[i + 1])
    if m.sum() > 50:
        centres.append((edges[i] + edges[i + 1]) / 2)
        A.append(a[m].mean())
        B.append(b[m].mean())
for f in sys.argv[3:]:
    Lb = imgmetrics.lab(np.asarray(Image.open(f).convert('RGB')))[0]
    img = lab_to_rgb(Lb, np.interp(Lb, centres, A), np.interp(Lb, centres, B))
    p = os.path.join(out, 'toned_' + os.path.splitext(os.path.basename(f))[0] + '.jpg')
    Image.fromarray(img).save(p, quality=92)
    print(p)
