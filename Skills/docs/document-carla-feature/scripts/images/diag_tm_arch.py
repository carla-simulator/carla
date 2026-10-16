import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
# Traffic Manager architecture, following TrafficManagerLocal::Update (LibCarla/source/carla/trafficmanager).
W, H = 1260, 760
s = Svg(W, H)
s.text(W / 2, 42, "Traffic Manager architecture", size=26, weight="bold", anchor="middle")
s.text(W / 2, 68, "One cycle per simulation step; the code is in LibCarla/source/carla/trafficmanager",
       size=14.5, color=C["sub"], anchor="middle")
# server column
s.box(1110, 100, 120, 560, "CARLA", ["server"], fill=C["soft_red"], stroke=C["red"], tsize=19)
# ALSM
s.box(60, 100, 930, 66, "Agent Lifecycle & State Management (ALSM)", ["reads every vehicle and walker from the server, once per cycle"],
      fill=C["soft_blue"], stroke=C["blue"], tsize=16, lsize=12.5)
s.line(1110, 133, 994, 133, color=C["ink"], arrow="ink")
s.text(1052, 124, "world state", size=12, color=C["sub"], anchor="middle")
# registry + state
s.box(60, 210, 200, 70, "Vehicle registry", ["TM vehicles, others"], fill=C["soft_purple"], stroke=C["purple"], tsize=15, lsize=12)
s.box(560, 210, 430, 70, "Simulation state", ["cached position, velocity, bounding box, light state"],
      fill=C["soft_blue"], stroke=C["blue"], tsize=15, lsize=12)
s.line(160, 166, 160, 206, arrow="ink")
s.line(775, 166, 775, 206, arrow="ink")
# control loop
LX, LY, LW, LH = 60, 320, 930, 190
s.rect(LX, LY, LW, LH, fill=C["card"], stroke=C["orange"], r=12, sw=2)
s.text(LX + 16, LY + 26, "Control loop", size=16, weight="bold", color=C["orange"])
s.text(LX + LW - 16, LY + 26, "each stage runs over all vehicles before the next one starts", size=12.5, color=C["sub"], anchor="end")
def stage(x, w, n, title, sub):
    s.rect(x, LY + 60, w, 90, fill=C["soft_orange"], stroke=C["orange"], r=8, sw=1.8)
    s.text(x + w / 2, LY + 92, n, size=12, color=C["orange"], anchor="middle", weight="bold")
    s.text(x + w / 2, LY + 114, title, size=15, weight="bold", anchor="middle")
    s.text(x + w / 2, LY + 134, sub, size=11.5, color=C["sub"], anchor="middle")
def barrier(x):
    s.rect(x, LY + 50, 10, 110, fill="#9aa5ad", stroke="#9aa5ad", r=2, sw=1)
stage(80, 170, "STAGE 1", "Localization", "path ahead, lane changes")
barrier(262)
stage(284, 170, "STAGE 2", "Collision", "hazards on the path")
barrier(466)
s.rect(488, LY + 50, 486, 110, fill="none", stroke=C["orange"], r=8, sw=1.2, dash="5,4")
s.text(900, LY + 176, "per vehicle, in sequence", size=12, color=C["sub"], anchor="middle", italic=True)
stage(500, 150, "STAGE 3", "Traffic light", "lights, stops, junctions")
stage(660, 150, "STAGE 4", "Motion planner", "target speed, PID")
stage(820, 145, "STAGE 5", "Vehicle lights", "indicators, brake, beams")
s.line(250, LY + 105, 262, LY + 105, arrow="ink", sw=1.5)
s.line(454, LY + 105, 466, LY + 105, arrow="ink", sw=1.5)
s.line(650, LY + 105, 660, LY + 105, arrow="ink", sw=1.5)
s.line(810, LY + 105, 820, LY + 105, arrow="ink", sw=1.5)
s.line(160, 280, 160, LY - 4, arrow="ink")
s.text(170, 304, "vehicles to update", size=12, color=C["sub"])
s.line(775, 280, 775, LY - 4, arrow="ink")
s.text(785, 304, "state read by every stage", size=12, color=C["sub"])
# helpers
s.box(80, 560, 250, 70, "In-Memory Map", ["map as a grid of waypoints"], fill=C["soft_green"], stroke=C["green"], tsize=15, lsize=12)
s.box(370, 560, 330, 70, "Path buffers & vehicle tracking", ["path of every vehicle (PBVT)"], fill=C["soft_green"], stroke=C["green"], tsize=15, lsize=12)
s.box(730, 560, 200, 70, "PID controller", ["throttle, brake, steer"], fill=C["soft_purple"], stroke=C["purple"], tsize=15, lsize=12)
s.line(140, 556, 140, LY + 154, arrow="ink", sw=1.5)                       # map -> localization
s.path(f"M220,{LY + 150} L220,{LY + 205} L400,{LY + 205} L400,556", arrow="ink", sw=1.5)  # localization writes paths
s.text(228, LY + 198, "writes paths", size=11.5, color=C["sub"])
s.line(440, 556, 440, LY + 154, arrow="ink", sw=1.5)                       # PBVT -> collision
s.line(575, 556, 575, LY + 154, arrow="ink", sw=1.5)                       # PBVT -> traffic light
s.line(685, 556, 685, LY + 154, arrow="ink", sw=1.5)                       # PBVT -> motion planner
s.text(452, 545, "read by stages 2-4", size=11.5, color=C["sub"])
s.line(760, LY + 154, 760, 556, arrow="ink", sw=1.5)
s.line(790, 556, 790, LY + 154, arrow="ink", sw=1.5)
# command array
s.box(1000, 380, 96, 70, "Command", ["array"], fill=C["soft_purple"], stroke=C["purple"], tsize=14, lsize=14)
s.line(990, 415, 998, 415, arrow="ink", sw=1.5)
s.line(1096, 415, 1108, 415, arrow="ink")
s.text(1048, 476, "apply_batch", size=11.5, color=C["sub"], anchor="middle", mono=True)
s.text(1048, 492, "(_sync)", size=11.5, color=C["sub"], anchor="middle", mono=True)
# legend
for i, (lab, fill, stroke, dash) in enumerate((("stage", C["soft_orange"], C["orange"], None),
                                              ("synchronization barrier", "#9aa5ad", "#9aa5ad", None),
                                              ("helper module", C["soft_green"], C["green"], None))):
    x = 80 + i * 250
    s.rect(x, 680, 22, 16, fill=fill, stroke=stroke, r=3, sw=1.2)
    s.text(x + 30, 693, lab, size=13, color=C["sub"])
s.text(1230, 730, "Stage 5 runs after the motion planner and adds SetVehicleLightState commands to the same batch.", size=12,
       color=C["sub"], anchor="end", italic=True)
out = sys.argv[1]
s.save(out + '/tm_architecture.svg')
render_png(out + '/tm_architecture.svg', out + '/tm_architecture.png', W, H)
