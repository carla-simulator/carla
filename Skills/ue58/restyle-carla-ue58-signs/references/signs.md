# Regional signs: matching, equivalences, demo signs

## How a sign is recognised

A `BP_Sign` (`/Game/Carla/Blueprints/LevelDesign/Signs/BP_Sign`) draws its plate
from a `BDA_TrafficSign_Base` data asset: a plate mesh, an atlas texture and a
cell (`id_X`, `id_Y`, 1-based, column then row). The regional catalog of the
CarlaDigitalTwinsTool content (`/CarlaDigitalTwinsTool/Carla/Blueprints/LevelDesign/Signs/Catalog/<style>/<category>/DA_*`,
class `USignDataAsset`) describes every sign with the same three fields plus a
`SignName`.

The atlases of the map and of the catalog have the same names and cells, so the
atlas texture name and cell of a map data asset identify the catalog entry it
paints, in any style: `T_VC_PrioritySignAtlas_01:4:2` and
`T_MUTCD_PrioritySignAtlas_01:3:1` are both `stop`. The atlas prefix (`T_VC_`,
`T_MUTCD_`, `T_GB_`) also tells the map's own style, which is the one that
restores. Names of the map's data
assets (`DA_SignalSquareSigns_VC`, `DA_WarningPSigns_VC_03`) say little; the cell
says what is painted. Data assets marked `unique` (own texture, such as the
campus direction boards) are never matched.

`references/sign_equivalences.json`:

- `cells`: `"<atlas>:<id_X>:<id_Y>"` of any style -> Vienna `SignName` (the
  canonical name). A MUTCD speed limit maps to `mph:<n>`: the plate does not say
  which km/h it stands for, so the script reads the km/h of the classic sign
  actor under it (tag `MUTCDOverlayOf:<actor>`), or converts the mph when there
  is none.
- `targets.<STYLE>`: Vienna `SignName` -> asset of the same rule in that style:
  the data asset under `/Game/Carla/Blueprints/LevelDesign/Signs/DataAssets/<STYLE>/<Category>`
  (all of them exist; the classic towns use the MUTCD ones), so no style needs
  the plugin. A catalog entry with no Content data asset would be built in memory.

## Equivalences

| Vienna sign | MUTCD | GB |
|---|---|---|
| `cyclists` | `DA_bicycle_crossing_W11_1` | `DA_bicycle_crossing` |
| `give_way` | `DA_yield_R1_2` | `DA_give_way` |
| `max_speed_5` ... `max_speed_130` | `DA_max_speed_<mph>_R2_1`, nearest mph (30 -> 20, 40 -> 20, 50 -> 30, 60 -> 35, 80 -> 50, 90 -> 55, 100 -> 60, 120 -> 75) | same km/h |
| `no_entry` | `DA_do_not_enter_R5_1` | `DA_no_entry` |
| `no_left_turn`, `no_right_turn` | `DA_no_left_turn_R3_2`, `DA_no_right_turn_R3_1` | `DA_no_left_turn`, `DA_no_right_turn` |
| `no_stopping` | `DA_no_stopping_any_time_R7_4` | `DA_no_stopping` |
| `one_way_left`, `one_way_right` | `DA_one_way_left_R6_2`, `DA_one_way_right_R6_2` | (none, stays as it is) |
| `parking` | (none, stays Vienna) | `DA_parking` |
| `pedestrian_crossing` | `DA_pedestrian_crossing_warning_W11_2` | `DA_pedestrian_crossing` |
| `pedestrian_crossing_ahead` | `DA_pedestrian_crossing_warning_W11_2` | `DA_pedestrian_crossing` |
| `road_hump` | `DA_bump_W8_1` | `DA_road_hump` |
| `road_narrows_left`, `road_narrows_right` | `DA_lane_ends_left_W4_2L`, `DA_lane_ends_right_W4_2R` ("left lane ends, merge right") | `DA_road_narrows_left`, `DA_road_narrows_right` |
| `roundabout_mandatory`, `roundabout_mandatory_clockwise` | `DA_roundabout_circulation_R6_5P` | `DA_roundabout` |
| `stop` | `DA_stop_R1_1` | `DA_stop` |
| `straight_ahead_mandatory` | `DA_straight_only_R3_5a` | `DA_straight_ahead` |
| `turn_right_here_mandatory` | `DA_turn_right_only_R3_5` | `DA_turn_right` |
| `wild_animals` | `DA_deer_crossing_W11_3` | `DA_wild_animals` |

Picked by meaning. The OpenDRIVE codes in the catalog are no help here: `101`
covers every generic danger sign.

Not switched: anything the catalog has no counterpart for (shared cycle and pedestrian path, merging with
priority, the four-symbol prohibition plates, Town15's campus boards).

To add an equivalence, add it to `EQUIVALENT` in `scripts/build_table.py` and
run it with the editor open. It reads the catalog through unreal-mcp and
rewrites the JSON.

## Numbers

Town15 (415 `BP_Sign`, with the two demo signs below): MUTCD switches 315, GB
341, `original` restores all of them. Unchanged in MUTCD: 45 four-symbol
prohibition, 26 parking, 11 cycle-and-pedestrian, and 21 others (campus boards,
arrows, bifurcation, merging).

Classic towns (MUTCD overlays): Town01 21, Town02 21, Town03 11, Town04 61,
Town05 25, Town06 95, Town07 95, Town10HD 5. Town07: Vienna switches 95, GB 87
(the 8 one-way signs stay MUTCD).

## Adding a sign for a demo

A street with only give-way signs shows nothing. Adding a couple of signs that
fit the place (a 30 km/h limit and a "pedestrian crossing ahead" before a
crossing) is fine for a demo, as long as the level is not saved.

1. `SceneTools.add_to_scene_from_class` with `actor_type`
   `/Game/Carla/Blueprints/LevelDesign/Signs/BP_Sign.BP_Sign_C`, a `name`, the
   transform, `parent: null`, `snap_to_ground: false`. The call reports errors
   ("Accessed None ... PoleAssetData"): a bare `BP_Sign` has no data assets
   yet. **The actor is created anyway**; find it by diffing
   `find_actors(actor_type=BP_Sign)` before and after.
2. `ObjectTools.set_properties` on it:
   `{"poleAssetData": {"refPath": ".../DataAssets_Pole/DA_Pole03.DA_Pole03"}, "signAssetData": {"refPath": <a Vienna data asset already in the map>}, "size": 1, "height": 1, "offset": {"x": 0, "y": 2, "z": 0}}`.
   The `offset` puts the plate 2 cm in front of the pole, as on every Town15
   sign; left at 0 the pole shows through the middle of the plate.
3. `ActorTools.set_label` and `add_tag` (e.g. `SignStyleSwapDemo`) so it can be
   found and removed with `SceneTools.remove_from_scene`.

Place it on the sidewalk next to an existing sign of the same street: copy that
sign's lateral position, move along the road direction, take z from
`SceneTools.trace_world` straight down. The front of the plate faces actor yaw
+ 90 degrees, towards the traffic it is meant for. After `set_actor_transform`
set any property (e.g. `size`) again: the plate is placed in world space by the
construction script and stays behind otherwise.

Town15 demo used for the CARLA 1.0 talk: the approach to the UAB entrance
roundabout (monolith). Reference board `BP_Sign99` "Vila Universitaria" at
(87780, 18787); road direction (0.503, 0.865); 30 km/h at -900 cm and
pedestrian-crossing-ahead at +700 cm along the road from it, yaw 150. Camera
(86861, 16312, 13420) looking at (88815, 20269, 13330).
