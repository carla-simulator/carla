#!/usr/bin/env python3
"""Insert captions under images: add_captions.py CAPTIONS.json [--root CARLA_ROOT]

CAPTIONS.json maps page -> {image path as written in the page: caption html}. An image that already has
a caption line under it is left alone. Write `&middot;` between provenance fields, <code>/<b> inside the
caption (it is already italic, so a nested <i> shows nothing)."""
import argparse, json, os, re
ap = argparse.ArgumentParser(); ap.add_argument('captions'); ap.add_argument('--root', default=os.path.abspath(os.path.join(os.path.dirname(__file__), *['..'] * 5)))
a = ap.parse_args()
for page, m in json.load(open(a.captions)).items():
    p = os.path.join(a.root, 'Docs', page); L = open(p, encoding='utf-8').read().split('\n'); out, n = [], 0
    for i, l in enumerate(L):
        out.append(l)
        for img, c in m.items():
            if re.search(r'!\[[^\]]*\]\(\s*' + re.escape(img) + r'\s*\)', l) and '<i>' not in ' '.join(L[i + 1:i + 3]):
                out.append(f'<div style="text-align: right"><i>{c}</i></div>'); n += 1
    open(p, 'w', encoding='utf-8').write('\n'.join(out)); print(page, n, '/', len(m))
