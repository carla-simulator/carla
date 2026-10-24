#!/usr/bin/env python3
"""
Multi-Format 3D Mesh Importer for CARLA Map Maker.

Supports Wavefront OBJ (.obj) and glTF 2.0 (.gltf, .glb) models.
Parses vertices, normals, UVs, and vertex colors, performs coordinate
transformations, splits sub-meshes by semantic naming or material rules
(Roads, Sidewalks, Buildings, Terrain, Walls, etc.), and packs them into
binary .cmesh layers with Chaos physics collision and ground-truth semantics.
"""

from __future__ import annotations

import fnmatch
import json
import math
from pathlib import Path
import re
import struct
from typing import Any, Sequence

try:
    from Util.MapBuilder.cmesh import write_cmesh_file
except ImportError:
    from cmesh import write_cmesh_file


class MeshSubpart:
    """Represents a discrete sub-mesh extracted from a 3D model."""

    def __init__(self, name: str, material: str = ""):
        self.name = name
        self.material = material
        self.vertices: list[tuple[float, float, float]] = []
        self.triangles: list[tuple[int, int, int]] = []
        self.normals: list[tuple[float, float, float]] = []
        self.uvs: list[tuple[float, float]] = []
        self.colors: list[tuple[int, int, int, int]] = []
        self.semantic_label: str = "Roads"
        self.material_preset: str = "road_asphalt"
        self.enable_collision: bool = True


DEFAULT_SEMANTIC_RULES = [
    {"pattern": "*road*", "label": "Roads", "material": "road_asphalt", "collision": True},
    {"pattern": "*street*", "label": "Roads", "material": "road_asphalt", "collision": True},
    {"pattern": "*asphalt*", "label": "Roads", "material": "road_asphalt", "collision": True},
    {"pattern": "*highway*", "label": "Roads", "material": "road_asphalt", "collision": True},
    {"pattern": "*sidewalk*", "label": "Sidewalks", "material": "sidewalk_concrete", "collision": True},
    {"pattern": "*walkway*", "label": "Sidewalks", "material": "sidewalk_concrete", "collision": True},
    {"pattern": "*pavement*", "label": "Sidewalks", "material": "sidewalk_concrete", "collision": True},
    {"pattern": "*curb*", "label": "Sidewalks", "material": "sidewalk_concrete", "collision": True},
    {"pattern": "*building*", "label": "Buildings", "material": "building_brick", "collision": True},
    {"pattern": "*house*", "label": "Buildings", "material": "building_brick", "collision": True},
    {"pattern": "*facade*", "label": "Buildings", "material": "building_concrete", "collision": True},
    {"pattern": "*wall*", "label": "Walls", "material": "wall_concrete", "collision": True},
    {"pattern": "*fence*", "label": "Fences", "material": "wall_concrete", "collision": True},
    {"pattern": "*barrier*", "label": "GuardRail", "material": "metal", "collision": True},
    {"pattern": "*guardrail*", "label": "GuardRail", "material": "metal", "collision": True},
    {"pattern": "*pole*", "label": "Poles", "material": "metal", "collision": True},
    {"pattern": "*lamp*", "label": "Poles", "material": "metal", "collision": True},
    {"pattern": "*tree*", "label": "Vegetation", "material": "wood", "collision": True},
    {"pattern": "*foliage*", "label": "Vegetation", "material": "grass", "collision": False},
    {"pattern": "*veg*", "label": "Vegetation", "material": "grass", "collision": False},
    {"pattern": "*terrain*", "label": "Terrain", "material": "grass", "collision": True},
    {"pattern": "*ground*", "label": "Terrain", "material": "grass", "collision": True},
    {"pattern": "*grass*", "label": "Terrain", "material": "grass", "collision": True},
    {"pattern": "*dirt*", "label": "Terrain", "material": "dirt", "collision": True},
]


def classify_submesh(
    name: str,
    material: str = "",
    custom_rules: Sequence[dict[str, Any]] | None = None,
) -> tuple[str, str, bool]:
    """Matches a sub-mesh name/material against semantic rules to determine label, material, and collision."""
    rules = list(custom_rules or []) + DEFAULT_SEMANTIC_RULES

    combined_key = f"{name} {material}".lower()
    for rule in rules:
        pat = rule.get("pattern", "").lower()
        if fnmatch.fnmatch(name.lower(), pat) or fnmatch.fnmatch(material.lower(), pat) or fnmatch.fnmatch(combined_key, pat):
            label = rule.get("label", "Roads")
            mat_preset = rule.get("material", "road_asphalt")
            col = bool(rule.get("collision", True))
            return label, mat_preset, col

    return "Roads", "road_asphalt", True


def parse_obj_file(
    filepath: str | Path,
    scale: float = 1.0,
    swap_yz: bool = False,
    flip_winding: bool = False,
) -> list[MeshSubpart]:
    """
    Parses a Wavefront OBJ file, splitting sub-meshes by object 'o', group 'g', or material 'usemtl'.
    """
    path = Path(filepath)
    raw_verts: list[tuple[float, float, float]] = []
    raw_norms: list[tuple[float, float, float]] = []
    raw_uvs: list[tuple[float, float]] = []

    # Map: subpart_key -> MeshSubpart
    subparts: dict[str, MeshSubpart] = {}
    current_part_name = path.stem
    current_mat = ""
    current_subpart = MeshSubpart(current_part_name, current_mat)
    subparts[current_part_name] = current_subpart

    # Deduplication map per subpart: (v_idx, vt_idx, vn_idx) -> local_vert_idx
    dedup_maps: dict[str, dict[tuple[int, int | None, int | None], int]] = {current_part_name: {}}

    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()

    for line in lines:
        line = line.strip()
        if not line or line.startswith("#"):
            continue

        tokens = line.split()
        tag = tokens[0]

        if tag == "v" and len(tokens) >= 4:
            x, y, z = float(tokens[1]) * scale, float(tokens[2]) * scale, float(tokens[3]) * scale
            if swap_yz:
                y, z = z, y
            raw_verts.append((x, y, z))

        elif tag == "vn" and len(tokens) >= 4:
            nx, ny, nz = float(tokens[1]), float(tokens[2]), float(tokens[3])
            if swap_yz:
                ny, nz = nz, ny
            raw_norms.append((nx, ny, nz))

        elif tag == "vt" and len(tokens) >= 3:
            u, v = float(tokens[1]), float(tokens[2])
            raw_uvs.append((u, v))

        elif tag in ("o", "g"):
            new_name = tokens[1] if len(tokens) > 1 else path.stem
            current_part_name = new_name
            if current_part_name not in subparts:
                subparts[current_part_name] = MeshSubpart(current_part_name, current_mat)
                dedup_maps[current_part_name] = {}
            current_subpart = subparts[current_part_name]

        elif tag == "usemtl" and len(tokens) > 1:
            current_mat = tokens[1]
            # Optionally split by material if object didn't split
            if len(current_subpart.triangles) > 0:
                mat_part_name = f"{current_part_name}_{current_mat}"
                current_part_name = mat_part_name
                if current_part_name not in subparts:
                    subparts[current_part_name] = MeshSubpart(current_part_name, current_mat)
                    dedup_maps[current_part_name] = {}
                current_subpart = subparts[current_part_name]
            else:
                current_subpart.material = current_mat

        elif tag == "f" and len(tokens) >= 4:
            face_verts: list[tuple[int, int | None, int | None]] = []
            for face_tok in tokens[1:]:
                parts = face_tok.split("/")
                v_idx = int(parts[0]) - 1 if parts[0] else 0
                if v_idx < 0:
                    v_idx = len(raw_verts) + v_idx + 1

                vt_idx = int(parts[1]) - 1 if len(parts) > 1 and parts[1] else None
                if vt_idx is not None and vt_idx < 0:
                    vt_idx = len(raw_uvs) + vt_idx + 1

                vn_idx = int(parts[2]) - 1 if len(parts) > 2 and parts[2] else None
                if vn_idx is not None and vn_idx < 0:
                    vn_idx = len(raw_norms) + vn_idx + 1

                face_verts.append((v_idx, vt_idx, vn_idx))

            # Fan triangulation for polygons/quads
            dedup = dedup_maps[current_part_name]
            for i in range(1, len(face_verts) - 1):
                poly_indices = [face_verts[0], face_verts[i], face_verts[i + 1]]
                if flip_winding:
                    poly_indices[1], poly_indices[2] = poly_indices[2], poly_indices[1]

                tri_indices: list[int] = []
                for vert_key in poly_indices:
                    if vert_key in dedup:
                        tri_indices.append(dedup[vert_key])
                    else:
                        new_local_idx = len(current_subpart.vertices)
                        v_i, vt_i, vn_i = vert_key
                        current_subpart.vertices.append(raw_verts[v_i])

                        if vn_i is not None and vn_i < len(raw_norms):
                            current_subpart.normals.append(raw_norms[vn_i])
                        else:
                            current_subpart.normals.append((0.0, 0.0, 1.0))

                        if vt_i is not None and vt_i < len(raw_uvs):
                            current_subpart.uvs.append(raw_uvs[vt_i])
                        else:
                            current_subpart.uvs.append((0.0, 0.0))

                        dedup[vert_key] = new_local_idx
                        tri_indices.append(new_local_idx)

                current_subpart.triangles.append((tri_indices[0], tri_indices[1], tri_indices[2]))

    # Filter out empty subparts
    result = [sp for sp in subparts.values() if len(sp.vertices) > 0 and len(sp.triangles) > 0]
    return result


def parse_gltf_file(
    filepath: str | Path,
    scale: float = 1.0,
) -> list[MeshSubpart]:
    """
    Parses a glTF 2.0 (.gltf or .glb) file, extracting primitives into MeshSubparts.
    """
    path = Path(filepath)
    is_glb = path.suffix.lower() == ".glb"

    data = path.read_bytes()
    bin_buffers: list[bytes] = []

    if is_glb:
        if len(data) < 12:
            raise ValueError(f"GLB file too short: {len(data)} bytes")
        magic, version, length = struct.unpack("<III", data[:12])
        if magic != 0x46546C67:  # 'glTF'
            raise ValueError(f"Invalid GLB magic: 0x{magic:08X}")

        offset = 12
        json_bytes = b""
        while offset < length:
            chunk_len, chunk_type = struct.unpack("<II", data[offset : offset + 8])
            offset += 8
            chunk_data = data[offset : offset + chunk_len]
            offset += chunk_len
            if chunk_type == 0x4E4F534A:  # 'JSON'
                json_bytes = chunk_data
            elif chunk_type == 0x004E4942:  # 'BIN'
                bin_buffers.append(chunk_data)

        gltf_dict = json.loads(json_bytes.decode("utf-8"))
    else:
        gltf_dict = json.loads(data.decode("utf-8"))
        for buf_info in gltf_dict.get("buffers", []):
            uri = buf_info.get("uri", "")
            if uri.startswith("data:"):
                import base64
                b64_str = uri.split(",", 1)[1]
                bin_buffers.append(base64.b64decode(b64_str))
            else:
                bin_buffers.append((path.parent / uri).read_bytes())

    buffer_views = gltf_dict.get("bufferViews", [])
    accessors = gltf_dict.get("accessors", [])

    def get_accessor_data(acc_idx: int) -> tuple[bytes, int, str, int]:
        acc = accessors[acc_idx]
        bv = buffer_views[acc["bufferView"]]
        buf_data = bin_buffers[bv.get("buffer", 0)]
        start = bv.get("byteOffset", 0) + acc.get("byteOffset", 0)
        count = acc["count"]
        comp_type = acc["componentType"]
        type_str = acc["type"]
        return buf_data[start:], count, type_str, comp_type

    subparts: list[MeshSubpart] = []

    for mesh_idx, mesh in enumerate(gltf_dict.get("meshes", [])):
        mesh_name = mesh.get("name", f"Mesh_{mesh_idx}")
        for prim_idx, prim in enumerate(mesh.get("primitives", [])):
            subpart_name = f"{mesh_name}_{prim_idx}" if len(mesh["primitives"]) > 1 else mesh_name
            subpart = MeshSubpart(subpart_name)

            attrs = prim.get("attributes", {})
            if "POSITION" not in attrs:
                continue

            # Read Positions
            pos_buf, count, _, _ = get_accessor_data(attrs["POSITION"])
            flat_pos = struct.unpack(f"<{count * 3}f", pos_buf[: count * 12])
            subpart.vertices = [(flat_pos[3 * i] * scale, flat_pos[3 * i + 1] * scale, flat_pos[3 * i + 2] * scale) for i in range(count)]

            # Read Normals
            if "NORMAL" in attrs:
                norm_buf, n_count, _, _ = get_accessor_data(attrs["NORMAL"])
                flat_norm = struct.unpack(f"<{n_count * 3}f", norm_buf[: n_count * 12])
                subpart.normals = [(flat_norm[3 * i], flat_norm[3 * i + 1], flat_norm[3 * i + 2]) for i in range(n_count)]

            # Read UVs
            if "TEXCOORD_0" in attrs:
                uv_buf, uv_count, _, _ = get_accessor_data(attrs["TEXCOORD_0"])
                flat_uv = struct.unpack(f"<{uv_count * 2}f", uv_buf[: uv_count * 8])
                subpart.uvs = [(flat_uv[2 * i], flat_uv[2 * i + 1]) for i in range(uv_count)]

            # Read Indices
            if "indices" in prim:
                idx_buf, idx_count, _, comp_type = get_accessor_data(prim["indices"])
                if comp_type == 5123:  # UNSIGNED_SHORT
                    indices = struct.unpack(f"<{idx_count}H", idx_buf[: idx_count * 2])
                elif comp_type == 5125:  # UNSIGNED_INT
                    indices = struct.unpack(f"<{idx_count}I", idx_buf[: idx_count * 4])
                else:
                    indices = struct.unpack(f"<{idx_count}B", idx_buf[:idx_count])
            else:
                indices = tuple(range(count))

            subpart.triangles = [(int(indices[3 * i]), int(indices[3 * i + 1]), int(indices[3 * i + 2])) for i in range(len(indices) // 3)]
            subparts.append(subpart)

    return subparts


def import_and_pack_custom_meshes(
    model_paths: Sequence[str | Path],
    output_dir: str | Path,
    custom_rules: Sequence[dict[str, Any]] | None = None,
    scale: float = 1.0,
) -> list[dict[str, Any]]:
    """
    Imports all 3D mesh files (.obj, .gltf, .glb), splits sub-meshes, applies
    semantic classification, writes .cmesh binary files to output_dir, and returns
    layer declarations suitable for carla-map.json.
    """
    out_path = Path(output_dir)
    out_path.mkdir(parents=True, exist_ok=True)

    layer_declarations: list[dict[str, Any]] = []

    for m_path in model_paths:
        path = Path(m_path)
        ext = path.suffix.lower()

        subparts: list[MeshSubpart] = []
        if ext == ".obj":
            subparts = parse_obj_file(path, scale=scale)
        elif ext in (".gltf", ".glb"):
            subparts = parse_gltf_file(path, scale=scale)
        else:
            raise ValueError(f"Unsupported 3D mesh format: '{ext}'. Supported: .obj, .gltf, .glb")

        for sp in subparts:
            label, mat_preset, collision = classify_submesh(sp.name, sp.material, custom_rules)
            sp.semantic_label = label
            sp.material_preset = mat_preset
            sp.enable_collision = collision

            clean_layer_name = re.sub(r"[^\w\-.]", "_", sp.name)
            cmesh_filename = f"{clean_layer_name}.cmesh"
            cmesh_filepath = out_path / cmesh_filename

            write_cmesh_file(
                cmesh_filepath,
                vertices=sp.vertices,
                triangles=sp.triangles,
                normals=sp.normals if sp.normals else None,
                uvs=sp.uvs if sp.uvs else None,
            )

            layer_declarations.append({
                "name": clean_layer_name,
                "mesh_file": f"Meshes/{cmesh_filename}",
                "semantic_label": label,
                "material_preset": mat_preset,
                "enable_collision": collision,
            })

    return layer_declarations
