import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
# Four consecutive frames: every packet is written every frame; event packets carry 0 records unless something happened.
PK = [("FrameStart", "g"), ("VisualTime", "g"), ("EventAdd", "e"), ("EventDel", "e"), ("EventParent", "e"),
      ("Collision", "e"), ("VehicleDoor", "e"), ("Position", "b"), ("State", "b"), ("Animation x7", "b"), ("FrameEnd", "g")]
FR = [("Frame 1", "3 spawned, 1 attached", {"EventAdd": "3", "EventParent": "1"}),
      ("Frame 2", "nothing happens", {}),
      ("Frame 3", "two vehicles collide", {"Collision": "1"}),
      ("Frame 4", "3 destroyed", {"EventDel": "3"})]
COLW, GAP, X0, Y0, RH = 270, 26, 30, 120, 36
W = X0 * 2 + 4 * COLW + 3 * GAP
H = Y0 + len(PK) * RH + 150
s = Svg(W, H)
s.text(W / 2, 44, "Example: four consecutive frames", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "Every frame writes every packet; an event packet holds records only in the frame where the event happens",
       size=14.5, color=C["sub"], anchor="middle")
for f, (name, what, ev) in enumerate(FR):
    x = X0 + f * (COLW + GAP)
    s.rect(x, Y0 - 30, COLW, len(PK) * RH + 76, fill=C["card"], stroke=C["line"], r=12)
    s.text(x + 16, Y0 - 4, name, size=18, weight="bold")
    s.text(x + COLW - 16, Y0 - 4, what, size=12.5, color=C["sub"], anchor="end")
    for k, (pk, kind) in enumerate(PK):
        y = Y0 + 14 + k * RH
        if kind == "e":
            on = pk in ev
            fill, stroke = (C["soft_orange"], C["orange"]) if on else (C["white"], C["line"])
            n = ev.get(pk, "0")
        else:
            fill, stroke = (C["soft_green"], C["green"]) if kind == "g" else (C["soft_blue"], C["blue"])
            n = None
        s.rect(x + 14, y, COLW - 28, RH - 8, fill=fill, stroke=stroke, r=5, sw=1.6 if kind == "e" and pk in ev else 1.2)
        col = C["ink"] if not (kind == "e" and pk not in ev) else C["sub"]
        s.text(x + 26, y + 19, pk, size=13.5, weight="bold" if col == C["ink"] else "normal", color=col)
        if n is not None:
            s.text(x + COLW - 26, y + 19, f"{n} record{'s' if n != '1' else ''}", size=12, mono=True,
                   color=C["orange"] if pk in ev else C["sub"], anchor="end", weight="bold" if pk in ev else "normal")
ly = Y0 + len(PK) * RH + 82
for i, (lab, fill, stroke) in enumerate((("frame start / end", C["soft_green"], C["green"]),
                                        ("event with records", C["soft_orange"], C["orange"]),
                                        ("event packet, 0 records (header + count only)", C["white"], C["line"]),
                                        ("positions, states, animation (every actor, every frame)", C["soft_blue"], C["blue"]))):
    x = X0 + (0, 210, 420, 800)[i]
    s.rect(x, ly, 22, 16, fill=fill, stroke=stroke, r=3, sw=1.2)
    s.text(x + 30, ly + 13, lab, size=13, color=C["sub"])
s.text(W / 2, ly + 50, "Animation x7: AnimVehicle, AnimWalker, VehicleLight, SceneLight, AnimVehicleWheels, AnimBiker, Weather. "
       "With additional data on, seven more packets go before FrameEnd.", size=12.5, color=C["sub"], anchor="middle", italic=True)
out = sys.argv[1]
s.save(out + '/recorder_frames_example.svg')
render_png(out + '/recorder_frames_example.svg', out + '/recorder_frames_example.png', W, H)
