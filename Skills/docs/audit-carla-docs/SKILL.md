---
name: audit-carla-docs
description: Finds undocumented and rotting documentation in a CARLA checkout — unreleased features, Python bindings missing from the API reference, documented API that no longer exists, sensors and catalogue actors missing from their reference pages, broken links, and pages unreachable from the nav. Static only, no build or server. Use when the user asks to "audit the docs", "find undocumented features", "what needs documenting", or is preparing a release or a new default branch.
license: MIT
compatibility: Any OS with python3 and a CARLA checkout on disk. Needs no built engine, no packaged server, and no running simulator.
metadata:
  group: docs
  prerequisites: scripts/check_env.sh
  reference: references/signals.md
---

# Audit CARLA documentation (static)

Reports what the repository documents badly or not at all, as a markdown list a
human triages. It **only reads files** — so it is fast, repeatable and safe, and
it is blind to anything that exists only at runtime. The companion runtime audit
covers that; this skill will not let you forget it.

> **Paths.** `scripts/…` and `references/…` are relative to the directory holding
> this SKILL.md, not to the user's working directory. Prefix them with its
> absolute path.

The deliverable is a **report**, never an edit. Deciding what to document, and
writing it, are separate jobs — see `document-carla-feature`.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Run the audit, writing the report to a file
- [ ] Step 3: Read the summary table and triage by signal, not top to bottom
- [ ] Step 4: Tell the user a runtime (Tier B) audit is still owed
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

Only two things are hard blockers: no CARLA checkout, and no `python3`. A missing
input file downgrades its signal to skipped rather than failing the run.

### Step 2: Run the audit

```bash
python3 scripts/audit_docs.py --root "${CARLA_ROOT:?}" --out
```

Writes `Docs/audit_carla_docs.md`. Drop `--out` entirely to print to stdout
instead; pass `--out <path>` to write somewhere else.

`--signal <name>` narrows the run, repeatable: `changelog`, `binding`, `stale`,
`catalogue-stale`,
`sensor`, `catalogue`, `link`, `orphan`. Narrow when iterating on one area; run them all before a
release.

### Step 3: Triage

The report opens with a count per signal. **Work by signal, not in file order** —
the five differ enormously in effort per finding, and mixing them wastes time:

| signal | what it means | typical effort |
|---|---|---|
| `changelog` | every unreleased feature, as a checklist | high — **you check each one by hand** |
| `binding` | class or member exposed to Python, absent from the ymls | medium — edit the `.yml`, regenerate |
| `stale` | documented API that no longer exists in the bindings | medium — usually a rename or a typo, not a deletion |
| `catalogue-stale` | a catalogue page names an actor nothing knows about | small — delete the entry and its image |
| `sensor` | sensor id absent from `ref_sensors.md` | medium |
| `catalogue` | vehicle/walker/prop absent from its catalogue page | medium |
| `link` | target page or anchor missing | low — usually a one-line fix |
| `orphan` | page not in the `mkdocs.yml` nav | low — nav entry, or delete the page |

Two cautions the report cannot resolve for you, both in
[references/signals.md](references/signals.md):

- The `changelog` section **makes no claim about whether anything is
  documented.** It lists every unreleased feature as an unticked checkbox; you
  decide. Judging coverage automatically was tried and was too generous to
  trust — see the reference.
- **`Docs/python_api.md` is generated** from `PythonAPI/docs/*.yml` by
  `PythonAPI/docs/doc_gen.py`. Fix a `binding` finding in the `.yml` and
  regenerate. Editing the `.md` is silently reverted by the next run.

### Step 4: Say what this audit could not see

Always close by telling the user a runtime audit is still owed, and why. The
script prints this too, but say it in your own summary — it is the single
easiest thing to forget, and the gap it leaves is invisible.

A static pass cannot see the blueprint library, the available maps, or
per-blueprint attributes: new vehicles, walkers and props are **content**, so
they appear in no source diff.

## Examples

**Example 1: preparing a branch for release**

User says: "audit the docs before we make ue58-dev the default"

```bash
bash scripts/check_env.sh
python3 scripts/audit_docs.py --root "${CARLA_ROOT:?}" --out
```

Result: `Docs/audit_carla_docs.md` with a count per signal. Report the totals, name the two
or three heaviest items, and state that a runtime audit is still owed.

**Example 2: checking one area while fixing it**

User says: "are there still broken links in the docs?"

```bash
python3 scripts/audit_docs.py --root "${CARLA_ROOT:?}" --signal link
```

Result: the link findings on stdout. No file is written; nothing is changed.

## Troubleshooting

**Error: `not a carla checkout (no CHANGELOG.md): <path>`**
Cause: `--root` or `CARLA_ROOT` points somewhere else.
Solution: pass `--root /path/to/carla`, or `export CARLA_ROOT=` before sourcing.

**A signal reports 0 findings and you expected some**
Cause: its input is missing, so it was skipped — `check_env.sh` prints a WARN per
missing input.
Solution: re-read the `check_env.sh` output; a skipped signal is not a clean one.

**`link` findings inside fenced code or HTML comments**
Cause: none — the audit strips both before scanning. A link reported as broken is
in live prose.

## Outputs

- `Docs/audit_carla_docs.md` (or stdout) — findings grouped by signal, with a
  count table and the runtime-audit warning. The audit excludes this file from
  its own link and orphan signals.
