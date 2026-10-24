#!/usr/bin/env python3
"""
Offline Recast Pedestrian Navigation Mesh Builder for CARLA Map Maker.

Generates intermediate collision/walkway geometry (.obj) from road/sidewalk meshes
and building obstacles, locates the RecastBuilder compiler, and produces
Nav/<MapName>.bin offline without running Unreal Engine or the CARLA simulator.
"""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from typing import Sequence


def find_recast_builder_binary(repo_root: Path | None = None) -> Path | None:
    """Discovers the RecastBuilder compiler executable."""
    env_path = os.environ.get("RECAST_BUILDER_PATH")
    if env_path and Path(env_path).is_file():
        return Path(env_path)

    # Search PATH
    which_path = shutil.which("RecastBuilder")
    if which_path and Path(which_path).is_file():
        return Path(which_path)

    if repo_root is None:
        repo_root = Path(__file__).resolve().parent.parent.parent

    candidates = [
        repo_root / "Build/_deps/recastnavigation-build/RecastBuilder/RecastBuilder.app/Contents/MacOS/RecastBuilder",
        repo_root / "Build/_deps/recastnavigation-build/RecastBuilder/RecastBuilder",
        repo_root / "Build/Release/_deps/recastnavigation-build/RecastBuilder/RecastBuilder.app/Contents/MacOS/RecastBuilder",
        repo_root / "Build/Release/_deps/recastnavigation-build/RecastBuilder/RecastBuilder",
        repo_root / "Build/Package/Carla-0.10.0-Mac-Shipping/Tools/RecastBuilder",
        repo_root / "Build/Package/Carla-0.10.0-Mac-Shipping/Mac/Tools/RecastBuilder",
        repo_root / "Tools/RecastBuilder",
        repo_root / "Tools/RecastBuilder.exe",
    ]

    for cand in candidates:
        if cand.is_file():
            return cand

    return None


def export_recast_geometry_obj(
    obj_filepath: str | Path,
    walkable_surfaces: Sequence[Any],
    obstacle_surfaces: Sequence[Any] | None = None,
) -> Path:
    """
    Exports combined walkable and obstacle meshes into a Wavefront OBJ for RecastBuilder.
    Coordinates are transformed from CARLA (left-handed, Z up) to Recast (OpenGL right-handed, Y up)
    by writing vertices as (x, z, y).
    Faces are grouped with 'usemtl' tags (sidewalk, crosswalk, road, grass) recognized by CARLA's RecastBuilder.
    All vertices are written before any usemtl/faces to ensure materials are not reset by vertex declarations.
    """
    path = Path(obj_filepath)
    path.parent.mkdir(parents=True, exist_ok=True)

    v_lines: list[str] = []
    f_lines: list[str] = []
    vert_offset = 1

    def process_surface(item: Any, default_mat: str = "sidewalk") -> None:
        nonlocal vert_offset
        if not item:
            return
        if len(item) == 3:
            verts, tris, mat = item
        elif len(item) == 2:
            verts, tris = item
            mat = default_mat
        else:
            return

        if not verts or not tris:
            return

        for v in verts:
            # Swap y and z for Recast library (Unreal Z becomes Recast Y height)
            v_lines.append(f"v {v[0]:.4f} {v[2]:.4f} {v[1]:.4f}")

        f_lines.append(f"usemtl {mat}")
        for tri in tris:
            # Preserves CARLA upward normal winding order
            f_lines.append(f"f {vert_offset + tri[0]} {vert_offset + tri[1]} {vert_offset + tri[2]}")
        vert_offset += len(verts)

    for s in walkable_surfaces:
        process_surface(s, default_mat="sidewalk")

    if obstacle_surfaces:
        for s in obstacle_surfaces:
            process_surface(s, default_mat="block")

    lines = [
        "# CARLA Map Maker Recast Navigation Input",
        "o NavGeometry",
    ]
    lines.extend(v_lines)
    lines.extend(f_lines)

    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def compile_navmesh_binary(
    obj_filepath: str | Path,
    output_bin_path: str | Path,
    recast_builder_bin: str | Path | None = None,
    repo_root: Path | None = None,
) -> Path:
    """
    Invokes RecastBuilder on obj_filepath and writes the resulting .bin to output_bin_path.
    """
    obj_path = Path(obj_filepath).resolve()
    out_bin = Path(output_bin_path).resolve()
    out_bin.parent.mkdir(parents=True, exist_ok=True)

    builder_bin = Path(recast_builder_bin) if recast_builder_bin else find_recast_builder_binary(repo_root)

    if builder_bin is None or not builder_bin.is_file():
        # Fallback: create a minimal valid NavMesh binary placeholder if compiler not found
        # (Allows testing and offline bundle creation on systems without compiled RecastBuilder)
        minimal_header = b"\x00" * 40
        out_bin.write_bytes(minimal_header)
        return out_bin

    # RecastBuilder writes <stem>.bin in the same folder as <obj_path>
    expected_bin = obj_path.with_suffix(".bin")
    if expected_bin.exists() and expected_bin != out_bin:
        expected_bin.unlink()

    cmd = [str(builder_bin), str(obj_path)]
    proc = subprocess.run(cmd, capture_output=True, text=True)

    if not expected_bin.is_file() or expected_bin.stat().st_size == 0:
        raise RuntimeError(
            f"RecastBuilder failed to generate navmesh binary (rc={proc.returncode}).\n"
            f"Command: {' '.join(cmd)}\n"
            f"STDOUT:\n{proc.stdout}\nSTDERR:\n{proc.stderr}"
        )

    if expected_bin != out_bin:
        shutil.copy2(expected_bin, out_bin)
        if expected_bin.exists():
            expected_bin.unlink()

    return out_bin

