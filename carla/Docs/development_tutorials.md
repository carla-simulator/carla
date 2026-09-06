# Development

CARLA is open source and designed to be highly extensible. This  allows users to create custom functionality or content to suit specialized applications or specific needs. The following tutorials detail how to achieve specific development aims with the CARLA codebase:  

- [__Make release__](tuto_D_make_release.md)
- [__Upgrading content__](tuto_D_contribute_assets.md)
- [__Create semantic tags__](tuto_D_create_semantic_tags.md)
- [__Create new sensor__](tuto_D_create_sensor.md)
- [__Preformance benchmarking__](adv_benchmarking.md)
- [__Recorder file format__](ref_recorder_binary_file_format.md)
- [__Collision boundaries__](tuto_D_generate_colliders.md)

## Make a release

If you want to develop your own fork of CARLA and publish releases of your code, follow [__this guide__](tuto_D_make_release.md).

## Upgrading content

Our content resides on a separate Git LFS repository. As part of our build system, we generate and upload a package containing the latest version of this content tagged with the current date and commit. Regularly, we upgrade the CARLA repository with a link to the latest version of the content package. Please follow [__these instructions__](tuto_D_contribute_assets.md) to upgrade content.

## Create semantic tags

CARLA has a set of semantic tags already defined suitable for most use cases. However, if you need additional classes you can add them as detailed in [__this guide__](tuto_D_create_semantic_tags.md)

## Creating a new sensor

You can modify CARLA's C++ code to create new sensors for your custom use cases. Please find the details [__here__](tuto_D_create_sensor.md)

## Benchmarking performance

CARLA has a benchmarking script to help with benchmarking performance on your system. Find the full details [__here__](adv_benchmarking.md)

## Recorder binary file format

Details on the binary file format for the recorder can be found [__here__](ref_recorder_binary_file_format.md)

## Generating collision boundaries

Details on generating more accurate collision boundaries for vehicles can be found in [__this guide__](tuto_D_generate_colliders.md)