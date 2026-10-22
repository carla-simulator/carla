---
name: document-carla-feature
description: Updates the CARLA documentation to match what changed in the code for a release — new features, behaviour changes, renames and removals — by running a pipeline of subagents with clean contexts, one nav section at a time, with every claim checked against the code before a person sees it. Also covers the Python API YAML, sensors, changelog and doc images. Use when the user asks to "document the release", "update the docs for what changed", "document this feature", "fix the audit findings", or names a nav section or a category such as "the Python API part".
license: MIT
compatibility: Any CARLA checkout. Edits files on disk and builds the docs with mkdocs; nothing is committed by the agents. The images category renders with a running CARLA server and needs Pillow, numpy and matplotlib. The python-api category regenerates Docs/python_api.md with PythonAPI/docs/doc_gen.py, which needs PyYAML.
metadata:
  group: docs
  prerequisites: scripts/check_env.sh
  reference: references/agents.md
---

# Document a CARLA release

Brings `Docs/` in line with what changed in the code since the last release. The main session only coordinates and talks to the user. Every piece of work runs in a subagent that starts with a clean context, and agents hand work to each other only through files, never through conversation.

> **Paths.** `scripts/…` and `references/…` are relative to the directory holding this SKILL.md. The working folder is `Build/docs-release/` in the CARLA checkout (`Build/` is git-ignored, so nothing there can be committed by accident).

## Read before starting

- [references/style.md](references/style.md): how a page is written, by page type, with examples of what was rejected before.
- [references/evidence.md](references/evidence.md): what counts as evidence, the ledger format, and when a claim becomes a visible TODO.
- [references/agents.md](references/agents.md): the brief each subagent gets. Send them as written, with the placeholders filled.
- `CLAUDE.local.md` in the checkout, if present: the user's house rules.

## Inputs

- **Baseline:** the previous release tag (`0.10.0`). Everything in `git log 0.10.0..HEAD` is in scope.
- **Target:** the release being documented (`1.0.0`). Pages are written for it, even before it is published.
- **Scope:** one or more mkdocs nav sections (the top-level entries of `mkdocs.yml` `nav:`), or all of them.

## The pipeline

```
main session ── coordinates, talks to the user, reads only summaries
  1. inventory    (1 agent)            -> changes.json
  2. mapping      (1 agent)            -> plan/<section>.json
  3. write        (1 agent / section)  -> edits + ledger/<page>.md
  4. verify       (1 agent / section)  -> verdicts/<page>.md
  5. fix          (1 agent / section, at most 2 rounds)
  6. consolidate  (1 agent / page)     -> whole-page pass, then re-verify what it moved or removed
  7. main session: mkdocs build --strict, coverage report, page-by-page review with the user
```

1. **Inventory.** One user-visible change per entry: new feature, behaviour change, rename or deprecation, removal. Sources are the `## LATEST Changes` section of `CHANGELOG.md` and `git log 0.10.0..HEAD` on the code, because the changelog misses things. Internal plumbing with no API, no flag and no behaviour change is dropped here, with the reason recorded.
2. **Mapping.** Each change gets the nav section and `page#section` a reader would look in (almost always an existing page), the kind of edit, and every place in `Docs/` that names a renamed or removed thing. A change that touches several sections is split, so each section's writer owns its pages alone.
3. **Write.** One change at a time: a small edit and one ledger row per claim it makes. Follows `style.md` for the page type.
4. **Verify.** A fresh agent that sees only the diff, the ledger and the repository, never the writer's reasoning. It tries to show each claim is wrong.
5. **Fix.** A fresh agent fixes the rejected rows. After two rounds, whatever is still unconfirmed becomes a visible TODO on the page and is flagged to the user.
6. **Consolidate.** A fresh agent reads each edited page as a whole and removes duplicated information, merges redundant sections, fixes contradictions and section order. It may move and delete, never add facts. A verifier then checks that nothing the page needs was lost.
7. **Hand-off.** The main session builds the docs strictly, writes the coverage report, and goes through the pages with the user one at a time: the diff, the ledger, the open questions. The user approves each page; only then is a commit message proposed, in plain words ("update the X page"), and shown before committing.

**Every subagent gets its own context.** Launch each one as a fresh agent (`general-purpose`), never as a fork: a fork inherits the main session's whole conversation, which is exactly what the pipeline avoids. Pass paths, not content, and keep each agent's report short; the main session's context grows only by those reports.

Sections run in parallel in steps 3 to 6: a page belongs to one nav section, so writers never touch the same file. Run one section first on a new release to check the briefs before running the rest.

The Workflow tool can run this as a fixed pipeline, but only when the user asks for a workflow. Otherwise run the same steps with Agent calls.

## Done means

- Every entry in `changes.json` is either documented at a named `page#section`, or skipped with a reason. That list is the coverage report.
- `mkdocs build --strict` passes.
- Every changed claim on every page has a ledger row marked confirmed, or a visible TODO.
- The user has approved the page.

## Categories

Some changes land in files with their own rules. The writer loads the matching reference only when a change needs it.

| category | files it edits | reference |
|---|---|---|
| `python-api` | `PythonAPI/docs/*.yml`, then regenerate | [references/python_api.md](references/python_api.md) |
| `changelog` | `CHANGELOG.md` | [references/changelog.md](references/changelog.md) |
| `sensors` | `Docs/ref_sensors.md` | [references/sensors.md](references/sensors.md) |
| `images` | `Docs/img/<page>/`, `Docs/img/catalogue/` | [references/images.md](references/images.md) |

[`audit-carla-docs`](../audit-carla-docs/SKILL.md) still finds gaps that are not tied to a release (undocumented bindings, broken links, pages missing from the nav). Its findings can be fed into step 2 as extra changes.

## Gotchas

- `Docs/python_api.md` and `Docs/bp_library.md` are **generated**. Edit the YAML or the generator and regenerate.
- **The code is the source of truth.** Other doc pages, `DOCS_REVIEW_PLAN.md`-style plans, commit titles, memory and the changelog are leads, not evidence. Changelog entries written during development are often no longer true.
- **Pages written for UE4 / 0.9.x** repeat the same stale facts. Grep for them and check each against the tree: `make launch|import|PythonAPI|package|rebuild` (now `cmake --build Build/Release --target ...` and `Util/Tools/Import.py`), `UE4_ROOT` (now `CARLA_UNREAL_ENGINE_PATH`), Unreal 4.26/4.27 and Visual Studio 2019 (5.8 and 2022), `.egg` files (wheels), `PythonAPI/carla/source` (`PythonAPI/carla/src`), `boost::shared_ptr` in bindings (`std::shared_ptr`), sensors overriding `Tick` (`ASensor::Tick` is `final`: override `PostPhysTick`) and `Stream.Send` (`SerializeAndSend`), `CarlaUE4` paths and `../img/` links. Content paths move too: list the folder before writing one.
- **Package layout changed.** The 0.10.0 package has `CarlaUnreal.sh` at its root; packages built from the UE 5.8 line put it in `Linux/`. Check a local package before writing any path inside one.
- **Generators that need a server** (`bp_doc_gen.py`, `catalogue_doc_gen.py`) can be broken for months without anyone noticing; run them after editing them.
