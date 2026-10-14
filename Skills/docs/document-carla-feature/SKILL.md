---
name: document-carla-feature
description: Writes the missing CARLA documentation that the audit found — Python API entries in PythonAPI/docs/*.yml, sensors in ref_sensors.md, catalogue actors, broken links and nav entries — one audit category at a time, in CARLA's house style. Use when the user asks to "document this feature", "write the docs for X", "fix the audit findings", or names a category such as "the Python API part".
license: MIT
compatibility: Any CARLA checkout. Static only — edits files on disk, no build and no running server. The python-api category regenerates Docs/python_api.md with PythonAPI/docs/doc_gen.py, which needs PyYAML.
metadata:
  group: docs
  prerequisites: scripts/check_env.sh
  requires: audit-carla-docs
  reference: references/python_api.md
---

# Document a CARLA feature

The writing half of the documentation programme. [`audit-carla-docs`](../audit-carla-docs/SKILL.md)
decides **what** is missing and writes `Docs/audit_carla_docs.md`; this skill
writes it.

> **Paths.** `scripts/…` and `references/…` are relative to the directory
> holding this SKILL.md, not to the user's working directory.

## One category at a time

The audit's signals are independent, and so is the work. Each category below
touches its own files, so one person can take one category, or several agents
can run all of them at once without colliding. **Never work two categories in
one pass** — the rules differ and the review gets muddled.

| category | audit signal | files it edits | status |
|---|---|---|---|
| `python-api` | `binding`, `stale` | `PythonAPI/docs/*.yml`, then regenerate | [references/python_api.md](references/python_api.md) |
| `changelog` | `changelog` | wherever the feature belongs | not written yet |
| `sensors` | `sensor` | `Docs/ref_sensors.md` | [references/sensors.md](references/sensors.md) |
| `catalogue` | `catalogue` | `Docs/catalogue_*.md` | not written yet |
| `links` | `link` | the page holding the link | not written yet |
| `nav` | `orphan` | `mkdocs.yml` | not written yet |

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Pick ONE category and read its reference in full
- [ ] Step 3: Read the matching section of Docs/audit_carla_docs.md
- [ ] Step 4: Triage — not every finding should be documented
- [ ] Step 5: Write the entries, following references/style.md
- [ ] Step 6: Remove what no longer exists, checking first that it was not renamed
- [ ] Step 7: Verify, and report what you skipped and why
```

**Step 4 is not optional.** The audit reports what it sees, not what deserves
an entry; it has no way to tell a public API from binding plumbing. Each
reference lists the exemptions for its category. Say in the report which
findings you skipped and why — a silent skip reads as an oversight later.

**Step 5** always obeys [references/style.md](references/style.md), which is
shared by every category: what a CARLA doc sentence says, and what it never
says.

**Step 6 removes, it does not deprecate.** The reference describes what exists
now, so a name the code no longer has is deleted outright — no tombstone and no
"removed in" marker. Check it was not simply renamed first; most of the time it
was, and then the entry moves instead of going.

## Gotchas

- `Docs/python_api.md` is **generated**. Editing it directly is lost on the
  next regeneration, and the audit will not notice.
- A finding is a candidate, not an instruction. Documenting an internal name
  makes the reference worse, not better.
- Re-run the audit after a pass. The counts moving is the only evidence the
  work landed.

## Output

Edited source files, and a report naming what was written, what was skipped
and why. Nothing is committed.
