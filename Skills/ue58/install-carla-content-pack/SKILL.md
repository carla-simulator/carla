---
name: install-carla-content-pack
description: Installs a CARLA content pack tarball into a CARLA UE 5.8 package (CarlaUnreal/Packs/), after checking that it fits that package — base release, engine, other installed packs that ship the same town, disk — and lists, replaces or removes installed packs. Also covers loading packs from outside the package with -carla-packs and CARLA_PACKS. Use when the user asks to "install a content pack", "add this pack to CARLA", "carla-pack install", "which packs are installed", "remove/update a pack", or a pack's maps or blueprints do not appear after installing.
license: MIT
compatibility: Any OS with Python 3.8+ (Linux verified; Windows Git Bash untested) and a CARLA 1.0+ UE 5.8 package with a CarlaUnreal/BaseRelease stamp. No source checkout needed; uses the package's Tools/carla_pack.py. Verified on a CARLA 1.0 Shipping package (ue58-dev 1ad2c15c9).
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/installing.md
---

# Install a CARLA content pack

> **Paths.** `scripts/…` and `references/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

The deliverable is a verified pack folder at `<package>/CarlaUnreal/Packs/<Pack>/`, which the server mounts at its next start. `scripts/packs.py` wraps `carla_pack.py install` and adds the checks it skips. Building a pack is [[create-carla-content-pack]]; the official AdditionalMaps pack is [[get-carla-additional-maps]]; mounting into a running server is [[manage-content-packs]].

## What "compatible" means

`carla_pack.py` and the server compare one string: the pack's `base_release` against the package's `CarlaUnreal/BaseRelease` (`carla-1.0-Linux`). Every 1.0 build carries that same string: the release, each weekly, each local package. So `Compatible: yes` only rules out another CARLA version. It does not prove that the pack was cooked against **this** package.

A DLC cook leaves out every asset its base package already had. A pack cooked against a package with a wider cook scope therefore references assets that a narrower package lacks, and it still installs and mounts without a word. Measured: the official `AdditionalMaps_1.0` names 26 base assets (`MI_LargeLandscape_Highway`, legacy facade and farm folders) that a local package cooking only Town10HD_Opt does not have. `carla_pack.py install` accepted it, and its towns loaded with content missing: Town_C had 7,423 building objects, against 56,665 with the same pack on the official package. Pair packs by origin:

| Package | Install |
|---|---|
| Downloaded release or weekly | the pack published **with it** (same folder, same upload) |
| Built locally | a pack built from the same tree **after** that package |

`packs.py check` warns on an engine-commit mismatch when the pack records one. [[get-carla-additional-maps]] `pair` compares asset registries when both release metadata files are at hand.

## Instructions

```
Progress:
- [ ] Step 1: bash scripts/check_env.sh — package, stamp, tool, disk
- [ ] Step 2: packs.py check <tarball> — stamp, engine, clashes, disk
- [ ] Step 3: packs.py install <tarball> — install + verify the installed folder
- [ ] Step 4: start/restart the server and confirm with manage-content-packs list
```

```bash
bash scripts/check_env.sh
source scripts/env.sh                                    # CARLA_PACKAGE_ROOT, CARLA_PACK_TOOL
python3 scripts/packs.py check   MyPack-1.0.0-carla-1.0-Linux.tar.gz
python3 scripts/packs.py install MyPack-1.0.0-carla-1.0-Linux.tar.gz   # [--replace] [--allow-clash]
python3 scripts/packs.py list
```

`CARLA_PACKAGE_ROOT` is the folder holding `CarlaUnreal.sh` (`CarlaUnreal.exe` on Windows). `env.sh` falls back to `CARLA_TARGET`, `$PWD`, then the newest `Build/<preset>/Package/Carla-*` of a ue58-dev checkout.

- **install** verifies every SHA-256, refuses unlisted files, checks the stamp and platform, extracts to `CarlaUnreal/Packs/<Pack>/`, then runs `verify` on the installed folder. Measured: 615 MB in 3.5 s, AdditionalMaps (7.5 GB, 35 files) in 46 s.
- **`--replace`** swaps an installed pack of the same name: the old folder is moved aside and comes back if the new tarball fails its checks. It refuses while a server from this package runs. **`--force`** only overrides the base-release check; use it only for a pack cooked from this exact package.
- **Clashes.** Two packs that ship the same town, or two maps with the same name at different paths (a town and an `--import` copy), both mount and the name is listed once. Only one copy loads, since `load_world()` takes names, not package paths. `install` refuses unless `--allow-clash`. Measured: AdditionalMaps' `/Game/Carla/Maps/Town03_Opt` and a test pack's `/SkFull/Maps/Town03_Opt` both mounted without a warning, `Town03_Opt` loaded AdditionalMaps' copy, and the other was unreachable by any path.
- **remove** (`packs.py remove <Pack>`) deletes the folder; it refuses while the server runs, because its paks are mounted.

### Step 4: Confirm on the server

Start or restart the server ([[run-carla-ue58-server]] `package`), then [[manage-content-packs]] `list`: the pack is `mounted` and its towns are in `get_available_maps()`. A Shipping server logs nothing about packs. A pack missing from the list was refused, and `manage-content-packs why <folder>` prints the reason.

### Packs outside the package

The server also scans the directories in `-carla-packs=<dir>[;<dir>]` and `$CARLA_PACKS` (`;`-separated). Each one is the **parent** of pack folders, as `CarlaUnreal/Packs/` is. Both were verified: `CARLA_PACKS=/data/packs ./CarlaUnreal.sh`, or `./CarlaUnreal.sh -carla-packs=/data/packs`. Through run-carla-ue58-server use `EXTRA=-carla-packs=/data/packs`; `EXTRA` splits on spaces, so put a path with spaces in `CARLA_PACKS`. Never put packs in `CarlaUnreal/Plugins/`.

## Examples

**Example 1: "install this pack into my package"**

`check_env.sh` → `packs.py check <tar>` (stamp OK, no clash) → `packs.py install <tar>` → restart the server → `manage-content-packs list` shows it mounted.

**Example 2: "update MyPack to 1.1"**

Stop the server, `packs.py install MyPack-1.1.0-....tar.gz --replace`, start it again. A running server cannot take new files for a pack it has loaded.

**Example 3: "install AdditionalMaps"**

Use [[get-carla-additional-maps]]: it picks the pack that matches the package and installs it through this script.

## Troubleshooting

**`base release: pack 'carla-1.0-Linux', package 'carla-1.1-Linux'`**: the pack was cooked for another CARLA version. Rebuild it against this package's release metadata ([[create-carla-content-pack]]).

**`package has no BaseRelease stamp`**: the package predates content packs (0.10.0 and older). Use a 1.0+ package.

**`<Pack> is already installed`**: use `--replace`, with the server stopped.

**The pack installed, the server lists it, and its town loads with missing meshes or crashes the server**: check the pairing first (see "What compatible means"; [[get-carla-additional-maps]] `pair` compares the registries). Measured: Town13 crashed a local Shipping package that cooks only Town10HD_Opt (`ATrafficLightManager::GenerateSignalsAndTrafficLights`) with a freshly built, correctly paired AdditionalMaps too, and loads on the official package with its own pack. So a crash is not proof of a bad pair; missing buildings are the pairing symptom.

More on the layout, discovery order and limits: [references/installing.md](references/installing.md).

## Outputs

- `<package>/CarlaUnreal/Packs/<Pack>/` — `carla-pack.json`, `<Pack>.uplugin`, `AssetRegistry.bin`, `Content/Paks/<Platform>/*.{pak,utoc,ucas}` and the loose `Content/Config` and `Content/Maps/{OpenDrive,Nav,TM}` files.
