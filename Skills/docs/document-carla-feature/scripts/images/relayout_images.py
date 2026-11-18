#!/usr/bin/env python3
"""relayout_images.py [--dry] [--ref=GIT_REF]: fix broken image refs (typically after merging a docs branch written
before the img/<page>/ layout).
- file already in the tree under another page's folder -> move it to img/shared/ and repoint every ref
- missing file -> restore the blob from --ref (the branch you merged) into img/<page>/ (img/shared/ if several pages use it)
Prints old -> new per ref. --dry: report only."""
import os, re, subprocess, sys, glob, collections, shutil
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), *['..'] * 5)); D = ROOT + '/Docs'; dry = '--dry' in sys.argv
REF = next((x.split('=', 1)[1] for x in sys.argv if x.startswith('--ref=')), 'HEAD')
IMG = re.compile(r'(!\[[^\]]*\]\(\s*<?)([^)\s>]+)|(<img[^>]*?src\s*=\s*["\'])([^"\']+)')
def refs():
    out = []
    for md in glob.glob(D + '/*.md'):
        t = open(md, encoding='utf-8').read()
        for m in IMG.finditer(t):
            p = m.group(2) or m.group(4)
            if p.startswith('http') or p.startswith('data:') or p.startswith('/'): continue
            q = p.split('#')[0]; q = q[3:] if q.startswith('../') else q
            if not os.path.exists(os.path.join(D, q)): out.append((os.path.basename(md), p, q))
    return out
broken = refs(); bypath = collections.defaultdict(set)
for page, p, q in broken: bypath[q].add(page)
plan = {}
for q, pages in sorted(bypath.items()):
    name = os.path.basename(q)
    here = [f for f in glob.glob(D + '/img/**/' + name, recursive=True)]
    if here:
        cur = os.path.relpath(here[0], D)
        owner = cur.split('/')[1]
        new = cur if owner == 'shared' else 'img/shared/' + name
        plan[q] = ('move', cur, new)
    else:
        sub = 'shared' if len(pages) > 1 else pages.copy().pop()[:-3]
        plan[q] = ('restore', 'Docs/' + q, f'img/{sub}/{name}')
for q, (kind, src, new) in plan.items(): print(f'{kind:7} {q} -> {new}  ({", ".join(sorted(bypath[q]))})')
if dry: sys.exit()
moved = {}
for q, (kind, src, new) in plan.items():
    dst = os.path.join(D, new); os.makedirs(os.path.dirname(dst), exist_ok=True)
    if kind == 'move':
        if src != new:
            subprocess.run(['git', 'mv', os.path.join(D, src), dst], check=True, cwd=ROOT); moved[src] = new
    else:
        blob = subprocess.run(['git', 'show', f'{REF}:{src}'], capture_output=True, check=True, cwd=ROOT).stdout
        open(dst, 'wb').write(blob)
rep = {q: new for q, (k, s, new) in plan.items()}; rep.update(moved)
for md in glob.glob(D + '/*.md'):
    t = open(md, encoding='utf-8').read(); o = t
    def sub(m):
        pre = m.group(1) or m.group(3); p = m.group(2) or m.group(4)
        q = p[3:] if p.startswith('../') else p
        return pre + rep.get(q, p)
    t = IMG.sub(sub, t)
    if t != o: open(md, 'w', encoding='utf-8').write(t); print('rewrote', os.path.basename(md))
