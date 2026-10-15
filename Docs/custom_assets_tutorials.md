# Custom assets

CARLA ships whole towns, vehicles, pedestrians and props, and it loads assets of your own alongside them. These are the procedures for getting one in.

| Page | What it covers |
| --- | --- |
| [__Adding props__](content_authoring_props.md) | Import a mesh and register it as a spawnable `static.prop.*`, or place one straight from a mesh path with `static.prop.mesh`. |
| [__Adding vehicles__](tuto_content_authoring_vehicles.md) | Model, rig and import a vehicle, set up its physics asset, animation and wheel blueprints, and register it in the blueprint library. |
| [__Vehicle materials__](tuto_content_authoring_vehicles_materials.md) | Instance the car-paint master material and tune its color, clear coat, flakes and dust. |
| [__Material customization__](tuto_A_material_customization.md) | The building and environment master materials, and the textures they read. |
| [__Create semantic tags__](tuto_D_create_semantic_tags.md) | Define a new semantic label so your assets are segmented as their own class. |
| [__Generate detailed colliders__](tuto_D_generate_colliders.md) | Build accurate collision meshes for a vehicle, for physics or for ray-cast sensors. |
| [__Customize vehicle suspension__](tuto_D_customize_vehicle_suspension.md) | Tune the Chaos suspension of a vehicle's wheels. |
| [__Content packs__](content_packs.md) | Ship maps, props, vehicles and walkers as a package that drops into any CARLA of the same release. |

!!! Note
    Pedestrian import has no page yet. The vetted procedure is in the `import-carla-ue58-walker` [agent skill](agent_skills.md), which covers binding the FBX to CARLA's shared pedestrian skeleton and registering the walker.

Maps are their own subject: see [Generate maps with OpenStreetMap](tuto_G_openstreetmap.md) for an OSM import and [content packs](content_packs.md) for shipping a map you have made.
