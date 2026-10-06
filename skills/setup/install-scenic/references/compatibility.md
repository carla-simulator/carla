# Scenic and CARLA — what has to line up

Detail layer for `install-scenic`.

## One interpreter, not two

Scenic talks to CARLA through the ordinary Python client. When a scenario says

```
model scenic.simulators.carla.model
```

Scenic imports `scenic.simulators.carla.simulator`, which does `import carla` at
module load. So the client must be importable **from the interpreter backing the
`scenic` CLI** — not merely installed somewhere on the machine.

This is easy to get wrong because a version manager puts a shim directory ahead of
every environment on `PATH`, so `which scenic` can resolve to a launcher that
picks a different environment than the `python` you were testing with. The symptom
is a `ModuleNotFoundError` for `carla` or for a world model, from a machine where
both are demonstrably installed.

Two habits avoid it entirely:

- install both into one dedicated virtualenv,
- invoke the CLI by full path, `<env>/bin/scenic`.

`verify` compares the CLI's parent directory against the interpreter's and fails
on a mismatch rather than letting it surface later.

## Blueprint tables are keyed on the client version

`scenic/simulators/carla/_blueprintData.py` holds `_IDS[carla_version][category]`
— hardcoded lists of blueprint ids per CARLA release, with a matching `_DIMS` for
object dimensions. Scenic reads the **client** distribution version to choose the
table. It never asks the server what exists.

Consequences worth internalising:

- **No table for that client version** → every category resolves empty → *every*
  scenario fails at sample time with "no blueprints recorded". Looks like a broken
  install; it is a version gap.
- **Client and server must match exactly.** A mismatched pair means Scenic hands
  out ids from the wrong release, and the failure lands at spawn time as an opaque
  `std::exception` from `blueprintLib.find()` that never names the id.
- **A category can be empty even when the table exists**, and the build can still
  contain such assets — the table is hand-maintained and lags content. Naming an
  id explicitly with `with blueprint "..."` bypasses the category entirely.
- **Upgrading Scenic is what adds a new CARLA release**, so prefer the newest
  version unless something pins you.

`install_scenic.py detect` prints the table versions; `plan` prints the available
Scenic versions when the current one has no table for your client.

## The wheel has no scenarios

The published wheel — and the sdist — ship the language, the domains and the
simulator interfaces, but **no `examples/` and no `assets/maps/`**. A fresh
`pip install scenic` therefore leaves you with nothing to run.

Three ways to get scenarios:

| Source | What you get |
|---|---|
| `install --clone <dir>` | upstream `examples/` + `assets/maps/CARLA/*.xodr` |
| a ScenarioRunner checkout | `srunner/scenic/*.scenic` and its own `assets/` |
| author them | [[create-scenic-scenario]] |

The two shipped copies of the carlaChallenge set are near-identical apart from the
`model` line, the map and the blueprint constants — upstream targets the CARLA
release it was written against, while a ScenarioRunner port is updated to the ids
and maps of the build it ships with.

## Dependency footprint

Scenic pulls a wide scientific stack — numpy, scipy, shapely, trimesh, matplotlib,
opencv, scikit-image, pygame — and will upgrade what it needs to. Put it in a
dedicated virtualenv with the client rather than a shared environment, so a
resolver decision cannot disturb unrelated work.

## SCENIC_ROOT

The skill library gates the `scenic` group on `SCENIC_ROOT`. A checkout is the
better value because it carries examples and map assets; for a pip-only install,
the installed package directory is a valid answer and keeps the group available:

```bash
export SCENIC_ROOT=$(python3 -c 'import os,scenic;print(os.path.dirname(scenic.__file__))')
```
