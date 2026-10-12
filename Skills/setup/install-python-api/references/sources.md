# Getting the `carla` client: sources, tags, and version rules

Detail layer for `install-python-api`. Facts checked against PyPI and a real
0.9.16 checkout in 2026-08; the tag matrix is queried live by `detect`, so treat
the table here as illustration, not as the source of truth.

## The four ways a `carla` client can exist

| # | Mechanism | Artifact | Notes |
|---|---|---|---|
| 1 | wheel bundled with a release/checkout | `PythonAPI/carla/dist/carla-<ver>-<cp tag>-<cp tag>-<platform>.whl` | matches the simulator you have; produced by `make PythonAPI` |
| 2 | PyPI | `pip install carla==<ver>` | partial interpreter/platform coverage, see below |
| 3 | egg (legacy) | `PythonAPI/carla/dist/carla-<ver>-py<X.Y>-linux-x86_64.egg` | nothing installs; the *directory containing it* goes on `PYTHONPATH` |
| 4 | build from source | — | `make PythonAPI` in a checkout → produces (1). See [[build-carla-ue4]] step 04 |

Distribution channels this skill does **not** handle: conda/RoboStack builds and
Docker images that pre-bake the client.

## PyPI coverage

Observed on pypi.org/project/carla:

| version | python tags | platforms |
|---|---|---|
| 0.9.16 | cp310, cp311, cp312 | `manylinux_2_31_x86_64`, `win_amd64` |
| 0.9.15 | cp37, cp38, cp39, cp310 (+cp27) | `manylinux_2_27_x86_64`, `win_amd64` |
| 0.9.14 | cp37, cp38 (+cp27) | same |

Consequences worth knowing before promising a user anything:

- **No macOS wheels at all**, any version. macOS users need a source build.
- **No cp313** yet — a 3.13 interpreter cannot install any published `carla`.
- `manylinux_2_31` needs glibc ≥ 2.31 (Ubuntu 20.04+); older distros must use a
  bundled wheel or build.
- The `carla` project on PyPI is versioned independently of a git checkout, so a
  development build (`0.10.0`, a branch, a hash) will have **no** PyPI counterpart
  — source 1 or 4 is the only option there.

## Wheel tags, and why they cannot be worked around

A CARLA wheel contains a compiled extension (`libcarla`), so its `cp3XX` tag is a
hard ABI constraint: `carla-0.9.16-cp310-...whl` loads **only** in CPython 3.10.
Two consequences that catch people:

- `PYTHONPATH` cannot bridge interpreter versions. Pointing 3.12 at a 3.10
  `site-packages` yields `ModuleNotFoundError: No module named 'carla'` — verified.
  The same applies to eggs.
- Copying a wheel between machines is fine; copying between Python minors is not.

`detect` prints `wheel tag cp310` for the target interpreter, and every source is
filtered by that tag before it is offered.

## Eggs

Releases up to roughly 0.9.12 shipped `.egg` instead of a wheel. There is nothing
to install:

```bash
export PYTHONPATH="/path/to/PythonAPI/carla/dist/carla-0.9.12-py3.7-linux-x86_64.egg:$PYTHONPATH"
```

The skill prints that line, and when the target interpreter is a venv it also
writes `carla-egg.pth` into that venv's `site-packages`, which makes the egg
permanent for that environment without touching the user's shell profile. Eggs are
deprecated (setuptools removed `easy_install`) but still work through
`PYTHONPATH`, since the egg is simply a directory/zip on `sys.path`.

## Version matching

The client and the simulator exchange a version string on connect. When they
differ, CARLA prints on **every** client construction:

```
WARNING: Client API version     = 0.9.16
WARNING: Simulator API version  = <other>
```

It usually still works — this is why the skill treats a mismatch as `WARN` — but
API added on one side is missing on the other, and serialisation of newer sensor
types can break outright. Rules of thumb:

- Running a *release* → install its bundled wheel (source 1). Guaranteed match.
- Running a *dev build* → use the wheel that build produced; PyPI will not have it.
- Talking to someone else's server → `verify` reports the server's version; install
  that exact version from PyPI if a matching tag exists.

`verify` queries `client.get_server_version()`; with no server reachable it says so
and exits 0, because a missing server does not make the install wrong.

## numpy

CARLA's bindings are compiled against the numpy **1.x** C API. Under numpy 2.x,
`import carla` raises at import time. The installer therefore pins `numpy<2` in the
same pip transaction (`numpy 1.26.4` is the last 1.x, and it has cp310-cp312
wheels). If a project genuinely needs numpy 2, it needs a separate environment
from the CARLA client.

## Where the client lands

Same rules as any pip install, and worth reporting to a user who asks:

| target | location |
|---|---|
| venv (`PYTHON=<venv>/bin/python`) | `<venv>/lib/python3.X/site-packages/carla/` |
| user install (no venv) | `~/.local/lib/python3.X/site-packages/carla/` — the skill adds `--user` automatically outside a venv |
| egg | not copied; referenced in place via `PYTHONPATH`/`.pth` |
