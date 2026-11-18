import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1260, 720
s = Svg(W, H)
s.text(W / 2, 44, "Asynchronous vs synchronous mode", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "Who decides when the simulation advances, and how much simulated time each step covers",
       size=15, color=C["sub"], anchor="middle")
X0, X1 = 200, 1220


def lane(y, label, color):
    s.text(X0 - 20, y + 5, label, size=15, weight="bold", anchor="end", color=color)
    s.line(X0, y, X1, y, color=C["line"], sw=2)


# --- async
s.rect(30, 95, 1200, 270, fill=C["card"], stroke=C["line"], r=14)
s.text(55, 128, "Asynchronous (default)", size=19, weight="bold")
s.text(55, 152, "The server ticks as fast as it can. The client reads whatever frame is current and may miss frames.",
       size=14, color=C["sub"])
lane(215, "server", C["green"])
lane(300, "client", C["blue"])
xs = [X0 + 20, X0 + 120, X0 + 190, X0 + 330, X0 + 400, X0 + 520, X0 + 600, X0 + 700, X0 + 820, X0 + 890]
for i, x in enumerate(xs):
    w = (xs[i + 1] - x - 8) if i + 1 < len(xs) else 60
    s.rect(x, 198, w, 34, fill=C["soft_green"], stroke=C["green"], r=5, sw=1.5)
    s.text(x + w / 2, 220, f"f{i + 1}", size=13, anchor="middle")
for x, f in ((X0 + 160, 2), (X0 + 560, 6), (X0 + 930, 10)):
    s.line(x, 236, x, 290, color=C["blue"], sw=2, arrow="blue", dash="5,4")
    s.rect(x - 52, 300, 104, 30, fill=C["soft_blue"], stroke=C["blue"], r=5)
    s.text(x, 320, f"reads f{f}", size=13, anchor="middle")
s.text(X0 + 20, 352, "shown with a variable time step: each frame covers as much simulated time as it took to compute",
       size=13.5, color=C["sub"], italic=True)

# --- sync
s.rect(30, 385, 1200, 315, fill=C["card"], stroke=C["line"], r=14)
s.text(55, 418, "Synchronous", size=19, weight="bold")
s.text(55, 442, "The server waits for the client's tick. Every tick advances exactly one fixed time step and all sensor data of that frame arrives.",
       size=14, color=C["sub"])
lane(500, "client", C["blue"])
lane(600, "server", C["green"])
step = 330
for i in range(3):
    x = X0 + 20 + i * step
    s.rect(x, 483, 110, 34, fill=C["soft_blue"], stroke=C["blue"], r=5)
    s.text(x + 55, 505, "tick request", size=13, anchor="middle")
    s.line(x + 55, 521, x + 75, 580, color=C["blue"], sw=2, arrow="blue")
    s.rect(x + 60, 583, 190, 34, fill=C["soft_green"], stroke=C["green"], r=5)
    s.text(x + 155, 605, f"frame {i + 1}: +0.05 s", size=13, anchor="middle")
    s.line(x + 240, 580, x + 262, 521, color=C["purple"], sw=2, arrow="purple")
    s.rect(x + 200, 483, 110, 34, fill=C["soft_purple"], stroke=C["purple"], r=5)
    s.text(x + 255, 505, "sensor data", size=13, anchor="middle")
# substeps zoom
zx, zy = 300, 640
s.text(55, zy + 32, "inside one tick (physics)", size=14, weight="bold", color=C["orange"])
for k in range(5):
    s.rect(zx + k * 38, zy + 12, 34, 30, fill=C["soft_orange"], stroke=C["orange"], r=4)
    s.text(zx + k * 38 + 17, zy + 32, "0.01", size=11.5, anchor="middle")
s.text(zx + 205, zy + 32, "a 0.05 s step simulated as 5 physics substeps of 0.01 s", size=13.5, color=C["sub"])
out = sys.argv[1]
s.save(out + '/sync_async_modes.svg')
render_png(out + '/sync_async_modes.svg', out + '/sync_async_modes.svg'.replace('.svg', '.png'), W, H)
