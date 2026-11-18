import sys, io, base64
sys.path.insert(0, '.')
from PIL import Image
from svgkit import Svg, C
# usage: diag_client_server.py <out_dir> <server_screenshot>
W, H = 1000, 400
s = Svg(W, H)
# server: a simulation screenshot clipped to a rounded panel
sx, sy, sw, sh = 30, 40, 540, 320
im = Image.open(sys.argv[2]).convert('RGB')
im = im.crop((0, 0, im.width, int(im.width * sh / sw))).resize((sw * 2, sh * 2), Image.LANCZOS)
buf = io.BytesIO()
im.save(buf, 'JPEG', quality=80)
b64 = base64.b64encode(buf.getvalue()).decode()
s.add(f'<clipPath id="srv"><rect x="{sx}" y="{sy}" width="{sw}" height="{sh}" rx="24"/></clipPath>')
s.add(f'<image x="{sx}" y="{sy}" width="{sw}" height="{sh}" preserveAspectRatio="xMidYMid slice" clip-path="url(#srv)" '
      f'href="data:image/jpeg;base64,{b64}"/>')
s.add(f'<rect x="{sx}" y="{sy + sh - 64}" width="{sw}" height="64" fill="#000000" fill-opacity="0.55" clip-path="url(#srv)"/>')
s.text(sx + 28, sy + sh - 26, "CARLA server", size=24, weight="bold", color=C["white"])
s.rect(sx, sy, sw, sh, fill="none", stroke=C["line"], r=24, sw=2)
# clients, each with its own connection
n, cx, cw, ch = 4, 760, 210, 56
for i in range(n):
    cy = 55 + i * 80
    s.box(cx, cy, cw, ch, "Client", fill=C["soft_blue"], stroke=C["blue"], tsize=18)
    y0, y1 = 140 + i * 40, cy + ch / 2
    s.path(f"M{sx + sw},{y0} C{sx + sw + 100},{y0} {cx - 100},{y1} {cx},{y1}", color=C["sub"], sw=2.5)
s.save(sys.argv[1] + '/client_server.svg')
