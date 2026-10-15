"""Render lidar / semantic lidar / radar captures of gallery.py into doc images."""
import json, math, os, sys
import numpy as np
from PIL import Image, ImageDraw
import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
D = sys.argv[1]; OUT = sys.argv[2]
meta = json.load(open(os.path.join(D, "meta.json")))
W, H, FOV = meta["w"], meta["h"], meta["fov"]
F = W / (2 * math.tan(math.radians(FOV / 2)))
PAL = np.array([(0,0,0),(128,64,128),(244,35,232),(70,70,70),(102,102,156),(190,153,153),(153,153,153),(250,170,30),
       (220,220,0),(107,142,35),(152,251,152),(70,130,180),(220,20,60),(255,0,0),(0,0,142),(0,0,70),(0,60,100),
       (0,80,100),(0,0,230),(119,11,32),(110,190,160),(170,120,50),(55,90,80),(45,60,150),(157,234,50),(81,0,81),
       (150,100,100),(230,150,140),(180,165,180),(180,130,70)], dtype=np.uint8)
NAMES = {1: "road", 2: "sidewalk", 3: "building", 6: "pole", 7: "traffic light", 8: "traffic sign", 9: "vegetation",
         12: "pedestrian", 14: "car", 15: "truck", 16: "bus", 20: "static", 24: "road line"}


def project(xyz):
    x, y, z = xyz[:, 0], xyz[:, 1], xyz[:, 2]
    m = x > 0.5
    u = W / 2 + F * y[m] / x[m]; v = H / 2 - F * z[m] / x[m]
    k = (u >= 0) & (u < W) & (v >= 0) & (v < H)
    return u[k], v[k], np.where(m)[0][k]


def overlay(base, u, v, colors, r=2):
    im = Image.open(os.path.join(D, base)).convert("RGB")
    im = Image.blend(im, Image.new("RGB", im.size, (0, 0, 0)), 0.35)
    d = ImageDraw.Draw(im)
    order = np.argsort(-np.asarray(v))  # nothing fancy
    for i in order:
        c = tuple(int(x) for x in colors[i]); d.ellipse((u[i] - r, v[i] - r, u[i] + r, v[i] + r), fill=c)
    return im


lid = np.load(os.path.join(D, "lidar.npy"))
xyz = lid[:, :3]
# lidar on camera, coloured by range (turbo)
u, v, idx = project(xyz)
rng = np.linalg.norm(xyz[idx], axis=1)
cm = plt.get_cmap("turbo")
cols = (cm(np.clip(rng / 50.0, 0, 1))[:, :3] * 255).astype(np.uint8)
overlay("rgb.png", u, v, cols, r=1.6).save(os.path.join(OUT, "lidar_on_camera.jpg"), quality=88)


def bev(points, colors, path, title, legend=None, extent=38):
    fig = plt.figure(figsize=(9, 9), dpi=120, facecolor="#1c232a")
    ax = fig.add_axes([0, 0, 1, 1]); ax.set_facecolor("#1c232a")
    # CARLA y is right: plot (y, x) so forward is up and right is right
    for r_ in (10, 20, 30, 40, 50, 60):
        ax.add_patch(plt.Circle((0, 0), r_, fill=False, color="#2c3640", lw=0.8))
        ax.text(r_ * 0.71 + 0.5, r_ * 0.71, f"{r_} m", color="#55636f", fontsize=9)
    ax.scatter(points[:, 1], points[:, 0], s=2.2, c=colors / 255.0, linewidths=0)
    ax.add_patch(plt.Rectangle((-0.95, -2.4), 1.9, 4.8, color="#e8eef2"))
    ax.set_xlim(-extent, extent); ax.set_ylim(-extent * 0.35, extent * 1.65); ax.set_aspect("equal"); ax.axis("off")
    ax.text(-extent + 2, extent * 1.65 - 4, title, color="white", fontsize=15, fontweight="bold", va="top")
    if legend:
        for i, (name, c) in enumerate(legend):
            yy = extent * 1.65 - 10 - i * 3.2
            ax.add_patch(plt.Rectangle((-extent + 2, yy - 1.0), 2.2, 2.0, color=np.array(c) / 255.0))
            ax.text(-extent + 5.5, yy, name, color="white", fontsize=11, va="center")
    fig.savefig(path, facecolor=fig.get_facecolor()); plt.close(fig)


z = xyz[:, 2]
hc = (cm(np.clip((z + 2.4) / 10.0, 0, 1))[:, :3] * 255).astype(np.uint8)
bev(xyz, hc, os.path.join(OUT, "lidar_bev.png"), "sensor.lidar.ray_cast  (colour = height)")
sem = np.load(os.path.join(D, "semlidar.npy"))
sxyz = np.stack([sem["x"], sem["y"], sem["z"]], 1); tags = sem["tag"].astype(int)
present = [t for t in NAMES if (tags == t).sum() > 50]
bev(sxyz, PAL[tags], os.path.join(OUT, "semantic_lidar_bev.png"), "sensor.lidar.ray_cast_semantic  (colour = tag)",
    legend=[(NAMES[t], PAL[t]) for t in present])
u, v, idx = project(sxyz)
overlay("rgb.png", u, v, PAL[tags[idx]], r=1.6).save(os.path.join(OUT, "semantic_lidar_on_camera.jpg"), quality=88)
# radar: [velocity, azimuth, altitude, depth]
rad = np.load(os.path.join(D, "radar.npy"))
vel, az, alt, dep = rad[:, 0], rad[:, 1], rad[:, 2], rad[:, 3]
rx = dep * np.cos(alt) * np.cos(az); ry = dep * np.cos(alt) * np.sin(az); rz = dep * np.sin(alt)
u, v, idx = project(np.stack([rx, ry, rz], 1))
rc = (cm(np.clip(dep[idx] / 60.0, 0, 1))[:, :3] * 255).astype(np.uint8)
overlay("rgb.png", u, v, rc, r=5).save(os.path.join(OUT, "radar_on_camera.jpg"), quality=88)
print("lidar", len(lid), "sem", len(sem), "radar", len(rad), "radar in view", len(idx))
