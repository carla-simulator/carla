# Evidence

Every fact a writer adds or changes on a page gets one row in the page's ledger. The verifier checks the rows, and the user spot-checks them, so a row must let someone confirm the claim in one step.

## What counts as evidence

| Counts | Does not count |
|---|---|
| The code, at `file:line` (bindings, CMake, scripts, config, C++) | Another doc page, including the one being edited |
| Output of a command you ran (`ls` of a package, `--help`, a snippet run against a server) | A commit title or PR description |
| The published artifact itself (release notes, a package listing, an image config read from the registry) | The changelog, a review plan, an audit report, memory |
| | "CI builds with 3.10, so the wheel is cp310" — an inference |

Leads are fine for finding where to look. A claim is only confirmed by the left column.

## How it was checked

Each row says one of:

- **read**: the code or artifact says so, at the cited place.
- **run**: the command or snippet was run and produced what the page says.
- **inferred**: reasoned from something else. Never enough on its own.

A command the page tells the reader to run is either **run**, or listed as not run in the hand-off.

## Ledger format

`Build/docs-release/ledger/<page>.md`, one table per page:

```markdown
| # | Change | Claim on the page | Evidence | Checked |
|---|---|---|---|---|
| 1 | ports | The server uses 2000, 2001 and 2002 | `Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Settings/CarlaSettings.cpp:135-137` | read |
| 2 | wheel path | The wheel is in `PythonAPI/carla/dist/` | `Unreal/Package/CopyCarlaAdditionalFiles.cmake:14`; `ls` of a local package | read, run |
| 3 | wheel name | The 1.0.0 wheel is cp310 | CI uses Python 3.10 (`.github/workflows/ue5_dev.yml:23`) | inferred |
```

`Change` is the id from `changes.json`, so the coverage report can be built from the ledgers.

## When it cannot be confirmed

- Something that will exist at release but does not yet (the 1.0.0 download link, the 1.0.0 Docker tag): write it for the target and put a visible marker next to it, `__(TODO: add the link to the 1.0.0 release once it is published)__`. Never an HTML comment: it has to show on the page so it gets tracked.
- Something that should be checkable but was not (a figure, a Windows path on a Linux machine): leave the existing text alone, or leave it out, and list it in the hand-off as unverified with what would settle it.
- Never state an inferred claim as fact.

## Verdicts

The verifier writes `Build/docs-release/verdicts/<page>.md`, one line per ledger row: `confirmed`, `wrong` (with what the evidence actually says) or `unconfirmed` (with what is missing). It also lists claims in the diff that have no ledger row; those count as `unconfirmed`.
