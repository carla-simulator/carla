## CARLA 0.5.1

  * Fixed issue server was destroyed on every reset, closing the connection
  * Fixed issue agent servers connect too late
  * Improvements to the python client
  * Added python client test suite for testing the release
  * Added image converter

## CARLA 0.5.0

  * Upgraded to Unreal Engine 4.17
    - Fixes memory leaks
    - Fixes crashes with C++ std classes
  * Redesigned CarlaServer
    - Faster, avoids unnecessary copies
    - Sends images as raw data (no compression)
    - Supports synchronous and asynchronous mode
    - Networking operation have a time-out
    - Synchronous methods have a time-out
    - Pure C interface for better compatibility
    - Unit tests with GoogleTest
  * New server-client protocol
    - Upgraded to proto3
    - Supports repeated fields
    - Optionally send information about all dynamic agents in the scene
    - Now sends transforms instead of locations only
    - Autopilot mode added to control
  * New build system to avoid linkage issues
  * Added autopilot mode
  * Added an on-board camera to the car
  * Added traffic lights and speed limit to player state
  * Added player pawn selection to config file
  * Improved blueprint interface of the C++ classes
  * Some performance improvements to vehicle controllers
  * Fix issues with depth material in Windows
  * Fix issues with random engine not being available for vehicles
  * Fixed issue that compiling a release hang when saving the road map

## CARLA 0.4.6

  * Add weather presets specific for each level

## CARLA 0.4.5

  * Add random seeds to config file
  * Improve logging

## CARLA 0.4.4

  * Fixed regression walkers despawning when stopping after seeing a car
  * Changed, collision is only registered if player moves faster than 1 km/h
  * Fixed issue walkers resume movement after sensing nothing, but the car is still there sometimes

## CARLA 0.4.3

  * Fixed issue with reward, intersect other lane wasn't sent to the client
  * Improvements to the AI of other vehicles, and how they detect pedestrians
  * Improvements to the AI of the pedestrians, trying to avoid slightly better the cars
  * Made roads collision channel WorldStatic

## CARLA 0.4.2

  * Fixed issues with the server-client protocol
  * More improvements to the AI of other vehicles, now they barely crash

## CARLA 0.4.1

  * Improved AI of other vehicles, still needs some adjustment, but now they crash much less

## CARLA 0.4.0

  * Made vehicle input more realistic, now reverse and brake use different input
  * Changed server-client protocol
    - CarlaSettings.ini is sent for every new episode
    - Control is extended with brake, reverse and handbrake
  * Set a clearer hierarchy for loading settings files
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
