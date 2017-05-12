## CARLA 0.2.4

  * Fixed serialization of road map resulting in a huge map size.

## CARLA 0.2.3

  * Fixed rounding errors in HUD (100% was shown as 99%, 30 FPS as 29 FPS).
  * Fixed crash when player goes out of road map.
  * Fixed several issues related to the transform of the road map (wasn't working in CARLA_ORIGIN_1).

## CARLA 0.2.2

  * Implemented signals for off-road and opposite lane invasion
  * Fixed linking issues (use Unreal's libpng)
  * Fixed memory leak in PNG compression

## CARLA 0.2.1

  * Fixed the memory leak related to protobuf issues
  * Fixed color shift in semantic segmentation and depth
  * Added in-game timestamp (now sending both OS and in-game)

## CARLA 0.2.0

  * Fixed Depth issues
  * Added semantic segmentation
  * Changed codification to PNG
  * Camera configuration through config INI file

## CARLA 0.1.1

  * Added build system for Windows and Linux

## CARLA 0.1.0

  * Added basic functionality
