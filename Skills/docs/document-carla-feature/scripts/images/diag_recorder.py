import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1260, 900
s = Svg(W, H)
s.text(W / 2, 44, "Recorder .log file layout", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "Every frame repeats the same packet sequence; a packet with no records still has its header",
       size=14.5, color=C["sub"], anchor="middle")
# file column
s.rect(30, 100, 330, 770, fill=C["card"], stroke=C["line"], r=14)
s.text(195, 132, "File", size=19, weight="bold", anchor="middle")
s.box(55, 150, 280, 150, "Info header", ["version   uint16  (1)", "magic     \"CARLA_RECORDER\"", "date      time_t", "map       string"],
      fill=C["soft_purple"], stroke=C["purple"], mono_lines=True, lsize=13)
for i, lab in enumerate(["Frame 1", "Frame 2", "Frame 3", "...", "Frame N"]):
    y = 320 + i * 62
    if lab == "...":
        s.text(195, y + 30, "...", size=22, anchor="middle", color=C["sub"]); continue
    s.rect(55, y, 280, 48, fill=C["soft_green"] if i else C["white"], stroke=C["green"], r=8, sw=2 if i == 0 else 1.5)
    s.text(195, y + 30, lab, size=16, weight="bold", anchor="middle")
s.path("M335,344 C 380,344 360,130 415,130", color=C["green"], sw=2, dash="6,4")
s.path("M335,344 C 380,344 360,840 415,840", color=C["green"], sw=2, dash="6,4")
# packet header
s.text(55, 690, "Every packet", size=16, weight="bold")
for i, (lab, w_) in enumerate((("id", 50), ("size", 70), ("payload (size bytes)", 140))):
    x = 55 + sum((50, 70, 140)[:i]) + i * 4
    s.rect(x, 705, w_, 40, fill=C["white"], stroke=C["ink"], r=4, sw=1.5)
    s.text(x + w_ / 2, 730, lab, size=13, anchor="middle")
s.text(55, 770, "id  uint8,  size  uint32", size=13, mono=True, color=C["sub"])
s.text(55, 794, "most payloads: uint16 count", size=13, mono=True, color=C["sub"])
s.text(55, 814, "  then the records", size=13, mono=True, color=C["sub"])
# frame expanded
s.rect(415, 100, 815, 770, fill=C["white"], stroke=C["green"], r=14, sw=2)
s.text(822, 132, "One frame = packets in this order", size=19, weight="bold", anchor="middle")
groups = [("Frame start", C["soft_green"], C["green"], [(0, "FrameStart", "frame id, duration, elapsed"), (20, "VisualTime", "visual time (double)")]),
          ("Events", C["soft_orange"], C["orange"], [(2, "EventAdd", "actor spawned (+ description)"), (3, "EventDel", "actor destroyed"),
                                                  (4, "EventParent", "attached to a parent"), (5, "Collision", "pair of actors that collided"),
                                                  (23, "VehicleDoor", "door opened / closed")]),
          ("Positions and states", C["soft_blue"], C["blue"], [(6, "Position", "location + rotation per actor"), (7, "State", "traffic light state")]),
          ("Animation", C["soft_blue"], C["blue"], [(8, "AnimVehicle", "steer, throttle, brake, gear"), (9, "AnimWalker", "walker speed"),
                                                  (10, "VehicleLight", "vehicle light state"), (11, "SceneLight", "street / building lights"),
                                                  (21, "AnimVehicleWheels", "wheel rotation and steer"), (22, "AnimBiker", "bike speed, engine rpm"),
                                                  (24, "Weather", "weather parameters")]),
          ("Additional data  (only when recording with additional data)", C["soft_purple"], C["purple"],
           [(12, "Kinematics", "velocities"), (13, "BoundingBox", "actor boxes"), (17, "TriggerVolume", "trigger boxes"),
            (14, "PlatformTime", "wall-clock time"), (15, "PhysicsControl", "vehicle physics"), (16, "TrafficLightTime", "light timings"),
            (19, "WalkerBones", "walker bone transforms")]),
          ("Frame end", C["soft_green"], C["green"], [(1, "FrameEnd", "closes the frame")])]
y = 152
for title, fill, stroke, items in groups:
    s.text(440, y + 14, title, size=14, weight="bold", color=stroke)
    y += 22
    cols = 2
    for k, (pid, name, what) in enumerate(items):
        cx = 440 + (k % cols) * 395; cy = y + (k // cols) * 34
        s.rect(cx, cy, 385, 28, fill=fill, stroke=stroke, r=5, sw=1.2)
        s.text(cx + 10, cy + 19, f"{pid:>2}", size=13, mono=True, weight="bold")
        s.text(cx + 40, cy + 19, name, size=13.5, weight="bold")
        s.text(cx + 375, cy + 19, what, size=12, color=C["sub"], anchor="end")
    y += ((len(items) + cols - 1) // cols) * 34 + 12
s.text(822, 858, "Id 18 (FrameCounter) is defined in CarlaRecorderPacketId but not written.", size=12.5, color=C["sub"], anchor="middle", italic=True)
out = sys.argv[1]
s.save(out + '/recorder_file_layout.svg')
render_png(out + '/recorder_file_layout.svg', out + '/recorder_file_layout.svg'.replace('.svg', '.png'), W, H)
