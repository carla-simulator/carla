# Traffic Sign DataAsset Generation — Resource & Problem Report

Prepared for: a planning agent that will design an implementation plan (script/tool) to
bulk-generate traffic sign `DataAsset`s in the CARLA Unreal project. This document is
inventory + problem statement only — no implementation has started, no plan is proposed here.

Project root: `/home/adas/CarlaUE5/Unreal/CarlaUnreal/`
All paths below are relative to `Content/Carla/` unless given as full `/Game/...` package paths.
Note: `Content/` is gitignored in this checkout — no git history is available for any asset
mentioned here; everything was inventoried by inspecting the live `.uasset` binaries.

## 1. Goal

Generate one `DataAsset` (subclass of `BDA_TrafficSign_Base`) per valid sign image found in a
set of texture atlases, fully configured (shape mesh, style/convention, category, diffuse
texture reference, normal texture reference, atlas cell coordinates), and organize all
generated assets into a clean, predictable folder structure under
`Content/Carla/Blueprints/LevelDesign/Signs/DataAssets/` (exact target subtree TBD by the
planning agent — see "Desired output structure" below), so that every real-world traffic sign
the project has artwork for becomes a placeable, correctly-configured `BP_Sign` entry.

This is data/content generation work (creating and populating `.uasset` DataAsset files, most
likely via UE5 Python editor scripting), not gameplay/C++ logic.

## 2. Resources

### 2.1 Base class: `BDA_TrafficSign_Base`

Path: `Blueprints/LevelDesign/Signs/BDAs/BDA_TrafficSign_Base.uasset`
Parent: `PrimaryDataAsset`.

Confirmed fields (display name — internal var — type):

| Display name | Internal name | Type | Notes |
|---|---|---|---|
| Sign Mesh | `SignMesh` | `StaticMesh` ref | shape mesh, see 2.4 |
| Sign Style | `SignStyle` | enum `Enum_SignsStyle` | convention |
| Category | `Category` | enum `Enum_TrafficSignCategories` | sign category |
| Unique | `Unique` | bool | see discrepancy note below |
| Diffuse | `Diffuse` | `Texture2D` ref | atlas or unique texture |
| Normal | `Normal` | `Texture2D` ref | atlas or unique normal map |
| Id X | `Id_X` | int | atlas column |
| Id Y | `Id_Y` | int | atlas row |

**Discrepancy to verify**: the tooltip string embedded in the asset for `Diffuse`/`Normal`
reads *"In case of Unique variable being true, this need to be a 4x4 atlas in a 4k texture"*.
This reads backwards relative to the user's own description (atlases = shared/non-unique
categories, `Textures/VC/` one-off images = the actual "unique" signs). Confirm the real
semantics of the `Unique` flag in-editor (open an existing DA with `Unique` checked, e.g. one
under `SignsDA/Unique/`) before relying on it.

### 2.2 Subclasses of `BDA_TrafficSign_Base`

Path: `Blueprints/LevelDesign/Signs/BDAs/`. All 7 are **empty pass-through subclasses** — zero
extra exposed fields beyond the base class, used purely as a type tag:

`BDA_Prohibited`, `BDA_Warning`, `BDA_Yields`, `BDA_Signal`, `BDA_SquareSignal`,
`BDA_SpeedLimits`, `BDA_Unique` — each name maps 1:1 to a value of `Enum_TrafficSignTypes`
(2.3).

Separate, unrelated hierarchy (not signs, do not use): `BDA_Pole_Base` (native
`PrimaryDataAsset`, fields `Pole Mesh` + `Sign Style`) and `BDA_Poles` (extends
`BDA_Pole_Base`) — these are for poles, not sign faces.

### 2.3 Enums

`Blueprints/LevelDesign/Signs/Enums/`:

- **`Enum_SignsStyle`** (maps to `Sign Style` / convention): `None`, `Vienna Convention
  (Europe)`, `MUTCD (America)`, `GB (China)`.
  **Important**: the folder/atlas prefix `GB` does **not** mean Great Britain — per this enum
  it stands for **China**. Do not assume UK when generating GB-prefixed assets.
- **`Enum_TrafficSignCategories`** (maps to `Category`): `Priority`, `Mandatory`,
  `Prohibitory`, `Warning`, `Information`, `Guide`, `Others`, `SpeedLimit` (8 values; one
  internal enumerator slot appears to have been deleted at some point in editor history, but
  8 valid display values exist today).
- **`Enum_TrafficSignTypes`** (maps 1:1 to the `BDA_*` subclasses, 2.2): `Prohibited`,
  `Signal`, `Yield`, `Warning`, `SquareSignal`, `Unique`, `SpeedLimit`.

Note `Category` (8 values) and `Type`/subclass (7 values) are **two different taxonomies**
that must both be set correctly per generated DA — they are not interchangeable, see problem
in 5.3.

### 2.4 Shape meshes

Path: `Static/TrafficSign/Signs/` (39 files). Distinct shape families:

- `SM_CircleShape` (round — mandatory/prohibitory in Vienna Convention)
- `SM_DangerSignShape` (triangle — warning)
- `SM_InvertedTriangleShape` (yield)
- `SM_OctogonalShape` (stop)
- `SM_SquareShape`
- `SM_RomboidShape` (priority/diamond)
- `SM_ArrowShape`, `SM_DoubleSign`, `SM_LongThinShape`
- Rectangular/informational variants: `SM_HorRect_01`...`13`, `SM_VertRect_01`...`11`,
  `SM_SlightRectangleShape[Sided]`, `SM_LargeSlightRectangleShape[Sided]`,
  `SM_SmallRectangle`, `SM_VerticalRectangle`

There is **no direct, reliable mapping from atlas/category name to shape mesh** — this is
problem 4.1 below.

### 2.5 Texture atlases — location A (primary, per user's brief)

Path: `Static/Signs/DataAssetsTextures/SignsAtlases/`. **4 convention subfolders**, not 3 as
initially assumed — `GB`, `MUTCD`, `VC`, and `Miscellaneous`:

- `GB/` — 9 atlases (`T_GB_GuideSignAtlas_01`, `InformationSignAtlas_01`,
  `MandatorySignAtlas_01`, `PrioritySignAtlas_01`, `ProhibitorySignAtlas_01/02/03`,
  `WarningSignAtlas_01/02`)
- `MUTCD/` — 11 atlases (same category names as GB plus `ProhibitorySignAtlas_04/05`)
- `VC/` — 10 atlases (same category set as GB plus `WarningSignAtlas_03`)
- `Miscellaneous/` — 1 atlas (`T_MiscellaneousSignAtlas_01`)

Total: 31 texture files, **diffuse only — no `_N` normal maps exist anywhere in this
location**. Naming pattern: `T_<ConventionCode>_<CategoryEnumValue>SignAtlas_NN`, where
`<CategoryEnumValue>` matches `Enum_TrafficSignCategories` (Guide/Information/Mandatory/
Priority/Prohibitory/Warning) — not `Enum_TrafficSignTypes`.

All atlases are stated by the user to be laid out as a 4×4 grid (confirm exact 4x4-ness
programmatically per texture, do not assume).

### 2.6 Texture atlases — location B (possible duplicate / partial overlap)

Path: `Static/Signs/TextureAtlas/ViennaConvention/`. Single folder, 10 files = 5 atlases each
with a matching `_N` normal map:

`T_ProhibitedSignAtlas-01(_N)`, `T_SignalSignAtlas_01(_N)`, `T_SquareSignAtlas_01(_N)`,
`T_WarningSignAtlas_01(_N)`, `T_YieldSignAtlas-01(_N)`.

Comparison with location A:
- Category naming here (`Prohibited`, `Signal`, `Square`, `Warning`, `Yield`) matches
  `Enum_TrafficSignTypes` (the BDA subclass taxonomy), **not** `Enum_TrafficSignCategories`
  used in location A — different taxonomy per location.
- Only "Warning" exists as a name in both locations, under different naming schemes
  (`T_VC_WarningSignAtlas_01/02/03` in A vs. `T_WarningSignAtlas_01` in B) — not confirmed
  whether these are the same artwork or different sign sets; needs visual comparison.
- No exact filename overlaps between A and B.
- Inconsistent hyphen/underscore usage even within B itself (`-01` vs `_01`).
- **This location has real, prepared normal maps** (not the flat generic `T_Flat_n` used
  everywhere in `BDA_TrafficSign_Base` fields on existing DAs, see 2.8) but covers far fewer
  signs than location A. Deciding which location is authoritative for Vienna Convention is a
  decision the plan must make explicit (see problem 4.3 / 5.4).

### 2.7 Unique (non-atlas) sign textures

Path: `Static/Signs/DataAssetsTextures/Textures/VC/` — 10 diffuse + 10 matching `_N` normal
maps, one-off custom signage (e.g. `T_UABDirections`, `T_RoundAboutCerdanyola`,
`T_BenvigutsALaAutonoma`, `T_Arrow`, `T_DoubleSign`, etc). Note one filename casing
inconsistency: `t_DoubleArrow(_N)` uses lowercase `t_` unlike every other file in this folder.

### 2.8 Existing DataAssets (prior art — use with caution, see problem 5)

Path: `Blueprints/LevelDesign/Signs/DataAssets/`, 4 subfolders, **214 DAs total**:

- `SignsDA/` (31) — folder pattern `<Category>/<GB|VC>/DA_<DescriptiveName>` — mostly
  well-named (e.g. `Warning/GB/DA_BumpAheadSign`, the example the user gave). Contains the
  cleanest naming in the project.
- `SignsDataAssets/` (29) — folder-per-category, no convention subfolder, descriptive names
  — several filenames duplicate ones in `SignsDA/` (e.g. `DA_BenvigutsAutonoma`).
- `SignsNoNamed/` (150, ~70% of all existing DAs) — folder pattern
  `<Category>/<GB|VC>/DA_<sequential integer>` (e.g. `Prohibitory/VC/DA_34`...`DA_67`) — no
  descriptive names, looks like a bulk/placeholder import.
- `ViennaConvention/` (4) — further duplicates of names already present elsewhere.

**Data-quality warning**: cross-checking the actual `BDA_*` class each DA instantiates (not
just its folder name) shows the folder taxonomy is not reliable — e.g. **all 150 files under
`SignsNoNamed/`, regardless of which category subfolder they sit in, instantiate
`BDA_Warning_C`**; files under `SignsDA/Unique/`, `ViennaConvention/SquareSignal/` and
`ViennaConvention/Unique/` all instantiate `BDA_Prohibited_C` regardless of folder name; only
`SpeedLimit` DAs and 3 `Guide` DAs have a class that actually matches their folder/category.
**Do not treat the existing 214 DAs as a reliable ground-truth mapping of category → class —
most of them appear mis-classified or placeholder.** They are useful only as folder-structure/
naming precedent, not as a source of correct shape/category/class assignments.

## 3. Desired output structure (per user's brief)

A new/reorganized tree under `Blueprints/LevelDesign/Signs/DataAssets/`, ordered:
**convention → category → individual DA**, named `DA_<DescriptiveSignName>_<ConventionCode>`
(e.g. `DA_BumpAheadSign_GB`, mirroring the existing `SignsDA/Warning/GB/DA_BumpAheadSign`
style but with the convention suffix baked into the name as well as the folder path). Exact
final layout (whether to keep/merge/retire the 4 existing folders in 2.8) is left to the
planning agent, but it must resolve the duplication already present there rather than add a
5th parallel structure.

## 4. Problems already identified by the user

1. **Shape ambiguity**: from an atlas name alone you can sometimes infer the shape (e.g.
   Vienna Convention "Prohibitory" → round), but for other categories (e.g. "Priority") the
   atlas alone doesn't tell you whether a given cell is a stop (octagon), yield (inverted
   triangle) or priority-road (square/diamond) sign — shape must be resolved per-cell, not
   per-atlas.
2. **Incomplete atlas grids**: not every atlas is a fully populated 4×4 grid — some cells are
   blank placeholders. Blank cells must be detected and skipped, not turned into DAs.
3. **Possible duplicate source**: location B (2.6) may fully or partially duplicate location A
   (2.5) for Vienna Convention, but has fewer signs and (uniquely) real prepared normal maps
   instead of the flat default. Needs resolving which source is authoritative, or whether both
   need merging.

## 5. Additional problems found during inventory (not yet flagged by the user)

1. **`GB` does not mean Great Britain** — per `Enum_SignsStyle` it is "GB (China)". Any
   assumption about UK-style signage for `GB`-prefixed assets would be wrong.
2. **A 4th convention exists that wasn't mentioned**: `MUTCD` (America) has its own atlas set
   in location A, same size/category pattern as `GB`/`VC`. No `MUTCD` DataAsset folder exists
   yet under section 2.8 — likely in scope too, needs explicit confirmation from the user.
3. **Two incompatible category taxonomies are already in live use**: location A atlases and
   most of `SignsDA`/`SignsDataAssets`/`SignsNoNamed` use `Enum_TrafficSignCategories` names
   (Guide/Information/Mandatory/Priority/Prohibitory/Warning/SpeedLimit) as folder names, while
   location B atlases and the `BDA_*` subclass names use `Enum_TrafficSignTypes`
   (Prohibited/Signal/Yield/Warning/SquareSignal/Unique/SpeedLimit). Every generated DA needs
   **both** fields set correctly and independently — folder name alone (in either taxonomy)
   is not sufficient to derive the other.
4. **`Diffuse`/`Normal` tooltip contradicts the user's stated design** (see 2.1) — verify
   real intended semantics of the `Unique` bool before generating anything for the "unique
   textures" folder (2.7).
5. **Existing 214 DAs are largely mis-classified** (see 2.8 data-quality warning) — cannot be
   used as an automatic class-assignment reference; at best as folder-structure precedent.
6. **Naming inconsistencies in source textures** that a generation script must tolerate:
   hyphen vs. underscore before the atlas index (`-01` vs `_01`) in location B; lowercase
   `t_DoubleArrow` vs. `T_` elsewhere in 2.7.

## 6. Stated future use-cases (context only — explicitly NOT to be implemented now)

The user wants the eventual DA catalog to support, later and separately:
- Spawning signs into a map read from some external input (format undecided).
- Editing one placed sign's config via API (e.g. changing a speed-limit DA from 80 to 100).
- Swapping an entire map's signage from one convention to another while preserving the same
  semantic signs.

These should only inform how consistently/predictably the DA catalog is organized and named
(so it's queryable later by category+convention+meaning) — no spawning/API/conversion logic
should be designed or built as part of this task.

## 7. Open questions for the planning agent to resolve (or raise back to the user)

1. Is `MUTCD` in scope for this generation pass, or Vienna Convention (+GB) only for now?
2. Which atlas source is authoritative for Vienna Convention: location A (2.5, no normals,
   more signs) or location B (2.6, real normals, fewer signs) — or should both be ingested and
   merged, with B's normals overriding A's flat default where a cell exists in both?
3. What should happen to the existing 214 DAs (2.8) — retire/replace, or leave as-is and only
   add newly generated ones alongside (risking further duplication)?
4. How should per-cell shape (4.1) actually be resolved, given atlas name alone is
   insufficient — visual inspection per cell, a manually curated per-atlas cell→shape map, or
   something else? This is a design decision, not a data-availability question — the
   inventory above only establishes that no existing mapping data solves it automatically.
