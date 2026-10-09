---
name: install-scenic
description: Installs Scenic, the probabilistic scenario description language, for the CARLA in use — pip installing it into the same interpreter that holds the CARLA client, optionally cloning the upstream repo for the example scenarios the wheel does not ship, and verifying that the `scenic` CLI, the client and Scenic's version-keyed blueprint table all agree. Use when the user asks to "install Scenic", "set up Scenic", "download Scenic", "pip install scenic", wants to run `.scenic` files, or has a Scenic whose model import or blueprint lookup fails.
license: MIT
compatibility: Linux/Windows, Python 3.8+. The real constraint is the CARLA client wheel, which is built per Python version — Scenic must go into that same interpreter. Nothing is built and no UE4/UE5 install is needed.
metadata:
  group: setup
  prerequisites: scripts/check_env.sh
  reference: references/compatibility.md
---

# Install Scenic

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Scenic is pure Python — **there is nothing to build** and `pip install scenic` is
the whole install. Two things make it go wrong anyway, and neither reports an
install error:

1. **The `scenic` CLI and the `carla` client must be in ONE interpreter.** Scenic
   loads the client itself when a scenario names the CARLA model. Split them and
   the failure arrives mid-run as a model import error, not at install time.
2. **Scenic's blueprint tables are keyed on the CARLA *client* version.** With no
   table for that version, every vehicle and prop category resolves empty and
   *every* scenario fails at sample time with "no blueprints recorded".

So the install order is **client first, Scenic second**, into the same
interpreter. `install_scenic.py` refuses to install into an interpreter with no
client unless you pass `--force`.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Pick the interpreter — the one that has the CARLA client
- [ ] Step 3: See the plan for this machine
- [ ] Step 4: Install (and clone examples if they are wanted)
- [ ] Step 5: Verify, and record `SCENIC_ROOT` and `PYTHON` with `set_config`
```

### Step 1-3: What is here, and what to do about it

```bash
PYTHON=/path/to/venv/bin/python bash scripts/check_env.sh

python3 scripts/install_scenic.py detect     # interpreter, Scenic, client, tables
python3 scripts/install_scenic.py plan       # the exact commands, and why
```

Every subcommand takes `--python` to act on another interpreter:

```bash
python3 scripts/install_scenic.py --python ~/.venvs/carla/bin/python detect
```

`detect` is the one to read first. It prints which CARLA versions the installed
Scenic has blueprint tables for, and flags a CLI that belongs to a *different*
environment than the interpreter — the pyenv-shim trap, which otherwise surfaces
much later as a confusing model import failure.

### Step 4: Install

```bash
# into the interpreter that has the client
python3 scripts/install_scenic.py install

# a specific version
python3 scripts/install_scenic.py install --version 3.1.1

# also clone upstream for the example scenarios and map assets
python3 scripts/install_scenic.py install --clone ~/Scenic
```

**The wheel ships world models only — no example scenarios and no map assets.**
`--clone` is how you get `examples/` and `assets/maps/CARLA/*.xodr`. Skip it if
you are running a ScenarioRunner checkout's own scenarios, which carry their own
copies under `srunner/scenic/`.

`install` runs `verify` when it finishes.

### Step 5: Verify and export

```bash
python3 scripts/install_scenic.py verify
```

Checks the five things that actually matter, and exits non-zero on any:

| Check | Why it matters |
|---|---|
| `scenic` importable | the install worked |
| CLI belongs to this interpreter | otherwise a run uses a different Scenic |
| `carla` client present | Scenic loads it for the CARLA model |
| `carla` is the extension, not a directory | a directory named `carla` on `sys.path` imports as an empty namespace package |
| blueprint table for this client version | otherwise every category is empty |
| `scenic.simulators.carla.simulator` imports | the real binding between Scenic and the client |

Then export the root, which is what the `scenic` skill group is gated on:

```bash
export SCENIC_ROOT=~/Scenic           # a checkout, for examples
# or, for a pip-only install, the installed package directory:
export SCENIC_ROOT=$(python3 -c 'import os,scenic;print(os.path.dirname(scenic.__file__))')
```

Running and authoring scenarios is out of scope here — see
[[run-scenic-scenario]] and [[create-scenic-scenario]].

### Recording the path

An `export` lasts until the shell exits. Persist `SCENIC_ROOT` and `PYTHON` instead, so the
next session — and `list_skills` — still knows where this went:

```
set_config({"SCENIC_ROOT": "<the checkout or package dir>",
            "PYTHON": "<the interpreter Scenic went into>"})
```

Without it the group this just enabled keeps reporting `available: false`,
and the next skill re-detects from scratch. `CARLA_ROOT` is the only CARLA
path to record: `set_config` derives the engine-specific variable itself.

## Examples

**Example 1: "install Scenic so I can run .scenic files"**

`install_scenic.py detect` to find the interpreter holding the client, then
`install --python <that> --clone ~/Scenic`, then `verify`. Export `SCENIC_ROOT`
and the scenic skills become available.

**Example 2: "Scenic says there are no bicycle blueprints"**

Not an install fault by itself. `detect` prints the table versions; if the client
version is listed, the *category* is empty in Scenic's data for that version even
though the build may contain such assets. Upgrading Scenic may add it; otherwise
name the id explicitly. See [[create-scenic-scenario]].

**Example 3: "scenic is installed but a scenario cannot import the model"**

Almost always the two halves in different environments. `verify` compares the
CLI's directory against the interpreter's and fails loudly on a mismatch.

## Troubleshooting

**Problem: `refusing to install: <python> has no carla client`**
Cause: the guard. Installing Scenic into a client-less interpreter looks fine and
fails later.
Solution: install the client there first ([[install-python-api]]), or `--force` if
you genuinely want Scenic alone.

**Problem: the `scenic` command runs a different Scenic than expected**
Cause: a version manager's shim directory precedes the environment on `PATH`.
Solution: call the CLI by full path — `<env>/bin/scenic` — which is what the
scenic skills' `env.sh` derives from `PYTHON`.

**Problem: `no blueprint table for client X`**
Cause: the installed Scenic predates that CARLA release.
Solution: `install` without `--version` to take the newest; `plan` prints the
available versions. If none has a table for that client, Scenic does not support
it yet.

**Problem: no example scenarios after installing**
Cause: expected — the wheel excludes `examples/`, and the sdist does too.
Solution: `install --clone <dir>`, or use a ScenarioRunner checkout's
`srunner/scenic/`.

**Problem: `pip install scenic` pulls a numpy that breaks the client**
Cause: Scenic's dependency set is wide (numpy, scipy, shapely, trimesh,
matplotlib, opencv).
Solution: install into a dedicated virtualenv alongside the client rather than a
shared one, so a resolver change cannot disturb other work.

## Outputs

Scenic installed into a named interpreter, `verify` reporting PASS on all checks,
and `SCENIC_ROOT` exported so the `scenic` skill group reports available.
Optionally an upstream checkout providing `examples/` and `assets/maps/`.

Version-keying, the examples question and the interpreter rule are covered in
[references/compatibility.md](references/compatibility.md).
