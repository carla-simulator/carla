#!/usr/bin/env python3
"""Read, explain and rescore a CARLA Leaderboard results.json.

    read_results.py RESULTS.json                 per-route table + global record
    read_results.py RESULTS.json --route 3       one route in full
    read_results.py RESULTS.json --as 2.1        recompute the scores under 2.1 rules
    read_results.py RESULTS.json --as 2.0        ... or 2.0
    read_results.py A.json B.json --diff         compare two runs
    read_results.py RESULTS.json --merge C.json  merge shards (like merge_statistics.py)

Rescoring is exact, not an estimate: the infraction *counts* recorded in the file
are the inputs to both formulas, so a 2.0 result can be scored as 2.1 without
re-running the simulation. That matters because the live leaderboard switched to
the 2.1 formula in March 2025 while `master` and `leaderboard-2.0` still use 2.0's.
"""
from __future__ import annotations

import argparse
import json
import statistics
import sys
from pathlib import Path

# LB 2.0: score_penalty = product(p_j ** count_j)
PENALTY_20 = {
    "collisions_pedestrian": 0.50,
    "collisions_vehicle": 0.60,
    "collisions_layout": 0.65,
    "red_light": 0.70,
    "scenario_timeouts": 0.70,
    "yield_emergency_vehicle_infractions": 0.70,
    "stop_infraction": 0.80,
}
# LB 2.1: score_penalty = 1 / (1 + sum(c_j * count_j))
PENALTY_21 = {
    "collisions_pedestrian": 1.00,
    "collisions_vehicle": 0.70,
    "collisions_layout": 0.60,
    "red_light": 0.40,
    "scenario_timeouts": 0.40,
    "yield_emergency_vehicle_infractions": 0.40,
    "stop_infraction": 0.25,
}
# Not a constant penalty in either version; handled separately.
SPECIAL = ("min_speed_infractions", "outside_route_lanes", "route_dev",
           "vehicle_blocked", "route_timeout")

BLOCKING = ("route_dev", "vehicle_blocked", "route_timeout")


def load(path: Path) -> dict:
    try:
        return json.loads(path.read_text())
    except FileNotFoundError:
        sys.exit(f"{path} not found")
    except json.JSONDecodeError as e:
        sys.exit(f"{path} is not valid JSON ({e}) — a run killed mid-write leaves it truncated")


def records(data: dict) -> list[dict]:
    cp = data.get("_checkpoint") or {}
    return cp.get("records", [])


def counts(rec: dict) -> dict[str, int]:
    """Infraction counts. The file stores a list of message strings per key."""
    out = {}
    for k, v in (rec.get("infractions") or {}).items():
        out[k] = len(v) if isinstance(v, list) else int(v or 0)
    return out


def rescore(rec: dict, version: str) -> tuple[float, float, float, list[str]]:
    """Return (score_route, score_penalty, score_composed, notes) under `version`.

    Mirrors StatisticsManager.compute_route_statistics for each version. The
    percentage-scaled infractions (min speed, outside route lanes) are the one place
    where the recorded data is lossy: the file keeps message strings, not the raw
    percentage the formula needs, so they are left out and flagged rather than
    approximated — which makes the result an upper bound on those routes.
    """
    c = counts(rec)
    route = float((rec.get("scores") or {}).get("score_route", 0.0))
    notes: list[str] = []
    table = PENALTY_20 if version == "2.0" else PENALTY_21

    if version == "2.0":
        penalty = 1.0
        for key, p in table.items():
            penalty *= p ** c.get(key, 0)
    else:
        total = sum(coeff * c.get(key, 0) for key, coeff in table.items())
        penalty = 1.0 / (1.0 + total)

    # The two percentage-scaled infractions are recorded as message strings, not as
    # the percentage the formula needs. Rather than invent a factor, leave them out
    # and say so: the rescored number is then an upper bound on the true score.
    for key, what in (("min_speed_infractions", "min speed"),
                      ("outside_route_lanes", "outside route lanes")):
        if c.get(key, 0):
            notes.append(f"{what} x{c[key]} NOT applied (scales with a percentage the file "
                         f"does not store) — this score is an upper bound")

    composed = max(route * penalty, 0.0)
    return route, penalty, composed, notes


def cmd_show(args) -> None:
    data = load(Path(args.results[0]))
    recs = records(data)
    if not recs:
        cp = data.get("_checkpoint", {})
        print(f"no completed routes in {args.results[0]}")
        print(f"entry_status: {data.get('entry_status')}   eligible: {data.get('eligible')}")
        if "progress" in cp:
            print(f"progress: {cp['progress']}  (a route in progress has index -1 and is not saved)")
        sys.exit(1)

    print(f"# {args.results[0]}")
    print(f"entry_status : {data.get('entry_status')}   eligible: {data.get('eligible')}")
    if data.get("sensors"):
        print(f"sensors      : {', '.join(data['sensors'])}")
    cp = data.get("_checkpoint", {})
    if "progress" in cp:
        done, total = (cp["progress"] + [None, None])[:2]
        print(f"progress     : {done}/{total} route(s)")

    if args.route is not None:
        sel = [r for r in recs if str(r.get("route_id")) == args.route
               or str(r.get("index")) == args.route]
        if not sel:
            ids = ", ".join(str(r.get("route_id")) for r in recs)
            sys.exit(f"no route {args.route!r}; present: {ids}")
        for r in sel:
            print()
            print(f"route_id : {r.get('route_id')}   index {r.get('index')}   status {r.get('status')}")
            s = r.get("scores", {})
            print(f"scores   : composed {s.get('score_composed')}  route {s.get('score_route')}"
                  f"  penalty {s.get('score_penalty')}")
            m = r.get("meta", {})
            print(f"meta     : length {m.get('route_length')} m, "
                  f"game {m.get('duration_game')} s, system {m.get('duration_system')} s")
            print("infractions:")
            for k, v in sorted((r.get("infractions") or {}).items()):
                n = len(v) if isinstance(v, list) else v
                if n:
                    print(f"  {k:38} {n}")
                    if isinstance(v, list) and args.verbose:
                        for msg in v[:args.verbose * 5]:
                            print(f"      {msg}")
        return

    print()
    print(f"{'ROUTE':22} {'STATUS':22} {'DRIVING':>8} {'ROUTE%':>7} {'PENALTY':>8}  TOP INFRACTIONS")
    for r in recs:
        s = r.get("scores", {})
        c = {k: v for k, v in counts(r).items() if v}
        top = ", ".join(f"{k}x{v}" for k, v in sorted(c.items(), key=lambda kv: -kv[1])[:3])
        print(f"{str(r.get('route_id'))[:22]:22} {str(r.get('status'))[:22]:22} "
              f"{s.get('score_composed', 0):>8.2f} {s.get('score_route', 0):>7.2f} "
              f"{s.get('score_penalty', 0):>8.3f}  {top}")

    gr = cp.get("global_record") or {}
    if gr:
        print()
        print(f"global status : {gr.get('status')}")
        mean, sd = gr.get("scores_mean", {}), gr.get("scores_std_dev", {})
        for key, label in (("score_composed", "driving score"),
                           ("score_route", "route completion"),
                           ("score_penalty", "infraction penalty")):
            print(f"  {label:20} {mean.get(key, 0):>8.3f}  (std dev {sd.get(key, 0):.3f})")
        infr = {k: v for k, v in (gr.get("infractions") or {}).items() if v}
        if infr:
            print("  infractions per km:")
            for k, v in sorted(infr.items(), key=lambda kv: -float(kv[1])):
                print(f"    {k:38} {v}")
        exc = (gr.get("meta") or {}).get("exceptions") or []
        if exc:
            print(f"  {len(exc)} exception(s) recorded:")
            for e in exc[:10]:
                print(f"    {e}")
    else:
        print("\n(no global_record — the run did not finish every route)")

    blocked = [r.get("route_id") for r in recs
               if any(counts(r).get(k, 0) for k in BLOCKING)]
    if blocked:
        print(f"\nroutes ended early (deviation/blocked/timeout): {blocked}")
    if data.get("eligible") is False:
        print("\nNOTE eligible=False — only entry_status 'Finished' is eligible. Any crashed,")
        print("     rejected or unfinished route makes the whole entry ineligible.")


def cmd_rescore(args) -> None:
    path = Path(args.results[0])
    data = load(path)
    recs = records(data)
    if not recs:
        sys.exit("no completed routes to rescore")
    target = args.as_version
    print(f"# rescoring {path} under Leaderboard {target} rules")
    print(f"# {'2.0: P = product(p_j ^ n_j)' if target == '2.0' else '2.1: P = 1 / (1 + sum(c_j * n_j))'}")
    print()
    print(f"{'ROUTE':22} {'STORED':>8} {'AS ' + target:>8} {'DELTA':>8}  NOTES")
    all_notes, stored_all, new_all = set(), [], []
    for r in recs:
        stored = float((r.get("scores") or {}).get("score_composed", 0.0))
        _, _, composed, notes = rescore(r, target)
        stored_all.append(stored)
        new_all.append(composed)
        all_notes |= set(notes)
        print(f"{str(r.get('route_id'))[:22]:22} {stored:>8.2f} {composed:>8.2f} {composed - stored:>+8.2f}"
              f"  {'; '.join(notes)}")
    print()
    print(f"mean stored     : {statistics.fmean(stored_all):.3f}")
    print(f"mean as {target}    : {statistics.fmean(new_all):.3f}")
    if len(new_all) > 1:
        print(f"std dev as {target} : {statistics.stdev(new_all):.3f}")
    for n in sorted(all_notes):
        print(f"CAVEAT {n}")
    print("\nInfraction counts are stored exactly, so every constant-penalty infraction")
    print("rescores exactly. The percentage-scaled ones (min speed, outside route lanes)")
    print("are stored only as messages, so they are left out and flagged above; a route")
    print("with those flags rescores to an upper bound on its true score.")


def cmd_diff(args) -> None:
    a, b = Path(args.results[0]), Path(args.results[1])
    ra = {str(r.get("route_id")): r for r in records(load(a))}
    rb = {str(r.get("route_id")): r for r in records(load(b))}
    keys = sorted(set(ra) | set(rb), key=lambda k: (len(k), k))
    print(f"# A = {a}\n# B = {b}")
    print(f"{'ROUTE':22} {'A':>8} {'B':>8} {'DELTA':>8}  INFRACTION CHANGES")
    sa, sb = [], []
    for k in keys:
        if k not in ra:
            print(f"{k[:22]:22} {'-':>8} {(rb[k].get('scores') or {}).get('score_composed', 0):>8.2f}  only in B")
            continue
        if k not in rb:
            print(f"{k[:22]:22} {(ra[k].get('scores') or {}).get('score_composed', 0):>8.2f} {'-':>8}  only in A")
            continue
        va = float((ra[k].get("scores") or {}).get("score_composed", 0))
        vb = float((rb[k].get("scores") or {}).get("score_composed", 0))
        sa.append(va)
        sb.append(vb)
        ca, cb = counts(ra[k]), counts(rb[k])
        changes = [f"{key} {ca.get(key,0)}->{cb.get(key,0)}"
                   for key in sorted(set(ca) | set(cb)) if ca.get(key, 0) != cb.get(key, 0)]
        print(f"{k[:22]:22} {va:>8.2f} {vb:>8.2f} {vb - va:>+8.2f}  {', '.join(changes)}")
    if sa:
        print(f"\nmean A {statistics.fmean(sa):.3f}   mean B {statistics.fmean(sb):.3f}   "
              f"delta {statistics.fmean(sb) - statistics.fmean(sa):+.3f}")


def cmd_merge(args) -> None:
    """Join shards, the way scripts/merge_statistics.py does."""
    base = load(Path(args.results[0]))
    seen = {str(r.get("route_id")) for r in records(base)}
    added = 0
    for extra in args.merge:
        for r in records(load(Path(extra))):
            rid = str(r.get("route_id"))
            if rid in seen:
                print(f"# skipping duplicate route {rid} from {extra}")
                continue
            base["_checkpoint"]["records"].append(r)
            seen.add(rid)
            added += 1
    base["_checkpoint"]["records"].sort(key=lambda r: r.get("index", 0))
    out = Path(args.out) if args.out else Path(args.results[0]).with_name("merged_results.json")
    out.write_text(json.dumps(base, indent=4))
    print(f"merged {added} route(s) into {out} ({len(seen)} total)")
    print("NOTE the global_record is NOT recomputed here — use the leaderboard's own")
    print("     scripts/merge_statistics.py if you need an official global score.")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("results", nargs="+", help="results.json (two for --diff)")
    ap.add_argument("--route", help="show one route id in full")
    ap.add_argument("--as", dest="as_version", choices=["2.0", "2.1"],
                    help="recompute scores under this version's rules")
    ap.add_argument("--diff", action="store_true", help="compare two results files")
    ap.add_argument("--merge", nargs="+", help="merge these shards into the first file")
    ap.add_argument("--out", help="output path for --merge")
    ap.add_argument("-v", "--verbose", action="count", default=0)
    args = ap.parse_args()

    if args.merge:
        cmd_merge(args)
    elif args.diff:
        if len(args.results) < 2:
            ap.error("--diff needs two results files")
        cmd_diff(args)
    elif args.as_version:
        cmd_rescore(args)
    else:
        cmd_show(args)


if __name__ == "__main__":
    main()
