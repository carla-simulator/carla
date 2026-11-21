"""Render results.jsonl into report.md."""

import json
from collections import Counter, defaultdict
from pathlib import Path

ORDER = ("FAIL", "XPASS", "XFAIL", "SKIP", "PASS")


def load(run_dir):
    p = Path(run_dir) / "results.jsonl"
    if not p.exists():
        return []
    rows = {}
    for line in p.read_text().splitlines():
        if line.strip():
            r = json.loads(line)
            rows[r["id"]] = r          # a re-run (--only) replaces the earlier row
    return list(rows.values())


def _tail(path, n=30):
    try:
        return "\n".join(Path(path).read_text(errors="replace").splitlines()[-n:])
    except OSError:
        return ""


def _fmt(v):
    return f"{v:.3g}" if isinstance(v, float) else str(v)


def write(run_dir):
    rows = load(run_dir)
    by_tier = defaultdict(Counter)
    for r in rows:
        by_tier[r["tier"]][r["status"]] += 1
    total = Counter(r["status"] for r in rows)

    md = [f"# CARLA system test report\n", f"Run directory: `{run_dir}`\n",
          "## Summary\n", "| Tier | " + " | ".join(ORDER) + " |",
          "|---|" + "---|" * len(ORDER)]
    for tier in sorted(by_tier):
        md.append(f"| {tier} | " + " | ".join(str(by_tier[tier][s]) for s in ORDER) + " |")
    md.append("| **all** | " + " | ".join(f"**{total[s]}**" for s in ORDER) + " |\n")

    md += ["## All tests\n", "| Test | Status | Time (s) | Message |", "|---|---|---|---|"]
    for r in rows:
        msg = (r.get("msg") or "").splitlines()[0][:140] if r.get("msg") else ""
        md.append(f"| `{r['id']}` | {r['status']} | {r.get('secs', 0):.0f} | {msg} |")

    stress = [r for r in rows if r["tier"] == "T5" and r.get("metrics")]
    if stress:
        md += ["\n## Stress metrics\n"]
        for r in stress:
            m = {k: v for k, v in r["metrics"].items() if not isinstance(v, (list, dict))}
            md.append(f"- `{r['id']}` ({r['status']}): " +
                      ", ".join(f"{k}={_fmt(v)}" for k, v in m.items()))

    bad = [r for r in rows if r["status"] in ("FAIL", "XPASS")]
    if bad:
        md += ["\n## Failures\n"]
        for r in bad:
            md += [f"### `{r['id']}` — {r['status']}\n", "```", (r.get("msg") or "")[:3000], "```"]
            if r.get("notes"):
                md.append("Notes: " + "; ".join(r["notes"]))
            md += ["\nLog tail:\n", "```", _tail(r.get("log", "")), "```\n"]

    out = Path(run_dir) / "report.md"
    out.write_text("\n".join(md) + "\n")
    return out, total
