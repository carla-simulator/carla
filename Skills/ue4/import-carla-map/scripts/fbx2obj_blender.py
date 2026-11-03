"""Blender-backed replacement for CARLA's FBX2OBJ (see references/maps.md).

Reproduces Util/DockerUtils/fbx/src/FBX2OBJ.cpp without the Autodesk SDK:

  * SetMaterials() assigns ONE material per mesh node by substring on the node
    name, first match wins; unmatched nodes fall back to "block". The names are
    the contract with RecastBuilder.
  * SaveScene() rotates the root -90 deg about X, turning the Z-up source into a
    Y-up OBJ. Blender's exporter does that via forward_axis/up_axis, so no
    manual rotation is applied here.

RecastBuilder only recognises road/sidewalk/crosswalk/grass, so "block" nodes --
a RoadRunner terrain mesh among them -- are left unclassified and come back out
of the navmesh as RC_WALKABLE_AREA (63) at default traversal cost. Setting
CARLA_TERRAIN_AREA=grass maps Terrain_* nodes to CARLA_AREA_GRASS instead, which
carries AREA_GRASS_COST and keeps walkers on the pavement. Off by default: the
stock FBX2OBJ does not do it.

Usage:  blender --background --python fbx2obj_blender.py -- <in.fbx> <out.obj>
Env:    CARLA_TERRAIN_AREA=grass|block   (default block, matching FBX2OBJ.cpp)
Exit:   0 ok, 2 bad args, 3 import failed, 4 no meshes, 5 zero polygons,
        6 Blender too old.
Needs:  Blender >= 3.3 for bpy.ops.wm.obj_export (the C++ OBJ exporter).
"""
import os
import sys

import bpy

# Checked before anything else: on older Blender the export call below is simply
# absent, and the failure would otherwise be an AttributeError at the end of a
# long conversion, inside a --background run nobody is watching.
if bpy.app.version < (3, 3, 0):
    print("FBX2OBJ_ERROR: Blender %d.%d is too old — wm.obj_export needs 3.3+. "
          "Install a newer Blender, or point BLENDER at one."
          % bpy.app.version[:2])
    sys.exit(6)

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
if len(argv) < 2:
    print("FBX2OBJ_ERROR: usage: blender --background --python %s -- <in.fbx> <out.obj>"
          % __file__)
    sys.exit(2)
src, dst = argv[0], argv[1]

# Substring -> RecastBuilder area material. Order mirrors the if/else chain in
# SetMaterials(); first match wins.
RULES = [
    ("Road_Road", "road"), ("Roads_Road", "road"),
    ("Road_Marking", "road"), ("Roads_Marking", "road"),
    ("Road_Curb", "road"), ("Roads_Curb", "road"),
    ("Road_Gutter", "road"), ("Roads_Gutter", "road"),
    ("Road_Sidewalk", "sidewalk"), ("Roads_Sidewalk", "sidewalk"),
    ("Road_Crosswalk", "crosswalk"), ("Roads_Crosswalk", "crosswalk"),
    ("Road_Grass", "grass"), ("Roads_Grass", "grass"),
]
AREAS = ("road", "sidewalk", "crosswalk", "grass", "block")

# Opt-in: reclassify the terrain instead of letting it fall through to "block".
TERRAIN_AREA = os.environ.get("CARLA_TERRAIN_AREA", "block").strip().lower()
if TERRAIN_AREA not in AREAS:
    print("FBX2OBJ_ERROR: CARLA_TERRAIN_AREA=%r must be one of %s"
          % (TERRAIN_AREA, ", ".join(AREAS)))
    sys.exit(2)


def area_for(node_name):
    for needle, area in RULES:
        if needle in node_name:
            return area
    if TERRAIN_AREA != "block" and "Terrain" in node_name:
        return TERRAIN_AREA
    return "block"


bpy.ops.wm.read_factory_settings(use_empty=True)

try:
    bpy.ops.import_scene.fbx(filepath=src)
except Exception as exc:  # noqa: BLE001 - surface any importer failure verbatim
    print("FBX2OBJ_ERROR: FBX import failed: %s" % exc)
    sys.exit(3)

materials = {a: bpy.data.materials.new(name=a) for a in AREAS}

meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
if not meshes:
    print("FBX2OBJ_ERROR: no mesh objects found in %s" % src)
    sys.exit(4)

counts = {}
for ob in meshes:
    area = area_for(ob.name)
    counts[area] = counts.get(area, 0) + 1
    ob.data.materials.clear()
    ob.data.materials.append(materials[area])
    print("Node %s : %s" % (ob.name, area))

total_verts = sum(len(o.data.vertices) for o in meshes)
total_polys = sum(len(o.data.polygons) for o in meshes)
print("FBX2OBJ_STATS: objects=%d verts=%d polys=%d areas=%s"
      % (len(meshes), total_verts, total_polys, counts))

# An empty OBJ is the failure this whole script exists to prevent: build.sh
# happily feeds one to RecastBuilder, which then silently writes no .bin.
if total_polys == 0:
    print("FBX2OBJ_ERROR: source mesh has zero polygons")
    sys.exit(5)

bpy.ops.wm.obj_export(
    filepath=dst,
    export_selected_objects=False,
    export_materials=True,
    export_triangulated_mesh=True,
    export_uv=False,
    export_normals=False,
    forward_axis="NEGATIVE_Z",
    up_axis="Y",
)
print("FBX2OBJ_OK: wrote %s" % dst)
