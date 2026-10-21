# Custom maps

A map is the 3D model of a town plus the definition of its road network, written as [__ASAM OpenDRIVE__](https://www.asam.net/standards/detail/opendrive/). CARLA ships several; a map of your own reaches the simulator by one of five routes.

| Route | What it is for | Needs a source build | Where |
| --- | --- | --- | --- |
| __Import an FBX + OpenDRIVE__ | Bringing a RoadRunner export into a source build and decorating it in the editor | yes | [Content authoring - maps](content_authoring_maps.md#import-the-road-network-into-carla); `Util/Tools/Import.py` and the `import-carla-ue58-map` [agent skill](agent_skills.md) |
| __Content pack__ | Shipping a finished map that drops into any CARLA package of the same release | to build the pack, not to use it | [Content packs](content_authoring_packs.md) |
| __Digital Twin Tool__ | Generating a decorated town (roads, buildings, vegetation) from an OpenStreetMap region in the editor; not usable end to end in this release | yes | [Digital Twin Tool](content_authoring_maps_digital_twin.md) |
| __OpenDRIVE standalone mode__ | Driving on a road network with no scenery, generated at runtime from an `.xodr` | no | [OpenDRIVE standalone mode](adv_opendrive.md) |
| __OpenStreetMap__ | Turning a real place into a drivable road network with no scenery; not available in this release (the `osm2odr` converter is not in the repository) | — | [Generate maps with OpenStreetMap](content_authoring_maps_openstreetmap.md) |

A map with scenery (a `.umap`, from an import or a content pack) needs three sidecar files beside it: the `.xodr` road network, the `.bin` pedestrian navigation mesh and, optionally, the Traffic Manager data. [Content packs — sidecar files](content_authoring_packs.md#sidecar-files) explains how to produce them, including the `RecastBuilder` and `FBX2OBJ` binaries that, when missing, silently leave a map with no navigation.

__Create a map in the editor__

- [__Content authoring - maps__](content_authoring_maps.md) — road network in RoadRunner, import into CARLA, then assets, traffic lights, signs, materials, road painter and vegetation
- [__RoadRunner__](roadrunner.md) — building the road network itself, and the export settings CARLA expects
- [__Digital Twin Tool__](content_authoring_maps_digital_twin.md) — procedural town from OpenStreetMap, and its traffic light, sign and vegetation tools

__Ship a map as a content pack__

- [__Content packs — overview__](content_authoring_packs.md)
- [__Author a map in the editor__](content_authoring_packs.md#2-author-in-the-editor)
- [__Add the map, OpenDRIVE and navigation files to a pack__](content_authoring_packs.md#3-add-maps-sidecar-files-and-catalogs)
- [__Build the pack against a release__](content_authoring_packs.md#4-build-the-pack-against-a-release)
- [__Install the pack and load the map__](content_authoring_packs.md#installing-a-pack)
- [__Towns available as content packs__](map_content_packs.md) — the CARLA towns the package does not ship

__Road networks with no scenery__

- [__OpenDRIVE standalone mode__](adv_opendrive.md)
- [__Generate maps with OpenStreetMap__](content_authoring_maps_openstreetmap.md)

__Use a map__

- [__Maps and navigation__](core_map.md) — loading maps, World Partition and converting legacy tiled towns
