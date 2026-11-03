#!/usr/bin/env python3
"""Check a file is a usable input for the vehicle pipeline — the FIRST thing to run.

    python3 check_input.py ~/models/SK_MyCar.fbx

Four gates, cheapest first, no editor involved (well under a second):

  1. FORMAT   it must be an FBX. Unreal imports FBX; .ma/.mb/.blend/.max/.c4d and
              USD are not importable here, and a wrong extension is the single most
              common wasted import.
  2. SKINNED  a vehicle is a SKELETAL mesh whose wheels are BONES — a rigid body
              belongs to import-carla-prop.
  3. RIG      it must carry the four canonical wheel bones. PhysX's PxVehicleDrive4W
              finds wheels by name, and so do the physics bodies, the WheelSetups and
              the animation blueprint.
  4. TEXTURES reported, not enforced: an FBX with no embedded media imports as one
              blank white material per slot, and nothing downstream can recover
              textures the file does not contain.

Exit codes: 0 usable, 1 rig mismatch, 2 not usable at all (wrong format / not skinned).
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

WHEEL_BONES = ("Wheel_Front_Left", "Wheel_Front_Right",
               "Wheel_Rear_Left", "Wheel_Rear_Right")
CHASSIS_BONE = "Vehicle_Base"

BONE_RE = re.compile(rb"[A-Za-z_][A-Za-z0-9_]{2,63}")
SKIN_MARKERS = (b"Cluster", b"Deformer")
TEXTURE_REF_RE = re.compile(rb"[A-Za-z0-9_.\\/:-]+\.(?:png|tga|jpg|jpeg|tif|tiff|bmp|exr)",
                            re.IGNORECASE)
# Image magic numbers. Their presence SUGGESTS embedded media but does not prove it:
# these byte sequences also occur by chance inside compressed geometry, so the report
# says "possible" and always shows the external references alongside.
EMBEDDED_MAGIC = ((b"\x89PNG\r\n\x1a\n", "png"), (b"\xff\xd8\xff", "jpeg"))

# Formats people hand over that Unreal cannot import.
FOREIGN = {
    ".ma": "Maya ASCII", ".mb": "Maya binary", ".blend": "Blender",
    ".max": "3ds Max", ".c4d": "Cinema 4D", ".obj": "OBJ (static geometry only)",
    ".usd": "USD", ".usda": "USD", ".usdc": "USD", ".gltf": "glTF", ".glb": "glTF",
    ".dae": "COLLADA", ".stl": "STL", ".ply": "PLY",
}


def looks_like_fbx(data: bytes) -> bool:
    return data[:23].startswith(b"Kaydara FBX Binary") or b"FBXHeaderExtension" in data[:4096]


def inspect(path: Path) -> dict:
    data = path.read_bytes()
    refs = sorted({m.group(0).decode("ascii", "replace") for m in TEXTURE_REF_RE.finditer(data)})
    embedded = [kind for magic, kind in EMBEDDED_MAGIC if magic in data]
    return {
        "bytes": len(data),
        "binary_fbx": data[:23].startswith(b"Kaydara FBX Binary"),
        "is_fbx": looks_like_fbx(data),
        "skinned": any(marker in data for marker in SKIN_MARKERS),
        "bones": {m.decode("ascii", "replace") for m in BONE_RE.findall(data)},
        "texture_refs": refs,
        "embedded_media": embedded,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("path", type=Path, help="the file to check")
    ap.add_argument("--json", action="store_true", help="machine-readable output")
    args = ap.parse_args()

    if not args.path.is_file():
        print(f"ERROR: no such file: {args.path}", file=sys.stderr)
        return 2

    suffix = args.path.suffix.lower()
    found = inspect(args.path)
    missing = [b for b in WHEEL_BONES if b not in found["bones"]]
    extra = []          # a vehicle rig may carry any number of extra bones
    has_chassis = CHASSIS_BONE in found["bones"]
    # Texture refs that are absolute paths from someone else's machine resolve nowhere.
    external = [r for r in found["texture_refs"]
                if r.startswith("/") or re.match(r"^[A-Za-z]:", r) or r.startswith("..")]
    beside = [r for r in found["texture_refs"]
              if (args.path.parent / Path(r.replace("\\", "/")).name).is_file()]

    report = {
        "path": str(args.path),
        "format": "fbx" if found["is_fbx"] else (FOREIGN.get(suffix) or "unknown"),
        "is_fbx": found["is_fbx"],
        "skinned": found["skinned"],
        "rig": "4-wheel" if not missing else "unknown",
        "missing_bones": missing,
        "wheel_bones_found": [b for b in WHEEL_BONES if b in found["bones"]],
        "has_chassis_bone": has_chassis,
        "embedded_media": found["embedded_media"],
        "texture_refs": found["texture_refs"],
        "external_texture_refs": external,
        "textures_beside_file": beside,
        "ok": bool(found["is_fbx"] and found["skinned"] and not missing),
    }

    if args.json:
        print(json.dumps(report, indent=2))
        if not found["is_fbx"] or not found["skinned"]:
            return 2
        return 0 if report["ok"] else 1

    print(f"[input] {args.path.name}  {found['bytes'] // 1024} KB")

    # 1. format
    if not found["is_fbx"]:
        label = FOREIGN.get(suffix, "not an FBX")
        print(f"[input] FAIL  {label} — Unreal cannot import this.")
        print("[input]       The pipeline takes an **FBX** (binary or ASCII).")
        print("[input]       Re-export from the authoring package as FBX, with")
        print("[input]       'Embed Media' enabled so the textures travel with it.")
        return 2
    print(f"[input] PASS  {'binary' if found['binary_fbx'] else 'ascii'} FBX")

    # 2. skinned
    if not found["skinned"]:
        print("[input] FAIL  no skin cluster — this is not a skinned mesh.")
        print("[input]       A CARLA vehicle is a SKELETAL mesh whose wheels are bones.")
        print("[input]       A rigid body belongs to import-carla-prop instead.")
        return 2
    print("[input] PASS  skinned")

    # 3. rig
    if missing:
        print(f"[input] FAIL  {len(missing)} of 4 wheel bones missing:")
        for bone in missing:
            print(f"[input]         {bone}")
        print("[input]       PxVehicleDrive4W finds wheels by these exact names; without")
        print("[input]       them the car cannot steer or roll. Rename the joints in the")
        print("[input]       authoring scene and re-export.")
        return 1
    print("[input] PASS  all 4 wheel bones present"
          + ("" if has_chassis else f" (no {CHASSIS_BONE}; the chassis will be inferred)"))

    # 4. textures (reported, never fatal)
    if found["embedded_media"] and not external:
        print(f"[input] PASS  possible embedded media ({', '.join(found['embedded_media'])} "
              "data found, no outside references)")
    elif beside:
        print(f"[input] PASS  {len(beside)} texture file(s) sit beside the FBX")
    elif external:
        if found["embedded_media"]:
            print(f"[input] NOTE  {', '.join(found['embedded_media'])} data present, but the "
                  "texture references below point outside;")
            print("[input]       treat the material result as unknown until you look at it.")
        print(f"[input] WARN  {len(external)} texture reference(s) are paths from another")
        print("[input]       machine and resolve nowhere here, e.g.")
        print(f"[input]         {external[0]}")
        print("[input]       Every material slot will import BLANK WHITE. Re-export with")
        print("[input]       'Embed Media', or place the texture files beside the FBX.")
    else:
        print("[input] WARN  no texture references at all — material slots will import")
        print("[input]       blank white. Nothing downstream can invent textures.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
