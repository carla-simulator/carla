# Runs INSIDE the Unreal Editor, through unreal-mcp's
# ProgrammaticToolset.execute_tool_script sandbox (only json/math/re/... importable,
# execute_tool() is the only way to touch the editor). restyle_signs.py fills the
# __PLACEHOLDERS__ in and either prints the result or sends it.
#
# For every BP_Sign in the level: work out which sign its data asset paints (atlas
# texture + cell, in any style), and point the actor at a data asset that paints the
# same sign in STYLE: a permanent one under /Game/Carla when the table names one,
# otherwise one made in memory from the catalog. The map's own data asset is kept in
# an actor tag, so asking for the map's own style (or "original") puts every sign
# back. Nothing is written to disk.
import json

STYLE = "__STYLE__"
CELLS = __CELLS__
TARGETS = __TARGETS__
BOUNDS = __BOUNDS__
DRY_RUN = __DRY_RUN__

SIGN_CLASS = "/Game/Carla/Blueprints/LevelDesign/Signs/BP_Sign.BP_Sign_C"
BDA_CLASS = "/Game/Carla/Blueprints/LevelDesign/Signs/BDAs/BDA_TrafficSign_Base.BDA_TrafficSign_Base_C"
SWAP_ROOT = "/Game/SignStyleSwap"
TAG = "SignStyleSwap:orig="
SIZE_TAG = "SignStyleSwap:size="
# Correction on top of matching the plates' largest side, tuned by eye on Town15: a US
# speed limit as tall as the Vienna disc it replaces, a US warning diamond a little
# larger than the 60 cm square it replaces.
SHAPE_FACTOR = {"SM_VertRect": 0.95, "SM_HorRect": 0.95, "SM_RomboidShape": 1.3}
STYLE_ENUM = {"VC": "Vienna Convention (Europe)", "MUTCD": "MUTCD (America)", "GB": "GB (China)"}
# Vienna speed limits the catalog has, km/h: where a MUTCD plate's km/h is not known
OVERLAY_TAG = "MUTCDOverlayOf:"
KMH = sorted(int(v.split("_")[-1]) for v in set(CELLS.values()) if v.startswith("max_speed_"))


def tool(name, args):
    return execute_tool(name, json.dumps(args))["returnValue"]


def get_props(path, props):
    return json.loads(tool("editor_toolset.toolsets.object.ObjectTools.get_properties",
                           {"instance": {"refPath": path}, "properties": props}))


def set_props(path, values):
    return tool("editor_toolset.toolsets.object.ObjectTools.set_properties",
                {"instance": {"refPath": path}, "values": json.dumps(values)})


def find_signs():
    args = {"name": "", "tag": "", "collision_channels": [], "actor_type": {"refPath": SIGN_CLASS}}
    if BOUNDS:
        args["bounds"] = {"min": dict(zip("xyz", BOUNDS[0])), "max": dict(zip("xyz", BOUNDS[1])), "isValid": True}
    return [a["refPath"] for a in tool("editor_toolset.toolsets.scene.SceneTools.find_actors", args)]


def get_tags(actor):
    return tool("editor_toolset.toolsets.actor.ActorTools.get_tags", {"actor": {"refPath": actor}})


def short(ref):
    return ref.split(".")[-1] if ref else ""


bda_cache = {}
mesh_cache = {}


def plate_extent(mesh_ref):
    """Largest of the plate's width and height (cm, mesh space)."""
    if mesh_ref not in mesh_cache:
        b = tool("editor_toolset.toolsets.static_mesh.StaticMeshTools.get_bounds", {"mesh": {"refPath": mesh_ref}})
        mesh_cache[mesh_ref] = max(b["max"]["x"] - b["min"]["x"], b["max"]["z"] - b["min"]["z"])
    return mesh_cache[mesh_ref]


def shape_k(mesh_ref):
    name = short(mesh_ref)
    for prefix, k in SHAPE_FACTOR.items():
        if name.startswith(prefix):
            return k
    return 1.0


def size_factor(original_bda, target_mesh):
    """How much to scale the actor so the new plate is as big as the old one, by shape."""
    old = (get_props(original_bda, ["signMesh"]).get("signMesh") or {}).get("refPath")
    if not old:
        return 1.0
    return round(plate_extent(old) / plate_extent(target_mesh) * shape_k(target_mesh) / shape_k(old), 3)


def style_of(bda_path):
    """VC, MUTCD or GB, from the atlas the data asset paints from."""
    atlas = short((get_props(bda_path, ["diffuse"]).get("diffuse") or {}).get("refPath"))
    for st in ("MUTCD", "GB", "VC"):
        if atlas.startswith("T_%s_" % st):
            return st
    return None


def legacy_kmh(actor, tags):
    """km/h of the sign actor a classic-town overlay stands on (the one with the logic)."""
    for t in tags:
        if t.startswith(OVERLAY_TAG):
            legacy = actor.rsplit(".", 1)[0] + "." + t[len(OVERLAY_TAG):]
            p = get_props(legacy, ["trafficSignState", "speedLimitKmh"])
            st = str(p.get("trafficSignState", ""))
            if st.startswith("SpeedLimit_"):
                return int(st.split("_")[-1])
            if p.get("speedLimitKmh"):
                return int(p["speedLimitKmh"])
    return None


def canonical(bda_path, actor=None, tags=()):
    """Which sign a data asset paints (by its Vienna name): its atlas texture and cell."""
    if bda_path not in bda_cache:
        p = get_props(bda_path, ["diffuse", "id_X", "id_Y", "unique"])
        key = "%s:%s:%s" % (short((p.get("diffuse") or {}).get("refPath")), p.get("id_X"), p.get("id_Y"))
        bda_cache[bda_path] = None if p.get("unique") else CELLS.get(key)
    name = bda_cache[bda_path]
    if name and name.startswith("mph:"):
        # A MUTCD plate says mph; the limit the road has is the km/h of the sign underneath.
        kmh = legacy_kmh(actor, tags) if actor else None
        if kmh is None:
            kmh = int(name[4:]) * 1.609344
        name = "max_speed_%d" % min(KMH, key=lambda k: abs(k - kmh))
    return name


made = {}


def target_bda(name):
    """The data asset that paints the sign in STYLE: a permanent one when the table names
    one under /Game/Carla, otherwise one made in memory from the catalog entry."""
    if name in made:
        return made[name]
    entry = TARGETS[name]
    if entry.startswith("/Game/Carla/"):
        made[name] = (entry, (get_props(entry, ["signMesh"]).get("signMesh") or {}).get("refPath"))
        return made[name]
    e = get_props(entry, ["SignMesh", "Diffuse", "Id_X", "Id_Y"])
    folder = "%s/%s" % (SWAP_ROOT, STYLE)
    # Named after the catalog entry; never create twice, a second create replaces the asset
    # and leaves every sign that used it pointing at a dead object.
    asset = short(entry)
    path = "%s/%s.%s" % (folder, asset, asset)
    if not tool("editor_toolset.toolsets.asset.AssetTools.exists", {"path": "%s/%s" % (folder, asset)}):
        tool("editor_toolset.toolsets.data_asset.DataAssetTools.create",
             {"folder_path": folder, "asset_name": asset, "asset_type": {"refPath": BDA_CLASS}})
    set_props(path, {"signMesh": e["SignMesh"], "signStyle": STYLE_ENUM[STYLE], "category": "Priority",
                     "unique": False, "diffuse": e["Diffuse"], "id_X": e["Id_X"], "id_Y": e["Id_Y"]})
    made[name] = (path, e["SignMesh"]["refPath"])
    return made[name]


def run():
    changed, kept, restored, failed = {}, {}, 0, []
    for actor in find_signs():
        props = get_props(actor, ["signAssetData", "poleAssetData"])
        cur = (props.get("signAssetData") or {}).get("refPath")
        if not cur:
            continue
        if not props.get("poleAssetData") or props.get("poleAssetData") == "None":
            # Its construction script raises on any change and aborts the whole call: leave it.
            failed.append("%s: no pole data asset, left as is" % short(actor))
            continue
        tags = get_tags(actor)
        orig_tags = [t for t in tags if t.startswith(TAG)]
        size_tags = [t for t in tags if t.startswith(SIZE_TAG)]
        original = orig_tags[0][len(TAG):] if orig_tags else cur
        size = get_props(actor, ["size"]).get("size", 1)
        orig_size = float(size_tags[0][len(SIZE_TAG):]) if size_tags else size
        if STYLE == "original" or style_of(original) == STYLE:
            # The map's own style: back to the map's own data asset.
            if orig_tags and (cur != original or size != orig_size):
                if not DRY_RUN:
                    try:
                        set_props(actor, {"signAssetData": {"refPath": original}, "size": orig_size})
                    except Exception as err:
                        failed.append("%s: %s" % (short(actor), str(err).splitlines()[-1][:120]))
                restored += 1
            if not DRY_RUN:
                # Drop our tags too, so nothing of the switch is left if the level is saved.
                for t in orig_tags + size_tags:
                    tool("editor_toolset.toolsets.actor.ActorTools.remove_tag", {"actor": {"refPath": actor}, "tag": t})
            continue
        name = canonical(original, actor, tags)
        if not name or name not in TARGETS:
            # No such sign in STYLE: the map's own plate, not whatever an earlier switch left.
            label = "%s (%s)" % (name, short(original)) if name else short(original)
            kept[label] = kept.get(label, 0) + 1
            if orig_tags and not DRY_RUN:
                try:
                    set_props(actor, {"signAssetData": {"refPath": original}, "size": orig_size})
                except Exception as err:
                    failed.append("%s: %s" % (short(actor), str(err).splitlines()[-1][:120]))
                for t in orig_tags + size_tags:
                    tool("editor_toolset.toolsets.actor.ActorTools.remove_tag", {"actor": {"refPath": actor}, "tag": t})
            continue
        if not DRY_RUN:
            if not orig_tags:
                tool("editor_toolset.toolsets.actor.ActorTools.add_tag", {"actor": {"refPath": actor}, "tag": TAG + original})
            if not size_tags:
                tool("editor_toolset.toolsets.actor.ActorTools.add_tag", {"actor": {"refPath": actor}, "tag": SIZE_TAG + repr(orig_size)})
            try:
                bda, mesh = target_bda(name)
                set_props(actor, {"signAssetData": {"refPath": bda}, "size": round(orig_size * size_factor(original, mesh), 3)})
            except Exception as err:
                failed.append("%s: %s" % (short(actor), str(err).splitlines()[-1][:120]))
        label = "%s -> %s" % (name, short(TARGETS[name]))
        changed[label] = changed.get(label, 0) + 1
    return {"style": STYLE, "dry_run": DRY_RUN, "changed": changed, "unchanged": kept,
            "restored": restored, "errors": failed, "changed_total": sum(changed.values())}
