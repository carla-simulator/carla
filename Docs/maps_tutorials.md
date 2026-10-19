# Custom maps

A map is the 3D model of a town plus the definition of its road network, written as [__ASAM OpenDRIVE__](https://www.asam.net/standards/detail/opendrive/). CARLA ships several; a map of your own reaches the simulator by one of five routes.

| Route | What it is for | Needs a source build | Where |
| --- | --- | --- | --- |
| __Import an FBX + OpenDRIVE__ | Bringing a RoadRunner export into a source build and decorating it in the editor | yes | [Content authoring - maps](tuto_content_authoring_maps.md#importing-your-road-network-into-carla); `Util/Tools/Import.py` and the `import-carla-ue58-map` [agent skill](agent_skills.md) |
| __Content pack__ | Shipping a finished map that drops into any CARLA package of the same release | to build the pack, not to use it | [Content packs](content_packs.md) |
| __Digital Twin Tool__ | Generating a decorated town (roads, buildings, vegetation) from an OpenStreetMap region in the editor; experimental | yes | [Digital Twin Tool](adv_digital_twin.md) |
| __OpenDRIVE standalone mode__ | Driving on a road network with no scenery, generated at runtime from an `.xodr` | no | [OpenDRIVE standalone mode](adv_opendrive.md) |
| __OpenStreetMap__ | Turning a real place into a drivable road network with no scenery | yes: the Python API built with `-DENABLE_OSM2ODR=ON` | [Generate maps with OpenStreetMap](tuto_G_openstreetmap.md) |

A map with scenery (a `.umap`, from an import, a content pack or the Digital Twin Tool) needs three sidecar files beside it: the `.xodr` road network, the `.bin` pedestrian navigation mesh and, optionally, the Traffic Manager data. [Content packs — sidecar files](content_packs.md#sidecar-files) explains how to produce them, including the `RecastBuilder` and `FBX2OBJ` binaries that, when missing, silently leave a map with no navigation.

__Create a map in the editor__

- [__Content authoring - maps__](tuto_content_authoring_maps.md) — road network in RoadRunner, import into CARLA, then assets, traffic lights, signs, materials, road painter and vegetation
- [__Create a large map in RoadRunner__](large_map_roadrunner.md)
- [__Digital Twin Tool__](adv_digital_twin.md) — procedural town from OpenStreetMap

__Ship a map as a content pack__

- [__Content packs — overview__](content_packs.md)
- [__Author a map in the editor__](content_packs.md#2-author-in-the-editor)
- [__Add the map, OpenDRIVE and navigation files to a pack__](content_packs.md#3-add-maps-sidecar-files-and-catalogs)
- [__Build the pack against a release__](content_packs.md#4-build-the-pack-against-a-release)
- [__Install the pack and load the map__](content_packs.md#installing-a-pack)
- [__Towns available as content packs__](map_content_packs.md) — the CARLA towns the package does not ship

__Road networks with no scenery__

- [__OpenDRIVE standalone mode__](adv_opendrive.md)
- [__Generate maps with OpenStreetMap__](tuto_G_openstreetmap.md)

__Use a map__

- [__Maps and navigation__](core_map.md) — loading maps, World Partition and converting legacy tiled towns
