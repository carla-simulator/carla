---
name: restyle-carla-ue58-signs
description: Switches the traffic signs of the map open in the CARLA UE 5.8 editor between the Vienna Convention (Europe), MUTCD (US) and GB (China) styles, live, through unreal-mcp — every regional BP_Sign whose sign has an equivalent in the target style gets that style's plate (speed limits converted to mph for MUTCD, from the road's real km/h), and switching to the map's own style, or to "original", restores the map's own signs. Works on Town15 (Vienna) and on the classic towns Town01-07 and Town10HD (MUTCD). Nothing is saved; closing the editor without saving leaves the map untouched. Use when the user asks to "show the signs in US / American / MUTCD style", "make Town15 look like China", "change the sign convention", "switch the signs back to European", or to compare sign conventions on the same street.
license: MIT
compatibility: Linux with a built ue58-dev tree, the full CarlaUnreal editor open (unreal-mcp on localhost:6256; not -game, not a package). Every plate comes from data assets under /Game/Carla (Blueprints/LevelDesign/Signs/DataAssets/<STYLE>), so switching needs no plugin; the CarlaDigitalTwinsTool sign catalog is only needed to regenerate the table (scripts/build_table.py). Works on maps built with the regional sign actor BP_Sign: Town15, and Town01-07 / Town10HD, whose classic signs carry a visual-only MUTCD BP_Sign. VERIFIED on ue58-dev 94f2985b4 + the MUTCD classic towns, Town07 - MUTCD -> Vienna (95 signs), -> GB (87), back to original (all restored).
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/signs.md
---

# Switch the sign convention of a map (UE 5.8 editor)

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

This is an **editor** operation. The sign style is not exposed by the CARLA
Python API and cannot change on a running server: it is part of the map's
content. The skill changes it in the open editor level, where the user sees it
in the viewport at once.

```
BP_Sign (map) --signAssetData--> map's data asset (atlas cell, VC / MUTCD / GB)
        |                               |  atlas + cell -> "stop", "max_speed_30", ...
        |                               v
        +--> data asset of the same sign in the target style:
             /Game/Carla/.../DataAssets/<STYLE>/<Category>/DA_* (in Content, no plugin)
             (a sign the table maps to a catalog entry only: made in memory under
              /Game/SignStyleSwap/<STYLE>/..., never saved; needs the plugin)
```

Two kinds of map carry `BP_Sign`:

- **Town15** is built with it, in the Vienna style.
- **Town01-07 and Town10HD** keep their classic sign actors for the traffic
  logic (state, trigger volume, OpenDRIVE match) with their meshes hidden, and
  draw a MUTCD `BP_Sign` over each one that has an equivalent. That `BP_Sign`
  has `bVisualOnly` set: it is not a `traffic.*` actor and never takes a signal.
  The skill restyles these plates like any other; the logic underneath never
  changes. Their tags `MUTCDOverlay` and `MUTCDOverlayOf:<classic actor>` mark
  them.

## Instructions

```
Progress:
- [ ] Step 1: Editor open on the map, check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Dry run in the target style, read what changes and what does not
- [ ] Step 3: Apply; tell the user what changed (counts per sign) and what was left
- [ ] Step 4: Back to the map's own signs (--style original) when asked; remind the user not to save
```

### Step 1: Editor and prerequisites

The editor must be the full CarlaUnreal editor with the map loaded, e.g.:

```bash
setsid "$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor" \
  "$CARLA_UE58_ROOT/Unreal/CarlaUnreal/CarlaUnreal.uproject" /Game/Carla/Maps/Town15/Town15 -vulkan -log &
bash scripts/check_env.sh
```

Any of the towns above works (`/Game/Carla/Maps/Town07_Opt`, ...). Town15
takes a few minutes to load. The MCP port opens early but does not
answer until the map is in: `check_env.sh` says "still loading" then; wait.

### Step 2: Dry run

```bash
python3 scripts/restyle_signs.py --style MUTCD --dry-run
```

It returns `changed` (sign -> catalog entry, with counts), `unchanged` (data
assets with no equivalent, by name) and `errors`. On Town15, MUTCD changes
about 250 of 415 signs: pedestrian crossings, humps, roundabouts, speed limits,
stops, direction arrows. Give-way signs, parking (MUTCD only), the campus
direction boards and the multi-symbol prohibition plates stay as they are.
On Town07, Vienna changes all 95 signs; GB changes 87 (GB has no one-way sign,
those stay MUTCD).

### Step 3: Apply

```bash
python3 scripts/restyle_signs.py --style MUTCD     # or VC, GB
```

Limit it to an area with `--bounds minx,miny,minz,maxx,maxy,maxz` (cm, world).
A full Town15 pass takes about a minute; the editor is busy meanwhile.

An agent with its own MCP connection to `unreal-mcp` can instead print the
sandbox script with `--print` and send it as
`call_tool(toolset_name="editor_toolset.toolsets.programmatic.ProgrammaticToolset",
tool_name="execute_tool_script", arguments={"script": ...})`. Same result.

### Step 4: Back, and the save warning

```bash
python3 scripts/restyle_signs.py --style original   # any map
```

Each switched sign keeps its original data asset and size in tags
(`SignStyleSwap:orig=...`, `SignStyleSwap:size=...`). Asking for the map's own
style (VC on Town15, MUTCD on the classic towns) or for `original` restores
exactly what the map had, from any style, and then removes the tags. A sign
with no equivalent in the requested style also goes back to its own plate,
whatever an earlier switch left on it. Tell the user: **do not save the level** (or answer "Don't Save" when the
editor asks on close). Saving would write the tags and the references to the
in-memory data assets into the map's actor files.

## Gotchas

- **Changing the plate component does nothing.** `BP_Sign` rebuilds its plate in
  the construction script from `signAssetData` on every property change; a
  mesh or material set on the component is overwritten at once. The skill
  therefore changes `signAssetData`, to a data asset made for the target sign.
- **A second `DataAssetTools.create` on the same path kills the first asset**,
  and every sign pointing at it ends up "pending kill". The script checks
  `AssetTools.exists` first and names each asset after its catalog entry.
- **A `BP_Sign` with no pole data asset aborts the whole call.** Its
  construction script raises "Accessed None ... PoleAssetData", which ends the
  sandbox script, not just that sign. The script skips such signs and lists them
  under `errors` (Town15 has one, `BP_Sign_C_327`).
- **Give-way needs the fixed triangle.** `SM_InvertedTriangleShape` (in
  `/Game/Carla` and in the CarlaDigitalTwinsTool content) used to have its UVs
  turned 120 degrees: invisible on the symmetric Vienna triangle, but "YIELD" and
  让 came out turned. With an older content checkout, leave `give_way` out.
- **MUTCD plates are fitted to their pictures.** The catalog's `SM_VertRect_10`
  and `SM_VertRect_03` map the whole atlas cell, so a speed limit showed white
  bands at its sides and a one-way sign was cut at the top. The Content data
  assets use `SM_VertRect_MUTCD_SpeedLimit`, `_OneWay` and `_LaneControl`,
  whose painted face maps exactly the picture and keeps its proportions.
- **Plate size.** The catalog meshes are not all drawn at road scale (a MUTCD
  `SM_VertRect_10` is 32x40 cm, a Vienna `SM_CircleShape` 81 cm). The script
  scales the actor's `size` by the ratio of the two plates' largest side (x0.95
  for rectangles, x1.3 for diamonds, tuned by eye on Town15) and keeps the
  original `size` in a tag for the way back.
- **Speed limits.** MUTCD gets the nearest mph sign (30 km/h -> 20 mph, 40 ->
  20, 60 -> 35, 80 -> 50; there is no 25 mph sign in the catalog). GB keeps
  km/h. A MUTCD plate does not say which km/h it stands for (30 and 40 km/h are
  both 20 mph), so on the classic towns the script reads the limit of the sign
  actor underneath and Vienna/GB show the road's real limit. The OpenDRIVE
  speed of the road does not change: the Traffic Manager drives the same.
- **Only the look changes.** Signal ids, trigger boxes and what the server reads
  from OpenDRIVE stay the same, and the change never reaches a packaged build.
- **Moving a `BP_Sign` with `set_actor_transform` leaves the plate behind**; it
  is placed in world space by the construction script. Set any property after
  moving it (e.g. `size`) to rebuild. Adding a sign for a demo: see
  references/signs.md.
- **Saving.** If the user wants to keep the level (e.g. with demo signs), save
  it only in its own style: a level saved in another style points at `/Game/SignStyleSwap`
  assets that exist only in memory. `SceneTools.save_actor` cannot save an actor
  added this session ("Asset does not exist" for its new external package): let
  the user press Ctrl+S in the editor, and uncheck anything under
  `/Game/SignStyleSwap` if the save dialog lists it.
- **The editor shows the editor view.** For a clean picture press `G` (game
  view) in the viewport: the road splines and actor icons disappear.

## Reference

`references/signs.md` — the full equivalence table, how the matching works, how
to regenerate it (`scripts/build_table.py`) and how to add demo signs.
