---
name: set-world-settings
description: Reads and changes a running CARLA server's WorldSettings via apply_settings — synchronous vs asynchronous mode, fixed_delta_seconds (fps), physics substepping, and no_rendering_mode — and keeps the Traffic Manager's sync state matched automatically. Use when the user asks to "enable/disable sync mode", "set synchronous mode", "run at a fixed fps/step", "turn off rendering", "make the sim deterministic", or "restore async mode", and as the prerequisite for keeping settings across a map load.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Does NOT need UE4_ROOT or a source checkout. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/world-settings.md
---

# Set CARLA world settings

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Change how a **running** server steps time and renders, from a client, and read
the result back. The deliverable is verified server state: after any change the
`WorldSettings` are re-read (`apply_settings` returns a frame id, not the state).

The core switch is **synchronous vs asynchronous**:

- **async** (default): the server free-runs on its own clock; the client observes.
- **sync**: the server advances **only when the client calls `world.tick()`**, at
  a fixed `fixed_delta_seconds` step — required for frame-aligned sensors and
  reproducible runs.

This skill couples the **Traffic Manager** to that switch automatically: `sync`
also sets the TM synchronous, `async` also sets it asynchronous. A mismatched
pair is a classic source of jittery or frozen traffic. Full semantics:
[references/world-settings.md](references/world-settings.md).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Show current settings
- [ ] Step 3: Apply sync / async / a field change (TM follows sync|async)
- [ ] Step 4: Verify the VERIFY block matches the intent
- [ ] (on exit) Restore async so the server self-ticks — never leave it stuck in sync
```

Commands need `CARLA_HOST`/`CARLA_PORT`/`TM_PORT` from `scripts/env.sh` (defaults
`127.0.0.1:2000`, TM `8000`). Prefix with `source scripts/env.sh` or export them.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

FAILs only on a missing `carla` module or an unreachable server. It also warns if
the server is already in sync mode (it will look "frozen" without a ticker).

### Step 2: Show current settings

```bash
source scripts/env.sh
python3 scripts/world_settings.py show
```

### Step 3: Apply

```bash
# synchronous mode at 20 fps (0.05 s step); TM set synchronous too
python3 scripts/world_settings.py sync --fps 20
# give the step directly instead of fps
python3 scripts/world_settings.py sync --delta 0.05
# sync + headless physics (no rendering)
python3 scripts/world_settings.py sync --fps 20 --no-rendering

# restore asynchronous mode; TM set asynchronous too
python3 scripts/world_settings.py async

# change individual fields without touching sync/TM (any WorldSettings field)
python3 scripts/world_settings.py set --no-rendering off
python3 scripts/world_settings.py set --fixed-delta none         # back to variable step
python3 scripts/world_settings.py set --deterministic-ragdolls on --max-culling-distance 500
python3 scripts/world_settings.py set --tile-stream-distance 2000 --actor-active-distance 1500
python3 scripts/world_settings.py set --substepping on --substeps 16 --substep-dt 0.0125
```

Low freedom on two fragile points: `fixed_delta_seconds` should stay `<= 0.1` for
stable physics (the script warns and auto-raises `max_substeps` to keep the
substep budget), and after enabling sync you **must** `world.tick()` to advance.

### Step 4: Verify

Each command prints a `VERIFY` block: `synchronous_mode`, `fixed_delta_seconds`
(+ derived fps), `no_rendering_mode`, substepping fields, and the TM sync state it
set. Confirm they match the request — e.g. `sync --fps 20` ⇒ `synchronous_mode =
True`, `fixed_delta_seconds = 0.05`, `traffic_manager sync = True`.

### On exit: restore async

Leaving the server in sync mode with nothing ticking makes it appear hung to the
next client. When a sync-mode task ends, run `async` to hand the clock back to the
server. This is the default job's clean-exit step, not an opt-in extra.

## Examples

**Example 1: set up a deterministic capture loop**

User says: "put the sim in sync mode at 20 fps"

`python3 scripts/world_settings.py sync --fps 20`. VERIFY: `synchronous_mode=True`,
`fixed_delta_seconds=0.05`, `traffic_manager sync=True`. Then tick from your client.

**Example 2: tear down cleanly**

User says: "I'm done, put it back to normal"

`python3 scripts/world_settings.py async`. VERIFY: `synchronous_mode=False`,
`fixed_delta_seconds=None`, `traffic_manager sync=False`. Server self-ticks again.

**Example 3: keep settings across a map load**

User says: "switch to Town02 without dropping my sync settings"

`sync --fps 20` here, then the [`load-map`](../load-map/SKILL.md) skill with
`--keep` (`reset_settings=False`) so the settings survive the load; tick after.

## Troubleshooting

**Problem: traffic is jittery / vehicles teleport in sync mode**
Cause: world is sync but the Traffic Manager is async (or vice-versa).
Solution: use `sync`/`async` here — they set both. Manually, `tm.set_synchronous_mode(True)`.

**Problem: the server looks frozen / clients time out after enabling sync**
Cause: nothing is ticking it. In sync mode the world advances only on `world.tick()`.
Solution: tick from your client, or run `async` to restore self-ticking.

**Problem: physics unstable / warning about the substep budget**
Cause: `fixed_delta_seconds` exceeds `max_substep_delta_time * max_substeps`.
Solution: keep the step `<= 0.1`; the script raises `max_substeps` to fit and warns.

**Problem: sim not fully reproducible even in sync mode**
Cause: the Traffic Manager's random seed is unset.
Solution: set a TM seed (owned by the traffic-manager skill) in addition to sync.

## Outputs

Server state, not a file: the running world now steps synchronously or
asynchronously as asked, with the TM matched. The `VERIFY` block is the record.

Field-by-field reference (every `WorldSettings` attribute, the substep math, the
sync/TM/determinism rules) is in [references/world-settings.md](references/world-settings.md).
