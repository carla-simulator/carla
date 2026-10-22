# Custom assets

CARLA ships whole towns, vehicles, pedestrians and props, and it loads assets of your own alongside them. This section covers how to make each kind and how to ship it; this page is the map of it.

Authoring happens in the Unreal Editor, so all of it needs a [source build](build_carla.md). The one exception is a [content pack](content_authoring_packs.md), which anyone can install into a CARLA package.

- [__Maps__](#maps)
- [__Vehicles__](#vehicles)
- [__Props and materials__](#props-and-materials)
- [__Shipping what you made__](#shipping-what-you-made)
- [__See also__](#see-also)

---
## Maps

A map is the 3D model of a town plus its road network. There are several ways to get one in, and [Custom maps](content_authoring_maps_overview.md) compares them before you commit to one.

| Page | What it covers |
| --- | --- |
| [__Custom maps__](content_authoring_maps_overview.md) | The routes a map can take into CARLA, what each needs, and the sidecar files a map cannot run without. |
| [__Create a map__](content_authoring_maps.md) | The full walkthrough: road network in [RoadRunner](roadrunner.md), import into CARLA, then assets, traffic lights, signs, materials, the road painter and vegetation. |
| [__OpenStreetMap__](content_authoring_maps_openstreetmap.md) | Turn a region of the real world into a drivable road network, with no scenery around it. |
| [__Digital Twin Tool__](content_authoring_maps_digital_twin.md) | Generate a decorated town from an OpenStreetMap region, inside the editor. |

---
## Vehicles

Two steps make a vehicle, in this order, and two more refine it.

| Page | What it covers |
| --- | --- |
| [__Create a vehicle__](content_authoring_vehicles.md) | Model, rig and import a vehicle, set up its physics asset, animation and wheel blueprints, wire its lights, and register it in the blueprint library. |
| [__Vehicle materials__](content_authoring_vehicles_materials.md) | Instance the car-paint master material and tune its colour, clear coat, flakes and dirt. A vehicle is not finished without it. |
| [__Collision meshes__](content_authoring_vehicles_colliders.md) | Build collision boundaries that follow the shape of the vehicle, for physics or for ray-cast sensors. |
| [__Suspension__](content_authoring_vehicles_suspension.md) | Set the Chaos suspension of each wheel in its wheel blueprint. |

---
## Props and materials

| Page | What it covers |
| --- | --- |
| [__Props__](content_authoring_props.md) | Import a mesh and register it as a spawnable `static.prop.*`, or place one straight from a mesh path with `static.prop.mesh`. |
| [__Material customization__](content_authoring_materials.md) | The master materials the static assets of a town are built on, the textures they read, and the parameters to change on an instance. |

---
## Shipping what you made

[__Content packs__](content_authoring_packs.md) bundle maps, props, vehicles and walkers into a package that drops into any CARLA of the same release, with no rebuild on the other end.

---
## See also

* [__Create semantic tags__](core_sensors_semantic_tags.md) — give your assets a semantic class of their own, so the segmentation sensors label them separately.
* [__Catalogue__](catalogue.md) — what CARLA already ships, before you make something new.

!!! Note
    Pedestrians can be imported as well, binding the model to CARLA's shared pedestrian skeleton and registering it as a `walker.pedestrian.*` blueprint. That procedure is not documented yet.
