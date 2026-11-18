"""Plot the tuto_G_traffic_manager set_path routes on the road network (from the Map API)."""
import sys, carla, numpy as np
import matplotlib; matplotlib.use("Agg"); import matplotlib.pyplot as plt
OUT = sys.argv[1]
import json
D = json.load(open(sys.argv[2]))
R1S, R1 = D["r1"][0], D["r1"][1:]
R2S, R2 = D["r2"][0], D["r2"][1:]
c = carla.Client("localhost", 2000); c.set_timeout(60); m = c.get_world().get_map(); sp = m.get_spawn_points()
wps = [w for w in m.generate_waypoints(1.0) if w.lane_type == carla.LaneType.Driving]
p = np.array([[w.transform.location.x, w.transform.location.y] for w in wps])
fig = plt.figure(figsize=(14, 11.5), dpi=110); ax = fig.add_axes([0.05, 0.04, 0.93, 0.86])
ax.scatter(p[:, 0], p[:, 1], s=2, c="#c5ced5", linewidths=0)
for start, idx, col, name in ((R1S, R1, "#d9534f", "Route 1"), (R2S, R2, "#2980b9", "Route 2")):
    pts = [sp[start].location] + [sp[i].location for i in idx]
    rid = 1 if start == R1S else 2
    tr = max((t for t in D["tracks"].values() if t["route"] == rid), key=lambda t: len(t["xy"]))
    xy = np.array(tr["xy"][1:])
    end = sp[idx[-1]].location   # after the last point the TM drives freely: cut there
    xy = xy[:int(np.argmin(np.hypot(xy[:, 0] - end.x, xy[:, 1] - end.y))) + 1]
    ax.plot(xy[:, 0], xy[:, 1], color=col, lw=3, alpha=0.8, label=name + " (driven by the Traffic Manager)")
    ax.scatter([sp[start].location.x], [sp[start].location.y], s=180, marker="*", color=col, edgecolor="black", zorder=5)
    ax.annotate("spawn %d" % start, (sp[start].location.x, sp[start].location.y), xytext=(8, 8), textcoords="offset points", color=col, fontsize=12, fontweight="bold")
    for i in idx:
        if i in R1 and i in R2:
            col_i = "#7e57c2"
        else:
            col_i = col
        l = sp[i].location
        ax.text(l.x, l.y, str(i), fontsize=9, color="white", ha="center", va="center", fontweight="bold",
                bbox=dict(boxstyle="round,pad=0.2", fc=col_i, ec="none"))
ax.set_aspect("equal"); ax.invert_yaxis(); ax.grid(color="#eef2f5"); ax.set_xlabel("x (m)"); ax.set_ylabel("y (m)")
from matplotlib.patches import Patch
hs, ls = ax.get_legend_handles_labels(); hs.append(Patch(color="#7e57c2")); ls.append("point in both routes")
ax.legend(hs, ls, loc="upper center", bbox_to_anchor=(0.5, 1.06), ncol=3, fontsize=12, frameon=False)
fig.suptitle("Town10HD_Opt: two set_path() routes converging (numbers = spawn point indices)", fontsize=16, fontweight="bold", y=0.985)
fig.savefig(OUT); print(OUT)
