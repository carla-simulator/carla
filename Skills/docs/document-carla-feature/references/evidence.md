# Evidence

Every fact on a page, or about to go on one, gets one row in the page's ledger before any text is written. The researcher writes the rows for new content, the verifier writes them for an existing page, and a fresh verifier confirms each one. The writer only uses confirmed rows, and the user spot-checks them, so a row must let someone confirm the claim in one step.

## What counts as evidence

| Counts | Does not count |
|---|---|
| The code, at `file:line` (bindings, CMake, scripts, config, C++) | Another doc page, including the one being edited |
| Output of a command you ran (`ls` of a package, `--help`, a snippet run against a server) | A commit title or PR description |
| The published artifact itself (release notes, a package listing, an image config read from the registry) | The changelog, a review plan, an audit report, memory |
| | "CI builds with 3.10, so the wheel is cp310" — an inference |

Leads are fine for finding where to look. A claim is only confirmed by the left column.

- **Uncommitted code** counts, but the row says `(working tree)`: the claim is only true once that code is committed, so the page edit goes in with it.
- **Examples are not claims.** Text the page offers as an illustration (a sample sentence in a style guide, placeholder values in a snippet) gets no row; the snippet itself still has to run.
- **Another project** (ScenarioRunner, Scenic, the Leaderboard, NVIDIA's tools) is checked against that project's own repository or published docs, cited by URL. A claim no such source settles stays `unconfirmed`; it is not an error on the page.

## How it was checked

Each row says one of:

- **read**: the code or artifact says so, at the cited place.
- **run**: the command or snippet was run and produced what the page says.
- **inferred**: reasoned from something else. Never enough on its own.

A command the page tells the reader to run is either **run**, or listed as not run in the hand-off. How to run a whole page without leaving anything behind is in [review.md](review.md#running-the-page).

## Ledger format

`Build/docs-work/ledger/<page>.md`, one table per page (or `ledger/<page>/<section>.md` when the page is verified by section):

```markdown
| # | Change | Claim on the page | Evidence | Checked |
|---|---|---|---|---|
| 1 | ports | The server uses 2000, 2001 and 2002 | `Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Settings/CarlaSettings.cpp:135-137` | read |
| 2 | wheel path | The wheel is in `PythonAPI/carla/dist/` | `Unreal/Package/CopyCarlaAdditionalFiles.cmake:14`; `ls` of a local package | read, run |
| 3 | wheel name | The 1.0.0 wheel is cp310 | CI uses Python 3.10 (`.github/workflows/ue5_dev.yml:23`) | inferred |
```

`Change` is a short label for the part of the target the claim belongs to, or the finding number from the review report. A row the writer adds because it needs a fact nobody confirmed has `pending` in `Checked`; it goes back to the verifier before the fact reaches the page.

## When it cannot be confirmed

- Something that will exist at release but does not yet (the 1.0.0 download link, the 1.0.0 Docker tag): write it for the target and put a visible marker next to it, `__(TODO: add the link to the 1.0.0 release once it is published)__`. Never an HTML comment: it has to show on the page so it gets tracked.
- Something that should be checkable but was not (a figure, a Windows path on a Linux machine): leave the existing text alone, or leave it out, and list it in the hand-off as unverified with what would settle it.
- Never state an inferred claim as fact.

## Verdicts

The verifier writes `Build/docs-work/verdicts/<page>.md`, one line per ledger row:

- `confirmed`;
- `wrong`, with what the evidence actually says;
- `unconfirmed`, with what is missing;
- `needs-run`, with the exact command or snippet that would settle it. The run agent replaces it with `confirmed` or `wrong` and the output.

Only `confirmed` rows reach the page as fact.
