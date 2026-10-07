#!/usr/bin/env python3
"""Compile and sample .scenic files with NO simulator. Triage before you run.

A `--simulate` failure conflates three very different causes. This separates them:

  COMPILE-FAIL  syntax, model import, missing .xodr, or a blueprint category
                Scenic has no entries for on this client version
  SAMPLE-FAIL   the scenario's `require`s cannot be met on its map — an empty
                domain, or rejection sampling exhausted
  (PASS)        the scene is constructible; anything left is a runtime/server
                problem, which is what a --simulate run then tests

Usage: sample_scenic.py [--iterations N] [--json OUT] FILE [FILE ...]
"""
from __future__ import annotations
import argparse, json, os, sys, time, warnings

warnings.filterwarnings("ignore")


def main() -> int:
    ap = argparse.ArgumentParser()
    # 2000 is well past the ~300 worst case seen on a dense urban map, so an
    # exhaustion here means the requirements are unsatisfiable, not unlucky.
    ap.add_argument("--iterations", type=int, default=2000)
    ap.add_argument("--json", default=None)
    ap.add_argument("files", nargs="+")
    a = ap.parse_args()

    import scenic

    rows = []
    for f in a.files:
        name = os.path.basename(f)
        row = {"file": f, "name": name}
        t0 = time.time()
        try:
            sc = scenic.scenarioFromFile(f, mode2D=True)
        except Exception as e:
            row.update(stage="compile", verdict="FAIL",
                       detail="%s: %s" % (type(e).__name__, str(e).replace("\n", " ")[:200]))
            print("  %-34s COMPILE-FAIL  %s" % (name, row["detail"][:90]), flush=True)
            rows.append(row); continue
        row["carla_map"] = str(sc.params.get("carla_map", "?"))
        try:
            scene, iters = sc.generate(maxIterations=a.iterations)
        except Exception as e:
            row.update(stage="sample", verdict="FAIL",
                       detail="%s: %s" % (type(e).__name__, str(e).replace("\n", " ")[:200]))
            print("  %-34s SAMPLE-FAIL   %s" % (name, row["detail"][:90]), flush=True)
            rows.append(row); continue
        bps = sorted({getattr(o, "blueprint", None) for o in scene.objects} - {None})
        row.update(stage="sample", verdict="PASS", iterations=iters,
                   objects=len(scene.objects), blueprints=bps,
                   seconds=round(time.time() - t0, 1))
        rows.append(row)
        print("  %-34s PASS  %4d iters  %2d objs  map=%-14s %s"
              % (name, iters, len(scene.objects), row["carla_map"],
                 ",".join(b.split(".")[-1] for b in bps)[:44]), flush=True)

    if a.json:
        json.dump(rows, open(a.json, "w"), indent=1)
    ok = sum(1 for r in rows if r["verdict"] == "PASS")
    print("\n  %d/%d constructible" % (ok, len(rows)))
    return 0 if ok == len(rows) else 1


if __name__ == "__main__":
    sys.exit(main())
