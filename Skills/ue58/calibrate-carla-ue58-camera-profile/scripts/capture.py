#!/usr/bin/env python3
"""Capture a few street-level frames with one or more camera profiles (to look at, or to measure).

    python3 capture.py PROFILE [PROFILE ...] [--out DIR] [--views 4] [--weather ClearNoon]
                       [--size 1440x1080] [--fov 50] [--set field=value ...] [--root CARLA_ROOT]

--set writes a temporary copy of the first profile with those fields changed
(e.g. --set filmGrainIntensity=1 colorSaturation.w=0) as <PROFILE>_Probe.json, captures
it, and deletes it: a quick way to see what one field does before calibrating.
Prints the imgmetrics row of every frame.
"""
import argparse
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import imgmetrics  # noqa: E402
import profile_lib  # noqa: E402

ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument('profiles', nargs='+')
ap.add_argument('--out', default='captures')
ap.add_argument('--root', default=os.environ.get('CARLA_UE58_ROOT') or os.environ.get('CARLA_PACKAGE_ROOT') or os.environ.get('CARLA_ROOT'))
ap.add_argument('--host', default='localhost')
ap.add_argument('--port', type=int, default=int(os.environ.get('CARLA_PORT', 2000)))
ap.add_argument('--weather')
ap.add_argument('--views', type=int, default=4)
ap.add_argument('--views-file')
ap.add_argument('--seed', type=int, default=7)
ap.add_argument('--size', default='1440x1080')
ap.add_argument('--fov', type=float, default=50.0)
ap.add_argument('--warmup', type=int, default=60)
ap.add_argument('--set', nargs='*', default=[])
a = ap.parse_args()
W, H = (int(v) for v in a.size.lower().split('x'))
probe = None
profiles = list(a.profiles)
if a.set:
    if not a.root:
        sys.exit('--set needs --root (or CARLA_ROOT) to write the probe profile')
    doc = profile_lib.load_base(a.root, profiles[0])
    for fv in a.set:
        f, _, v = fv.partition('=')
        profile_lib.set_field(doc['settings'], f, json.loads(v))
    probe = profile_lib.write_profile(a.root, profiles[0] + '_Probe', doc)
    profiles[0] += '_Probe'
sess = profile_lib.Session(a.host, a.port, a.weather)
try:
    views = profile_lib.views_from_file(a.views_file) if a.views_file else profile_lib.pick_views(sess.world, a.views, a.seed)
    for p in profiles:
        files = sess.capture(p, views, os.path.join(a.out, p), W, H, a.fov, a.warmup)
        rows = [imgmetrics.metrics(f) for f in files]
        sys.stdout.write(imgmetrics.table(rows + [imgmetrics.median(rows)], files + ['MEDIAN ' + p]))
finally:
    sess.close()
    if probe:
        os.remove(probe)
