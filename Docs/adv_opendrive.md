# ASAM OpenDRIVE standalone mode

CARLA can run a full simulation from an ASAM OpenDRIVE file alone, with no 3D assets of your own. The simulator parses the `.xodr`, builds a drivable world around its road definition, and populates it with the traffic lights, signs and pedestrian navigation the file describes. Any map you have as OpenDRIVE can therefore be driven in CARLA in a few seconds.

- [__Load a map__](#load-a-map)
- [__Generation parameters__](#generation-parameters)
- [__What is generated__](#what-is-generated)
- [__Console controls__](#console-controls)
- [__Mesh generation__](#mesh-generation)
- [__Limitations__](#limitations)

---

## Load a map

[`Client.generate_opendrive_world()`](python_api.md#carla.Client.generate_opendrive_world) takes the __contents__ of the file, not its path, and blocks until the world is ready.

```py
with open('/path/to/map.xodr') as f:
    client.generate_opendrive_world(f.read())
```

`PythonAPI/util/config.py` does the same from a shell, and takes a __path__:

```sh
python3 config.py -x /path/to/map.xodr
```

!!! Note
    If the server answers `opendrive could not be correctly parsed`, check that it can write to `CarlaUnreal/Content/Carla/Maps/OpenDrive/`. The server needs that directory to parse the file.

---

## Generation parameters

The second argument of `generate_opendrive_world()` is a [`carla.OpendriveGenerationParameters`](python_api.md#carla.OpendriveGenerationParameters). It is optional, and `config.py` always uses the defaults.

| Parameter | Default | What it does |
| --------- | ------- | ------------ |
| `vertex_distance` | 2.0 m | Spacing of the mesh vertices. Larger is coarser; too small makes the mesh heavy to work with. |
| `max_road_length` | 50.0 m | Length of each mesh chunk. The world is split so the engine can cull what is off screen; very small chunks trade culling for object count. |
| `wall_height` | 1.0 m | Height of the walls raised at the outer boundary of the road, which stop a vehicle driving off into the void. |
| `additional_width` | 0.6 m | Widening applied to junction lanes, 0.3 m per side, as a margin against falling off. |
| `smooth_junctions` | True | Reinterprets the OpenDRIVE where lanes meet at different heights, so a junction does not self-occlude. False builds the junction exactly as written. |
| `enable_mesh_visibility` | True | False keeps the road collidable but stops rendering it, which saves most of the rendering cost. |
| `enable_pedestrian_navigation` | True | Builds the navigation mesh so walkers can use the sidewalks and crosswalks. |

```py
params = carla.OpendriveGenerationParameters(
    vertex_distance=2.0,
    wall_height=1.0,
    smooth_junctions=True)

client.generate_opendrive_world(xodr, params)
```

---

## What is generated

The world is no longer just a road surface in the void. From the same file CARLA builds:

- __The road mesh__, lane by lane, including the elevation and the `lateralProfile` of each road: both `superelevation` and `shape` are applied, so cambered and crowned roads are reproduced rather than flattened.
- __A ground plane__ around the road, following its elevation and skirted outwards past the road bounds, so the horizon is terrain instead of void.
- __Road markings and crosswalks__, drawn as decals from the OpenDRIVE lane markings, including markings through junctions.
- __Traffic lights, stop and yield signs__, placed from the signals in the file and wired into junction controllers.
- __Street furniture__ — lamp posts, vegetation and signage — scattered along the roadside with PCG.
- __Pedestrian navigation__, over the sidewalks and crosswalks the file defines.

Everything comes from the `.xodr`, so an error in the file becomes an error in the simulation. Junctions are the usual source, since that is where many lanes meet.

![opendrive_standalone](img/adv_opendrive/opendrive_town01_overview.webp)

---

## Console controls

| Command | Effect |
| ------- | ------ |
| `carla.OpenDrive.StreetFurniture 0` | Generate the bare road, with no lamps, vegetation or signage. `1` is the default. |
| `carla.MapGenQA` | Re-run the generated-map checks — road support, terrain invading the road, objects left on the roadway — and log what they find. They also run automatically after generation. |

A client can add its own geometry to a generated world with the `spawn_custom_mesh` RPC, which takes vertices, triangles and a material.

---

## Mesh generation

The mesh is the whole feature: the simulation is only as good as the surface it is built on.

It is produced in chunks of `max_road_length` rather than as one object, so the engine can cull what is not visible, large maps stay manageable, and a defect in one part of the road stays in that part.

Junctions get special treatment. Where lanes meet at different heights, the raw definition makes the higher lane cut through the lower one, and a vehicle crossing the junction climbs or clips. `smooth_junctions` reinterprets those heights so the surfaces join:

![opendrive_meshissue](img/adv_opendrive/opendrive_meshissue.jpg)
<div style="text-align: right"><i>Higher lanes blocking the ones below them at a junction. <code>smooth_junctions</code> prevents this.</i></div>

Junction lanes are also widened by `additional_width`, and the outer boundary of the road is walled to `wall_height`, both to keep vehicles on the surface where the definition is least precise.

---

## Limitations

- __Sidewalk height is fixed.__ Sidewalks must stand above the road for collisions to register, and RoadRunner does not export the height, so CARLA applies one value everywhere.
- __Junction smoothing changes the geometry.__ It is on by default because it prevents the defect above, but the result is not a literal reading of the file. Turn it off when the mesh has to match the definition exactly.
- __The file is the source of truth.__ Overlapping lanes, inconsistent elevations or malformed junctions are reproduced faithfully. Validate the `.xodr` before blaming the simulation.
