# Custom assets

CARLA has a wealth of assets available out of the box including full towns and cities with road networks, buildings and infrastructure, vehicles and pedestrians to populate your simulations. However, for many applications, you may want to add your own assets and CARLA is fully capable of loading new assets created entirely by the user for maximum extensability. 

The following documentation details numerous techniques for creating your own assets and adding them to CARLA.

- [__Adding props__](tuto_A_add_props.md)
- [__Adding vehicles__](tuto_A_add_vehicle.md)
- [__Packaging assets__](tuto_A_create_standalone.md) 
- [__Material customisation__](tuto_A_material_customization.md)

## [Adding props](tuto_A_add_props.md)

Props are the assets populating the scene, other than the roads and vehicles. That includes streetlights, buildings, trees, and much more. The simulator can ingest new props anytime in a simple process. This is really useful to create customized environments in a map. [__This document__](tuto_A_add_props.md) demonstrates how to create and include custom props. 

## Adding vehicles

Vehicles are the bread and butter of CARLA. They serve to simulate other road users and act as a virtual emulation of the vehicle that an autonomous agent is built to control. CARLA has a large, growing library of vehicles out of the box, but for specialised applications, CARLA is capable of loading custom designed vehicles. [__This document__](tuto_A_add_vehicle.md) details how to create and import custom vehicles.

## Packaging assets

It is a common practice in CARLA to manage assets with standalone packages. Keeping them aside allows to reduce the size of the build. These asset packages can be easily imported into a CARLA package anytime. They also become really useful to easily distribute assets in an organized way. [__This document__](tuto_A_create_standalone.md) demonstrates how to package assets for use in CARLA.

## Custom materials

The CARLA team prepares every asset to run under certain default settings. However, users that work in a build from source can modify these to best suit their needs. [__This document__](tuto_A_material_customization.md) demonstrates how to achieve this. 