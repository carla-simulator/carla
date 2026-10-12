"""Composite every frame of a hybrid_video.py run (A = no cars, B = cars, E = engine) and write PNGs.
usage: hybrid_video_finish.py RUN_DIR [--soft 0.5] [--shadow-min 0.3] [--erode 1]
Writes RUN_DIR/frames/{naive,comp}_NNNN.png and RUN_DIR/frames/neural_NNNN.png.
"""
import argparse, json, os
import numpy as np
from PIL import Image
from scipy.ndimage import gaussian_filter, binary_erosion, distance_transform_edt

ap = argparse.ArgumentParser(); ap.add_argument("run"); ap.add_argument("--soft", type=float, default=0.5)
ap.add_argument("--shadow-min", type=float, default=0.3); ap.add_argument("--erode", type=int, default=1, help="mask erosion in px before the soft alpha (silhouette pixels are anti-aliased against the CARLA sky; --edge-extend keeps the colours of thin parts)")
ap.add_argument("--alpha-sigma", type=float, default=0.5, help="gaussian applied to the mask for the soft alpha (0.8 left thin limbs semi-transparent; 0 = hard)")
ap.add_argument("--soft-rel", type=float, default=0.02, help="depth-test softness as a fraction of the layer distance (max with --soft): the engine depth is noisier far away")
ap.add_argument("--depth-bias", type=float, default=0.3, help="metres added in favour of the layer: an actor standing on the ground is at the same distance as the ground under its feet")
ap.add_argument("--ego-depth", type=float, default=3.0, help="vehicle pixels closer than this are the ego proxy: drop them")
ap.add_argument("--empty-opa", type=float, default=0.6, help="engine pixels with opacity below this are floaters/sky: never occlude a car")
ap.add_argument("--unmix", type=int, default=0, help="remove the CARLA background from the cars' silhouette pixels using the catchers-only pass A image "
                                                    "(experimental: the soft mask is not the true coverage, so it over-corrects into a dark outline; off by default)")
ap.add_argument("--shadow-sigma", type=float, default=1.5, help="low-pass applied to both colour images before the cast-shadow ratio")
ap.add_argument("--jobs", type=int, default=16, help="worker processes (frames are independent)")
ap.add_argument("--edge-extend", type=int, default=1, help="push the colour of the nearest mask-interior pixel into the silhouette band, where the CARLA layer is anti-aliased against its own sky (kills the bright rim)")
a = ap.parse_args()
meta = json.load(open(f"{a.run}/meta.json"))
unmix = bool(a.unmix)
os.makedirs(f"{a.run}/frames", exist_ok=True)
GROUND = [1, 2, 24, 22, 10, 9]
SYN = [12, 13] + list(range(14, 20))      # synthetic actors: Pedestrian, Rider, vehicles


def composite(k):
    """One frame: returns its stats dict, or None when its layers are missing."""
    try:
        A = np.load(f"{a.run}/A_{k:04d}.npz"); B = np.load(f"{a.run}/B_{k:04d}.npz"); E = np.load(f"{a.run}/E_{k:04d}.npz")
    except FileNotFoundError:
        print("frame", k, "missing"); return None
    Bg = E["color"].astype(np.float32); dist = E["distance"]; opa = E["opacity"]
    R = B["rgb"].astype(np.float32); sem = B["sem"]; dc = B["dist"]          # Euclidean distance along each lens ray (rt_lens_distance)
    R0 = A["rgb"].astype(np.float32)                                           # pass A carries the catchers-only colour; labels/distance come from B
    M = np.isin(sem, SYN) & (dc > a.ego_depth)
    if a.erode > 0: M = binary_erosion(M, iterations=a.erode)
    Mf = M.astype(np.float32); R_raw = R
    if a.edge_extend and M.any():
        iy, ix = distance_transform_edt(~M, return_distances=False, return_indices=True)
        R = R[iy, ix]                       # outside the (eroded) mask every pixel takes its nearest interior colour; only the alpha band is ever used
    delta = dist - dc + a.depth_bias; soft = np.maximum(a.soft, a.soft_rel * dc)
    vis = np.clip(0.5 + delta / (2 * soft), 0, 1); vis = np.where(opa < a.empty_opa, 1.0, vis)
    alpha = np.clip(gaussian_filter(Mf * vis, a.alpha_sigma), 0, 1) * (gaussian_filter(Mf, a.alpha_sigma) > 0.02) if a.alpha_sigma > 0 else Mf * vis
    if unmix:
        # silhouette pixels of the car layer are anti-aliased against the CARLA background (sky, proxy road);
        # in native mode the catchers-only image is that background at the same tick, so unmix it:
        # R = a*car + (1-a)*R0  ->  car = (R - (1-a)*R0) / a, with a = the soft mask coverage
        cov = np.clip(gaussian_filter(Mf, 0.8), 0, 1)
        band = (cov > 0.08) & (cov < 0.97)
        a_ = np.maximum(cov, 0.08)[..., None]
        R = np.where(band[..., None], np.clip((R - (1 - a_) * R0) / a_, 0, 255), R)
    # cast shadow from the no-car pass (low-passed: the ratio of two independently path-traced images is noisy per pixel)
    Rl = gaussian_filter(R_raw.mean(axis=2), a.shadow_sigma) if a.shadow_sigma > 0 else R_raw.mean(axis=2)     # ratio from the untouched layer
    R0l = gaussian_filter(R0.mean(axis=2), a.shadow_sigma) if a.shadow_sigma > 0 else R0.mean(axis=2)
    ratio = np.clip(Rl / np.maximum(R0l, 1.0), a.shadow_min, 1.0)
    ground = np.isin(sem, GROUND) & (~M)
    vis0 = np.clip(0.5 + (dist - dc) / (2 * max(a.soft, 0.25)), 0, 1); vis0 = np.where(opa < 0.05, 0.0, vis0)
    shadow = gaussian_filter(1.0 - (1.0 - ratio) * ground * vis0, 1.0)
    Bs = Bg * shadow[..., None]
    C = alpha[..., None] * R + (1 - alpha[..., None]) * Bs
    naive = np.isin(sem, SYN)[..., None] * R + (~np.isin(sem, SYN))[..., None] * Bg
    Image.fromarray(C.clip(0, 255).astype(np.uint8)).save(f"{a.run}/frames/comp_{k:04d}.png")
    Image.fromarray(naive.clip(0, 255).astype(np.uint8)).save(f"{a.run}/frames/naive_{k:04d}.png")
    Image.fromarray(Bg.astype(np.uint8)).save(f"{a.run}/frames/neural_{k:04d}.png")
    st = dict(k=k, syn=int(M.sum()), vis=float(alpha.sum()), shadow_px=int(((shadow < 0.9) & ground).sum()))
    if k % 50 == 0: print("frame", k, st, flush=True)
    return st


if __name__ == "__main__":
    # one process per core: the frames are independent and the PNG encoding dominates
    from multiprocessing import Pool
    with Pool(max(1, min(a.jobs, os.cpu_count() or 1))) as pool:
        stats = [st for st in pool.map(composite, range(int(meta["frames"])), chunksize=4) if st is not None]
    json.dump(stats, open(f"{a.run}/frames/stats.json", "w"))
    print("done", len(stats), "frames")
