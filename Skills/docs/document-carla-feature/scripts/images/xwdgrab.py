"""Grab one X11 window (by id) with xwd and save it as PNG: xwdgrab.py WINDOW_ID OUT.png"""
import struct, subprocess, sys
from PIL import Image
raw = subprocess.run(["xwd", "-silent", "-id", sys.argv[1]], capture_output=True, check=True).stdout
h = struct.unpack(">25I", raw[:100])
header_size, fmt, depth, w, hgt = h[0], h[2], h[3], h[4], h[5]
bpp, bpl, ncolors = h[11], h[12], h[19]
off = header_size + ncolors * 12
data = raw[off:off + bpl * hgt]
order = h[7]  # byte order: 0 LSB first
mode = "BGRX" if order == 0 else "XRGB"
img = Image.frombuffer("RGB", (w, hgt), data, "raw", mode, bpl, 1)
img.save(sys.argv[2]); print(w, hgt, bpp, mode)
