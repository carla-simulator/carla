# Agent briefs

Send these as written, filling the `{placeholders}`. Each agent starts with a clean context: it knows only its brief and the files the brief names. It reports back a short summary; its real output is the files it writes. `{W}` is `Build/docs-work` in the CARLA checkout `{ROOT}`. `{SKILL}` is the directory holding this skill. A long page verified by section has its ledger and verdicts in `{W}/ledger/<page>/*.md` and `{W}/verdicts/<page>/*.md` instead of one file each; a brief that names the ledger or the verdicts reads all of them.

| Brief | Pipeline step | Edits pages |
|---|---|---|
| 1. Research | 1. Facts, new content | no |
| 2. Verify | 1. Facts | no |
| 3. Run | 1. Facts, `needs-run` rows | no |
| 4. Edit | 3. Edit | yes |
| 5. Whole-page pass | 1. Facts (`report`), 4. Whole page (`edit`) | in `edit` only |
| 6. Check | 5. Check | no |

Every brief ends with the same constraints:

> The documentation is written for CARLA {RELEASE}; the previous release is {PREVIOUS}. {RELEASE} may not be tagged yet, so the code can still carry the {PREVIOUS} version string: a page naming {RELEASE} as the current release is correct, not a claim to confirm against that string. Work only in `{ROOT}`. Do not commit, push, or edit files outside the ones named. Do not launch a CARLA server or the editor. Do not trust other doc pages, plans, commit titles or the changelog as evidence; the code is the source of truth ({SKILL}/references/evidence.md). Report in under 15 lines: what you wrote, where, and anything you could not do.

## 1. Research

> Your target is {TARGET}, placed at {PLACEMENT}. Read `{SKILL}/references/evidence.md` in full, and the format reference the target needs (`python_api.md`, `sensors.md`, `changelog.md`, `images.md`), if any.
>
> Find every fact a reader needs from the new content: names, signatures, defaults, paths, commands, values, behaviour, limits. Read the code for each one, and write one row per fact to `{W}/ledger/<page>.md`, with the evidence at `file:line`. A fact you can only establish by running something is marked `needs-run`, with the exact command or snippet. Do not write any page text.

## 2. Verify (fresh)

> You are checking facts you did not establish. Do not guess anyone's reasoning. Read `{SKILL}/references/evidence.md` and `{SKILL}/references/review.md`.
>
> Mode `{MODE}`:
>
> - `page`: read `Docs/{PAGE}` (only the `{SECTION}` section, if one is named). List every factual claim on it in `{W}/ledger/{PAGE}.md` (or `{W}/ledger/{PAGE}/{SECTION}.md`), in the ledger format, with the evidence you found. Also do the checks `review.md` assigns to the verifier: stale facts and style.
> - `ledger`: read the rows of `{W}/ledger/{PAGE}.md` that have no verdict yet. For each, go to the cited evidence yourself. Also check every `needs-run` procedure is self-contained and safe as written (start state, command, what to look for, cleanup that leaves nothing behind), and say where one is not.
>
> For every row, decide whether the evidence supports the claim exactly as stated (names, paths, numbers, versions, defaults, platform). Try to show it is wrong: look for a newer change, a different platform, a different build configuration. A row you can only settle by running something is marked `needs-run`, with the exact command or snippet.
>
> Write the verdicts to `{W}/verdicts/{PAGE}.md` (or `{W}/verdicts/{PAGE}/{SECTION}.md`) as described in `evidence.md`. Do not edit the page.

## 3. Run (one at a time)

> A CARLA server (or the editor) is running at `{ENDPOINT}`. Read `{SKILL}/references/review.md#running-the-page`. For each `needs-run` row in `{W}/verdicts/{PAGE}.md`, run the command or snippet exactly as written and record what happened: `confirmed` or `wrong` with the actual output, trimmed. Nothing is saved: `git status` of the code repository and of `Unreal/CarlaUnreal/Content/Carla` is the same before and after, and every actor you spawn is destroyed. Leave the server as you found it (asynchronous mode).

## 4. Edit

> Your target is {TARGET}, placed at {PLACEMENT}. Read `{SKILL}/references/style.md` and `{SKILL}/references/evidence.md` in full, `CLAUDE.local.md` in `{ROOT}` if it exists, and the format reference the target needs, if any. Your inputs are the confirmed rows in `{W}/ledger/<page>.md` and `{W}/verdicts/<page>.md`, and the findings marked agreed in `{W}/review/<page>.md`.
>
> For a page that already exists, before editing: run `git log --oneline -- Docs/<page>` to find the page's last reviewed text, as `style.md` defines it, so you know whose wording you are keeping. Decide its page type from `style.md`.
>
> Every fact you put on the page comes from a confirmed row. If you need one that has no confirmed row, add it to the ledger as `pending`, leave it off the page, and list it in your report. On an existing page, make the smallest edit that applies the agreed findings, in the page's own voice, and do not reword text no finding names. New content takes the shape in `style.md` for its type. Rows the user agreed to leave unconfirmed follow "When it cannot be confirmed" in `evidence.md`.
>
> Edit only the pages named in the placement.

## 5. Whole-page pass (fresh, one agent per page)

> Read `Docs/{PAGE}` as a whole, as a reader would, and `{SKILL}/references/style.md`. Look for, in this order: information stated more than once on the page (keep the one in the place a reader needs it, remove or link the rest); sections that overlap or say the same thing (merge them); statements that contradict each other; sections out of the order a reader needs them; a section list under the intro that does not match the `##` headings. Keep a use-case list under the intro if it is in the page's original text. Also note, without editing, content that duplicates another page and anything a reader needs to finish the page's task that it does not give.
>
> Mode `{MODE}`: `report` writes the findings only; `edit` applies them. In `edit` you may move, merge and delete. You may not add facts or reword text beyond what a merge requires. Write `{W}/consolidate/{PAGE}.md`: each finding or change, quoting what was removed and where its information now lives.

## 6. Check (fresh)

> You are checking that a page says what its confirmed facts say. Read `git diff -- Docs/{PAGE}`, `{W}/ledger/{PAGE}.md`, `{W}/verdicts/{PAGE}.md` and `{W}/consolidate/{PAGE}.md`.
>
> For every claim the diff adds or changes, find its confirmed row and decide whether the page states it exactly as the row does: same name, path, number, default, platform, and no wider than the evidence (a Linux-only fact written for both platforms is a mismatch). List every claim in the diff that has no confirmed row. For every removal in `consolidate`, confirm the information is still on the page, or was genuinely redundant. Check the diff against `{SKILL}/references/style.md` for the page type.
>
> Write `{W}/check/{PAGE}.md`: one line per mismatch, missing row or lost information, with what the page says and what it should say. Do not edit the page.
