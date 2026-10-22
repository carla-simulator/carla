# Subagent briefs

Send these as written, filling the `{placeholders}`. Each agent starts with a clean context: it knows only its brief and the files the brief names. It reports back a short summary; its real output is the files it writes. `{W}` is `Build/docs-release` in the CARLA checkout `{ROOT}`. `{SKILL}` is the directory holding this skill.

Every brief ends with the same constraints:

> Work only in `{ROOT}`. Do not commit, push, or edit files outside the ones named. Do not trust other doc pages, plans, commit titles or the changelog as evidence; the code is the source of truth ({SKILL}/references/evidence.md). Report in under 15 lines: what you wrote, where, and anything you could not do.

## 1. Inventory

> List every user-visible change between `{BASELINE}` and `HEAD` in `{ROOT}`. Start from the `## LATEST Changes` section of `CHANGELOG.md`, then read `git log {BASELINE}..HEAD --no-merges` on the code (not `Docs/`) to find changes the changelog misses. A change is user-visible if it adds or changes an API, a command-line flag, a build option, a file or path a user touches, a default, or observable behaviour. Drop internal refactors, and record each drop with its reason.
>
> Write `{W}/changes.json`: a list of `{id, kind: new|changed|renamed|removed, summary, names: [...identifiers, flags, paths...], evidence: [file:line or commit], source: changelog|git|both}` and a separate list `dropped: [{summary, reason}]`. Check that each `names` entry exists (or, for removals, no longer exists) in the tree.

## 2. Mapping

> Read `{W}/changes.json`, `mkdocs.yml` and `Docs/`. For each change decide the nav section and the `page#section` where a reader would look for it: an existing page whenever one fits; a new page only when nothing does, and then name where it goes in the nav. For each change also grep `Docs/` (excluding generated `python_api.md` and `bp_library.md`) for every name in `names`, and list each occurrence that has to change.
>
> Kind of edit, one per target: `update-fact` (path, version, flag, default), `add-item` (sentence, bullet or table row in an existing list or paragraph), `add-section`, `remove` (only after checking the thing was not renamed), `regenerate` (YAML source, then generator). A change touching several sections is split into one entry per section.
>
> Write `{W}/plan/<section>.json` per nav section, each a list of `{change_id, page, anchor, edit, note}`, and `{W}/plan/unplaced.json` for changes you could not place, with the reason.

## 3. Write (one agent per nav section)

> Read `{SKILL}/references/style.md` and `{SKILL}/references/evidence.md` in full, and `CLAUDE.local.md` in `{ROOT}` if it exists. Then work through `{W}/plan/{SECTION}.json`, one entry at a time.
>
> For each page, before editing: run `git log --oneline -- Docs/<page>` and `git show {BASELINE}:Docs/<page>` (or the closest older version) so you know the page's original wording. Decide its page type (reference, narrative, task) from `style.md`.
>
> For each entry: make the smallest edit that brings the page in line with the code, in the page's own voice; read the code that backs every fact you write; add one ledger row per claim to `{W}/ledger/<page>.md`. Do not reword text the entry does not require you to change. Load a category reference (`python_api.md`, `sensors.md`, `images.md`, `changelog.md`) only when the entry needs it. Run any command or snippet you put on the page if this machine can; otherwise mark it not run.
>
> Edit only the pages in your plan file.

## 4. Verify (one agent per nav section, fresh)

> You are checking another writer's work. You have not seen their reasoning and should not guess it. For each page in `{PAGES}`: read `git diff -- Docs/<page>` and `{W}/ledger/<page>.md`. For every ledger row, go to the cited evidence and decide whether it supports the claim exactly as the page states it (names, paths, numbers, versions, defaults, platform). Try to show it is wrong: look for a newer change, a different platform, a different build configuration. Also list every factual claim in the diff that has no ledger row.
>
> Write `{W}/verdicts/<page>.md` as described in `{SKILL}/references/evidence.md`. Do not edit the page.

## 5. Fix (fresh, at most two rounds)

> For each page in `{PAGES}`, read `{W}/verdicts/<page>.md`. For every `wrong` or `unconfirmed` row: correct the page and the ledger from the code, or, if it cannot be confirmed, follow "When it cannot be confirmed" in `{SKILL}/references/evidence.md`. Touch only the lines the verdicts name.

## 6. Consolidate (one agent per page, fresh)

> Read `Docs/{PAGE}` as a whole, as a reader would, and `{SKILL}/references/style.md`. Fix, in this order: information stated more than once on the page (keep the one in the place a reader needs it, remove or link the rest); sections that overlap or say the same thing (merge them); statements that contradict each other; sections out of the order a reader needs them; an in-page table of contents (remove it; the sidebar already lists the headings). Keep a use-case list under the intro if it is in the page's original text. Also note, without editing, content that duplicates another page.
>
> You may move, merge and delete. You may not add facts or reword text beyond what a merge requires. Write `{W}/consolidate/{PAGE}.md`: each change you made, quoting what was removed and where its information now lives.

Then run step 4 again on the page, with this addition to the brief: "Also check `{W}/consolidate/{PAGE}.md`: confirm that every piece of information that was removed is still on the page, or was genuinely redundant."
