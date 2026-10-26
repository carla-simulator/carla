---
name: document-carla-feature
description: Writes and reviews CARLA documentation for a target the user names — a page, a section or a feature — in CARLA's house style. Facts come first: every claim is checked against the code by a fresh agent before any text is written, and the finished text is checked against those facts before a person sees it. Covers Docs/ pages, the Python API YAML, ref_sensors.md, the changelog and doc images. Use when the user asks to "document this feature", "write the docs for X", "review this page", "check this docs page against the code", "fix this page", "update the doc images", or names a page, a section or a category such as "the Python API part".
license: MIT
compatibility: Any CARLA checkout. Edits files on disk and builds the docs with mkdocs; nothing is committed by the agents. Images render with a running CARLA server and need Pillow, numpy, matplotlib and Chrome. Python API entries regenerate Docs/python_api.md with PythonAPI/docs/doc_gen.py, which needs PyYAML.
metadata:
  group: docs
  prerequisites: scripts/check_env.sh
  reference: references/agents.md
---

# Write and review CARLA documentation

One pipeline, always on a target the user names: a page, a section of a page, or a feature. It starts from an existing page, from new content to add, or from both, and the order is the same: **check the facts, agree on the changes, then write.** Nothing reaches a page that a fresh agent has not confirmed against the code.

The main session talks to the user and coordinates. Every other step runs in a fresh agent (`general-purpose`, never a fork: a fork inherits the whole conversation, which is what a clean check avoids). Agents hand work to each other only through files; pass them paths, not content, and keep their reports short.

> **Paths.** `scripts/…` and `references/…` are relative to the directory holding this SKILL.md. The working folder `{W}` is `Build/docs-work/` in the CARLA checkout (`Build/` is git-ignored, so nothing there can be committed by accident).

## Inputs

- **Target:** the page, section or feature the user names.
- **Release:** the release the docs are written for (`1.0.0`) and the previous one (`0.10.0`). The target release may not be tagged yet, so the version in the code can still be the previous one; pass both to every brief.

## Read before starting

- [references/agents.md](references/agents.md): the brief each agent gets. Send them as written, with the placeholders filled.
- [references/evidence.md](references/evidence.md): what counts as evidence, the ledger, verdicts, and when a claim becomes a visible TODO.
- [references/review.md](references/review.md): what a review checks and how the findings are reported.
- [references/style.md](references/style.md): how a page is written, by page type, and the shape of a new section or page.
- `CLAUDE.local.md` in the checkout, if present: the user's house rules.

## The pipeline

```
0. Placement    main session                     -> shown to the user
1. Facts        verify (page), research + verify (new content), whole-page pass (report)
                run queue: one at a time, one server
2. Report       main session                     -> the user agrees
3. Edit         writer, from confirmed rows and agreed findings only
4. Whole page   whole-page pass (edit)
5. Check        fresh agent: the diff says exactly what the ledger says
6. Hand-off     main session: build, the user approves, commit message
```

0. **Placement** (main session). Decide where a reader would look for the target: an existing page almost always; a new section when the page has no place for it; a new page only when nothing fits, and then where it goes in the `mkdocs.yml` nav. Grep `Docs/` (excluding the generated `python_api.md` and `bp_library.md`) for every name the target renames or removes; each occurrence is part of the job. Show the placement to the user. A review of an existing page has nothing to place; it starts at step 1.
1. **Facts.** No page is edited in this step.
   - *Existing page*: the verifier (brief 2, mode `page`) lists every claim on the page in the ledger with its verdict. The whole-page pass (brief 5, mode `report`) runs alongside it.
   - *New content*: the researcher (brief 1) writes the ledger of facts the new content needs; a fresh verifier (brief 2, mode `ledger`) confirms each row.
   - Claims that need a server or an editor are marked `needs-run`; the run agent (brief 3) takes them one at a time (see [Running things](#running-things)).
2. **Report** (main session). Write the findings and the plan for new content to `{W}/review/<page>.md` in the format in [review.md](references/review.md), and go through it with the user. Only the findings they agree to go forward. Rows that stay unconfirmed follow "When it cannot be confirmed" in [evidence.md](references/evidence.md).
3. **Edit** (brief 4). The writer works only from confirmed ledger rows and agreed findings. A fact it needs that has no confirmed row is added as `pending` and left off the page; pending rows go back through step 1 before the writer uses them.
4. **Whole page** (brief 5, mode `edit`): duplication, contradictions, section order. It moves, merges and deletes; it adds no facts.
5. **Check** (brief 6, fresh). Every changed claim in the diff matches a confirmed row exactly, no claim is missing a row, and nothing the whole-page pass removed was lost. A mismatch goes back to the writer once and is checked again; what is still wrong after that is reported to the user.
6. **Hand-off** (main session). `mkdocs build --strict`, then grep the output for `anchor` (dead in-page anchors only print as `INFO`), and check the page is in the nav or linked from one that is. Go through the page with the user: the diff, the ledger, the open points. Only after they approve it, propose a commit message in plain words ("update the X page") and show it before committing.

## Short path for trivial edits

A target is trivial when all of these hold: it changes existing text on one page, a few lines at most; it adds no section; it rests on at most about three facts; and none of them needs a server or the editor. A stale path, a renamed flag, a version number, a broken link or a typo usually is. Anything else takes the full pipeline.

1. **Facts.** The main session writes the ledger rows, with the evidence at `file:line`, and a fresh verifier (brief 2, mode `ledger`) confirms them.
2. **Proposal.** Show the user each edit as the old line and the new line, with its ledger row, in one message. This replaces the placement and the report.
3. **Edit.** Once the user agrees, the main session applies exactly the proposed lines. There is no whole-page pass and no check: the edit is the confirmed row, word for word.
4. **Hand-off.** Build only if the edit touches a link, an anchor or the nav. Then the commit message, as in the full pipeline.

**Wording and style only.** An edit that changes no fact (rewording a few sentences, fixing grammar, heading depth, admonition casing, table format, Linux before Windows, unwrapping hard-wrapped prose) skips step 1: there is nothing to verify. The proposal says that no fact changes, and shows the old and new text side by side so the user can see that the meaning did not move either. A mechanical change (casing, wrapping, order) can cover the whole page; rewording stays within a few sentences, and only where the text is wrong or the user asked, because [style.md](references/style.md) keeps a page's original wording. If the rewording has to touch a fact to read well, it is no longer style-only: that fact goes through step 1.

Switch to the full pipeline as soon as the target stops being trivial: the verifier finds a row wrong for a reason that reaches beyond the line, a fact turns out to need running, or the fix spreads to more lines or pages than proposed.

## Orchestration

- **One writer per page**, so the page keeps one voice and no two agents edit the same file.
- **Long pages get one verifier per `##` section** when the page is longer than about 100 lines or has more than four `##` sections; short neighbouring sections can share one, up to about 70 lines per verifier. Decide from the page itself, before any claim is counted. The verifiers run in parallel and each writes `{W}/ledger/<page>/<section>.md` and `{W}/verdicts/<page>/<section>.md`.
- **Several pages run in parallel**, at most about four at a time, each through its own pipeline. A rename that touches several pages has one placement listing every occurrence, then one pipeline per page. The user reviews the pages one by one, in nav order.
- **`{W}/STATUS.md`** records where each page is, one line per page, rewritten whole after every agent report so a new session can carry on from it:

  ```markdown
  - build_faq.md: full; facts done (31 ok, 4 wrong); report agreed FAQ-1..5; edit done; check 1 mismatch, fixed; approved
  - agent_skills.md: short (fact); facts 3 confirmed; proposed
  ```

- **Agent calls by default.** The Workflow tool can run the same pipeline as a fixed script for a multi-page update, but only when the user asks for a workflow.

### Running things

Static checks (reading the code) run in parallel. Anything that needs a server or an editor does not: the GPU fits one at a time (editor ~15 GB, server ~8 GB on 24 GB), and parallel clients break each other in synchronous mode.

- The main session owns the one server or editor: it starts it, and stops it when the queue is empty.
- Verifiers and researchers never launch one. They mark the row `needs-run` with the exact command or snippet.
- The run agent (brief 3) takes the queue one row at a time against that server, and writes the result into the verdicts.
- **Long commands** (a build, a cook, a package) outlast an agent's command timeout. The main session runs them in the background and owns any service they need (a Zen server, for instance); the run agent then checks the results and runs the short rows. Run them in a disposable environment (a fresh worktree) when they overwrite build output, and check the free disk space against what they will write before starting.

## Done means

- Every changed claim on the page has a confirmed ledger row, or a visible TODO.
- Every command the page tells the reader to run was run, or is listed as not run.
- In the full pipeline, the check in step 5 found no mismatch, or the user accepted the ones left.
- `mkdocs build --strict` passes and the page is reachable.
- The user has approved the page.

## Formats

Some targets land in files with their own rules. The researcher and the writer load the matching reference only when the target needs it.

| format | files it edits | reference |
|---|---|---|
| Python API | `PythonAPI/docs/*.yml`, then regenerate | [references/python_api.md](references/python_api.md) |
| Sensors | `Docs/ref_sensors.md` | [references/sensors.md](references/sensors.md) |
| Changelog | `CHANGELOG.md` | [references/changelog.md](references/changelog.md) |
| Images | `Docs/img/<page>/`, `Docs/img/catalogue/` | [references/images.md](references/images.md) |

[`audit-carla-docs`](../audit-carla-docs/SKILL.md) finds targets: undocumented bindings, broken links, pages missing from the nav.

## Gotchas

- `Docs/python_api.md` and `Docs/bp_library.md` are **generated**. Edit the YAML or the generator and regenerate.
- **The code is the source of truth.** Other doc pages, review plans, commit titles, memory and the changelog are leads, not evidence. Changelog entries written during development are often no longer true.
- **Pages written for UE4 / 0.9.x** repeat the same stale facts; [review.md](references/review.md) has the list to grep for.
- **Package layout changed.** The 0.10.0 package has `CarlaUnreal.sh` at its root; packages built from the UE 5.8 sources put it in `Linux/`. Check a local package before writing any path inside one.
- **Generators that need a server** (`bp_doc_gen.py`, `catalogue_doc_gen.py`) can be broken for months without anyone noticing; run them after editing them.
