## CARLA 0.8.4

  * Community contribution: ROS bridge by @laurent-george
  * New vehicle: Tesla Model 3
  * Added an option to _"CarlaSettings.ini"_ to disable bikes and motorbikes
  * Fixed missing collision of vehicles introduced in 0.8.3
  * Improved stability of bikes and motorbikes
  * Improved autopilot turning behaviour at intersections, now using front wheels positions as reference
  * Temporarily removed Kawasaki Ninja motorbikes because the model was having some stability issues

## CARLA 0.8.3

  * Added two-wheeled vehicles, 3 bicycles and 4 motorbikes
  * Several art optimizations (CARLA is now about 10% faster)
    - Improved the performance of vegetation assets, adjusted LOD and culling distance, set billboards where possible
    - Drastically reduced the number of polygons of the landscape while keeping the original shape
    - Removed some high-cost unnecessary assets
    - Remodelled Mustang and NissanMicra, now with less polygons and materials, better textures and LOD
    - Remodelled building SM_TerracedHouse_01, now with more polygons but less materials and better textures
  * CARLA releases include now a Dockerfile for building docker images
  * Change in HUD: replace "FPS" by "Simulation Step"
  * The current map name is now included in the scene description message sent to the client
  * Adapted "manual_control.py" and "view_start_positions.py" to use the map name sent by the simulator
  * Improved the vehicle spawning algorithm, now it tries to spawn as much cars as possible even if there are not enough spawn points
  * "Setup.sh" is now faster and accepts an argument to run multiple jobs in parallel
  * Fixed foliage distance culling using wrong distance in "Low Mode"
  * Fixed NissanMicra slightly turning left when driving straight

## CARLA 0.8.2

  * Revamped driving benchmark
    - Changed name from benchmark to driving benchmark
    - Fully Redesigned the architecture of the module
    - Added a lot more documentation
    - Now you can stop and resume the benchmarks you run
  * Rolled back vehicle's location to the pivot of the mesh instead of the center of the bounding box
  * Added relative transform of the vehicle's bounding box to the measurements, player and non-players
  * Added "frame number" to each sensor measurement so it is possible to sync all the measurements based on the frame they are produced
  * Improved vehicle spawner to better handle spawning failures
  * Walkers use now a closer angle to detect vehicles, so they don't stop moving if a car passes nearby
  * Fixed lighting artefact causing the road to change its brightness depending on the distance to the camera
  * Fixed captured images overexposed in Low mode
  * Fixed illegal character in asset name
  * Fixed editing sun azimuth angle in CarlaWeadther.ini had no effect
  * Fixed crash when using a non-standard image size in DirectX (Windows)
  * Fixed issue with using multiple "SceneCaptureToDiskCamera"

## CARLA 0.8.1

  * New Python example for visualizing the player start positions
  * Fixed box extent of non-player agents was sent in centimeters instead of meters
  * Fixed speed limits were sent in km/h instead of m/s
  * Fixed issue in Volkswagen T2 wheels causing it to overturn

## CARLA 0.8.0

  * Upgraded to Unreal Engine 4.18
  * Created our own pedestrian 3D models free to use and distribute
  * Removed Epic's Automotive Materials dependencies
  * 360 Lidars support (similar to Velodyne HDL-32E or VLP-16) thanks to Anton Pechenko (Yandex)
    - Ray-cast based
    - Configurable settings
    - Added methods to save points to disk as PLY file
  * Added quality level settings
    - Low: low quality graphics, about 3 times faster with one camera
    - Epic: best quality (as before)
  * Measurements now use SI units
    - Locations:    m
    - Speed:        m/s
    - Acceleration: m/s^2
    - Collisions:   kg*m/s
    - Angles:       degrees
  * Added API methods to convert depth images to a point cloud
    - New method "image_converter.depth_to_local_point_cloud"
    - A supplementary image can be passed to attach colors to the points
    - New client example generates a point cloud in world coordinates
    - Added Transform class to Python API
  * Performance optimizations
    - Significant speed improvements in both Epic and Low modes
    - Fixed materials and improved shaders for roads, architecture, sidewalks, foliage, landscapes, cars, walkers, reflections, water
    - Execution of a set of Project and Engine parameters to improve performance (quality, vsync, AO, occlusion)
    - Generation of the road pieces using static meshes and actors instead of a single actor with instanced meshes
      - Improved performance since now is able to apply occlusion and draw distance
    - Images are captured asynchronously in the render thread
      - In asynchronous mode, images may arrive up to two frames later
      - In synchronous mode, game thread is blocked until images are ready
    - Blueprint code optimizations for vehicles, walkers, and splines
    - Added a way to configure different quality levels with culling distance and materials configuration
  * Refactored sensor related code to ease adding new sensors in the future
  * Added vehicle box extent to player measurements
  * Removed the player from the list of non-player agents
  * Adjusted bounding boxes to vehicles' height
  * Changed vehicles' center to match bounding box
  * Added autopilot mode to manual_control.py
  * Added quality level options to manual_control.py and client_example.py
  * Replaced background landscape and trees by a matte painting
  * Fixed road map generated some meshes twice
  * Small improvements to Windows support
    - Fixed issues with the Makefile
    - Fixed asset names too long or containing special characters

## CARLA 0.7.1

  * New Python API module: Benchmark
    - Defines a set of tasks and conditions to test a certain agent
    - Contains a starting benchmark, CoRL2017
    - Contains Agent Class: Interface for benchmarking AIs
  * New Python API module: Basic Planner (Temporary Hack)
    - Provide routes for the agent
    - Contains AStar module to find the shortest route
  * Other Python API improvements
    - Converter class to convert between Unreal world and map units
    - Metrics module to summarize benchmark results
  * Send vehicle's roll, pitch, and yaw to client (orientation is now deprecated)
  * New RoutePlanner class for assigning fixed routes to autopilot (IntersectionEntrance has been removed)
  * Create a random engine for each vehicle, which greatly improves repeatability
  * Add option to skip content download in Setup.sh
  * Few small fixes to the city assets

## CARLA 0.7.0

  * New Python client API
    - Cleaner and more robust
    - Compatible with Python 2 and 3
    - Improved exception handling
    - Improved examples
    - Included methods for parsing the images
    - Better documentation
    - Protocol: renamed "ai_control" to "autopilot_control"
    - Merged testing client
    - Added the maps for both cities, the client can now access the car position within the lane
  * Make CARLA start without client by default
  * Added wind effect to some trees and plants
  * Improvements to the existing weather presets
  * Build script: skip content download if up-to-date

## CARLA 0.6.0

  * Included Unreal project and reorganised folders
  * Enabled semantic segmentation by default
  * Added Felipe's Python client
  * New build system (Linux only)
  * Few fixes to city assets

## CARLA 0.5.4

  * Added command-line parameter -carla-no-hud
  * Remove override gamma from weather settings
  * Fixed issue road map generation hangs cooking command
  * Organise Python client and make sample script
  * Rename maps
    - CARLA_ORIGIN_0 --> Town02
    - CARLA_ORIGIN_1 --> Town01
  * Fixed Carla-Cola machine falling at begin play

## CARLA 0.5.3

  * Fixed issues with weather
  * Fixed missing building

## CARLA 0.5.2

  * Autopilot mode has been removed, now server sends AI control together with measurements every frame
  * State and position of traffic lights and signs are now included in the measurements too
  * Added a python console client
  * Fixed crash when client sends an invalid player start
  * Fixed some issues with the dynamic weather not looking as it used to do
  * Fixed some collision boxes missing

## CARLA 0.5.1

  * Fixed issue server was destroyed on every reset, closing the connection
  * Fixed issue agent servers connect too late
  * Improvements to the python client
  * Added python client test suite for testing the release
  * Added image converter
  * Fixed missing floor on CARLA_ORIGIN_0
  * Changed sidewalk texture
  * Improvements on the physics of some vehicles
  * More props and decals added to the cities

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
  * Added more content; 7 vehicles, 30 pedestrians, many decals and props
  * Randomized pedestrian clothing
  * Many improvements and fixes to the city levels and assets
  * Added sub-surface scattering to vegetation
  * Added key binding to change weather during play
  * Added key binding to toggle autopilot mode
  * Added a second camera to the player

## CARLA 0.4.6

  * Add weather presets specific for each level
  * Some map fixes, adjust weather presets specific for each level
  * Fixed regression that some walkers may go at extremely slow and fast speeds

## CARLA 0.4.5

  * Add random seeds to config file
  * Improve logging
  * Removed rotation of map CARLA_ORIGIN_1

## CARLA 0.4.4

  * Fixed regression walkers despawning when stopping after seeing a car
  * Changed, collision is only registered if player moves faster than 1 km/h
  * Fixed issue walkers resume movement after sensing nothing, but the car is still there sometimes
  * Few improvements to the city assets

## CARLA 0.4.3

  * Fixed issue with reward, intersect other lane wasn't sent to the client
  * Improvements to the AI of other vehicles, and how they detect pedestrians
  * Improvements to the AI of the pedestrians, trying to avoid slightly better the cars
  * Made roads collision channel WorldStatic
  * Tune several vehicles' physics and engine
  * Fixed issue with vehicles bouncing back after hitting a pedestrian
  * Add bigger box to pedestrians to avoid accidents
  * Make vehicles spawn in order instead of randomly

## CARLA 0.4.2

  * Fixed issues with the server-client protocol
  * More improvements to the AI of other vehicles, now they barely crash
  * Improved the physics of some vehicles
  * Tweak the city for better AI of other vehicles

## CARLA 0.4.1

  * Improved AI of other vehicles, still needs some adjustment, but now they crash much less
  * Fixed wrong semantic segmentation label of the poles of traffic lights and signs
  * Added randomized vehicle license plates
  * Few improvements to the city assets

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
  * Fixed issue with server not running below 30 fps at fixed frame rate, added physics sub-stepping
  * Fixed issues with some weather settings
  * Added randomized pedestrians with their AI and animations
  * Added other vehicles with their AI and physics
  * Added traffic lights and signs
  * Tweaked capture image to look similar to main camera
  * Changed car input to match settings in plugin
  * General improvements to levels and assets

## CARLA 0.3.0

  * Added basic dynamic weather functionality
    - Weather and sun light can be changed during game
    - Presets stored in config file CarlaWeather.ini
    - Added some presets for dynamic weather
  * Add basic functionality to spawn pedestrians
  * Split road meshes for intersections and turns for better precission of the road map
  * Better debug for road map
  * Implemented collision count for other cars and pedestrians
  * Command line argument -carla-settings now accepts relative paths
  * Improved performance when semantic segmentation is disabled
  * Improved tagger system
  * Implemented nav-mesh and spawn points for pedestrians
  * Added new cars
  * Added dynamic street lights
  * General improvements to levels and assets
  * Make the car jump

## CARLA 0.2.4

  * Fixed serialization of road map resulting in a huge map size
  * Some optimizations in the vegetation
  * Implemented more LODS

## CARLA 0.2.3

  * Fixed rounding errors in HUD (100% was shown as 99%, 30 FPS as 29 FPS)
  * Fixed crash when player goes out of road map
  * Fixed several issues related to the transform of the road map (wasn't working in CARLA_ORIGIN_1)
  * Make custom depth pass disable by default (semantic segmentation won't work by default)
  * Fixed road width in T-intersections
  * Implement road LOD
  * Fixed missing assets

## CARLA 0.2.2

  * Implemented signals for off-road and opposite lane invasion
  * Fixed linking issues (use Unreal's libpng)
  * Fixed memory leak in PNG compression
  * Added boundaries to the map
  * Several fixes in the map content

## CARLA 0.2.1

  * Fixed the memory leak related to protobuf issues
  * Fixed color shift in semantic segmentation and depth
  * Added in-game timestamp (now sending both OS and in-game)

## CARLA 0.2.0

  * Fixed Depth issues
  * Fixed random crash due to an invalid player start position
  * Added semantic segmentation
  * Changed codification to PNG
  * Camera configuration through config INI file

## CARLA 0.1.1

  * Added build system for Windows and Linux
  * Added more content

## CARLA 0.1.0

  * Added basic functionality
