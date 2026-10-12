---
name: install-python-api
description: Installs the CARLA Python client (the `carla` package) into a chosen interpreter so the other python-api skills work — from a wheel bundled with your release or checkout, from PyPI (`pip install carla==X.Y.Z`), or by putting an older release's .egg on PYTHONPATH. Detects what is already present, picks the source that matches your simulator's version, pins numpy<2, and verifies `import carla` plus the client/server version match. Use when the user asks to "install the CARLA Python API", "pip install carla", "set up the carla python package", or when a skill reports "cannot import carla".
license: MIT
compatibility: Linux, macOS or Windows with a Python 3.10+ interpreter and pip. Needs either a local CARLA release/checkout containing PythonAPI/carla/dist, or network access to PyPI. Installs only the client — it never touches the simulator, and needs no running server (a reachable one is used to check the version match).
metadata:
  group: setup
  prerequisites: scripts/check_env.sh
  reference: references/sources.md
---

# Install the CARLA Python client

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

The bootstrap step everything else in `python-api` depends on: without an
importable `carla`, every one of those skills stops at
`FAIL cannot import carla`. This skill fixes that, into **the interpreter you
choose**, and tells you when the client it installed does not match the
simulator you are running.

It is client-side only. Nothing here builds, launches or modifies a simulator.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: detect  — what is installed, which sources exist, what matches
- [ ] Step 3: install — auto-picks the best source, then verifies
- [ ] Step 4: verify against a running server when you have one
- [ ] Step 5: record `PYTHON` with `set_config` — the other skills read it
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Steps 2-4: detect, install, verify

```bash
source scripts/env.sh

python3 scripts/install_python_api.py detect
python3 scripts/install_python_api.py install                       # auto
python3 scripts/install_python_api.py install --dry-run             # show the pip line only
python3 scripts/install_python_api.py install --source pypi --version 0.9.16
python3 scripts/install_python_api.py verify
```

`install` is a no-op when `carla` already imports (`--force` to reinstall), and
every path ends by importing `carla` in the target interpreter — a pip exit code
is not treated as proof.

## Which interpreter (the part people get wrong)

`PYTHON` selects the target; it defaults to `python3` on PATH, and it is the same
variable the rest of the `python-api` group uses, so one setting covers them all.

```bash
PYTHON=/path/to/venv/bin/python python3 scripts/install_python_api.py install
```

**This matters when an agent runs the skill.** If the MCP server was started with
`uvx`/`pipx`/`npx`, that launcher puts *its own* isolated environment first on
PATH, so bare `python3` is the tool's interpreter — commonly a different minor
version, and never the one that talks to CARLA. Installing the client there
achieves nothing. `check_env.sh` recognises those paths and says so. Set `PYTHON`
in the MCP client's `env` block once:

```json
"env": { "PYTHON": "/home/me/carla-venv/bin/python" }
```

## Sources, and why the order is not arbitrary

`auto` walks them in this order:

| # | Source | When it applies | Why this rank |
|---|---|---|---|
| 1 | **local wheel** — `<root>/PythonAPI/carla/dist/carla-*-<cp tag>-*.whl` | `CARLA_PACKAGE_ROOT` or `CARLA_UE4_ROOT` is set and ships one | it is the only artifact **guaranteed to match your simulator** |
| 2 | **PyPI** — `pip install carla==X.Y.Z` | any interpreter PyPI has a wheel for | clean and offline-free, but you must name the right version |
| 3 | **egg** — `carla-*-py<X.Y>-*.egg` | older releases that ship an egg | nothing to install; it goes on `PYTHONPATH` (written as a `.pth` inside a venv) |

Not covered here: **building from source** — that is [[build-carla-ue4]] step 04
(`make PythonAPI`), the right answer when no wheel exists for your interpreter.
Conda and Docker distributions are also out of scope.

PyPI coverage is real but partial — `detect` prints the live matrix, e.g.
`0.9.16 -> cp310, cp311, cp312` (linux + windows, no macOS wheels). Ask for an
interpreter it has no wheel for and the skill refuses **before** pip does, naming
the tags that exist.

## Version matching

CARLA's client and simulator are expected to be the same version; a mismatch
produces `WARNING: Client API version = X / Simulator API version = Y` on every
connection and subtly missing API. So:

- source 1 (local wheel) inherits the version from the release you actually have;
- `detect`/`verify` query a reachable server and report `match` or `MISMATCH`;
- a mismatch is a **WARN, not a failure** — it is often intentional (talking to a
  remote server), so the skill reports it and continues.

`numpy<2` is pinned in the same pip transaction: CARLA's bindings are built
against the numpy 1.x C API and crash on import under 2.x.

## Examples

**Example 1: only a downloaded release, no client installed**

User says: "I downloaded CARLA and pip install carla-agentic-tools, now what?"

```bash
export CARLA_PACKAGE_ROOT=~/CARLA_0.9.16
bash scripts/check_env.sh
python3 scripts/install_python_api.py install     # finds the bundled cp310 wheel
```

**Example 2: no local files at all, remote server**

User says: "the simulator runs on another machine, I just need the client"

`install --source pypi --version 0.9.16`, then `verify` with
`CARLA_HOST=<that machine>` to confirm the versions agree.

**Example 3: a skill just told the agent `cannot import carla`**

Run `detect`, then `install`. If `detect` shows the target interpreter inside a
`uv`/`pipx` cache path, set `PYTHON` to the real environment first — otherwise the
install lands where nothing will use it.

### Recording the path

An `export` lasts until the shell exits. Persist `PYTHON` instead, so the
next session — and `list_skills` — still knows where this went:

```
set_config({"PYTHON": "<the interpreter used above>"})
```

Without it the group this just enabled keeps reporting `available: false`,
and the next skill re-detects from scratch. `CARLA_ROOT` is the only CARLA
path to record: `set_config` derives the engine-specific variable itself.

## Verify

```bash
python3 scripts/install_python_api.py verify
```

`PASS: carla <version> importable by <interpreter>`, plus a server comparison when
one is reachable. Independent check:

```bash
"${PYTHON:-python3}" -c "import carla; print(carla.Client('127.0.0.1',2000).get_server_version())"
```

## Troubleshooting

**Problem: `PyPI has no cp313 wheel for carla==0.9.16`**
Cause: no wheel exists for that interpreter.
Solution: use a 3.10-3.12 interpreter (`PYTHON=…`), install a bundled wheel, or
build it ([[build-carla-ue4]] step 04).

**Problem: installed fine, but a skill still says `cannot import carla`**
Cause: the skill ran under a different interpreter than the one installed into —
almost always an isolated MCP server env.
Solution: `detect` prints the target; set `PYTHON` consistently in the client's
`env` block.

**Problem: `error: externally-managed-environment` (PEP 668)**
Cause: a system interpreter (Ubuntu 24.04+, Debian) refuses direct installs.
Solution: install into a venv and point `PYTHON` at it. `check_env.sh` flags this
before you hit it.

**Problem: `import carla` works but every connection warns about API versions**
Cause: client and simulator versions differ.
Solution: prefer the bundled wheel from the release you run, or
`install --source pypi --version <server version>` (`verify` prints the server's).

**Problem: an egg was found but `import carla` still fails**
Cause: the egg's Python version must match the interpreter exactly, and it needs
to be on `PYTHONPATH`.
Solution: use the printed `export PYTHONPATH=…` line, or install into a venv where
the skill can write the `.pth` for you.

## Outputs

A `carla` package importable by the target interpreter (or, for an egg, a
`PYTHONPATH`/`.pth` entry), plus `numpy<2`. Nothing else on the system changes.

Source details, the PyPI tag matrix, egg mechanics and version-matching rules:
[`references/sources.md`](references/sources.md).
