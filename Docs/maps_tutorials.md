# Custom maps

A map is the 3D model of a town plus the definition of its road network, written as [__ASAM OpenDRIVE__](https://www.asam.net/standards/detail/opendrive/). CARLA ships several; a map of your own reaches the simulator by one of four routes.

| Route | What it is for | Needs a source build | Where |
| --- | --- | --- | --- |
| __Content pack__ | Shipping a finished map that drops into any CARLA package of the same release | to build the pack, not to use it | [Content packs](content_packs.md) |
| __Import an FBX + OpenDRIVE__ | Bringing a RoadRunner export into a source build as a cooked map | yes | `Util/Tools/Import.py`, and the `import-carla-ue58-map` [agent skill](agent_skills.md) |
| __OpenDRIVE standalone mode__ | Driving on a road network with no scenery, generated at runtime from an `.xodr` | no | [OpenDRIVE standalone mode](adv_opendrive.md) |
| __OpenStreetMap__ | Turning a real place into a drivable road network | no | [Generate maps with OpenStreetMap](tuto_G_openstreetmap.md) |

Whichever route, a finished map needs three sidecar files beside the `.umap`: the `.xodr` road network, the `.bin` pedestrian navigation mesh and, optionally, the Traffic Manager data. [Content packs — sidecar files](content_packs.md#sidecar-files) explains how to produce them, including the `RecastBuilder` trap that silently leaves a map with no navigation.

!!! Note
    Authoring a map inside the CARLA editor has no page of its own yet. The closest guide is the content pack walkthrough, which covers duplicating an existing town into a pack and adding a map of your own.

- [__Content packs — overview__](content_packs.md)
- [__Author a map in the editor__](content_packs.md#2-author-in-the-editor)
- [__Add the map, OpenDRIVE and navigation files to a pack__](content_packs.md#3-add)
- [__Build the pack against a release__](content_packs.md#4-build)
- [__Install the pack and load the map__](content_packs.md#installing-a-pack)
- [__Maps and navigation__](core_map.md) — loading maps, World Partition and converting legacy tiled towns
