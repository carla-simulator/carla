# CARLA contribution rules (ue58-dev), condensed

Sources, which win on any disagreement:
`Docs/cont_contribution_guidelines.md`, `Docs/cont_coding_standard.md`,
`Docs/dev_testing.md`, `.github/pull_request_template.md`,
`.github/workflows/ue5_pr.yml`, `.github/ISSUE_TEMPLATE/*.md`, `.github/issue_template.md`. Re-read them if this file looks stale.

## Branches

| What | Where |
|---|---|
| Code, config, docs, skills | GitHub `carla-simulator/carla`, base **`ue58-dev`** |
| Feature branch | `username/name_of_the_contribution`, made from `ue58-dev` |
| Art (maps, vehicles, walkers, props) | Bitbucket content repo, branch `ue58-dev-carla`, own branch `contributors/<name>`, reviewed by the art team |
| UE 4.26 code | `ue4-dev` (not covered by this skill) |

Keep the branch up to date with `ue58-dev`. Discuss large or overlapping work
first on GitHub Discussions or the Discord server, and check the issue board.

## Issues and discussions

| Need | Venue |
|---|---|
| Bug | Issue, `.github/ISSUE_TEMPLATE/bug_report.md`: Setup (CARLA version, platform, Python, GPU, drivers), bug, steps to reproduce, expected behaviour, logs (`Unreal/CarlaUnreal/Saved/Logs`), scripts, screenshots, context |
| Feature request | Issue, `.github/ISSUE_TEMPLATE/feature_request.md`: problem, desired solution, alternatives, context |
| Question / support | GitHub Discussions (the issue template sends questions there) |
| CI workflow change | **Discussion or Discord, agreed before the PR**: what is happening, why a workflow is needed, what it runs, on which triggers, at what cost in runners and time. Linked in the PR |
| Large or overlapping work, unsure how to proceed | Discussion or Discord first; check the issue board |

**Never open an issue or a discussion without first searching the open and closed
issues, pull requests and discussions for a duplicate.** If one exists, add to it
(new information or a 👍) instead. A related thread gets linked from the new one.

Before reporting a bug: check it has not been reported, read the docs and the
FAQ (`Docs/build_faq.md`), and give the branch and commit, UE version, platform
and Python version. Keep threads focused and single-themed.

## Pre-PR checklist (from the guidelines)

- [ ] Branch up to date with `ue58-dev` and tested with the latest changes.
- [ ] Compiles: `cmake --build Build --target carla-client carla-python-api carla-server`.
- [ ] LibCarla unit tests pass (`libcarla_test_server`, `libcarla_test_client`).
- [ ] *(Strongly recommended, not required)* The package builds locally. CI
      builds it too, and cook or shipping-only failures otherwise cost a CI round trip.
- [ ] Smoke tests pass against a server on port 3654:
      `cd PythonAPI/test && python3 -m nose2 -v $(cat smoke_test_list.txt)`.
- [ ] New or modified tests add real value (see Tests).
- [ ] Any new or changed CI workflow was agreed beforehand, and the discussion is linked.
- [ ] `CHANGELOG.md` entry under `## LATEST Changes` for any user-visible change.
- [ ] Documentation extended if necessary.

## Coding standard

General: spaces, not tabs; no trailing whitespace.

Python:
- Code ≤ 120 columns (`max-line-length=120` in `PythonAPI/.pylintrc`), comments ≤ 80.
- Python 3 only; the wheel installs only on the Python CARLA was configured with.
- `pylint --rcfile=PythonAPI/.pylintrc <files>` gives no errors or warnings.
  The exceptions for external modules such as `numpy` and `pygame` are in the rcfile.
- PEP 8 (`autopep8` where possible).

C++:
- C++20. Clang from the UE toolchain on Linux, MSVC (VS 2022) on Windows.
- No errors and no warnings (`ENABLE_ALL_WARNINGS`: `-Wall -Wextra` / `/W4`).
- No `throw`; use `carla::throw_exception`.
- `try`/`catch` inside `#ifndef LIBCARLA_NO_EXCEPTIONS`, so the code builds with
  `ENABLE_EXCEPTIONS` off.
- Unreal code (CarlaUnreal, Carla plugin) follows Epic's coding standard, but with
  spaces. LibCarla uses a variation of Google's style guide.
- Comments ≤ 80 columns; code may exceed that rarely, when it is clearer.

## Tests

Every test runs in CI on every PR, so each must earn its compute.
- Rejected: empty tests, tests with no assertions, tests that only check that
  something imports or spawns, tests that duplicate existing coverage.
- Do not weaken an existing test to make it pass (removing asserts, widening
  tolerances, skipping). If the expected behaviour changed, explain it in the PR.
- The PR says what each new or modified test checks and which regression it catches.

| Suite | Location | Server |
|---|---|---|
| LibCarla unit | `LibCarla/source/test/` | none |
| Python unit | `PythonAPI/test/unit/` | none |
| Smoke | `PythonAPI/test/smoke/`, list in `smoke_test_list.txt` | port 3654 |
| Integration | `PythonAPI/test/API/`, list in `integration_tests_list.txt` | port 2000 |

## CI workflows

No PR that adds or changes `.github/workflows/` without a prior agreement with
the core maintainers, on GitHub Discussions or Discord. The proposal covers what
problem the workflow addresses, why a workflow is needed, and what it runs, on
which triggers, at what cost in runners and time. The discussion is linked in the
PR. Unagreed workflow changes are not merged.

## What CI runs on a PR to ue58-dev

`.github/workflows/ue5_pr.yml` → `_ci-ubuntu.yml` on Ubuntu, Python 3.10:
`cmake --preset Release -DENABLE_ROS2=ON -DBUILD_LIBCARLA_TESTS=ON`. It builds
the client, the Python API and the server, then builds and runs the LibCarla
unit tests, then builds the package.
**No Python unit, smoke or integration tests.**

Skipped entirely (`paths-ignore`) when every changed file is under `Docs/**`,
`**/*.md`, `mkdocs.yml`, `docs_hooks.py` or `PythonAPI/docs/**`.

## Generated files: never edit by hand

| File | Source | Generator |
|---|---|---|
| `Docs/python_api.md` | `PythonAPI/docs/*.yml` | `PythonAPI/docs/doc_gen.py` |
| `Docs/bp_library.md` | blueprints of a running server | `PythonAPI/docs/bp_doc_gen.py` |
| `Docs/catalogue_*.md` | a running simulator | `PythonAPI/docs/catalogue_doc_gen.py` |

## Docs

Markdown under `Docs/`, built with MkDocs (`pip3 install -r Docs/requirements.txt`).
Preview with `python3 -m mkdocs serve`. Check with `python3 -m mkdocs build --strict`.
It fails on links to missing pages, but missing anchors are only `INFO` lines, so
search the output for `does not contain an anchor`. Style:
`Docs/cont_doc_standard.md`.

## PR template sections (`.github/pull_request_template.md`)

Description · `Fixes #` · Where has this been tested? (Platform(s), Python
version(s), Unreal Engine version(s)) · Tests · CI workflows · Possible Drawbacks.
