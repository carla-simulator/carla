# Development

CARLA is open source and designed to be highly extensible. This allows users to create custom functionality or content to suit specialized applications or specific needs. The following tutorials detail how to achieve specific development aims with the CARLA codebase:

- [__Make release__](tuto_D_make_release.md)
- [__Updating the content__](tuto_D_contribute_assets.md)
- [__Create semantic tags__](tuto_D_create_semantic_tags.md)
- [__Create new sensor__](tuto_D_create_sensor.md)
- [__Running the tests__](testing.md)
- [__Recorder file format__](ref_recorder_binary_file_format.md)
- [__Collision boundaries__](content_authoring_vehicles_colliders.md)

## Make a release

If you want to develop your own fork of CARLA and publish releases of your code, follow [__this guide__](tuto_D_make_release.md).

## Updating the content

The maps, vehicles, walkers and props live in a Git LFS repository of their own, `carla-content`, with a content branch for each code branch. [__Updating the content__](tuto_D_contribute_assets.md) covers how a checkout gets it, how to update it, and how a content change is merged.

## Create semantic tags

CARLA has a set of semantic tags already defined suitable for most use cases. However, if you need additional classes you can add them as detailed in [__this guide__](tuto_D_create_semantic_tags.md).

## Creating a new sensor

You can modify CARLA's C++ code to create new sensors for your custom use cases. Please find the details [__here__](tuto_D_create_sensor.md).

## Running the tests

How to build and run the LibCarla unit tests and the Python unit, smoke and integration tests, and what CI runs, is described [__here__](testing.md).

## Recorder binary file format

Details on the binary file format for the recorder can be found [__here__](ref_recorder_binary_file_format.md).

## Generating collision boundaries

Details on generating more accurate collision boundaries for vehicles can be found in [__this guide__](content_authoring_vehicles_colliders.md)