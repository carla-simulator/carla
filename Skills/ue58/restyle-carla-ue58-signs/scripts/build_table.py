#!/usr/bin/env python3
"""Regenerates references/sign_equivalences.json from the regional sign catalog
loaded in the editor (CarlaDigitalTwinsTool content). Run it when the catalog changes.

  python3 build_table.py            # writes ../references/sign_equivalences.json

"cells" maps an atlas cell ("<texture>:<Id_X>:<Id_Y>") of any style's catalog to the
sign it paints, by its Vienna name (the canonical name); MUTCD speed limits map to
"mph:<n>", since the plate does not say which km/h limit it stands for. "targets"
maps a canonical sign to the asset that paints it in each style. Speed limits go to
the same number in GB and to the nearest mph in MUTCD. Targets point at the data
assets under /Game/Carla/Blueprints/LevelDesign/Signs/DataAssets/<STYLE> when one
exists (the classic towns use the MUTCD ones), so switching needs no plugin; the
catalog is only read to build the table.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.dont_write_bytecode = True  # no __pycache__ inside the skill library
sys.path.insert(0, HERE)
import unreal_mcp  # noqa: E402

CATALOG = "/CarlaDigitalTwinsTool/Carla/Blueprints/LevelDesign/Signs/Catalog"
OUT = os.path.join(HERE, "..", "references", "sign_equivalences.json")

# Vienna sign -> (MUTCD sign, GB sign), by catalog SignName. Picked by meaning, not by
# OpenDRIVE code: the codes are shared too loosely (101 is every "danger" sign).
EQUIVALENT = {
    "stop": ("stop", "stop"),
    "give_way": ("yield", "give_way"),
    "pedestrian_crossing": ("pedestrian_crossing_warning", "pedestrian_crossing"),
    "pedestrian_crossing_ahead": ("pedestrian_crossing_warning", "pedestrian_crossing"),
    "road_hump": ("bump", "road_hump"),
    "wild_animals": ("deer_crossing", "wild_animals"),
    "cyclists": ("bicycle_crossing", "bicycle_crossing"),
    "roundabout_mandatory": ("roundabout_circulation", "roundabout"),
    "roundabout_mandatory_clockwise": ("roundabout_circulation", "roundabout"),
    "straight_ahead_mandatory": ("straight_only", "straight_ahead"),
    "turn_right_here_mandatory": ("turn_right_only", "turn_right"),
    "no_stopping": ("no_stopping_any_time", "no_stopping"),
    "parking": (None, "parking"),
    "one_way_left": ("one_way_left", None),
    "one_way_right": ("one_way_right", None),
    "no_left_turn": ("no_left_turn", "no_left_turn"),
    "no_right_turn": ("no_right_turn", "no_right_turn"),
    "no_entry": ("do_not_enter", "no_entry"),
    # MUTCD "left lane ends, merge right" is the road narrowing on the left
    "road_narrows_left": ("lane_ends_left", "road_narrows_left"),
    "road_narrows_right": ("lane_ends_right", "road_narrows_right"),
}
# Permanent data assets per style, preferred over the catalog.
CONTENT = "/Game/Carla/Blueprints/LevelDesign/Signs/DataAssets"
SPEED = re.compile(r"max_speed_(\d+)")

SCRIPT = '''import json
P = %s
def run():
    out = {}
    for p in P:
        name = p.split("/")[-1]
        out[p] = json.loads(execute_tool("editor_toolset.toolsets.object.ObjectTools.get_properties", json.dumps(
            {"instance": {"refPath": p + "." + name},
             "properties": ["SignName", "Style", "Diffuse", "Id_X", "Id_Y"]}))["returnValue"])
    return out
'''


def main():
    sid = unreal_mcp.session()
    paths = json.loads(unreal_mcp.call(sid, "editor_toolset.toolsets.asset.AssetTools", "find_assets",
                                       {"folder_path": CATALOG, "name": "DA_", "asset_type": None}))["returnValue"]
    cat = unreal_mcp.run_script(sid, SCRIPT % json.dumps(paths))
    by = {}
    for p, v in cat.items():
        by.setdefault((v["Style"], v["SignName"]), p + "." + p.split("/")[-1])
    mph = sorted(int(m.group(1)) for (s, n) in by if s == "MUTCD" for m in [SPEED.fullmatch(n)] if m)

    content = {}
    for st in ("VC", "MUTCD", "GB"):
        found = json.loads(unreal_mcp.call(sid, "editor_toolset.toolsets.asset.AssetTools", "find_assets",
                                           {"folder_path": "%s/%s" % (CONTENT, st), "name": "DA_", "asset_type": None}))["returnValue"]
        content[st] = {p.split("/")[-1]: p + "." + p.split("/")[-1] for p in found}

    def asset(style, sign):
        p = by[(style, sign)]
        return content[style].get(p.split(".")[-1], p)

    def cell(v):
        return "%s:%s:%s" % (v["Diffuse"]["refPath"].split(".")[-1], v["Id_X"], v["Id_Y"])

    cells, targets = {}, {"VC": {}, "MUTCD": {}, "GB": {}}
    # MUTCD/GB sign -> Vienna name; where several Vienna signs share one, the first by name
    back = {"MUTCD": {}, "GB": {}}
    for vc in sorted(EQUIVALENT):
        for st, other in zip(("MUTCD", "GB"), EQUIVALENT[vc]):
            if other:
                back[st].setdefault(other, vc)
    for p, v in cat.items():
        name, style = v["SignName"], v["Style"]
        speed = SPEED.fullmatch(name)
        if style == "VC" and (name in EQUIVALENT or speed):
            cells[cell(v)] = name
        elif style == "GB" and (name in back["GB"] or speed):
            cells[cell(v)] = back["GB"].get(name, name)
        elif style == "MUTCD" and speed:
            cells[cell(v)] = "mph:%s" % speed.group(1)
        elif style == "MUTCD" and name in back["MUTCD"]:
            cells[cell(v)] = back["MUTCD"][name]
        if style != "VC" or not (name in EQUIVALENT or speed):
            continue
        targets["VC"][name] = asset("VC", name)
        if speed:
            kmh = int(speed.group(1))
            want = {"MUTCD": "max_speed_%d" % min(mph, key=lambda s: abs(s - kmh * 0.621371)), "GB": name}
        else:
            want = dict(zip(("MUTCD", "GB"), EQUIVALENT[name]))
        for st, target in want.items():
            if target and (st, target) in by:
                targets[st][name] = asset(st, target)
            elif target:
                print("not in the %s catalog: %s (for %s)" % (st, target, name))
    json.dump({"cells": cells, "targets": targets}, open(OUT, "w"), indent=1, sort_keys=True)
    print("%d cells; VC %d, MUTCD %d, GB %d targets -> %s"
          % (len(cells), len(targets["VC"]), len(targets["MUTCD"]), len(targets["GB"]), os.path.normpath(OUT)))


if __name__ == "__main__":
    main()
