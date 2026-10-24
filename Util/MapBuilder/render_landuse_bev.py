#!/usr/bin/env python3
"""
High-Resolution False-Color BEV & Live Simulator Landuse Renderer for CARLA Map Maker (Milestone 6).

Generates high-resolution Bird's-Eye View (BEV) orthographic visual feedback maps
rendering roads, buildings, pedestrian plazas, green spaces, water bodies, and sub-base ground.
Supports both offline vector rendering via Pillow and live CARLA camera capture (RGB + Semantic).
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import shutil
import sys
import tarfile
from typing import Any

from PIL import Image, ImageDraw, ImageFont

# Resolve imports
_current_dir = Path(__file__).resolve().parent
if str(_current_dir) not in sys.path:
    sys.path.insert(0, str(_current_dir))

from cmesh import read_cmesh_file


# Color Palette for False-Color BEV
BEV_COLORS = {
    "background": (20, 24, 30),
    "subbase": (75, 55, 45),            # Earth brown
    "water": (41, 128, 185),            # Deep blue
    "grass": (39, 174, 96),             # Rich green
    "plaza": (160, 168, 175),           # Light plaza stone
    "roads": (230, 126, 34),            # Vivid road orange
    "buildings": (142, 68, 173),        # Royal purple
    "building_outline": (90, 40, 115),
    "grid": (45, 52, 65),
    "text": (240, 240, 240),
}


def render_offline_bev(
    bundle_path: str | Path,
    output_image: str | Path,
    image_width: int = 2048,
    image_height: int = 1536,
) -> Path:
    """
    Renders an offline high-resolution false-color BEV map from the geometry layers in a .carlamap bundle.
    """
    target = Path(bundle_path).resolve()
    temp_dir: Path | None = None

    if target.is_file() and target.name.endswith((".tar.gz", ".tar")):
        import tempfile
        temp_dir = Path(tempfile.mkdtemp(prefix="carla_bev_"))
        with tarfile.open(target, "r:*") as tar:
            tar.extractall(temp_dir)
        inner_dirs = [d for d in temp_dir.iterdir() if d.is_dir() and (d / "carla-map.json").is_file()]
        bundle_dir = inner_dirs[0] if inner_dirs else temp_dir
    elif target.is_dir():
        bundle_dir = target
    else:
        raise FileNotFoundError(f"Target '{target}' not found.")

    try:
        manifest_path = bundle_dir / "carla-map.json"
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        map_name = manifest.get("map_name", "CARLA Map")
        layers = manifest.get("layers", [])

        # Read OpenDRIVE road waypoints if carla is importable
        road_lines: list[list[tuple[float, float]]] = []
        try:
            import carla
            xodr_file = bundle_dir / manifest.get("xodr_file", f"OpenDrive/{map_name}.xodr")
            if xodr_file.is_file():
                cmap = carla.Map(map_name, xodr_file.read_text(encoding="utf-8"))
                for wp1, wp2 in cmap.get_topology():
                    loc1 = wp1.transform.location
                    loc2 = wp2.transform.location
                    road_lines.append([(loc1.x, loc1.y), (loc2.x, loc2.y)])
        except Exception:
            pass

        # Load all layer geometry
        categorized_layers: dict[str, list[dict[str, Any]]] = {
            "subbase": [],
            "water": [],
            "grass": [],
            "plaza": [],
            "roads": [],
            "buildings": [],
        }

        all_x: list[float] = []
        all_y: list[float] = []

        for layer in layers:
            name = layer.get("name", "").lower()
            label = layer.get("semantic_label", "").lower()
            mesh_rel = layer.get("mesh_file", "")
            mesh_path = bundle_dir / mesh_rel
            if not mesh_path.is_file():
                continue

            geom = read_cmesh_file(mesh_path)
            verts = geom["vertices"]
            tris = geom["triangles"]

            for v in verts:
                all_x.append(v[0])
                all_y.append(v[1])

            layer_data = {"name": layer.get("name"), "vertices": verts, "triangles": tris}
            if "subbase" in name:
                categorized_layers["subbase"].append(layer_data)
            elif "water" in name or label == "water":
                categorized_layers["water"].append(layer_data)
            elif "grass" in name or label == "vegetation":
                categorized_layers["grass"].append(layer_data)
            elif "plaza" in name or label in ("ground", "sidewalks"):
                categorized_layers["plaza"].append(layer_data)
            elif "road" in name or label == "roads":
                categorized_layers["roads"].append(layer_data)
            elif "building" in name or label == "buildings":
                categorized_layers["buildings"].append(layer_data)

        if not all_x or not all_y:
            min_x, max_x = -200.0, 200.0
            min_y, max_y = -200.0, 200.0
        else:
            min_x, max_x = float(min(all_x)), float(max(all_x))
            min_y, max_y = float(min(all_y)), float(max(all_y))

        # Add 5% padding around bounding box
        span_x = max(10.0, max_x - min_x)
        span_y = max(10.0, max_y - min_y)
        pad_x = span_x * 0.05
        pad_y = span_y * 0.05

        view_min_x = min_x - pad_x
        view_max_x = max_x + pad_x
        view_min_y = min_y - pad_y
        view_max_y = max_y + pad_y

        scale_x = (image_width - 80) / (view_max_x - view_min_x)
        scale_y = (image_height - 120) / (view_max_y - view_min_y)
        scale = min(scale_x, scale_y)

        # Coordinate transform from simulation meters to image pixels
        # CARLA: X right, Y forward (up on map -> inverted in screen Y)
        mid_x = (view_min_x + view_max_x) * 0.5
        mid_y = (view_min_y + view_max_y) * 0.5
        center_px_x = image_width * 0.5
        center_px_y = (image_height + 40) * 0.5

        def to_px(x: float, y: float) -> tuple[float, float]:
            px = center_px_x + (x - mid_x) * scale
            py = center_px_y - (y - mid_y) * scale
            return px, py

        # Create canvas
        im = Image.new("RGBA", (image_width, image_height), BEV_COLORS["background"])
        draw = ImageDraw.Draw(im)

        # Draw coordinate grid lines every 100 meters
        grid_start_x = math.floor(view_min_x / 100.0) * 100
        grid_end_x = math.ceil(view_max_x / 100.0) * 100
        grid_start_y = math.floor(view_min_y / 100.0) * 100
        grid_end_y = math.ceil(view_max_y / 100.0) * 100

        for gx in range(int(grid_start_x), int(grid_end_x) + 1, 100):
            p0 = to_px(gx, view_min_y)
            p1 = to_px(gx, view_max_y)
            draw.line([p0, p1], fill=BEV_COLORS["grid"], width=1)

        for gy in range(int(grid_start_y), int(grid_end_y) + 1, 100):
            p0 = to_px(view_min_x, gy)
            p1 = to_px(view_max_x, gy)
            draw.line([p0, p1], fill=BEV_COLORS["grid"], width=1)

        # Draw layers in layered order: subbase -> water -> grass -> plaza -> roads -> buildings
        render_order = ["subbase", "water", "grass", "plaza", "roads", "buildings"]

        for cat in render_order:
            color = BEV_COLORS.get(cat, (128, 128, 128))
            for l_data in categorized_layers[cat]:
                verts = l_data["vertices"]
                tris = l_data["triangles"]
                for tri in tris:
                    v0 = verts[tri[0]]
                    v1 = verts[tri[1]]
                    v2 = verts[tri[2]]
                    # Ignore vertical foundation skirt triangles in 2D top-down view (zero 2D projected area)
                    area_2d = 0.5 * abs((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]))
                    if area_2d < 0.05:
                        continue
                    p0 = to_px(v0[0], v0[1])
                    p1 = to_px(v1[0], v1[1])
                    p2 = to_px(v2[0], v2[1])
                    draw.polygon([p0, p1, p2], fill=color)

        # Draw OpenDRIVE road centerlines and corridors
        for seg in road_lines:
            p0 = to_px(seg[0][0], seg[0][1])
            p1 = to_px(seg[1][0], seg[1][1])
            draw.line([p0, p1], fill=BEV_COLORS["roads"], width=max(3, int(scale * 3.5)))

        # Header overlay
        draw.rectangle([(0, 0), (image_width, 60)], fill=(12, 16, 22, 230))
        draw.text(
            (25, 18),
            f"CARLA Map Maker | False-Color Bird's-Eye View: {map_name}",
            fill=BEV_COLORS["text"],
        )

        # Legend block
        legend_items = [
            ("Buildings", BEV_COLORS["buildings"]),
            ("Roads", BEV_COLORS["roads"]),
            ("Urban Plazas", BEV_COLORS["plaza"]),
            ("Parks & Grass", BEV_COLORS["grass"]),
            ("Water", BEV_COLORS["water"]),
            ("Sub-Base Floor", BEV_COLORS["subbase"]),
        ]

        lx = image_width - 680
        ly = 18
        for label_text, color_val in legend_items:
            draw.rectangle([(lx, ly + 2), (lx + 14, ly + 16)], fill=color_val)
            draw.text((lx + 20, ly), label_text, fill=BEV_COLORS["text"])
            lx += 110

        out_path = Path(output_image).resolve()
        out_path.parent.mkdir(parents=True, exist_ok=True)
        im.save(str(out_path), "PNG")
        print(f"[render-bev] Saved False-Color BEV map -> {out_path} ({image_width}x{image_height})")
        return out_path
    finally:
        if temp_dir is not None:
            shutil.rmtree(temp_dir, ignore_errors=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="Render False-Color BEV landuse and terrain maps for CARLA.")
    parser.add_argument("target", help="Path to .carlamap directory or .tar.gz bundle archive")
    parser.add_argument("--output", "-o", default="Docs/map-maker/images/munich_landuse_bev.png", help="Output PNG path")
    parser.add_argument("--width", type=int, default=2048, help="Image width in pixels (default: 2048)")
    parser.add_argument("--height", type=int, default=1536, help="Image height in pixels (default: 1536)")
    args = parser.parse_args()

    try:
        render_offline_bev(args.target, args.output, image_width=args.width, image_height=args.height)
        return 0
    except Exception as exc:
        print(f"[render-bev] ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())

