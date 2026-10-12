"""Run the engine's Harmonizer (Difix) on PNG files, inside the nre-ga container.
usage (via /tmp/hybrid/runpy): harmonize.py IN.png OUT.png [IN2.png OUT2.png ...] [--color-transfer]
"""
import sys, time
import numpy as np, torch
from PIL import Image
from nre.difix.model import DifixModelFactory

args = [x for x in sys.argv[1:] if not x.startswith("--")]
import glob, os
if len(args) == 1 and os.path.isdir(args[0]):   # directory mode: comp_*.png -> harm_*.png
    srcs = sorted(glob.glob(os.path.join(args[0], "comp_*.png")))
    args = []
    for s_ in srcs:
        d_ = s_.replace("comp_", "harm_")
        if not os.path.exists(d_): args += [s_, d_]
    print("directory mode:", len(args) // 2, "frames to do", flush=True)
ct = "--color-transfer" in sys.argv
URL = "https://huggingface.co/nvidia/DiffusionHarmonizer/blob/main/harmonizer_nontemporal.pt"
t0 = time.time()
res = tuple(int(x) for x in [a for a in sys.argv[1:] if a.startswith("--res=")][0][6:].split("x")) if any(a.startswith("--res=") for a in sys.argv[1:]) else (576, 1024)
print("harmonizer resolution", res, flush=True)
model = DifixModelFactory.get(URL, "/home/.cache/nre/harmonizer", "harmonizer_nontemporal.pt", res)
print("harmonizer loaded %.1f s" % (time.time() - t0), flush=True)
for src, dst in zip(args[0::2], args[1::2]):
    img = np.asarray(Image.open(src).convert("RGB")).astype(np.float32) / 255.0
    h, w = img.shape[:2]
    flat = torch.from_numpy(img).cuda().reshape(h * w, 3)
    for i in range(2 if src == args[0] else 1):
        t1 = time.time()
        with torch.no_grad():
            out = model(flat, torch.Size((h, w)), ct)
        torch.cuda.synchronize()
        if len(args) <= 4 or args.index(src) % 100 == 0: print("%s: %.2f s" % (src, time.time() - t1), flush=True)
    out = (out.reshape(h, w, 3).clamp(0, 1) * 255).to(torch.uint8).cpu().numpy()
    Image.fromarray(out).save(dst)
    if len(args) <= 4: print("saved", dst, flush=True)
