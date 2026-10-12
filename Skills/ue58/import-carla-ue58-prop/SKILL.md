---
name: import-carla-ue58-prop
description: Imports FBX meshes into CARLA on UE 5.8 as spawnable props — drives the editor headlessly to import the mesh, derives the prop size from its measured bounds, and registers it in PropParameters.json so it appears in the blueprint library as static.prop.<name>. Handles the FBX unit-scale problem, verifies the result against a running server, and can revert cleanly. Use when the user asks to "import a prop", "add a custom mesh/object to CARLA", "make my model spawnable", or a prop imports but never shows up in the blueprint library.
license: MIT
compatibility: Linux with a built ue58-dev tree (engine + CarlaUnreal editor) and the content cloned. Needs CARLA_UNREAL_ENGINE_PATH; the live verification also needs a running server and an importable `carla`. VERIFIED end to end on ue58-dev HEAD 718efd7cc, engine 5.8.0, CARLA 0.10.0 - imported, registered, spawned in the editor and via the API, then reverted.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/props.md
---

# Import a prop on UE 5.8

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Three steps, and the middle one is where UE4 recipes break:

```
FBX  --(editor, headless)-->  StaticMesh in Content
     --(PropParameters.json)-->  static.prop.<name> in the blueprint library
     --(PropActorFactory)-->  spawnable actor
```

**Registration moved.** On UE4 a prop is declared in a `*.Package.json` that
`UCarlaBlueprintRegistry::LoadPropDefinitions` scans for. On ue58 that function
has **zero callers — it is dead code**. Props are loaded by `APropActorFactory`
from `Content/Carla/Config/PropParameters.json` (key `"Props"`, fields
`Name`/`Mesh`/`Size`). Writing a `.Package.json` changes nothing: verified by
doing it and watching the prop count stay at 83.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: plan — see where the asset lands and what it will be called
- [ ] Step 3: import — get the scale right on the first go if you can
- [ ] Step 4: RESTART the server or editor, then verify
```

### Step 2-3: Plan, then import

```bash
source scripts/env.sh

python3 scripts/import_prop.py plan   ~/meshes/Bench.fbx
python3 scripts/import_prop.py import ~/meshes/Bench.fbx --tag Static
```

A directory works too, and a level of subdirectory is read as the tag — the same
shape as the destination:

```
~/meshes/
├── Static/Bench.fbx        ->  /Game/Carla/Static/Static/Bench
└── Building/Kiosk.fbx      ->  /Game/Carla/Static/Building/Kiosk
```

| Flag | Effect |
|---|---|
| `--tag` | subfolder under the destination root (default `Static`) |
| `--scale` | `import_uniform_scale`; **see below** |
| `--size` | override the derived `Tiny`/`Small`/`Medium`/`Big` |
| `--dest-root` | default `/Game/Carla/Static` — keep it under `/Game/Carla` |
| `--no-register` | import the mesh without touching the registry |
| `--no-combine` / `--no-collision` | disable mesh combining / auto collision |
| `--verbose` | stream the editor log |

### Get the scale right

**FBX unit metadata is frequently absent or wrong, and nothing warns you.** A door
from CARLA's own `HoudiniEngine/Pieces` imported at `--scale 1.0` measured
**0.43 × 250 × 400 m** — a 400-metre door, spawnable and towering over the town.
At `--scale 0.01` the same file measured **0.43 × 2.5 × 4.0 m** and classified as
`Medium`.

`import` prints the measured dimensions and warns when the longest side exceeds
30 m, which is the practical "this is building-scale" threshold. Re-import with the
right factor; `replace_existing` is on, so it overwrites cleanly.

Size, when not overridden, comes from the measured longest dimension:
`<=0.5 m Tiny`, `<=2 m Small`, `<=8 m Medium`, else `Big`.

### Keep the destination under `/Game/Carla`

An asset elsewhere in `Content/` imports fine and works in the editor, but a
**packaged** server resolves content only under `/Game/Carla/...` — the same
single-fallback limitation that stops nested large maps loading from a package
([[run-carla-ue58-server]]). `list` flags any registered prop whose mesh sits
outside `/Game/Carla`.

### Step 4: Restart, then verify

**Prop definitions are read once at startup.** A prop registered while a server is
running will not appear until you restart it.

```bash
cd ../run-carla-ue58-server && bash scripts/run_server.sh stop
DETACH=1 bash scripts/run_server.sh game

cd ../import-carla-ue58-prop
python3 scripts/import_prop.py verify --name Bench --spawn
python3 scripts/import_prop.py list
```

`verify` checks the registry entry, that the `.uasset` is on disk, that the
blueprint is in the library with the right `mesh_path`, and — with `--spawn` —
spawns it, reports its bounding box and destroys it.

### Undo

```bash
python3 scripts/import_prop.py revert --name Bench          # dry run
python3 scripts/import_prop.py revert --name Bench --yes
```

Unregisters and deletes the asset directory. `import` and `revert` both back
`PropParameters.json` up to `PropParameters.json.bak` first.

## Examples

**Example 1: "add this bench mesh so I can spawn it"**

`plan`, then `import ~/bench.fbx`. If the printed dimensions look wrong, re-import
with `--scale`. Restart the server, `verify --spawn`. The blueprint is
`static.prop.bench`.

**Example 2: "I imported a prop but it's not in the blueprint library"**

Two causes in order of likelihood: the server was not restarted, or the prop was
registered in a `.Package.json` (dead on ue58). `list` shows what
`PropParameters.json` actually contains — that is the only file that counts.

**Example 3: "import a folder of props with tags"**

Lay them out as `Static/…`, `Dynamic/…`, `Building/…` and pass the parent
directory. Each subfolder becomes the tag and the destination subfolder.

**Example 4: "my prop is invisible to semantic segmentation"**

Expected. `semantic_tags` comes back empty on an imported prop, and ue58 has no
`GenerateTaggedMaterialsRegistry` commandlet (UE4 ran it across packages), so the
materials must be tagged by hand.

## Troubleshooting

**Problem: the blueprint never appears**
Cause: server not restarted, or the entry went into a `.Package.json`.
Solution: restart; use `list` to confirm `PropParameters.json` holds it.

**Problem: `the editor produced no result file (exit 255)`**
Cause: the in-editor script raised. `print()` and `unreal.log()` from the
pythonscript commandlet do **not** reach the log, so the result file is the only
channel — but `LogPython: Error` does appear.
Solution: re-run with `--verbose` to see the traceback.

**Problem: the prop is enormous or microscopic**
Cause: FBX unit scale.
Solution: `--scale 0.01` for centimetre-authored files. The dimensions printed on
import are the ground truth.

**Problem: `PropParameters.json missing`**
Cause: the content repository is not cloned into
`Unreal/CarlaUnreal/Content/Carla`.
Solution: clone `carla-content` branch `ue58-dev-carla` ([[build-carla-ue58]]).

**Problem: import succeeds but no StaticMesh is produced**
Cause: the FBX holds only a skeletal mesh, or only curves/cameras.
Solution: `import` reports how many assets were created; a prop needs a static
mesh. Skeletal assets are vehicles/walkers, not props.

**Problem: the prop spawns but falls through the world or floats**
Cause: `--no-collision`, or the mesh origin is not at its base.
Solution: re-import with collision, or fix the pivot in the DCC tool.

**Problem: works in the editor, missing on a packaged server**
Cause: the asset is outside `/Game/Carla/`.
Solution: re-import with the default `--dest-root`; `list` flags offenders.

**Problem: `Util/Tools/Import.py` seemed like the obvious tool**
Cause: it is a map pipeline. `generate_json_package` only auto-detects maps and
hardcodes `'props': []`, it ignores commandlet exit codes on POSIX, and it
registers into the dead `.Package.json` path.
Solution: this skill drives the editor directly.

## Outputs

A `StaticMesh` (plus its materials) under `/Game/Carla/Static/<Tag>/<Name>`, an
entry in `PropParameters.json` with a backup beside it, and a
`static.prop.<name>` blueprint after the next server start. `plan`, `list` and
`verify` are read-only; `revert` removes both halves.

Registration mechanics, the UE4→ue58 differences and the measured scale evidence
are in [references/props.md](references/props.md).
