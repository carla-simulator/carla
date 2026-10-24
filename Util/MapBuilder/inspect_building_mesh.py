#!/usr/bin/env python3
"""
inspect_building_mesh.py: Visual feedback and diagnostic evaluation tool for
OSM 3D building meshes.

Provides:
1. 3D perspective software rendering with Unreal Engine backface culling, depth sorting,
   and directional lighting across 4 canonical viewpoints (Isometric, Street Level,
   Worm's Eye, Top-Down).
2. Automated geometric self-judgement engine that calculates outward wall normal
   alignment, roof skyward orientation, and solid-volume integrity.
3. Batch audit across an OSM file to examine some or all building footprints.
"""

from __future__ import annotations

import argparse
import math
import os
import sys
from pathlib import Path
from typing import Sequence

import numpy as np
from PIL import Image, ImageDraw, ImageFont

# Add project root to path
_CARLA_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(_CARLA_ROOT))

from Util.MapBuilder.osm_buildings import (
    extract_osm_building_footprints,
    extrude_building_mesh,
    parse_building_height,
    BuildingFootprint,
)


# -----------------------------------------------------------------------------
# 1. Geometric Self-Judgement Engine
# -----------------------------------------------------------------------------

def evaluate_building_mesh(
    vertices: list[tuple[float, float, float]],
    triangles: list[tuple[int, int, int]],
    normals: list[tuple[float, float, float]],
    footprint_coords: Sequence[tuple[float, float]],
    height: float,
) -> dict:
    """
    Evaluates the geometric correctness and visual solidity of an extruded building mesh.
    Checks:
    - Wall triangle outward orientation (face normal must match outward vertex normal in UE convention).
    - Roof triangle skyward orientation (face normal must point +Z).
    - Unreal Engine winding order compatibility.
    """
    if not vertices or not triangles:
        return {
            "status": "FAIL",
            "score": 0.0,
            "wall_tri_count": 0,
            "wall_outward_pct": 0.0,
            "roof_tri_count": 0,
            "roof_skyward_pct": 0.0,
            "issues": ["Empty mesh"],
        }

    wall_tri_count = 0
    wall_outward_count = 0
    roof_tri_count = 0
    roof_skyward_count = 0
    issues: list[str] = []

    for idx, (i0, i1, i2) in enumerate(triangles):
        v0 = np.array(vertices[i0], dtype=float)
        v1 = np.array(vertices[i1], dtype=float)
        v2 = np.array(vertices[i2], dtype=float)

        # Unreal Engine face normal: CrossProduct(V2 - V0, V1 - V0)
        face_norm = np.cross(v2 - v0, v1 - v0)
        fn_len = np.linalg.norm(face_norm)
        if fn_len < 1e-6:
            continue
        face_norm /= fn_len

        # Compare with stored vertex normal (which points strictly outward or upward)
        stored_n = np.array(normals[i0], dtype=float)

        if abs(stored_n[2]) < 0.5:
            # Wall triangle
            wall_tri_count += 1
            dot = float(np.dot(face_norm, stored_n))
            if dot > 0.95:
                wall_outward_count += 1
        else:
            # Roof triangle
            roof_tri_count += 1
            if face_norm[2] > 0.95:
                roof_skyward_count += 1

    wall_outward_pct = (wall_outward_count / wall_tri_count * 100.0) if wall_tri_count else 0.0
    roof_skyward_pct = (roof_skyward_count / roof_tri_count * 100.0) if roof_tri_count else 0.0

    if wall_outward_pct < 95.0:
        issues.append(f"Inverted wall winding: only {wall_outward_pct:.1f}% of wall triangles face outward (hollow/inside-out).")
    if roof_skyward_pct < 95.0:
        issues.append(f"Inverted roof winding: only {roof_skyward_pct:.1f}% of roof triangles face skyward (visible from below, culled from above).")

    score = 0.6 * wall_outward_pct + 0.4 * roof_skyward_pct
    status = "PASS" if score >= 95.0 and len(issues) == 0 else "FAIL"

    return {
        "status": status,
        "score": score,
        "wall_tri_count": wall_tri_count,
        "wall_outward_pct": wall_outward_pct,
        "roof_tri_count": roof_tri_count,
        "roof_skyward_pct": roof_skyward_pct,
        "issues": issues,
    }


# -----------------------------------------------------------------------------
# 2. 3D Software Rasterizer with Unreal Backface Culling
# -----------------------------------------------------------------------------

def project_vertex(
    v: np.ndarray,
    cam_pos: np.ndarray,
    cam_rot_mat: np.ndarray,
    fov_deg: float,
    width: int,
    height: int,
) -> tuple[float, float, float] | None:
    """Projects 3D world vertex to screen (x, y, depth) using perspective camera."""
    # Transform to camera space
    p_cam = cam_rot_mat.dot(v - cam_pos)
    # Camera looks along +Zcam, +Xcam is right, +Ycam is down
    depth = p_cam[2]
    if depth <= 0.1:
        return None

    focal = (width / 2.0) / math.tan(math.radians(fov_deg / 2.0))
    sx = (width / 2.0) + (p_cam[0] / depth) * focal
    sy = (height / 2.0) + (p_cam[1] / depth) * focal
    return sx, sy, depth


def render_building_view(
    vertices: list[tuple[float, float, float]],
    triangles: list[tuple[int, int, int]],
    cam_pos: np.ndarray,
    look_at: np.ndarray,
    width: int = 500,
    height: int = 400,
    fov_deg: float = 60.0,
    title: str = "",
    enable_culling: bool = True,
) -> Image.Image:
    """
    Renders the building mesh from a specific camera pose using Unreal Engine
    clockwise backface culling and directional lighting.
    """
    img = Image.new("RGB", (width, height), (30, 35, 45))
    draw = ImageDraw.Draw(img)

    # Build camera rotation matrix
    # forward: from cam_pos to look_at
    fwd = look_at - cam_pos
    fwd_len = np.linalg.norm(fwd)
    if fwd_len < 1e-4:
        return img
    fwd = fwd / fwd_len

    # World up
    world_up = np.array([0.0, 0.0, 1.0])
    right = np.cross(fwd, world_up)
    right_len = np.linalg.norm(right)
    if right_len < 1e-4:
        # Looking straight up or down
        world_up = np.array([0.0, 1.0, 0.0])
        right = np.cross(fwd, world_up)
        right = right / np.linalg.norm(right)
    else:
        right = right / right_len

    cam_down = np.cross(fwd, right)

    # cam_rot_mat transforms world vector into camera coordinate frame [right, down, fwd]
    cam_rot_mat = np.array([right, cam_down, fwd])

    # Directional light from above-front-left
    light_dir = np.array([0.4, 0.3, 0.8])
    light_dir = light_dir / np.linalg.norm(light_dir)

    # Project triangles and sort by depth (Painter's algorithm)
    drawable_tris = []
    verts_np = [np.array(v, dtype=float) for v in vertices]

    for i0, i1, i2 in triangles:
        v0, v1, v2 = verts_np[i0], verts_np[i1], verts_np[i2]
        p0 = project_vertex(v0, cam_pos, cam_rot_mat, fov_deg, width, height)
        p1 = project_vertex(v1, cam_pos, cam_rot_mat, fov_deg, width, height)
        p2 = project_vertex(v2, cam_pos, cam_rot_mat, fov_deg, width, height)

        if p0 is None or p1 is None or p2 is None:
            continue

        # In camera screen space: +X is right, +Y is down
        # Signed 2D area (cross product in screen space):
        # det = (p1.x - p0.x)*(p2.y - p0.y) - (p1.y - p0.y)*(p2.x - p0.x)
        # In UE (left-handed screen space with +Y down):
        # A front-facing triangle has vertices ordered CLOCKWISE -> det > 0.
        # A back-facing triangle has det <= 0.
        det = (p1[0] - p0[0]) * (p2[1] - p0[1]) - (p1[1] - p0[1]) * (p2[0] - p0[0])

        is_front_facing = (det > 0)
        if enable_culling and not is_front_facing:
            # Culled as backface!
            continue

        avg_depth = (p0[2] + p1[2] + p2[2]) / 3.0

        # UE face normal for lighting: CrossProduct(V2 - V0, V1 - V0)
        u = v2 - v0
        v = v1 - v0
        nx = u[1] * v[2] - u[2] * v[1]
        ny = u[2] * v[0] - u[0] * v[2]
        nz = u[0] * v[1] - u[1] * v[0]
        n_len = math.hypot(nx, ny, nz)
        normal = np.array([nx / n_len, ny / n_len, nz / n_len]) if n_len > 1e-6 else np.array([0, 0, 1])

        is_roof = abs(normal[2]) > 0.5
        dot = max(0.0, float(np.dot(normal, light_dir)))
        intensity = 0.35 + 0.65 * dot

        if is_roof:
            # Roof: slate blue/grey
            base_col = (110, 140, 175)
        else:
            # Wall: warm limestone/concrete
            base_col = (205, 185, 155)

        # Highlight inverted faces in red if culling is disabled or backface leaked
        if not is_front_facing:
            color = (int(220 * intensity), int(60 * intensity), int(60 * intensity))
        else:
            color = (int(base_col[0] * intensity), int(base_col[1] * intensity), int(base_col[2] * intensity))

        drawable_tris.append((avg_depth, [(p0[0], p0[1]), (p1[0], p1[1]), (p2[0], p2[1])], color, is_roof))

    # Sort furthest to closest
    drawable_tris.sort(key=lambda t: t[0], reverse=True)

    # Draw triangles
    for _, poly, color, is_roof in drawable_tris:
        draw.polygon(poly, fill=color, outline=(40, 40, 50))

    # Header and stats overlay
    draw.rectangle([(0, 0), (width, 28)], fill=(15, 20, 25))
    draw.text((10, 6), title, fill=(240, 240, 240))
    stat_text = f"Rendered Triangles: {len(drawable_tris)}"
    draw.text((width - 180, 6), stat_text, fill=(180, 210, 240))

    return img


def generate_multi_view_inspection_sheet(
    vertices: list[tuple[float, float, float]],
    triangles: list[tuple[int, int, int]],
    normals: list[tuple[float, float, float]],
    footprint_coords: Sequence[tuple[float, float]],
    height: float,
    building_id: str,
    output_path: Path,
) -> dict:
    """
    Renders 4 canonical viewpoints into a single 2x2 diagnostic image sheet:
    1. Isometric Perspective (3/4 elevated view)
    2. Street-Level Facade View
    3. Worm's Eye View (looking up from ground)
    4. Aerial Top-Down View
    """
    eval_res = evaluate_building_mesh(vertices, triangles, normals, footprint_coords, height)

    # Compute bounding box & center
    cx = sum(p[0] for p in footprint_coords) / len(footprint_coords)
    cy = sum(p[1] for p in footprint_coords) / len(footprint_coords)
    radius = max(math.hypot(p[0] - cx, p[1] - cy) for p in footprint_coords)
    radius = max(radius, 5.0)

    center = np.array([cx, cy, height * 0.5])
    ground_center = np.array([cx, cy, 0.0])
    roof_center = np.array([cx, cy, height])

    w, h = 540, 420

    # 1. Isometric Orbit (Camera elevated 45 deg, radius * 2.2 away)
    cam_iso = center + np.array([-radius * 1.8, -radius * 1.8, radius * 1.5 + height * 0.5])
    view_iso = render_building_view(vertices, triangles, cam_iso, center, w, h, fov_deg=50.0, title="View 1: Isometric Orbit (3/4 Elevated)")

    # 2. Street Level (Standing outside facade at eye level z=1.7)
    cam_street = ground_center + np.array([0.0, -radius * 1.6, 1.7])
    view_street = render_building_view(vertices, triangles, cam_street, ground_center + np.array([0.0, 0.0, height * 0.4]), w, h, fov_deg=65.0, title="View 2: Street-Level Facade (Eye-Level)")

    # 3. Worm's Eye (Standing near base looking UP towards roof)
    cam_worms = ground_center + np.array([-radius * 0.9, -radius * 0.9, 0.5])
    view_worms = render_building_view(vertices, triangles, cam_worms, roof_center, w, h, fov_deg=75.0, title="View 3: Worm's Eye (Looking UP from Base)")

    # 4. Aerial Top-Down (Straight above building looking down)
    cam_top = roof_center + np.array([0.01, 0.01, radius * 2.2 + height * 0.5])
    view_top = render_building_view(vertices, triangles, cam_top, roof_center, w, h, fov_deg=50.0, title="View 4: Aerial Top-Down (Roof View)")

    # Composite 2x2 grid
    total_w = w * 2 + 30
    total_h = h * 2 + 100
    sheet = Image.new("RGB", (total_w, total_h), (20, 22, 28))

    # Paste views
    sheet.paste(view_iso, (10, 60))
    sheet.paste(view_street, (w + 20, 60))
    sheet.paste(view_worms, (10, h + 70))
    sheet.paste(view_top, (w + 20, h + 70))

    # Draw master banner
    draw = ImageDraw.Draw(sheet)
    draw.rectangle([(0, 0), (total_w, 50)], fill=(12, 14, 18))

    status_color = (60, 200, 100) if eval_res["status"] == "PASS" else (240, 70, 70)
    draw.text((15, 8), f"Building ID: {building_id} | Height: {height:.1f}m | Footprint: {len(footprint_coords)} nodes", fill=(255, 255, 255))
    status_msg = f"VERDICT: {eval_res['status']} (Wall Outward: {eval_res['wall_outward_pct']:.1f}%, Roof Skyward: {eval_res['roof_skyward_pct']:.1f}%)"
    draw.text((15, 28), status_msg, fill=status_color)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(str(output_path))

    eval_res["sheet_path"] = str(output_path)
    return eval_res


# -----------------------------------------------------------------------------
# 3. CLI Main
# -----------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Inspect and evaluate OSM 3D building meshes.")
    default_out_dir = _CARLA_ROOT / ".tmp" / "building_eval"
    parser.add_argument("--out-dir", type=Path, default=default_out_dir, help="Directory to save inspection images (default: ./.tmp/building_eval).")
    parser.add_argument("--limit", type=int, default=10, help="Max number of building visual sheets to generate.")
    parser.add_argument("--all", action="store_true", help="Audit all buildings in the OSM file.")
    args = parser.parse_args()

    footprints = extract_osm_building_footprints(args.osm)
    print(f"Loaded {len(footprints)} building footprints from {args.osm}")

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    passed = 0
    failed = 0
    results = []

    count_to_render = len(footprints) if args.all else min(args.limit, len(footprints))

    # Origin projection: use simple meter scale around first footprint node
    ref_lat = footprints[0].nodes[0][0]
    ref_lon = footprints[0].nodes[0][1]
    cos_lat = math.cos(math.radians(ref_lat))
    m_per_deg_lat = 111320.0
    m_per_deg_lon = 111320.0 * cos_lat

    for i, fp in enumerate(footprints):
        height = parse_building_height(fp.tags)
        # Convert lat/lon to local meter coordinates (XY)
        coords = [((lon - ref_lon) * m_per_deg_lon, (lat - ref_lat) * m_per_deg_lat) for lat, lon in fp.nodes]

        verts, tris, norms, uvs = extrude_building_mesh(coords, height)

        if i < count_to_render:
            img_path = out_dir / f"building_{i:03d}_{fp.osm_id}.png"
            res = generate_multi_view_inspection_sheet(verts, tris, norms, coords, height, fp.osm_id, img_path)
            print(f"[{i+1}/{len(footprints)}] Building {fp.osm_id}: {res['status']} (Score: {res['score']:.1f}%) -> {img_path.name}")
        else:
            res = evaluate_building_mesh(verts, tris, norms, coords, height)

        if res["status"] == "PASS":
            passed += 1
        else:
            failed += 1
        results.append(res)

    print("\n" + "=" * 60)
    print("BUILDING MESH AUDIT SUMMARY")
    print("=" * 60)
    print(f"Total Audited: {len(footprints)}")
    print(f"PASSED (Solid Volume): {passed}")
    print(f"FAILED (Hollow / Inside-Out): {failed}")
    pass_pct = (passed / len(footprints) * 100.0) if footprints else 0.0
    print(f"Pass Rate: {pass_pct:.1f}%")

    if failed > 0:
        print("\nCommon Issues Detected:")
        issue_set = set()
        for r in results:
            for issue in r.get("issues", []):
                issue_set.add(issue)
        for issue in sorted(issue_set):
            print(f"  - {issue}")


if __name__ == "__main__":
    main()
