"""Town road network from the Map API: lanes, junctions, spawn points with indices."""
import sys, carla, numpy as np
import matplotlib; matplotlib.use("Agg"); import matplotlib.pyplot as plt
out = sys.argv[1]
c = carla.Client("localhost", 2000); c.set_timeout(60); m = c.get_world().get_map()
wps = m.generate_waypoints(1.0)
fig = plt.figure(figsize=(16, 13), dpi=110); ax = fig.add_axes([0.06, 0.04, 0.92, 0.86])
# lane centre lines as points (driving only), junction lanes orange
drv = [w for w in wps if w.lane_type == carla.LaneType.Driving]
pj = np.array([[w.transform.location.x, w.transform.location.y] for w in drv if w.is_junction])
pn = np.array([[w.transform.location.x, w.transform.location.y] for w in drv if not w.is_junction])
ax.scatter(pn[:, 0], pn[:, 1], s=2.4, c="#9aa7b1", linewidths=0, label="driving lane (centre line)")
ax.scatter(pj[:, 0], pj[:, 1], s=2.4, c="#e69138", linewidths=0, label="lane inside a junction")
sp = m.get_spawn_points()
for i, t in enumerate(sp):
    l, f = t.location, t.get_forward_vector()
    ax.annotate("", xy=(l.x + f.x * 3.2, l.y + f.y * 3.2), xytext=(l.x, l.y),
                arrowprops=dict(arrowstyle="-|>", color="#2980b9", lw=1.4, mutation_scale=9))
    ax.text(l.x, l.y, str(i), fontsize=7.5, color="#1b4f72", ha="center", va="center", fontweight="bold",
            bbox=dict(boxstyle="round,pad=0.12", fc="white", ec="#2980b9", lw=0.6))
from matplotlib.lines import Line2D
h_sp = Line2D([], [], color="#2980b9", marker=">", markersize=2, lw=1.4, label="spawn point index (arrow = heading)")
ax.set_aspect("equal"); ax.invert_yaxis()   # top view as in the editor: +x right, +y down
ax.set_xlabel("x (m)"); ax.set_ylabel("y (m)  (CARLA +y points down in this top view)")
ax.grid(color="#eef2f5", lw=0.6); hs, ls = ax.get_legend_handles_labels()
ax.legend(hs + [h_sp], ls + [h_sp.get_label()], loc="upper center", bbox_to_anchor=(0.5, 1.045), ncol=3, fontsize=11, markerscale=4, frameon=False)
name = m.name.split("/")[-1]
fig.suptitle(f"{name}: road network and the {len(sp)} indices of world.get_map().get_spawn_points()", fontsize=17, fontweight="bold", y=0.985)
fig.savefig(out); print(out, len(sp), len(wps))
