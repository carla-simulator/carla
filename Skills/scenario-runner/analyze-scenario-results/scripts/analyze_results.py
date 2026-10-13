#!/usr/bin/env python3
"""Summarise, compare and re-measure ScenarioRunner results.

    analyze_results.py summary  <dir|file...>          criteria pass/fail per scenario
    analyze_results.py compare  <dirA> <dirB>          criterion-level diff
    analyze_results.py metrics  --list                 bundled example metrics
    analyze_results.py metrics  --metric M --log L [--criteria C]

`summary` and `compare` are pure file readers — no carla, no server. `metrics`
shells out to metrics_manager.py, which DOES need a running server: it replays the
recording to recover the map for the Waypoint API.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

# ScenarioRunner appends the timestamp with no separator:
#   ControlLoss_1 + 2025-08-14-10-22-31 -> ControlLoss_12025-08-14-10-22-31.json
STAMP = re.compile(r"^(?P<name>.*?)(?P<stamp>\d{4}-\d{2}-\d{2}-\d{2}-\d{2}-\d{2})$")


def split_stamp(stem: str) -> tuple[str, str]:
    m = STAMP.match(stem)
    return (m.group("name"), m.group("stamp")) if m else (stem, "")


def sr_root() -> Path | None:
    for c in [os.environ.get("SCENARIO_RUNNER_ROOT"), os.getcwd(),
              str(Path.home() / "scenario_runner")]:
        if c and (Path(c) / "metrics_manager.py").is_file():
            return Path(c).resolve()
    return None


def collect(paths: list[str]) -> list[Path]:
    out: list[Path] = []
    for p in paths:
        path = Path(p)
        if path.is_dir():
            out += sorted(f for f in path.rglob("*") if f.suffix in {".json", ".xml", ".txt"})
        elif path.is_file():
            out.append(path)
        else:
            print(f"# skipping {p}: not found", file=sys.stderr)
    return out


def parse_json(path: Path) -> dict | None:
    try:
        data = json.loads(path.read_text())
    except Exception:
        return None
    # Two shapes land in .json files: ScenarioRunner's result dump, and the
    # criteria dump written next to a --record recording. Tell them apart by keys.
    crits: list[dict] = []
    if isinstance(data, dict) and "criteria" not in data and all(
            isinstance(v, dict) for v in data.values()) and data:
        for name, body in data.items():
            if not isinstance(body, dict):
                continue
            status = body.get("test_status") or body.get("_test_status")
            crits.append({"name": name,
                          "status": str(status) if status else "?",
                          "actual": body.get("actual_value"),
                          "expected": body.get("expected_value_success")})
        if crits:
            return {"kind": "criteria-dump", "criteria": crits}
    if isinstance(data, dict):
        for key in ("criteria", "records", "results"):
            if key in data and isinstance(data[key], list):
                for c in data[key]:
                    if isinstance(c, dict):
                        crits.append({"name": c.get("name") or c.get("criterion", "?"),
                                      "status": criterion_status(c),
                                      "actual": c.get("actual_value", c.get("actual")),
                                      "expected": c.get("expected_value_success",
                                                        c.get("expected")),
                                      "optional": bool(c.get("optional", False))})
                # ScenarioRunner also records the run's own verdict; keep it, because
                # a scenario can fail for reasons no single criterion reports.
                return {"kind": "result", "criteria": crits,
                        "overall": (None if "success" not in data
                                    else ("SUCCESS" if data["success"] else "FAILURE"))}
    return {"kind": "unknown", "criteria": crits, "raw": data}


def parse_junit(path: Path) -> dict:
    crits = []
    try:
        root = ET.parse(path).getroot()
    except ET.ParseError:
        return {"kind": "junit-broken", "criteria": crits}
    for case in root.iter("testcase"):
        failed = case.find("failure") is not None or case.find("error") is not None
        crits.append({"name": case.attrib.get("name", "?"),
                      "status": "FAILURE" if failed else "SUCCESS",
                      "actual": None, "expected": None})
    return {"kind": "junit", "criteria": crits}


def parse_txt(path: Path) -> dict:
    # The .txt is the pretty table; recover criterion rows by their status token.
    crits = []
    for line in path.read_text(errors="replace").splitlines():
        m = re.search(r"(\w+Test|\w+Criterion)\b.*?\b(SUCCESS|FAILURE|ACCEPTABLE)\b", line)
        if m:
            crits.append({"name": m.group(1), "status": m.group(2),
                          "actual": None, "expected": None})
    return {"kind": "txt", "criteria": crits}


def load(path: Path) -> dict:
    if path.suffix == ".json":
        return parse_json(path) or {"kind": "bad-json", "criteria": []}
    if path.suffix == ".xml":
        return parse_junit(path)
    return parse_txt(path)


def criterion_status(c: dict) -> str:
    """Normalise the several shapes ScenarioRunner has written over time.

    From 0.9.15 the JSON writer emits a BOOLEAN `success` per criterion
    (result_writer.py), with no `test_status` field at all. Reading only
    `test_status`/`status` left every criterion as "?", which `failed()` then
    counted as a pass — so a failing run was reported as "all passed".
    """
    for key in ("test_status", "status"):
        if c.get(key) is not None:
            return str(c[key])
    if isinstance(c.get("success"), bool):
        return "SUCCESS" if c["success"] else "FAILURE"
    return "?"


def failed(c: dict) -> bool:
    st = c["status"].upper()
    return "FAIL" in st or "ERROR" in st


def cmd_summary(args) -> None:
    files = collect(args.paths)
    if not files:
        sys.exit("no result files found — run with OUTPUT=1 JSON=1 OUTPUT_DIR=<dir>")
    any_fail = False
    print(f"{'SCENARIO':38} {'WHEN':21} {'CRIT':>5} {'FAIL':>5}  FAILING CRITERIA")
    for f in files:
        name, stamp = split_stamp(f.stem)
        res = load(f)
        crits = res["criteria"]
        if not crits:
            print(f"{name:38} {stamp:21} {'-':>5} {'-':>5}  (no criteria parsed from {f.suffix}: {res['kind']})")
            continue
        bad = [c["name"] for c in crits if failed(c)]
        # A run can be marked failed with every criterion green (an exception mid
        # scenario, a watchdog fire). Trust the recorded verdict over the tally.
        overall = res.get("overall")
        note = ", ".join(bad) or "all passed"
        if overall == "FAILURE" and not bad:
            note = "run marked FAILED with no failing criterion"
        any_fail |= bool(bad) or overall == "FAILURE"
        print(f"{name:38} {stamp:21} {len(crits):>5} {len(bad):>5}  {note}")
    if args.verbose:
        print()
        for f in files:
            res = load(f)
            for c in res["criteria"]:
                if failed(c) or args.verbose > 1:
                    extra = ""
                    if c.get("actual") is not None:
                        extra = f"  actual={c['actual']} expected={c.get('expected')}"
                    print(f"  {f.name}: {c['name']:32} {c['status']}{extra}")
    sys.exit(1 if any_fail else 0)


def cmd_compare(args) -> None:
    def index(paths: list[str]) -> dict[str, dict[str, str]]:
        out: dict[str, dict[str, str]] = {}
        for f in collect(paths):
            name, _ = split_stamp(f.stem)
            res = load(f)
            # Later timestamps overwrite earlier ones for the same scenario.
            out.setdefault(name, {}).update({c["name"]: c["status"] for c in res["criteria"]})
        return out

    a, b = index([args.a]), index([args.b])
    names = sorted(set(a) | set(b))
    if not names:
        sys.exit("nothing to compare")
    changes = 0
    for n in names:
        ca, cb = a.get(n, {}), b.get(n, {})
        if not ca:
            print(f"+ {n}: only in {args.b}")
            continue
        if not cb:
            print(f"- {n}: only in {args.a}")
            continue
        for crit in sorted(set(ca) | set(cb)):
            sa, sb = ca.get(crit, "<absent>"), cb.get(crit, "<absent>")
            if sa != sb:
                arrow = "WORSE" if ("FAIL" in sb.upper() and "FAIL" not in sa.upper()) else \
                        ("BETTER" if ("FAIL" in sa.upper() and "FAIL" not in sb.upper()) else "changed")
                print(f"{arrow:7} {n:34} {crit:30} {sa} -> {sb}")
                changes += 1
    print(f"\n{changes} criterion change(s) between {args.a} and {args.b}")


def cmd_metrics(args) -> None:
    root = sr_root()
    if root is None:
        sys.exit("no scenario_runner checkout found — export SCENARIO_RUNNER_ROOT")
    examples = root / "srunner" / "metrics" / "examples"
    if args.list:
        print(f"# {examples}")
        for f in sorted(examples.glob("*.py")):
            if f.name == "basic_metric.py":
                print(f"  {f.name:34} (base class, not runnable on its own)")
            else:
                print(f"  {f.name}")
        data = root / "srunner" / "metrics" / "data"
        if data.is_dir():
            print(f"\n# sample recordings in {data} (may predate your CARLA version)")
            for f in sorted(data.glob("*.log")):
                print(f"  {f.name}  + {f.stem}_criteria.json")
        return
    if not args.metric or not args.log:
        sys.exit("metrics needs --metric and --log (or --list)")
    log = Path(args.log)
    if not log.is_absolute():
        # --record paths are relative to SCENARIO_RUNNER_ROOT, which is where
        # people naturally copy the path from.
        cand = root / args.log
        log = cand if cand.exists() else log
    if not log.exists():
        sys.exit(f"recording {log} not found (--record writes under {root})")
    cmd = [sys.executable, str(root / "metrics_manager.py"),
           "--metric", args.metric, "--log", str(log),
           "--host", args.host, "--port", str(args.port)]
    crit = args.criteria
    if not crit:
        guess = log.with_suffix(".json")
        if guess.exists():
            crit = str(guess)
            print(f"# using criteria file {guess}")
    if crit:
        cmd += ["--criteria", crit]
    print("# " + " ".join(cmd))
    print("# NOTE metrics_manager needs a RUNNING server: it replays the log to recover the map")
    sys.exit(subprocess.run(cmd, cwd=str(root)).returncode)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("summary")
    p.add_argument("paths", nargs="+")
    p.add_argument("-v", "--verbose", action="count", default=0)
    p.set_defaults(func=cmd_summary)
    p = sub.add_parser("compare")
    p.add_argument("a")
    p.add_argument("b")
    p.set_defaults(func=cmd_compare)
    p = sub.add_parser("metrics")
    p.add_argument("--list", action="store_true")
    p.add_argument("--metric")
    p.add_argument("--log")
    p.add_argument("--criteria")
    p.add_argument("--host", default=os.environ.get("CARLA_HOST", "127.0.0.1"))
    p.add_argument("--port", type=int, default=int(os.environ.get("CARLA_PORT", 2000)))
    p.set_defaults(func=cmd_metrics)
    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
