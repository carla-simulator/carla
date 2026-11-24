#!/usr/bin/env python3
"""Fit a CARLA camera profile to reference photos.

    python3 calibrate.py --refs DIR --name NewProfile --root CARLA_ROOT [--base Cinematic]
                         [--weather ClearNoon] [--views 4] [--iters 8] [--fixed vignetteIntensity=0.6 ...]
                         [--fit ev,contrast,lift,sat,ta,tb,grain,vignette] [--out DIR]

Loop: write <NewProfile>.json into the server's Content/Carla/Config/PostProcess, spawn a
camera with it at a few street-level views, measure the frames with imgmetrics.py, and
move the grading parameters so the medians of the frames match the medians of the
references (damped Gauss-Newton; the first round measures how each parameter moves each
metric). The best profile found stays in place and is copied into --out with a report and
a REAL | CARLA sheet.

Parameters (all FPostProcessSettings fields, UE applies them in this order per pixel,
in linear light before the filmic tone curve: saturation, contrast, gamma, gain, offset):
  ev        autoExposureBias       exposure compensation (EV)      -> L_p50
  contrast  colorContrast.w        contrast around 18% grey        -> L_p05, L_p95
  lift      colorOffset.w          added after the gain            -> L_p05 (black level)
  sat       colorSaturation.w      0 = monochrome                  -> chroma_sd
  ta, tb    colorGain.xyz          gain along a* (magenta-green) and b* (yellow-blue), applied
                                   after the saturation, so a monochrome image can be toned
                                   -> a_mid, b_mid
  grain     filmGrainIntensity     film grain                      -> grain
  vignette  vignetteIntensity      darker corners                  -> vignette (scene dependent:
                                   leave it out of --fit when the references are not street views)
"""
import argparse
import json
import math
import os
import shutil
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import imgmetrics  # noqa: E402
import profile_lib  # noqa: E402

PARAMS = {  # name: (step for the finite difference, low, high)
    'ev': (0.4, -6.0, 6.0),
    'contrast': (0.12, 0.3, 2.5),
    'lift': (0.006, -0.03, 0.08),
    'sat': (0.12, 0.0, 2.0),
    'ta': (0.08, -1.2, 1.2),
    'tb': (0.08, -1.2, 1.2),
    'grain': (0.3, 0.0, 1.0),
    'vignette': (0.2, 0.0, 2.5),
}
TARGETS = {  # metric: scale (a residual of 1 = this many units off)
    'L_p05': 3.0, 'L_p50': 3.0, 'L_p95': 3.0, 'a_mid': 1.0, 'b_mid': 1.0, 'chroma_sd': 1.0, 'grain': 0.2,
    'vignette': 3.0,
}
FIELD = {  # simple one-field parameters
    'ev': 'autoExposureBias', 'contrast': 'colorContrast.w', 'lift': 'colorOffset.w', 'sat': 'colorSaturation.w',
    'grain': 'filmGrainIntensity', 'vignette': 'vignetteIntensity',
}
LUMA = np.array([0.2126, 0.7152, 0.0722])


def gain_rgb(base_gain, ta, tb):
    g = np.exp(ta * np.array([0.5, -0.5, 0.0]) + tb * np.array([0.35, 0.35, -0.7]))
    g = g / (g @ LUMA)  # keep the luminance: exposure is 'ev'
    return [float(base_gain[i] * g[i]) for i in range(3)]


def apply(doc0, x, names):
    doc = json.loads(json.dumps(doc0))
    s = doc['settings']
    v = dict(zip(names, x))
    for n, f in FIELD.items():
        if n in v:
            profile_lib.set_field(s, f, float(v[n]))
    if 'ta' in v or 'tb' in v:
        bg = [profile_lib.get_field(doc0['settings'], 'colorGain.' + c) for c in 'xyz']
        for c, g in zip('xyz', gain_rgb(bg, v.get('ta', 0.0), v.get('tb', 0.0))):
            profile_lib.set_field(s, 'colorGain.' + c, g)
    return doc


def start_point(doc, names):
    s = doc['settings']
    init = {n: profile_lib.get_field(s, f) for n, f in FIELD.items()}
    init.update(ta=0.0, tb=0.0)
    return np.array([float(init[n]) for n in names])


def sheet(refs, frames, out, h=300):
    def row(files):
        ims = []
        for f in files[:6]:
            im = Image.open(f).convert('RGB')
            ims.append(im.resize((round(im.width * h / im.height), h), Image.LANCZOS))
        return ims
    r1, r2 = row(refs), row(frames)
    w = max(sum(i.width for i in r) for r in (r1, r2))
    c = Image.new('RGB', (w, 2 * h + 8), (20, 20, 20))
    for y, r in ((0, r1), (h + 8, r2)):
        x = 0
        for im in r:
            c.paste(im, (x, y))
            x += im.width
    c.save(out, quality=88)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--refs', required=True, help='directory (or files) of reference photos')
    ap.add_argument('--name', required=True, help='new profile name (file name without .json)')
    ap.add_argument('--root', default=os.environ.get('CARLA_UE58_ROOT') or os.environ.get('CARLA_PACKAGE_ROOT') or os.environ.get('CARLA_ROOT'),
                    help='ue58-dev tree or package root the server runs from')
    ap.add_argument('--base', default='Cinematic', help='shipped profile to start from')
    ap.add_argument('--host', default='localhost')
    ap.add_argument('--port', type=int, default=int(os.environ.get('CARLA_PORT', 2000)))
    ap.add_argument('--weather', help='carla.WeatherParameters preset for the captures, e.g. ClearNoon')
    ap.add_argument('--views', type=int, default=4)
    ap.add_argument('--views-file', help='JSON list of {"location":[x,y,z],"rotation":[pitch,yaw,roll]}')
    ap.add_argument('--seed', type=int, default=7)
    ap.add_argument('--size', default='1440x1080', help='capture resolution (use the references aspect)')
    ap.add_argument('--fov', type=float, default=50.0)
    ap.add_argument('--warmup', type=int, default=60, help='ticks per view for auto exposure to settle')
    ap.add_argument('--iters', type=int, default=8)
    ap.add_argument('--tol', type=float, default=1.0, help='stop once the cost is below this')
    ap.add_argument('--fit', default='ev,contrast,lift,sat,ta,tb,grain,vignette')
    ap.add_argument('--fixed', nargs='*', default=[], help='field=value set before fitting (JSON values)')
    ap.add_argument('--out', help='work directory (default ./calib_<name>)')
    a = ap.parse_args()
    if not a.root:
        sys.exit('--root (or CARLA_ROOT) is required: the tree or package the server runs from')
    names = [n for n in a.fit.split(',') if n]
    for n in names:
        if n not in PARAMS:
            sys.exit('unknown parameter %s (choose from %s)' % (n, ','.join(PARAMS)))
    out = os.path.abspath(a.out or 'calib_' + a.name)
    os.makedirs(out, exist_ok=True)
    W, H = (int(v) for v in a.size.lower().split('x'))

    ref_files = imgmetrics.expand([a.refs])
    if not ref_files:
        sys.exit('no reference images in %s' % a.refs)
    ref = imgmetrics.median([imgmetrics.metrics(f) for f in ref_files])
    keys = [k for k in TARGETS if k not in ('grain', 'vignette') or k in names]
    print('references (%d):' % len(ref_files), ' '.join('%s=%.2f' % (k, ref[k]) for k in imgmetrics.KEYS))

    doc0 = profile_lib.load_base(a.root, a.base)
    for fv in a.fixed:
        f, _, v = fv.partition('=')
        profile_lib.set_field(doc0['settings'], f, json.loads(v))
    x = start_point(doc0, names)
    lo = np.array([PARAMS[n][1] for n in names])
    hi = np.array([PARAMS[n][2] for n in names])
    x = np.clip(x, lo, hi)

    sess = profile_lib.Session(a.host, a.port, a.weather)
    views = profile_lib.views_from_file(a.views_file) if a.views_file else profile_lib.pick_views(sess.world, a.views, a.seed)
    log = []
    n_eval = [0]

    def evaluate(xv, tag):
        n_eval[0] += 1
        doc = apply(doc0, xv, names)
        profile_lib.write_profile(a.root, a.name, doc)
        d = os.path.join(out, 'eval_%02d_%s' % (n_eval[0], tag))
        files = sess.capture(a.name, views, d, W, H, a.fov, a.warmup)
        m = imgmetrics.median([imgmetrics.metrics(f) for f in files])
        r = np.array([(m[k] - ref[k]) / TARGETS[k] for k in keys])
        cost = float(r @ r)
        log.append(dict(eval=n_eval[0], tag=tag, x=dict(zip(names, map(float, xv))), metrics=m, cost=cost, dir=d))
        print('[%02d %-8s] cost %8.2f | %s | %s' % (n_eval[0], tag, cost,
              ' '.join('%s=%.3g' % (n, v) for n, v in zip(names, xv)),
              ' '.join('%s=%.1f' % (k, m[k]) for k in keys)), flush=True)
        return r, cost, doc, files

    try:
        r, cost, doc, files = evaluate(x, 'start')
        best = (cost, x.copy(), doc, files)
        # Jacobian by forward differences, then Broyden updates
        J = np.zeros((len(keys), len(names)))
        for j, n in enumerate(names):
            step = PARAMS[n][0]
            xj = x.copy()
            xj[j] = xj[j] + step if xj[j] + step <= hi[j] else xj[j] - step
            rj, cj, dj, fj = evaluate(xj, 'd_' + n)
            J[:, j] = (rj - r) / (xj[j] - x[j])
            if cj < best[0]:
                best = (cj, xj.copy(), dj, fj)
        lam = 0.3
        for it in range(a.iters):
            if cost < a.tol:
                break
            JTJ = J.T @ J
            g = J.T @ r
            dx = -np.linalg.solve(JTJ + lam * np.diag(np.diag(JTJ) + 1e-6), g)
            xn = np.clip(x + dx, lo, hi)
            rn, cn, dn, fn = evaluate(xn, 'it%d' % (it + 1))
            s = xn - x
            if s @ s > 0:
                J += np.outer((rn - r) - J @ s, s) / (s @ s)
            if cn < cost:
                x, r, cost = xn, rn, cn
                lam = max(lam / 3, 1e-3)
                if cn < best[0]:
                    best = (cn, xn.copy(), dn, fn)
            else:
                lam *= 4
    finally:
        sess.close()

    cost, x, doc, files = best
    p = profile_lib.write_profile(a.root, a.name, doc)
    shutil.copy(p, os.path.join(out, a.name + '.json'))
    sheet(ref_files, files, os.path.join(out, 'sheet.jpg'))
    final = log[[l['cost'] for l in log].index(cost)]['metrics']
    json.dump(dict(reference=ref, final=final, params=dict(zip(names, map(float, x))), log=log),
              open(os.path.join(out, 'calibration.json'), 'w'), indent=1)
    rep = ['# %s calibration' % a.name, '', 'Base `%s`, %d references, %d views, weather %s, %d evaluations, cost %.2f.'
           % (a.base, len(ref_files), len(views), a.weather or '(as is)', n_eval[0], cost), '',
           '| metric | references | CARLA |', '|---|---|---|']
    rep += ['| %s | %.2f | %.2f |' % (k, ref[k], final[k]) for k in imgmetrics.KEYS]
    rep += ['', 'Parameters: ' + ', '.join('%s %.3f' % (n, v) for n, v in zip(names, x)), '',
            'Profile: `%s`' % p, '', '![sheet](sheet.jpg)']
    open(os.path.join(out, 'report.md'), 'w').write('\n'.join(rep) + '\n')
    print('\n'.join(rep[2:-2]))
    print('profile written:', p)
    print('work dir:', out)


if __name__ == '__main__':
    main()
