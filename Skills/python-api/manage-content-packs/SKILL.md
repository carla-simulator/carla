---
name: manage-content-packs
description: Lists, mounts and unmounts CARLA content packs on a running UE 5.8 server through the Python API (get_content_packs, mount_content_pack, unmount_content_pack), shows which maps and blueprints a pack adds, and finds out why a pack is missing when a Shipping server logs nothing. Use when the user asks "which packs are loaded", "mount this pack without restarting", "unmount a pack", "why is my pack/map/blueprint not showing up", or wants a pack's town or vehicles on a server that is already running.
license: MIT
compatibility: Any OS with the CARLA 1.0+ (UE 5.8) Python client and a reachable server built with content-pack support. Does not need a source checkout. Verified against a CARLA 1.0 Shipping package on ue58-dev 1ad2c15c9.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
---

# Manage content packs on a running server

> **Paths.** `scripts/…` below is relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix it with its absolute path or the command is not found.

Acts on a **running** server; starting one is [[run-carla-ue58-server]], putting a pack into a package is [[install-carla-content-pack]], building one is [[create-carla-content-pack]]. Every command prints what the server reports afterwards, so the result is checked rather than assumed.

## Instructions

```
Progress:
- [ ] Step 1: bash scripts/check_env.sh — client, server and the content-pack API
- [ ] Step 2: content_packs.py list — what is mounted, what is only known
- [ ] Step 3: mount / unmount / why, as asked
- [ ] Step 4: read back: list again, load_world for a pack town
```

```bash
source scripts/env.sh                       # CARLA_HOST / CARLA_PORT, default 127.0.0.1:2000
python3 scripts/content_packs.py list
python3 scripts/content_packs.py mount /path/on/server/CarlaUnreal/Packs/MyPack
python3 scripts/content_packs.py unmount MyPack --switch-to Town10HD_Opt
python3 scripts/content_packs.py why /path/on/server/MyPack
```

## What the server does

- **At start** it mounts every pack folder found under `<package>/CarlaUnreal/Packs/`, under each directory of `-carla-packs=<dir>[;<dir>]`, and under each directory of `$CARLA_PACKS`. Those are **parent** directories: each subfolder holding a `carla-pack.json` is a pack. Only game processes discover packs; the full editor does not.
- **`mount_content_pack(path)`** takes a pack folder or its `carla-pack.json`, as the **server** sees the path. The pack's vehicles, walkers and props are in `get_blueprint_library()` at once, with no episode change. Its towns need `load_world()`.
- **`unmount_content_pack(name)`** returns `True`, or raises `RuntimeError` with the reason: one of its maps is the current world, or one of its objects is still loaded. Load another map, or destroy the actors or `reload_world()`, then retry.
- **`get_content_packs()`** lists mounted and known-but-unmounted packs (`mounted=False`) with `name`, `version`, `base_release`, `path` and `maps`.

## Gotchas (measured on a Shipping server)

- **A refused pack is invisible.** A Shipping server writes no `Content packs:` lines to its log, and a pack it refused at start is simply missing from `get_content_packs()`. `why <folder>` calls `mount_content_pack` on it, which returns the reason: for example `was built against base release 'carla-0.9.9-Linux' but this server is 'carla-1.0-Linux'; rebuild the pack against this release`. If the mount succeeds, the server never found the pack at start (wrong directory level, or the path was not passed).
- **Stale blueprint list after unmount.** `get_blueprint_library()` still lists an unmounted pack's blueprints until the next `load_world`/`reload_world`, for every client, a fresh one included; measured 2 props listed after a successful unmount, 0 after `reload_world()`.
- **A refused unmount changes nothing.** With a pack mesh still in use (`objects from the pack are still loaded`) the pack stays mounted and its props stay spawnable. Actors whose class is a CARLA `/Game` asset (a catalog entry pointing at a base blueprint) do not block the unmount.
- **One name per server.** After unmounting, mounting a copy of the same pack from another folder fails with `pack name '<Pack>' collides with the plugin at '<first path>'`, because the first folder stays registered. Remount the same folder, or restart the server to switch copies.
- **Tarballs do not mount.** `mount_content_pack('<pack>.tar.gz')` reads the whole file as JSON and fails after seconds with `manifest is not valid JSON`; `mount` refuses a tarball before calling the server. Install or extract it first.
- **Updated files need a restart** once anything from the pack has been loaded: Unreal does not reload a package already in memory.
- **Two packs shipping one town both mount.** The map is listed once and resolves to one of them, and nothing warns. Keep each town in one pack.

## Examples

**Example 1: "what packs does the server have?"**

`content_packs.py list` → one line per pack with mounted state, base release and maps, then the current map and its owning pack, then every available map.

**Example 2: "mount MyPack and load its town"**

`content_packs.py mount <server path>/MyPack` prints its maps and the blueprints it added. Then `client.load_world('<Map>')`, as for any map.

**Example 3: "my pack's town is not in get_available_maps"**

`list` shows no `MyPack` → `why <package>/CarlaUnreal/Packs/MyPack` prints the server's refusal. A base-release mismatch means rebuilding the pack ([[create-carla-content-pack]]) against this package's release metadata.

## Outputs

Nothing on disk. Mounting and unmounting change the running server only; a restart goes back to whatever its pack directories hold.
