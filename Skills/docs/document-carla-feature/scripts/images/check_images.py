#!/usr/bin/env python3
"""Image checks for Docs/: broken image refs, images without a caption, files nothing references.

    python3 check_images.py [--root CARLA_ROOT] [--no-unreferenced]

A caption is the line `<div class="img-caption">...</div>` right under the image.
Generated pages (python_api, bp_library, catalogue_*) and logos are not checked for captions.
Exits 1 when there are broken refs, so it can gate a push."""
import argparse, glob, os, re, sys, urllib.parse
ap = argparse.ArgumentParser(); ap.add_argument('--root', default=os.path.abspath(os.path.join(os.path.dirname(__file__), *['..'] * 5)))
ap.add_argument('--no-unreferenced', action='store_true'); a = ap.parse_args()
D = os.path.join(a.root, 'Docs')
IMG = re.compile(r'!\[[^\]]*\]\(\s*<?([^)\s>]+)|<img[^>]*?src\s*=\s*["\']([^"\']+)|<source[^>]*?srcset\s*=\s*["\']([^"\']+)|<video[^>]*?src\s*=\s*["\']([^"\']+)')
GEN = {'python_api.md', 'bp_library.md', 'catalogue_vehicles.md', 'catalogue_props.md', 'catalogue_pedestrians.md'}
SKIP_UNREF = ('img/catalogue/', 'img/snipets_images/', 'img/logos/', 'img/btn/', 'img/carla_ue5_readme_img.webp')
used, broken, nocap = set(), [], []
for md in sorted(glob.glob(D + '/**/*.md', recursive=True)):
    page = os.path.relpath(md, D); L = open(md, encoding='utf-8').read().split('\n')
    for i, l in enumerate(L):
        for m in IMG.finditer(l):
            p = next(g for g in m.groups() if g)
            if re.match(r'^(https?:)?//', p) or p.startswith('data:'): continue
            q = urllib.parse.unquote(p.split('#')[0].split('?')[0]); q = q[3:] if q.startswith('../') else q.lstrip('/')
            used.add(os.path.normpath(q))
            if not os.path.exists(os.path.join(D, q)): broken.append(f'{page}:{i+1}  {p}')
            elif page not in GEN and 'img/logos/' not in q and 'img-caption' not in ' '.join(L[i:i+3])[len(l[:m.start()]):]:
                nocap.append(f'{page}:{i+1}  {q}')
print(f'broken image refs: {len(broken)}'); [print('  ' + b) for b in broken]
print(f'images without caption: {len(nocap)}'); [print('  ' + n) for n in nocap]
if not a.no_unreferenced:
    files = [os.path.relpath(f, D) for f in glob.glob(D + '/img/**/*', recursive=True) if os.path.isfile(f)]
    unref = [f for f in sorted(files) if f not in used and not f.startswith(SKIP_UNREF)]
    print(f'unreferenced files: {len(unref)}'); [print('  ' + u) for u in unref]
sys.exit(1 if broken else 0)
