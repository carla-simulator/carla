# Custom maps

In CARLA, a map includes the 3D model of a town and a definition of its road network. The road network is defined through the [__OpenDRIVE__](https://www.asam.net/standards/detail/opendrive/) standard. CARLA provides a diverse array of maps out of the box ready to use for a multitude of applications. Users can also create their own maps and load them into CARLA. In CARLA UE5 a custom map is shipped as a __content pack__: it is authored in the CARLA editor, built once against a CARLA release with the `carla-pack` tool and then dropped into any CARLA package of that release, without re-packaging CARLA. The following pages detail the necessary steps for creating and loading custom maps into CARLA.

* [__Content packs — overview__](content_packs.md)
* [__Author a map in the editor__](content_packs.md#2-author-the-content-in-the-editor)
* [__Add the map, OpenDRIVE and navigation files to a pack__](content_packs.md#3-add-maps-catalogs-and-assets)
* [__Build the pack against a release__](content_packs.md#build-against-a-release)
* [__Install the pack and load the map__](content_packs.md#install-a-pack)
* [__Generate a map from OpenStreetMap__](tuto_G_openstreetmap.md)
* [__Maps and navigation__](core_map.md) — loading maps, World Partition and converting legacy tiled towns
