#!/usr/bin/env python3
"""Check a file is a usable input for the walker pipeline — the FIRST thing to run.

    python3 check_input.py ~/models/SK_MyPed.fbx

Four gates, cheapest first, no editor involved (well under a second):

  1. FORMAT   it must be an FBX. Unreal imports FBX; .ma/.mb/.blend/.max/.c4d and
              USD are not importable here, and a wrong extension is the single most
              common wasted import.
  2. SKINNED  a walker is a SKELETAL mesh — a static mesh belongs to import-carla-prop.
  3. RIG      it must be skinned to CARLA's GEN3 skeleton: exactly these 26 `crl_*`
              bones, which is what lets ABP_GEN3 and every AS_*_G3 animation apply
              with no retargeting.
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

GEN3_BONES = frozenset({
    "crl_root",
    "crl_hips__C", "crl_spine__C", "crl_spine01__C", "crl_neck__C", "crl_Head__C",
    "crl_eye__L", "crl_eye__R",
    "crl_shoulder__L", "crl_shoulder__R",
    "crl_arm__L", "crl_arm__R",
    "crl_foreArm__L", "crl_foreArm__R",
    "crl_hand__L", "crl_hand__R",
    "crl_thigh__L", "crl_thigh__R",
    "crl_leg__L", "crl_leg__R",
    "crl_foot__L", "crl_foot__R",
    "crl_toe__L", "crl_toe__R",
    "crl_toeEnd__L", "crl_toeEnd__R",
})

BONE_RE = re.compile(rb"crl_[A-Za-z0-9_]+")
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
    missing = sorted(GEN3_BONES - found["bones"])
    extra = sorted(found["bones"] - GEN3_BONES)
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
        "rig": "GEN3" if (not missing and not extra) else "unknown",
        "missing_bones": missing,
        "unexpected_bones": extra,
        "embedded_media": found["embedded_media"],
        "texture_refs": found["texture_refs"],
        "external_texture_refs": external,
        "textures_beside_file": beside,
        "ok": bool(found["is_fbx"] and found["skinned"] and not missing and not extra),
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
        print("[input]       A walker must be a SKELETAL mesh. A static mesh belongs")
        print("[input]       to import-carla-prop instead.")
        return 2
    print(f"[input] PASS  skinned, {len(found['bones'])} crl_* bones")

    # 3. rig
    if missing or extra:
        print("[input] FAIL  rig is NOT GEN3")
        if missing:
            print(f"[input]       missing {len(missing)}: {', '.join(missing[:8])}"
                  + (" ..." if len(missing) > 8 else ""))
        if extra:
            print(f"[input]       unexpected {len(extra)}: {', '.join(extra[:8])}"
                  + (" ..." if len(extra) > 8 else ""))
        print("[input]       Skin to CARLA's GEN3 skeleton (26 crl_* bones) and")
        print("[input]       re-export, or retarget first. Importing anyway gives a")
        print("[input]       walker no CARLA animation can drive.")
        return 1
    print("[input] PASS  rig is GEN3 — ABP_GEN3 and every AS_*_G3 animation apply as-is")

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
