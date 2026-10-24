#!/usr/bin/env python3
"""
Binary .cmesh format encoder, decoder, and validator for CARLA Map Maker.

Format specification (20-byte little-endian header + packed binary data):
  Magic:              uint32 (0x53454D43, 'CMES')
  Version:            uint32 (1)
  VertexCount:        uint32 (N_v)
  IndexCount:         uint32 (N_t, multiple of 3)
  Flags:              uint32 (bit 0: HasNormals, bit 1: HasUVs, bit 2: HasColors)
  Vertices:           float32[N_v * 3] (meters: X right, Y forward, Z up)
  Normals (opt):      float32[N_v * 3]
  UV0 (opt):          float32[N_v * 2]
  Colors (opt):       uint8[N_v * 4] (RGBA)
  Triangles:          uint32[N_t] (0-based indices)
"""

from __future__ import annotations

import os
from pathlib import Path
import struct
from typing import Any, Sequence

CMESH_MAGIC = 0x53454D43  # 'C' | ('M' << 8) | ('E' << 16) | ('S' << 24)
CMESH_VERSION_1 = 1
CMESH_HEADER_SIZE = 20

CMESH_FLAG_HAS_NORMALS = 1 << 0
CMESH_FLAG_HAS_UVS = 1 << 1
CMESH_FLAG_HAS_COLORS = 1 << 2


def pack_cmesh(
    vertices: Sequence[tuple[float, float, float] | list[float]],
    triangles: Sequence[int | tuple[int, int, int] | list[int]],
    normals: Sequence[tuple[float, float, float] | list[float]] | None = None,
    uvs: Sequence[tuple[float, float] | list[float]] | None = None,
    colors: Sequence[tuple[int, int, int, int] | list[int]] | None = None,
) -> bytes:
    """Packs 3D mesh arrays into a binary .cmesh buffer."""
    vertex_count = len(vertices)
    if vertex_count == 0:
        raise ValueError("Cannot pack empty mesh (vertex_count == 0)")

    # Flatten triangles
    flat_indices: list[int] = []
    if triangles and isinstance(triangles[0], (list, tuple)):
        for tri in triangles:
            if len(tri) != 3:
                raise ValueError(f"Triangle tuple must have length 3, got {len(tri)}")
            flat_indices.extend(tri)
    else:
        flat_indices = [int(idx) for idx in triangles]

    index_count = len(flat_indices)
    if index_count == 0 or (index_count % 3) != 0:
        raise ValueError(f"IndexCount must be non-zero multiple of 3, got {index_count}")

    # Validate indices
    for idx in flat_indices:
        if idx < 0 or idx >= vertex_count:
            raise ValueError(f"Index {idx} out of range [0, {vertex_count - 1}]")

    flags = 0
    if normals is not None and len(normals) > 0:
        if len(normals) != vertex_count:
            raise ValueError(f"Normals count ({len(normals)}) must match vertex count ({vertex_count})")
        flags |= CMESH_FLAG_HAS_NORMALS

    if uvs is not None and len(uvs) > 0:
        if len(uvs) != vertex_count:
            raise ValueError(f"UVs count ({len(uvs)}) must match vertex count ({vertex_count})")
        flags |= CMESH_FLAG_HAS_UVS

    if colors is not None and len(colors) > 0:
        if len(colors) != vertex_count:
            raise ValueError(f"Colors count ({len(colors)}) must match vertex count ({vertex_count})")
        flags |= CMESH_FLAG_HAS_COLORS

    header = struct.pack(
        "<IIIII",
        CMESH_MAGIC,
        CMESH_VERSION_1,
        vertex_count,
        index_count,
        flags,
    )

    chunks = [header]

    # Vertices
    flat_verts: list[float] = []
    for v in vertices:
        flat_verts.extend((float(v[0]), float(v[1]), float(v[2])))
    chunks.append(struct.pack(f"<{len(flat_verts)}f", *flat_verts))

    # Normals
    if flags & CMESH_FLAG_HAS_NORMALS and normals is not None:
        flat_norms: list[float] = []
        for n in normals:
            flat_norms.extend((float(n[0]), float(n[1]), float(n[2])))
        chunks.append(struct.pack(f"<{len(flat_norms)}f", *flat_norms))

    # UVs
    if flags & CMESH_FLAG_HAS_UVS and uvs is not None:
        flat_uvs: list[float] = []
        for uv in uvs:
            flat_uvs.extend((float(uv[0]), float(uv[1])))
        chunks.append(struct.pack(f"<{len(flat_uvs)}f", *flat_uvs))

    # Colors
    if flags & CMESH_FLAG_HAS_COLORS and colors is not None:
        flat_cols: list[int] = []
        for c in colors:
            flat_cols.extend((int(c[0]) & 0xFF, int(c[1]) & 0xFF, int(c[2]) & 0xFF, int(c[3]) & 0xFF))
        chunks.append(struct.pack(f"<{len(flat_cols)}B", *flat_cols))

    # Triangle indices
    chunks.append(struct.pack(f"<{index_count}I", *flat_indices))

    return b"".join(chunks)


def unpack_cmesh(data: bytes) -> dict[str, Any]:
    """Unpacks and returns decoded geometry from binary .cmesh buffer."""
    if len(data) < CMESH_HEADER_SIZE:
        raise ValueError(f"CMesh buffer too short: {len(data)} bytes (expected >= {CMESH_HEADER_SIZE})")

    magic, version, vertex_count, index_count, flags = struct.unpack(
        "<IIIII", data[:CMESH_HEADER_SIZE]
    )

    if magic != CMESH_MAGIC:
        raise ValueError(f"Invalid CMesh magic 0x{magic:08X} (expected 0x{CMESH_MAGIC:08X})")
    if version != CMESH_VERSION_1:
        raise ValueError(f"Unsupported CMesh version {version} (supported: 1)")
    if vertex_count == 0:
        raise ValueError("VertexCount is zero")
    if index_count == 0 or (index_count % 3) != 0:
        raise ValueError(f"IndexCount ({index_count}) must be a positive multiple of 3")

    has_normals = bool(flags & CMESH_FLAG_HAS_NORMALS)
    has_uvs = bool(flags & CMESH_FLAG_HAS_UVS)
    has_colors = bool(flags & CMESH_FLAG_HAS_COLORS)

    expected_len = CMESH_HEADER_SIZE + vertex_count * 3 * 4
    if has_normals:
        expected_len += vertex_count * 3 * 4
    if has_uvs:
        expected_len += vertex_count * 2 * 4
    if has_colors:
        expected_len += vertex_count * 4 * 1
    expected_len += index_count * 4

    if len(data) < expected_len:
        raise ValueError(f"CMesh buffer truncated: got {len(data)} bytes, expected {expected_len}")

    offset = CMESH_HEADER_SIZE

    # Read vertices
    vert_bytes = vertex_count * 12
    flat_verts = struct.unpack(f"<{vertex_count * 3}f", data[offset : offset + vert_bytes])
    vertices = [(flat_verts[3 * i], flat_verts[3 * i + 1], flat_verts[3 * i + 2]) for i in range(vertex_count)]
    offset += vert_bytes

    normals: list[tuple[float, float, float]] | None = None
    if has_normals:
        norm_bytes = vertex_count * 12
        flat_norms = struct.unpack(f"<{vertex_count * 3}f", data[offset : offset + norm_bytes])
        normals = [(flat_norms[3 * i], flat_norms[3 * i + 1], flat_norms[3 * i + 2]) for i in range(vertex_count)]
        offset += norm_bytes

    uvs: list[tuple[float, float]] | None = None
    if has_uvs:
        uv_bytes = vertex_count * 8
        flat_uvs = struct.unpack(f"<{vertex_count * 2}f", data[offset : offset + uv_bytes])
        uvs = [(flat_uvs[2 * i], flat_uvs[2 * i + 1]) for i in range(vertex_count)]
        offset += uv_bytes

    colors: list[tuple[int, int, int, int]] | None = None
    if has_colors:
        col_bytes = vertex_count * 4
        flat_cols = struct.unpack(f"<{vertex_count * 4}B", data[offset : offset + col_bytes])
        colors = [(flat_cols[4 * i], flat_cols[4 * i + 1], flat_cols[4 * i + 2], flat_cols[4 * i + 3]) for i in range(vertex_count)]
        offset += col_bytes

    # Read indices
    indices = list(struct.unpack(f"<{index_count}I", data[offset : offset + index_count * 4]))
    triangles = [(indices[3 * i], indices[3 * i + 1], indices[3 * i + 2]) for i in range(index_count // 3)]

    # Compute bounding box
    min_x = min(v[0] for v in vertices)
    max_x = max(v[0] for v in vertices)
    min_y = min(v[1] for v in vertices)
    max_y = max(v[1] for v in vertices)
    min_z = min(v[2] for v in vertices)
    max_z = max(v[2] for v in vertices)

    return {
        "magic": magic,
        "version": version,
        "flags": flags,
        "vertex_count": vertex_count,
        "index_count": index_count,
        "triangle_count": index_count // 3,
        "vertices": vertices,
        "normals": normals,
        "uvs": uvs,
        "colors": colors,
        "indices": indices,
        "triangles": triangles,
        "bounds": {
            "min": (min_x, min_y, min_z),
            "max": (max_x, max_y, max_z),
            "size": (max_x - min_x, max_y - min_y, max_z - min_z),
        },
    }


def write_cmesh_file(
    filepath: str | Path,
    vertices: Sequence[tuple[float, float, float] | list[float]],
    triangles: Sequence[int | tuple[int, int, int] | list[int]],
    normals: Sequence[tuple[float, float, float] | list[float]] | None = None,
    uvs: Sequence[tuple[float, float] | list[float]] | None = None,
    colors: Sequence[tuple[int, int, int, int] | list[int]] | None = None,
) -> Path:
    """Packs and writes geometry to a .cmesh file."""
    path = Path(filepath)
    path.parent.mkdir(parents=True, exist_ok=True)
    buf = pack_cmesh(vertices, triangles, normals=normals, uvs=uvs, colors=colors)
    path.write_bytes(buf)
    return path


def read_cmesh_file(filepath: str | Path) -> dict[str, Any]:
    """Reads and unpacks geometry from a .cmesh file."""
    return unpack_cmesh(Path(filepath).read_bytes())


def validate_cmesh_data(data: bytes) -> tuple[bool, str]:
    """Validates buffer against .cmesh binary specification."""
    try:
        res = unpack_cmesh(data)
        return True, f"Valid CMesh (v{res['version']}): {res['vertex_count']} verts, {res['triangle_count']} tris"
    except Exception as exc:
        return False, str(exc)

