# Installing content packs: detail for install-carla-content-pack

## Contents

- [Installed layout](#installed-layout)
- [Discovery at start](#discovery-at-start)
- [What carla_pack.py install checks](#what-carla_packpy-install-checks)
- [Pairing a pack with a package](#pairing-a-pack-with-a-package)
- [Measured](#measured)

## Installed layout

```text
<package>/CarlaUnreal/Packs/<Pack>/
  carla-pack.json                       manifest: name, version, base_release, engine, platform, maps, catalogs, files{path: sha256}
  <Pack>.uplugin                        content-only plugin, ExplicitlyLoaded
  AssetRegistry.bin                     the pack's cooked registry, appended to the server's on mount
  Content/Paks/<Platform>/<Pack>CarlaUnreal-<Platform>.{pak,utoc,ucas}
  Content/Config/*.json                 catalogs, read by file name
  Content/Maps/{OpenDrive,Nav,TM}/      sidecars, read beside the containers
```

`<package>/CarlaUnreal/BaseRelease` (and `<package>/BaseRelease`) holds the package's stamp; `<package>/VERSION` holds the Carla, content and engine commits.

## Discovery at start

`UCarlaContentPackManager::Discover()` runs in game processes only (not the editor, not commandlets) and scans, in order:

1. `<package>/CarlaUnreal/Packs/`
2. each directory of `-carla-packs=<dir>[;<dir>]` (quote the value when a path has spaces)
3. each directory of `$CARLA_PACKS`

Each is a parent directory; every subfolder with a `carla-pack.json` is mounted before the first episode. A pack that fails is skipped with `Content packs: '<manifest>' not mounted: <reason>` in Development builds; **Shipping logs nothing** and the pack is just absent from `client.get_content_packs()`. `client.mount_content_pack(<folder>)` returns the reason.

## What carla_pack.py install checks

- every file against its SHA-256, and no file outside the manifest
- `base_release` == the package's `BaseRelease` (`--force` skips)
- the platform against `CarlaUnreal/Binaries/<Platform>`
- no installed pack of the same name (`--force` replaces)

It does **not** check the engine commit, the cook scope of the package, or other packs shipping the same town. `packs.py` adds the engine-commit and clash checks; the cook scope needs both base registries (`get-carla-additional-maps pair`).

## Pairing a pack with a package

The stamp is `carla-<version>-<platform>`. Every build of one version shares it, so the stamp separates versions, not builds. What a DLC pack depends on is the exact set of assets its base cooked: references to assets the base had are left out of the pack. A package that cooked fewer maps lacks some of those assets.

Measured on 2026-11-19 (FName tables of the cooked registries):

| Base | `/Game` names |
|---|---|
| `CARLA_1.0_release-metadata` (release) | 9,874 |
| `CARLA_UE5_Weekly_release-metadata` | 9,874, same set as the release |
| local Shipping package, scope Town10HD_Opt + OpenDriveMap | 5,327, a strict subset |

The official `AdditionalMaps_1.0` references 26 names that the release base has and the local package lacks (`MI_LargeLandscape_Highway/Mountain/Rural`, legacy facade and farm folders). `carla_pack.py install` accepts it into the local package (`Compatible: yes`), and the towns load with content missing:

| Town | Official pack on the official package | Official pack on the local Town10-only package |
|---|---|---|
| Town_C | 56,665 | 7,423 |
| Town07_Opt | 22 | 4 |
| Town03_Opt | 804 | 804 |

## Measured

| Step | Result |
|---|---|
| install 615 MB pack | 3.5 s |
| install AdditionalMaps (7.5 GB, 35 files) | 46 s |
| second install without `--force` | refused, exit 1 |
| install into a package stamped `carla-0.9.9-Linux` | refused, exit 1 (`inspect --server` exits 3) |
| `CARLA_PACKS=<dir>` | mounted |
| `-carla-packs=<dir>` | mounted |
| base-mismatched pack via `CARLA_PACKS` on Shipping | absent from `get_content_packs()`, no log line |
