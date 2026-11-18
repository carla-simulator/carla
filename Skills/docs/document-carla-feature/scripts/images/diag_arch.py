import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1260, 640
s = Svg(W, H)
s.text(W / 2, 44, "CARLA client-server architecture", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "Default ports shown; the streaming and secondary ports follow the RPC port (N, N+1, N+2)",
       size=15, color=C["sub"], anchor="middle")
# client column
s.rect(30, 100, 360, 510, fill=C["card"], stroke=C["line"], r=14)
s.text(210, 132, "Client side  (your processes)", size=18, weight="bold", anchor="middle")
s.box(55, 155, 310, 92, "Python client", ["connects to the server host", "on the RPC port"],
      fill=C["soft_blue"], stroke=C["blue"])
s.box(55, 265, 310, 72, "C++ client", ["LibCarla client library"], fill=C["soft_blue"], stroke=C["blue"])
s.box(55, 355, 310, 92, "Traffic Manager", ["drives autopilot vehicles via RPC", "TM server in the first client, :8000"],
      fill=C["soft_orange"], stroke=C["orange"])
s.box(55, 465, 310, 120, "Your code on top", ["agents (BasicAgent, ...)", "ScenarioRunner / Leaderboard", "AV stacks, data collection"],
      fill=C["white"], stroke=C["line"])
# server
s.rect(560, 100, 400, 510, fill=C["soft_green"], stroke=C["green"], r=14, sw=2)
s.text(760, 132, "CARLA server", size=19, weight="bold", anchor="middle")
s.text(760, 154, "Unreal Engine 5.8", size=14, color=C["sub"], anchor="middle")
items = [("World / episode", "map, weather, settings, tick"), ("Actors + physics", "vehicles, walkers, traffic lights (Chaos)"),
         ("Sensors + rendering", "cameras, LiDAR, radar... (Lumen)"), ("Recorder", "logs the simulation for replay"),
         ("Native ROS 2", "publishes sensors, takes control")]
for i, (t, l) in enumerate(items):
    s.box(585, 172 + i * 86, 350, 72, t, [l], fill=C["white"], stroke=C["green"], tsize=16, lsize=13)
# arrows client <-> server
def link(y, label, sub, color, both=True, back=False):
    x1, x2 = 395, 555
    if back:
        s.line(x2, y, x1, y, color=color, sw=3, arrow=[k for k, v in C.items() if v == color][0])
    else:
        s.line(x1, y, x2, y, color=color, sw=3, arrow=[k for k, v in C.items() if v == color][0])
        if both:
            s.line(x2, y + 8, x1, y + 8, color=color, sw=3, arrow=[k for k, v in C.items() if v == color][0])
    s.text((x1 + x2) / 2, y - 26, label, size=15, weight="bold", color=color, anchor="middle")
    s.text((x1 + x2) / 2, y - 9, sub, size=12.5, color=C["sub"], anchor="middle")
link(215, "RPC  :2000", "commands, queries, tick", C["blue"])
link(330, "Streaming  :2001", "sensor data to clients", C["purple"], both=False, back=True)
# secondary servers + ROS
s.box(1000, 160, 235, 110, "Secondary servers", ["multi-GPU rendering", "connect to :2002"], fill=C["white"], stroke=C["line"], tsize=16)
s.line(995, 215, 965, 215, color=C["sub"], sw=2.5, arrow="sub")
s.line(965, 225, 995, 225, color=C["sub"], sw=2.5, arrow="sub")
s.box(1000, 470, 235, 110, "ROS 2 nodes", ["RViz, Autoware, rosbag", "DDS topics, no bridge"], fill=C["soft_purple"], stroke=C["purple"], tsize=16)
s.line(940, 525, 995, 525, color=C["purple"], sw=3, arrow="purple")
s.line(995, 535, 940, 535, color=C["purple"], sw=3, arrow="purple")
out = sys.argv[1]
s.save(out + '/client_server_architecture.svg')
render_png(out + '/client_server_architecture.svg', out + '/client_server_architecture.svg'.replace('.svg', '.png'), W, H)
