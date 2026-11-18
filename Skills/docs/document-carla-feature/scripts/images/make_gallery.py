"""Final doc images from a gallery capture dir: per-sensor webp + composite grid."""
import os, sys
import numpy as np
from PIL import Image, ImageDraw, ImageFont
D, OUT = sys.argv[1], sys.argv[2]; os.makedirs(OUT, exist_ok=True)
S = os.path.join(D, "out")
FONT = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 26)
def L(p): return Image.open(p).convert("RGB")
# instance: colourise unique (G,B) ids, keep sky/unlabelled dark
ins = np.asarray(Image.open(os.path.join(D, "instance.png")).convert("RGB")).astype(np.int64)
ids = ins[..., 1] + 256 * ins[..., 2] + 65536 * ins[..., 0]
u, inv = np.unique(ids, return_inverse=True)
rng = np.random.default_rng(4); lut = rng.integers(40, 255, size=(len(u), 3)).astype(np.uint8)
col = lut[inv.reshape(ids.shape)]
col[ins[..., 0] == 11] = (20, 24, 30)  # sky tag in R
Image.fromarray(col).save(os.path.join(S, "instance_vis.png"))
tiles = [("RGB  sensor.camera.rgb", L(os.path.join(D, "rgb.png"))),
         ("Depth (logarithmic)", L(os.path.join(D, "depth_log.png"))),
         ("Semantic segmentation", L(os.path.join(D, "semantic.png"))),
         ("Instance segmentation", L(os.path.join(S, "instance_vis.png"))),
         ("Normals", L(os.path.join(D, "normals.png"))),
         ("LiDAR projected on camera", L(os.path.join(S, "lidar_on_camera.jpg"))),
         ("Semantic LiDAR projected", L(os.path.join(S, "semantic_lidar_on_camera.jpg"))),
         ("Radar detections", L(os.path.join(S, "radar_on_camera.jpg"))),
         ("Depth (raw encoding)", L(os.path.join(D, "depth_raw.png")))]
tw, th, pad = 640, 360, 8
g = Image.new("RGB", (3 * tw + 4 * pad, 3 * th + 4 * pad), (255, 255, 255))
for i, (name, im) in enumerate(tiles):
    x, y = pad + (i % 3) * (tw + pad), pad + (i // 3) * (th + pad)
    t = im.resize((tw, th), Image.LANCZOS); d = ImageDraw.Draw(t, "RGBA")
    w = d.textlength(name, font=FONT); d.rectangle((0, 0, w + 24, 42), fill=(0, 0, 0, 165))
    d.text((12, 7), name, font=FONT, fill=(255, 255, 255)); g.paste(t, (x, y))
g.save(os.path.join(OUT, "sensor_gallery.webp"), quality=86)
def web(src, name, w=1280):
    im = L(src); im = im.resize((w, round(im.height * w / im.width)), Image.LANCZOS)
    im.save(os.path.join(OUT, name), quality=86)
web(os.path.join(D, "rgb.png"), "sensor_rgb.webp")
web(os.path.join(D, "semantic.png"), "sensor_semantic_segmentation.webp")
web(os.path.join(S, "instance_vis.png"), "sensor_instance_segmentation.webp")
web(os.path.join(D, "instance.png"), "sensor_instance_segmentation_raw.webp")
web(os.path.join(D, "normals.png"), "sensor_normals.webp")
web(os.path.join(S, "lidar_on_camera.jpg"), "sensor_lidar_on_camera.webp")
web(os.path.join(S, "radar_on_camera.jpg"), "sensor_radar_on_camera.webp")
web(os.path.join(S, "semantic_lidar_on_camera.jpg"), "sensor_semantic_lidar_on_camera.webp")
web(os.path.join(S, "lidar_bev.png"), "sensor_lidar_bev.webp", 900)
web(os.path.join(S, "semantic_lidar_bev.png"), "sensor_semantic_lidar_bev.webp", 900)
# depth pair raw | log
a, b = L(os.path.join(D, "depth_raw.png")).resize((960, 540)), L(os.path.join(D, "depth_log.png")).resize((960, 540))
p = Image.new("RGB", (1928, 540), (255, 255, 255)); p.paste(a, (0, 0)); p.paste(b, (968, 0))
dd = ImageDraw.Draw(p, "RGBA")
for x, n in ((0, "raw (24-bit depth in R,G,B)"), (968, "carla.ColorConverter.LogarithmicDepth")):
    w = dd.textlength(n, font=FONT); dd.rectangle((x, 0, x + w + 24, 42), fill=(0, 0, 0, 165)); dd.text((x + 12, 7), n, font=FONT, fill="white")
p.save(os.path.join(OUT, "sensor_depth_raw_vs_log.webp"), quality=86)
for f in sorted(os.listdir(OUT)): print(f, os.path.getsize(os.path.join(OUT, f)) // 1024, "KB")
