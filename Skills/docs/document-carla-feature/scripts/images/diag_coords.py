import math, sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1200, 730
s = Svg(W, H)
s.text(W / 2, 44, "CARLA frame vs ROS 2 / REP-103 (FLU) frame", size=26, weight="bold", anchor="middle")
s.text(W / 2, 74, "Top view of a vehicle: the nose points up the page, +z points out of the page towards you",
       size=15, color=C["sub"], anchor="middle")


def panel(x0, title, subtitle, ysign, fill, stroke, rules):
    s.rect(x0, 100, 540, 450, fill=fill, stroke=stroke, sw=2, r=14)
    s.text(x0 + 270, 136, title, size=21, weight="bold", anchor="middle")
    s.text(x0 + 270, 162, subtitle, size=15, color=C["sub"], anchor="middle")
    cx, cy = x0 + 270, 350
    # car body (top view)
    s.rect(cx - 45, cy - 95, 90, 190, fill=C["white"], stroke=C["sub"], r=26, sw=2)
    s.rect(cx - 33, cy - 55, 66, 40, fill=C["line"], stroke=C["sub"], r=8, sw=1)   # windscreen
    for dx in (-52, 46):
        for dy in (-70, 45):
            s.rect(cx + dx, cy + dy, 8, 28, fill=C["ink"], stroke=C["ink"], r=3, sw=1)
    # axes
    s.line(cx, cy, cx, cy - 165, color=C["red"], sw=4, arrow="red")
    s.text(cx + 12, cy - 160, "+x  forward", size=17, color=C["red"], weight="bold")
    ex = cx + ysign * 175
    s.line(cx, cy, ex, cy, color=C["green"], sw=4, arrow="green")
    lab = "+y  right" if ysign > 0 else "+y  left"
    s.text(ex + (10 if ysign > 0 else -10), cy - 12, lab, size=17, color=C["green"], weight="bold",
           anchor="start" if ysign > 0 else "end")
    s.circle(cx, cy, 11, fill=C["white"], stroke=C["blue"], sw=3)
    s.circle(cx, cy, 3.5, fill=C["blue"], stroke=C["blue"], sw=1)
    s.text(cx - 58, cy + 34, "+z up", size=15, color=C["blue"], weight="bold", anchor="end")
    # yaw arc from +x towards +y
    r = 120
    a0, a1 = -90, -90 + ysign * 55
    def pt(a):
        return cx + r * math.cos(math.radians(a)), cy + r * math.sin(math.radians(a))
    (x1, y1), (x2, y2) = pt(a0 + ysign * 8), pt(a1)
    sweep = 1 if ysign > 0 else 0
    s.path(f"M{x1:.1f},{y1:.1f} A{r},{r} 0 0 {sweep} {x2:.1f},{y2:.1f}", color=C["orange"], sw=3.5, arrow="orange")
    tx, ty = pt(a0 + ysign * 32)
    s.text(tx + ysign * 26, ty - 4, "+yaw", size=17, color=C["orange"], weight="bold", anchor="middle")
    for i, (k, v) in enumerate(rules):
        s.text(x0 + 30, 488 + i * 24, k, size=15, weight="bold")
        s.text(x0 + 110, 488 + i * 24, v, size=15, color=C["sub"])


panel(40, "CARLA / Unreal", "left-handed, z-up, metres, degrees", +1, C["soft_blue"], C["blue"],
      [("+yaw", "nose turns right (clockwise seen from above)"), ("+pitch", "nose up"), ("+roll", "right side drops")])
panel(620, "ROS 2 / REP-103 (FLU)", "right-handed, z-up, metres, degrees", -1, C["soft_green"], C["green"],
      [("+yaw", "nose turns left (counter-clockwise)"), ("+pitch", "nose down"), ("+roll", "right side drops")])

# conversion strip
s.rect(40, 570, 1120, 140, fill=C["card"], stroke=C["line"], r=12)
s.text(70, 604, "Conversion = mirror of the y axis, S = diag(1, -1, 1)", size=17, weight="bold")
s.text(70, 634, "location   (x, y, z)            ->  (x, -y, z)", size=15, mono=True)
s.text(70, 660, "rotation   (roll, pitch, yaw)   ->  (roll, -pitch, -yaw)", size=15, mono=True)
s.text(70, 686, "matrix     M                    ->  S·M·S", size=15, mono=True)
out = sys.argv[1]
s.save(out + '/coordinate_frames.svg')
render_png(out + '/coordinate_frames.svg', out + '/coordinate_frames.svg'.replace('.svg', '.png'), W, H)
