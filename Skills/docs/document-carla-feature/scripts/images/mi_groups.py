"""mi_groups.py GRAB.png OUTPREFIX [x0]: split a material instance editor grab into one PNG per parameter group.

GRAB.png is an xwdgrab.py capture of the material instance editor window; x0 is where the Details panel starts
(default 580 for a 1400 px wide window with the viewport on the left). Group headers are found by their background
(grey 36) and their expand triangle; the last group ends above the "Save Sibling / Save Child" row. Writes
OUTPREFIX_00.png, OUTPREFIX_01.png, ... in panel order and prints the y ranges. Look at the crops and name them by hand.
"""
import sys
import numpy as np
from PIL import Image

def bands(mask):
    ys = np.where(mask)[0]; out = []; s = p = None
    for y in ys:
        if s is None: s = y
        elif y - p > 1: out.append((s, p)); s = y
        p = y
    if s is not None: out.append((s, p))
    return out

img = Image.open(sys.argv[1]).convert('RGB')
x0 = int(sys.argv[3]) if len(sys.argv) > 3 else 580
img = img.crop((x0, 90, img.width, img.height))
a = np.asarray(img.convert('L')).astype(int)
grp = [b for b in bands(a[:, 200] == 36) if b[1] - b[0] > 12 and a[b[0]:b[1], 8:20].max() >= 120]
sec = [b for b in bands(a[:, 200] == 47) if b[1] - b[0] > 12 and a[b[0]:b[1], 8:20].max() >= 100]
first_sec = next((x[0] for x in sec if x[0] > grp[0][0]), a.shape[0])
grp = [g for g in grp if g[0] < first_sec]
for i, g in enumerate(grp):
    y1 = grp[i + 1][0] if i + 1 < len(grp) else first_sec - 34
    img.crop((0, g[0], min(800, img.width), y1)).save('%s_%02d.png' % (sys.argv[2], i))
    print(i, g[0], y1)
