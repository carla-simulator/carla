---
name: get-carla-additional-maps
description: Gets the AdditionalMaps content pack (Town01-07_Opt, Town11, Town13, Town_C, Mine_01, Eixample) for a CARLA UE 5.8 package and installs it — downloads the pack published with a release or weekly build, or builds it on a ue58-dev source checkout with the package-additional-maps target (optionally from a custom map list), checks that the pack really matches the package, and installs it. Use when the user asks for "additional maps", "Town13/Town03 on CARLA 1.0", "install AdditionalMaps", "the towns missing from the package", "package-additional-maps", or a town from AdditionalMaps crashes or is missing.
license: MIT
compatibility: Linux verified (Windows packs are listed and downloadable, install untested). Downloading needs curl and ~15 GB; building needs a ue58-dev checkout that has built the package (package-carla-ue58), its engine, and ~25 GB. Verified 2026-11-19 against CARLA 1.0 (release, weekly and a local Shipping package).
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
---

# Get the AdditionalMaps pack

> **Paths.** `scripts/…` below is relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix it with its absolute path or the command is not found.

The 1.0 package ships `Town10HD_Opt`, `Town12` and `Town15`. The other towns come as one content pack, `AdditionalMaps`, built and published with each package. The deliverable is that pack installed in the package it was built for, with its towns loadable. UE4 releases (0.9.x) use `ImportAssets.sh` instead; that is [[download-carla]].

## The rule: the pack must come from the same build as the package

A pack is cooked against one package's asset registry and leaves out everything that package already has. Every 1.0 build is stamped `carla-1.0-Linux`, so `carla-pack install` cannot tell one build's pack from another's and accepts both. Measured on a custom package that cooks only Town10HD_Opt: the official `AdditionalMaps_1.0` names 26 base assets the package lacks, installs as `Compatible: yes`, and loads its towns with content missing: Town_C has 7,423 building objects against 56,665 on the official package, Town07_Opt 4 against 22. The release and weekly bases are identical (9,874 `/Game` names each), so their packs are interchangeable today; a custom `CARLA_MAPS_TO_COOK` package is not.

| Package | AdditionalMaps |
|---|---|
| release `CARLA_1.0` | `AdditionalMaps_1.0.tar.gz`, same folder |
| weekly `CARLA_UE5_Weekly` | `CARLA_UE5_Weekly_AdditionalMaps.tar.gz`, same upload |
| nightly `CARLA_UE5_Latest` | none: no pack is published for it; use the weekly or the release |
| built locally | build it from the same tree, right after the `package` target |

## Instructions

```
Progress:
- [ ] Step 1: bash scripts/check_env.sh — which package, downloaded or local
- [ ] Step 2: get the matching pack: A (download) or B (build)
- [ ] Step 3: additional_maps.py pair — check the pack against the package
- [ ] Step 4: additional_maps.py install, restart the server, load a town
```

```bash
bash scripts/check_env.sh
source scripts/env.sh        # CARLA_PACKAGE_ROOT, CARLA_UE58_ROOT, CARLA_PRESET
```

### Step 2A: Download (release or weekly package)

```bash
python3 scripts/additional_maps.py urls --channel release --version 1.0      # or --channel weekly
python3 scripts/additional_maps.py download --channel release --dest ~/carla-downloads [--with-package]
```

`urls` sends HEAD requests for the package, the pack and the release metadata, then checks they come from one upload (the pack lands minutes after the package; measured 1-3 min on every 2026-11 pair, Linux and Windows). `download` fetches the pack and the metadata, resumable with `curl -C -`, and checks each size against `Content-Length`. `--with-package` also downloads and extracts the matching package (14 GB) when there is none yet. Measured: the 6.8 GB pack in 3 min 44 s.

### Step 2B: Build (locally built package)

```bash
python3 scripts/additional_maps.py build                                   # the 12 towns of Util/ContentPacks/AdditionalMaps.json
python3 scripts/additional_maps.py build --maps Town03_Opt,Town13 --fresh  # your own selection
```

Without a list, `build` runs `cmake --build Build/<preset> --target package-additional-maps`. That target cooks against `Unreal/CarlaUnreal/Releases/<release>`, which the last `package` build wrote, and writes `Build/<preset>/Package/AdditionalMaps-<version>-<release>.tar.gz`. So build the package first, then the pack, and rebuild the pack whenever the package is rebuilt.

With `--maps`/`--maps-file`, `build` calls `carla-pack create` with the target's arguments and your list, and leaves the CMake cache alone. **`create` only adds maps:** an existing `Plugins/Packs/AdditionalMaps/` keeps every town registered before, and a shorter list still cooks all of them (measured: a one-map list still put 12 maps on `-MapsToCook`). `--fresh` deletes those pack sources first, so only your list ships. The CMake way to select maps is `-DCARLA_ADDITIONAL_MAPS_FILE=<json>`, and it has the same additive behaviour.

A map the package already cooks is refused (`the base release already ships ...`); keep the two lists disjoint.

### Step 3: Check the pair

```bash
python3 scripts/additional_maps.py pair <AdditionalMaps tarball>                 # local: age check
python3 scripts/additional_maps.py pair <tarball> --pack-base <metadata the pack was cooked against> \
                                                  --package-base <the package's metadata>
```

Without `--pack-base`, `pair` warns when the pack is older than the package's metadata (rebuild it). With both metadata files it lists the pack's base-asset names that the package lacks, and fails when there are any. The names come from the registries' FName tables (packages and folders). Measured: official pack vs release base 0, vs weekly base 0, vs a Town10-only local package 26.

### Step 4: Install and load

```bash
python3 scripts/additional_maps.py install <AdditionalMaps tarball>    # through install-carla-content-pack
```

Then restart the server ([[run-carla-ue58-server]]) and load a town: `client.load_world('Town13')`. [[manage-content-packs]] `list` shows `AdditionalMaps mounted` with its 12 maps. Remove any other pack that ships one of these towns first; `install` refuses a clash.

## Examples

**Example 1: "I downloaded CARLA 1.0, give me Town13"**

`urls --channel release` (pair OK) → `download --channel release --dest ~/dl` → `install ~/dl/AdditionalMaps_1.0.tar.gz` → restart → `load_world('Town13')`.

**Example 2: "I built the package myself, I need Town03 and Town05"**

`build --maps Town03_Opt,Town05_Opt --fresh` → `pair <tarball>` → `install <tarball>`.

**Example 3: "Town13 crashes the server, the other towns load"**

Run `pair` with both metadata files first: it names base assets the pack expects and the package lacks. If it is clean, the pairing is not the cause. Measured: Town13 crashed a local Shipping package (ue58-dev `1ad2c15c9`, cooking only Town10HD_Opt) in `ATrafficLightManager::GenerateSignalsAndTrafficLights` at `BeginPlay` with every pack tried, a freshly built AdditionalMaps included. It loads on the official 1.0 package with its pack and in editor `-game`. Eixample crashed the same way only with the mismatched official pack. The narrow cook scope and the older tree are both candidates. Report that as a CARLA bug with the package's `VERSION` and the crash's `PCallStackHash`.

## Troubleshooting

**`this channel has no complete package + AdditionalMaps pair`**: the upload for that platform/channel is missing or in progress. Use the other channel, or retry later.

**`WARN pack and package were uploaded N h apart`**: probably not one build. Wait for the next upload rather than mixing.

**`the base release already ships /Game/Carla/Maps/<Town>`**: the package cooks that town (`CARLA_MAPS_TO_COOK`). Drop it from the list.

**`2 maps are called Town03_Opt; pass the one you mean`**: another pack under `Plugins/Packs/` holds an `add --import` copy of that town. Move that pack out of `Plugins/Packs/` while the target runs (it only reads names from `AdditionalMaps.json`), or build with `--maps-file` listing `/Game/Carla/Maps/<Town>` paths.

**`<Pack> is already installed`**: `install --replace`, with the server stopped.

## Outputs

- Downloads: `<dest>/AdditionalMaps_<version>.tar.gz` (or `CARLA_UE5_Weekly_AdditionalMaps.tar.gz`) and the release metadata; with `--with-package`, the extracted package.
- Build: `Build/<preset>/Package/AdditionalMaps-<version>-<release>.tar.gz` and the unpacked `Build/<preset>/Package/AdditionalMaps/`.
- Install: `<package>/CarlaUnreal/Packs/AdditionalMaps/`.
