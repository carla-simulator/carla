---
name: invertedai-traffic
description: Populates a running CARLA server with Inverted AI traffic — NPC vehicles driven by Inverted AI's learned, real-data driving model over its API instead of the rule-based Traffic Manager. Sets up the API key in a git-ignored .env and the SDK in its own venv, verifies the key, runs a compact co-simulation loop (N cars around a point, traffic lights synced on Town10HD, optional CARLA recording), or the full shipped invertedai_traffic.py example (walkers, IAI log and birdview GIF). Use when the user asks for "Inverted AI", "IAI", "realistic/learned/human-like traffic", "NPCs driven by Inverted AI", or to compare TM traffic with Inverted AI.
license: MIT
compatibility: Linux, Python 3.8-3.12 for the invertedai venv (the pinned SDK forces numpy 1.26), an Inverted AI API key, network access to the Inverted AI API, and a reachable, already-running CARLA server. Traffic-light sync only on Town10HD. Tested on CARLA 1.0 (UE 5.8, ue58-dev, Town10HD_Opt) with invertedai 0.0.22 on an academic key.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/invertedai.md
---

# Inverted AI traffic

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Inverted AI drives NPC vehicles with a model learned from real traffic. CARLA
does no driving for them: every 0.1 s the client sends all agents' states to
the API, gets the next states back and teleports the actors. That makes the
traffic look human (hesitation, gap acceptance, varied speeds) at the cost of
one API round trip per step, so it runs slower than real time and uses the
key's quota. For rule-based local traffic use `spawn-vehicles` /
`control-traffic` instead.

Two entry points:

* **`iai_client.py`** (this skill): `ping`, `run`, `status`, `clean`. Cars only,
  around a chosen point, on the map that is already loaded. It never reloads
  the map, and it restores the world settings and destroys its NPCs on exit.
* **`run_example.sh`**: the full `PythonAPI/examples/invertedai_traffic.py`
  (CARLA walkers, waypoint goals, IAI JSON log and birdview GIF, video capture).
  The example **always reloads the map**. On exit it destroys what it spawned
  and puts the world back in asynchronous mode. The wrapper's
  `restore_world.py` checks this, because an older example or a crash leaves a
  frozen synchronous world: it restores async mode and reports what is left.
  `CLEAN=1` destroys every vehicle, walker and walker controller left, which
  is safe only right after the reload.

## Instructions

### Step 0: The key, once

The key goes in the **git-ignored `.env` at the root of the CARLA tree** that
holds `Skills/`, so it is not exported in every shell:

```bash
cd <CARLA_ROOT>
git check-ignore -v .env          # must print a .gitignore rule; if not, stop
printf 'IAI_API_KEY=%s\n' '<the key>' >> .env
chmod 600 .env
```

`env.sh` reads only `IAI_API_KEY` and `IAI_VENV` from that file with a literal
`KEY=value` parse. It does not `source` the file, so the file cannot run
commands. An exported `IAI_API_KEY` still wins over the file. The key is never
printed, only its last four characters. **Never** write the key into a tracked
file, a command line that gets logged, or a script. Do not `cat` the `.env`.
Another file can be used with `IAI_ENV_FILE=/path`.

### Step 1: Install (once) and check

```bash
scripts/install_iai.sh                      # venv at ~/.venvs/carla-iai, SDK at CARLA's pinned commit
BASE_PY=python3.12 scripts/install_iai.sh   # if python3 is 3.13+
scripts/check_env.sh                        # offline: venv, imports, key present, .env ignored + 600, server
scripts/check_env.sh --online               # plus one authenticated API call: proves the key works
```

If the venv lives elsewhere, add `IAI_VENV=/path/to/venv` to the same `.env`.
Run the client with the venv's interpreter, which is `$IAI_PY` after
`source scripts/env.sh`. Do not use plain `python3`.

### Step 2: Ping

```bash
source scripts/env.sh
"$IAI_PY" scripts/iai_client.py ping       # location_info for the loaded map; touches nothing
```

### Step 3: Run

```bash
# 30 Inverted AI cars around Town10HD's centre for 30 s of sim time
"$IAI_PY" scripts/iai_client.py run -n 30
# reproducible, recorded, longer
"$IAI_PY" scripts/iai_client.py run -n 40 --seed 7 --steps 600 --record /tmp/iai.log
# until Ctrl+C (the clean-up still runs)
"$IAI_PY" scripts/iai_client.py run -n 30 --hold
# another area: centre and side of the populated square, CARLA metres
"$IAI_PY" scripts/iai_client.py run -n 20 --center 0 0 --size 150
```

`run` puts the world in synchronous mode at 0.1 s (the only step Inverted AI
supports) and becomes the ticker. If the world is already synchronous, it warns
and ticks it anyway. Don't run it next to another client that ticks.

### Step 4: Full example

```bash
N=30 W=10 SEED=7 LEN=60 scripts/run_example.sh ./iai_out --record --iai-log
```

The example loads `carla:Town10HD` (→ `Town10HD_Opt`) even when it is already
loaded, so allow about a minute for the reload on UE 5. Outputs go to
`./iai_out/logs/carla_record.log` and `./iai_out/<ts>/iai_log/`.

### Verify / clean up

```bash
"$IAI_PY" scripts/iai_client.py status     # NPC count (role_name=iai_npc) and the clock
"$IAI_PY" scripts/iai_client.py clean      # destroy leftovers, world back to async
```

`run` ends with a line like the one measured on Town10HD (`-n 30 --seed 7
--steps 200`): `200 steps (20 s sim), 24/28 vehicles moved > 2 m, final mean
speed 10.7 km/h, large_drive 384 ms/step`. A 0.1 s step costs about 0.4 s of
wall time, so expect about 4x slower than real time. Plan recordings, not live
demos. If few cars moved, see Troubleshooting.

The SDK prints `Commercial access denied and fallback to check for academic
access.` on every authenticated call with an academic key. It is not an
error; the scripts filter it out of their own output.

## Examples

- "Fill Town10 with realistic traffic using Inverted AI": `check_env.sh`, then
  `run -n 40 --hold`.
- "Is my Inverted AI key working?": `check_env.sh --online` or `ping`.
- "Record 1 minute of IAI traffic and replay it from the junction": `run -n 30
  --steps 600 --record /abs/iai.log`, then `replay-recording` on that file. The
  replay needs neither the key nor the venv.
- "Compare TM and Inverted AI": `spawn-vehicles spawn --hold` vs `run --hold`,
  in the same spot.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| `IAI_API_KEY not set` | Not in `.env` at the tree root, or a typo in the key name. Run `check_env.sh`, which shows where it looked. |
| `InvalidAPIKeyError` / `AuthenticationError` | Wrong or expired key. Rotate it at inverted.ai and update `.env`. |
| `RateLimitError` | The quota was hit. Use fewer agents (`-n`) or fewer steps. |
| `invertedai not importable` / numpy errors | The wrong interpreter. Use `$IAI_PY`, not `python3`. Re-run `install_iai.sh`. |
| `carla not importable` in the venv | The venv was made without `--system-site-packages`, or the wheel is in another user site. Install the wheel into the venv with `install-python-api`. |
| Few spawned (`spawned 12/30`) | Initial states overlapped existing actors. Clear the area or change `--seed`/`--center`. |
| NPCs run red lights | The map is not Town10HD, so there is no light mapping. |
| Server froze after a crash | The world was left synchronous. Run `iai_client.py clean`. |
| Example prints `KeyError: '4378'` (a bare `'4378'` in older examples) after "Generating birdview GIF" | It comes from the SDK's GIF renderer, not from the light mapping, which matches IAI's ids exactly. The recording, the JSON log and a GIF are still written; the wrapper flags it as WARN. |
| `AttributeError: Off` from `TrafficLightState` | Older examples used `TrafficLightState.Off`; the SDK's off state is `TrafficLightState.none`. Fixed in the example and in `iai_client.py`. |
| Frozen cars/walkers left after the example | An older example that never cleaned up, or a crash. `CLEAN=1 run_example.sh …`, or by hand: `restore_world.py HOST PORT 1`. |

More detail on the API and its limits: [references/invertedai.md](references/invertedai.md).

## Outputs

- NPC vehicles tagged `role_name=iai_npc`, removed on exit unless `--keep` is
  passed.
- Optional CARLA recorder log (`--record`).
- Example: `run.txt`, `logs/carla_record.log`, `<ts>/iai_log/*.json|*.gif`.
