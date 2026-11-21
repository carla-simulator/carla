#!/usr/bin/env python3
"""Switches the traffic signs of the level open in the editor to another convention.

  python3 restyle_signs.py --style MUTCD            # Vienna -> US (speeds in mph)
  python3 restyle_signs.py --style GB               # Vienna -> China
  python3 restyle_signs.py --style VC               # Vienna (Town15: back to its own signs)
  python3 restyle_signs.py --style original         # back to the map's own signs, any map
  python3 restyle_signs.py --style MUTCD --dry-run  # report only
  python3 restyle_signs.py --style MUTCD --bounds 80000,15000,12000,95000,30000,15000
  python3 restyle_signs.py --style MUTCD --print    # print the sandbox script instead
                                                    # (to send it with your own MCP client)

Needs the editor open with unreal-mcp listening (see SKILL.md).
"""
import argparse
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.dont_write_bytecode = True  # no __pycache__ inside the skill library
TABLE = os.path.join(HERE, "..", "references", "sign_equivalences.json")
TEMPLATE = os.path.join(HERE, "sandbox_template.py")


def build(style, bounds=None, dry_run=False, table=TABLE):
    t = json.load(open(table))
    s = open(TEMPLATE).read()
    return (s.replace("__STYLE__", style)
             .replace("__CELLS__", json.dumps(t["cells"]))
             .replace("__TARGETS__", json.dumps(t["targets"].get(style, {})))
             .replace("__BOUNDS__", repr(bounds))
             .replace("__DRY_RUN__", "True" if dry_run else "False"))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--style", required=True, choices=["VC", "MUTCD", "GB", "original"])
    ap.add_argument("--bounds", help="minx,miny,minz,maxx,maxy,maxz in cm: only signs inside")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--print", action="store_true", help="print the sandbox script, do not run it")
    a = ap.parse_args()
    bounds = None
    if a.bounds:
        v = [float(x) for x in a.bounds.split(",")]
        if len(v) != 6:
            ap.error("--bounds needs six numbers")
        bounds = [v[:3], v[3:]]
    script = build(a.style, bounds, a.dry_run)
    if a.print:
        print(script)
        return
    sys.path.insert(0, HERE)
    import unreal_mcp
    res = unreal_mcp.run_script(unreal_mcp.session(), script)
    print(json.dumps(res, indent=1))


if __name__ == "__main__":
    main()
