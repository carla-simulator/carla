## CARLA 0.4.0

  * Made vehicle input more realistic, now reverse and brake use different input
  * Changed server-client protocol
    - CarlaSettings.ini is sent for every new episode
    - Control is extended with brake, reverse and handbrake
  * Made camera post-process settings able to change depending on the weather
  * Added basic functionality for NPC vehicles
  * Some improvements to the walker spawner
  * Generate road map metadata on save
  * Added command-line switch -carla-no-networking
  * Improved verbosity control of CarlaServer
  * Fixed issue with server that two threads used 100% CPU
  * Fixed issue with the attachment of the main camera to the player
  * Fixed issues with CarlaServer interface with Unreal, does not use STL containers anymore

## CARLA 0.3.0

  * Added basic dynamic weather functionality
    - Weather and sun light can be changed during game
    - Presets stored in config file CarlaWeather.ini
  * Add basic functionality to spawn pedestrians
  * Split road meshes for intersections and turns for better precission of the road map
  * Better debug for road map
  * Implemented collision count for other cars and pedestrians
  * Command line argument -carla-settings now accepts relative paths
  * Improved performance when semantic segmentation is disabled
  * Improved tagger system

## CARLA 0.2.4

  * Fixed serialization of road map resulting in a huge map size

## CARLA 0.2.3

  * Fixed rounding errors in HUD (100% was shown as 99%, 30 FPS as 29 FPS)
  * Fixed crash when player goes out of road map
  * Fixed several issues related to the transform of the road map (wasn't working in CARLA_ORIGIN_1)

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
