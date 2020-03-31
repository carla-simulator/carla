# Retrieve simulation data

Learning an efficient way to retrieve simulation data is essential in CARLA. This holistic tutorial is advised for both, newcomers and more experienced users. It starts from the very beginning and gradually dives into different possibilities and options in CARLA.  

A simulation is created and with custom settings and traffic. An ego vehicle is set to roam around the city. The simulation is recorded, so that later it can be queried to find the highlights. Then that original simulation is played back, and exploited to the limit. Add new sensors to retrieve consistent data, change the conditions, or create different outputs.  

* [__Overview__](#overview)  
* [__Set the simulation__](#set-the-simulation)  
	* [Map setting](#map-setting)
	* [Weather setting](#weather-setting)
* [__Set traffic__](#set-traffic)  
	* [CARLA traffic](#CARLA-traffic)
	* [SUMO co-simulation traffic](#sumo-co-simulation-traffic)
* [__Set the ego vehicle__](#set-the-ego-vehicle)  
	* [Spawn the ego vehicle](#spawn-the-ego-vehicle)
	* [Place the spectator](#place-the-spectator)
* [__Set basic sensors__](#set-basic-sensors)  
	* [RGB camera](#rgb-camera)
	* [Detectors](#detectors)
	* [Other sensors](#other-sensors)
* [__No-rendering-mode__](#no-rendering-mode)  
	* [Simulate at fast pace](#simulate-at-fast-pace)
	* [Manual control without rendering](#manual-control-without-rendering)
* [__Record and retrieve data__](#record-and-retrieve-data)  
	* [Start recording](#start-recording)
	* [Capture data](#capture-data)
	* [Stop recording](#stop-recording)
* [__Set advanced sensors__](#set-advanced-sensors)  
	* [Depth camera](#depth-camera)
	* [Semantic segmentation camera](#semantic-segmentation-camera)
	* [LIDAR raycast sensor](#lidar-raycast-sensor)
	* [Radar sensor](#radar-sensor)
* [__Exploit the recording__](#exploit-the-recording)  
	* [Query the events](#query-the-recording)
	* [Choose a fragment](#choose-a-fragment)
	* [Add new sensors](#add-new-sensors)
	* [Change conditions](#change-conditions)
	* [Reenact the simulation](#reenact-the-simulation)
* [__Tutorial scripts__](#tutorial-scripts)  

---
## Overview

There are some common mistakes in the process of retrieving simulation data, such as flooding the simulator with sensors, storing useless data, or trying too hard to find a specific event. However, there is a proper path to follow in order to get a simulation ready, so that data can be replicated, examined and altered at will.  

!!! Important
    This tutorial uses the [__CARLA 0.9.8 deb package__](start_quickstart.md). There may be changes depending on your CARLA version and installation, specially regarding paths.

This tutorial presents a general overview of the process, while providing some alternative paths to fulfill different purposes. 
Different scripts will be used along the tutorial. All of them are already provided in CARLA, mostly for generic purposes.  

* __config.py__ used to change the simulation settings. The town map, disable rendering, set a fixed time-step...  
	* `carla/PythonAPI/utils/config.py`
* __dynamic_weather.py__ to create interesting weather conditions.  
	* `carla/PythonAPI/examples/config.py`
* __spawn_npc.py__ to spawn some AI controlled vehicles and walkers.  
	* `carla/PythonAPI/examples/config.py`
* __manual_control.py__ spawns an ego vehicle and provides control over it.  
	* `carla/PythonAPI/examples/config.py`

However, two scripts mentioned along the tutorial that cannot be found in CARLA. They contain the fragments of code cited, and the full code can be found in the last section of the tutorial.

* __tutorial_ego.py__ spawns with some basic sensors, and enables autopilot. The spectator is placed where at the spawning position. The recorder starts at the very beginning and stops when the script is finished.  
* __tutorial_replay.py__ reenacts the simulation that __tutorial_ego.py__ recorded. There are different fragments of code to query the recording, spawn some advanced sensors, change weather conditions and reenact fragments of the recording.  

This serves a twofold purpose. First of all, to encourage user to build their own scripts, making sure they gain full understanding of what the code is doing. In addition to this, the tutorial can go down different paths depending on the final intentions. The final scripts only gather the different fragments to create a possible output, but they should not be seen as a strict process. Retrieving data in CARLA is as powerful as the user wants it to be. 

!!! Important
    This tutorial requires some knowledge in Python.


---
## Set the simulation

The first thing to do is set the simulation ready to a desired environment.  

Run CARLA. 

```sh
cd /opt/carla/bin
./CarlaUE.sh
```

### Map setting

Choose a map for the simulation to run. Take a look at the [map documentation](core_map.md#carla-maps) to learn more about their specific attributes. For the sake of this tutorial, __Town01__ is chosen. 

Open a new terminal. Change the map using the __config.py__ script. 

```
cd /opt/carla/PythonAPI/utils
./config.py --map Town01
```
This script has many other different options that, for the sake of simplicity, are left out in this tutorial. Here is a brief summary. 

<details>
<summary> Optional arguments in <b>config.py</b> </summary>

```sh
  -h, --help            show this help message and exit
  --host H              IP of the host CARLA Simulator (default: localhost)
  -p P, --port P        TCP port of CARLA Simulator (default: 2000)
  -d, --default         set default settings
  -m MAP, --map MAP     load a new map, use --list to see available maps
  -r, --reload-map      reload current map
  --delta-seconds S     set fixed delta seconds, zero for variable frame rate
  --fps N               set fixed FPS, zero for variable FPS (similar to
                        --delta-seconds)
  --rendering           enable rendering
  --no-rendering        disable rendering
  --no-sync             disable synchronous mode
  --weather WEATHER     set weather preset, use --list to see available
                        presets
  -i, --inspect         inspect simulation
  -l, --list            list available options
  -b FILTER, --list-blueprints FILTER
                        list available blueprints matching FILTER (use '*' to
                        list them all)
  -x XODR_FILE_PATH, --xodr-path XODR_FILE_PATH
                        load a new map with a minimum physical road
                        representation of the provided OpenDRIVE
```
</details>
<br>

### Weather setting

Each town is loaded with a specific weather that fits it, however this can be set at will. There are two main approaches to it. The first one would set a custom weather condition, the other one would create a dynamic weather that changes conditions over time. CARLA provides a script for each approach. However, as setting a specific weather condition will be done later in this tutorial, so let's create a dynamic environment for now. 

Open a new terminal and run __dynamic_weather.py__. This scripts allows to set the ratio at which the weather changes, being `1.0` the defaul setting. 

```sh
cd /opt/carla/PythonAPI/util
./dynamic_weather.py --speed FACTOR 1.0
```

!!! Note
    The script to set custom weather conditions is [__weather.py__](http://carla.org/2020/03/09/release-0.9.8/#weather-extension) in `PythonAPI/util/`.  

---
## Set traffic

Simulate traffic is one of the best ways to bring the city to life. It is also necessary to retrieve data for urban environments. There are different options to do so in CARLA.  

### CARLA traffic and pedestrians

The CARLA traffic is managed by the [Traffic Manager](adv_traffic_manager.md) module. As for pedestrians, each of them has their own [carla.WalkerAIController](python_api.md#carla.WalkerAIController). 

Open a new terminal, and run __spawn_npc.py__ to spawn vehicles and walkers. Let's just spawn 30 vehicles and the same amount of walkers. 

```sh
cd /opt/carla/PythonAPI/examples
./spawn_npc.py -n 30 --safe
```

<details>
<summary> Optional arguments in <b>spawn_npc.py</b> </summary>

```sh
  -h, --help            show this help message and exit
  --host H              IP of the host server (default: 127.0.0.1)
  -p P, --port P        TCP port to listen to (default: 2000)
  -n N, --number-of-vehicles N
                        number of vehicles (default: 10)
  -w W, --number-of-walkers W
                        number of walkers (default: 50)
  --safe                avoid spawning vehicles prone to accidents
  --filterv PATTERN     vehicles filter (default: "vehicle.*")
  --filterw PATTERN     pedestrians filter (default: "walker.pedestrian.*")
  -tm_p P, --tm_port P  port to communicate with TM (default: 8000)
  --sync                Synchronous mode execution
```
</details>
<br>

### SUMO co-simulation traffic

CARLA can run a co-simulation with SUMO. This allows for creating traffic in SUMO that will be propagated to CARLA. This co-simulation is bidirectional. Spawning vehicles in CARLA will do so in SUMO.  

Right now this is available for CARLA 0.9.8 and later, in __Town01__, __Town04__, and __Town05__. The first one is the most stable, and the one chosen for this tutorial.  

* First of all, install SUMO. 
```sh
sudo add-apt-repository ppa:sumo/stable
sudo apt-get update
sudo apt-get install sumo sumo-tools sumo-doc
```
* Set the environment variable SUMO_HOME.
```sh
echo "export SUMO_HOME=/usr/share/sumo" >> ~/.bashrc && source ~/.bashrc
```
* With the CARLA server on, run the [SUMO-CARLA synchrony script](https://github.com/carla-simulator/carla/blob/master/Co-Simulation/Sumo/run_synchronization.py). 
```sh
cd ~/carla/Co-Simulation/Sumo
python run_synchronization.py -c examples/Town01.sumocfg
```
* A SUMO window should have opened. __Press Play__ in order to start traffic in both simulations. 
```
> "Play" on SUMO window.
```

The traffic generated by this script is an example created by the CARLA team. By default it spawns the same vehicles following the same routes. These can be changed by the user in SUMO. 

!!! Warning
    Right now, SUMO co-simulation is a beta feature. Vehicles do not have physics nor take into account CARLA traffic lights. 

---
## Set the ego vehicle

The script __tutorial_ego.py__ does many things.  

* Start recording the simulation. 
* Spawn the ego vehicle. 
* Spawn a RGB camera attched to it. 
* Position the spectator camera where the ego vehicle is. 
* Let the ego vehicle roam around in autopilot mode. 
* Stop the recorder when the script is stopped, and destroy the ego vehicle and its sensor. 

It is quite straightforward, as the intention is to keep things simple. This tutorial will explain each of this steps in depth.  


### Spawn the ego vehicle

Vehicles controlled by the user are referred as "ego". The attribute `role_name` is set to `ego` to differenciate them. Other attributes that can be set, some with recommended values. First, an ego vehicle will be spawned using any of the vehicle blueprints in the library. In this case, the color will be changed at random to one of these. 

Call the map to get a list of spawn points recommended by the developers. Choose one of them, and use it to spawn the ego vehicle. 

```py        
# --------------
# Spawn ego vehicle
# --------------
ego_bp = world.get_blueprint_library().find('vehicle.tesla.model3')
ego_bp.set_attribute('role_name','ego')
print('\nEgo role_name is set')
ego_color = random.choice(ego_bp.get_attribute('color').recommended_values)
ego_bp.set_attribute('color',ego_color)
print('\nEgo color is set')

spawn_points = world.get_map().get_spawn_points()
number_of_spawn_points = len(spawn_points)

if 0 < number_of_spawn_points:
    random.shuffle(spawn_points)
    ego_transform = spawn_points[0]
    ego_vehicle = world.spawn_actor(ego_bp,ego_transform)
    print('\nEgo is spawned')
else: 
    logging.warning('Could not found any spawn points')
```

### Place the spectator

Now that the ego vehicle and the sensor have been spawned, it is time to find them. The spectator actor controls the simulation view. Find it and move it where the ego vehicle is. To find the ego vehicle, use one of the snapshots that the vehicle sends to the world on every tick.  

```py
# --------------
# Spectator on ego position
# --------------
spectator = world.get_spectator()
world_snapshot = world.wait_for_tick()
ego_snapshot = world_snapshot.find(ego_vehicle.id)
spectator.set_transform(ego_snapshot.get_transform())
print('\nSpectator located where ego')
```

---
## Set basic sensors

The process to spawn any sensor is quite similar.  

__1.__ Use the library to find sensor blueprints.  
__2.__ Set specific attributes for the sensor. This is crucial, as the attributes will determine the data retrieved.  
__3.__ Attach the sensor to the ego vehicle. That means that its transform is __relative to its parent__. The `AttachmentType.SpringArm` will make that the camera position updates smooth, with little eases regarding its parent.  
__4.__ Add a `listen()` method. This is the key element. A [__lambda__](https://www.w3schools.com/python/python_lambda.asp) method that will be called each time the sensor listens for data. The argument is the sensor data retrieved.  

Having this basic guideline in mind, let's set some basic sensors for the ego vehicle. 

### RGB camera

This sensor generates realistic shots of the scene. It is the sensor with more settable attributes in all of them, but it is also a fundamental one. It should be understood as a real camera, with attributtes such as `focal_distance`, `shutter_speed` or `gamma` to determine how it would work internally. There is also a specific set of attributtes to define the lens distorsion, and lots of advanced attributes. For example, the `lens_circle_multiplier` can be used to achieve an effect similar to an eyefish lens. 

Learn all about them in the [RGB camera documentation](ref_sensors.md#rgb-camera). For the sake of simplicity, the `tutorial_ego` scripts only sets the most commonly used attributes of this sensor. 
* `image_size_x` and `image_size_y` will change the resolution of the output image. 
* `fov` is the horizontal field of view of the camera. This is usually changes in correlation with the resolution, to get a similar view. 

After setting the attributes, it is time to locate the sensor. The script places the camera in the hood of the car, and pointing forward. It will capture the front view of the car. 

The data is retrieved as a [carla.Image](python_api.md#carla.Image) on every step. The listen method saves these to disk. The path can be altered at will, and the name of each shot is coded to be based on the simulation frame where the shot was taken.  

```py
# --------------
# Spawn attached RGB camera
# --------------
cam_bp = None
cam_bp = world.get_blueprint_library().find('sensor.camera.rgb')
cam_bp.set_attribute("image_size_x",str(1920))
cam_bp.set_attribute("image_size_y",str(1080))
cam_bp.set_attribute("fov",str(105))
cam_location = carla.Location(2,0,1)
cam_rotation = carla.Rotation(0,180,0)
cam_transform = carla.Transform(cam_location,cam_rotation)
ego_cam = world.spawn_actor(cam_bp,cam_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.SpringArm)
ego_cam.listen(lambda image: image.save_to_disk('tutorial/output/%.6d.png' % image.frame))
```

### Detectors

These sensors retrieve data when the object they are attached to registers a specific event. There are three type of detector sensors, each one describing one type of event.
* [__Collision detector.__](ref_sensors.md#collision-detector) Retrieves collisions between its parent and other actors.
* [__Lane invasion detector.__](ref_sensors.md#lane-invasion-detector) Registers when its parent crosses a lane marking.
* [__Obstacle detector.__](ref_sensors.md#obstacle-detector) Detects possible obstacles ahead of its parent.

The data they retrieve is commonly printed. This will be helpful later when deciding which part of the simulation is going to be reenacted. In fact, the collisions can be explicitely queried using the recorder. Let's take a look at their description in `tutorial_ego.py`. 

Only the obstacle detector blueprint has attributes to be set. Here are some important ones. 

* `sensor_tick` is commonplace for sensors that retrieve data on every step. It sets the sensor to retrieve data only after `x` seconds pass. 
* `distance` and `hit-radius` determine the debug line used to detect obstacles ahead. 
* `only_dynamics` determines if static objects should be taken into account or not. By default, any object is considered. 

The script sets the sensor to only consider dynamic objects. The intention is to avoid unnecessary data. If the vehicle collides with any static object, it will be detected by the collision sensor.  

```py
# --------------
# Spawn attached collision
# --------------
col_bp = world.get_blueprint_library().find('sensor.other.collision')
col_location = carla.Location(0,0,0)
col_rotation = carla.Rotation(0,0,0)
col_transform = carla.Transform(col_location,col_rotation)
ego_col = world.spawn_actor(col_bp,col_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def collision_callback(colli):
    print(colli)
ego_col.listen(lambda colli: collision_callback(colli))

# --------------
# Spawn attached Lane invasion
# --------------
lane_bp = world.get_blueprint_library().find('sensor.other.lane_invasion')
lane_location = carla.Location(0,0,0)
lane_rotation = carla.Rotation(0,0,0)
lane_transform = carla.Transform(lane_location,lane_rotation)
ego_lane = world.spawn_actor(lane_bp,lane_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def collision_callback(lane):
    print(lane)
ego_lane.listen(lambda lane: collision_callback(lane))

# --------------
# Spawn attached Obstacle detector
# --------------
obs_bp = world.get_blueprint_library().find('sensor.other.obstacle')
obs_bp.set_attribute("only_dynamics",str(True))
obs_location = carla.Location(0,0,0)
obs_rotation = carla.Rotation(0,0,0)
obs_transform = carla.Transform(obs_location,obs_rotation)
ego_obs = world.spawn_actor(obs_bp,obs_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def collision_callback(obs):
    print(obs)
ego_obs.listen(lambda obs: collision_callback(obs))
```

### Other sensors

Among this category, only two sensors will be considered for the time being. 

* [__GNSS sensor.__](ref_sensors.md#collision-detector) Retrieves the geolocation of the sensor.
* [__IMU sensor.__](ref_sensors.md#collision-detector) Comprises an accelerometer, a gyroscope, and a compass.

To get the general measures for the vehicle object, these two sensors are spawned centered to it. 

The attributes available for these sensors mostly set the mean or standard deviation parameter in the noise model of the measure. This is useful to get more realistic measures. However, the script only sets on attribute.  

* `sensor_tick`. As this measures are not supposed to vary significantly between steps, it is okay to retrieve the data every so often. In this case, it is set to be printed every three seconds.  

```py
# --------------
# Spawn attached GNSS
# --------------
gnss_bp = world.get_blueprint_library().find('sensor.other.gnss')
gnss_location = carla.Location(0,0,0)
gnss_rotation = carla.Rotation(0,0,0)
gnss_transform = carla.Transform(gnss_location,gnss_rotation)
gnss_bp.set_attribute("sensor_tick",str(3.0))
ego_gnss = world.spawn_actor(gnss_bp,gnss_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def collision_callback(gnss):
    print(gnss)
ego_gnss.listen(lambda gnss: collision_callback(gnss))

# --------------
# Spawn attached IMU
# --------------
imu_bp = world.get_blueprint_library().find('sensor.other.imu')
imu_location = carla.Location(0,0,0)
imu_rotation = carla.Rotation(0,0,0)
imu_transform = carla.Transform(imu_location,imu_rotation)
imu_bp.set_attribute("sensor_tick",str(3.0))
ego_imu = world.spawn_actor(imu_bp,imu_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def collision_callback(imu):
    print(imu)
ego_imu.listen(lambda imu: collision_callback(imu))
```

---
## No-rendering mode

### Simulate at fast pace 

The [no-rendering mode](adv_rendering_options.md) where the intention is to run an initial simulation that will be later played again to retrieve data. 

Disabling the rendering will save up a lot of work to the simulation. As the GPU is not used, the server can work at full speed. This could be useful to simulate complex conditions at a fast pace. The best way to do so would be by setting a fixed time-step for. Running an asynchronous server with a fixed time-step, the only limitation for the simulation would be the inner logic of the server. 

This configuration can be used for example with the __tutorial_ego.py__, where an ego vehicles and some vehicles are spawned and roam around the city. The same `config.py` used to [set the map](#map-setting) disable rendering, and set a fixed time-step. 

```
cd /opt/carla/PythonAPI/utils
./config.py --no-rendering --delta-seconds 0.05 # Never greater than 0.1s
```

!!! Warning
    Read the [documentation](adv_synchrony_timestep.md) before messing around with with synchrony and time-step.

### Manual control without rendering

The script `PythonAPI/examples/no_rendering_mode.py` provides some sight of what is happening. It creates a minimalistic aerial view with Pygame, that will follow the ego vehicle. This could be used along with __manual_control.py__ to create a specific route with barely no cost, record it, and then play it back and exploit it to gather data. 

```
cd /opt/carla/PythonAPI/examples
python manual_control.py
```

```
cd /opt/carla/PythonAPI/examples
python no_rendering_mode.py --no-rendering
```

<details>
<summary> Optional arguments in <b>no_rendering_mode.py</b> </summary>

```sh
optional arguments:
  -h, --help           show this help message and exit
  -v, --verbose        print debug information
  --host H             IP of the host server (default: 127.0.0.1)
  -p P, --port P       TCP port to listen to (default: 2000)
  --res WIDTHxHEIGHT   window resolution (default: 1280x720)
  --filter PATTERN     actor filter (default: "vehicle.*")
  --map TOWN           start a new episode at the given TOWN
  --no-rendering       switch off server rendering
  --show-triggers      show trigger boxes of traffic signs
  --show-connections   show waypoint connections
  --show-spawn-points  show recommended spawn points
```
</details>
<br>

!!! Note
    In this mode, GPU-based sensors will retrieve empty data. Cameras are useless, but detectors

---
## Record and retrieve data

### Start recording

The [__recorder__](adv_recorder.md) can be started at anytime. The script does it at the very beginnning, in order to capture everything, including the spawning of the first actors. If no path is detailed, the log will be saved into `CarlaUE4/Saved`. 

```py
# --------------
# Start recording
# --------------
client.start_recorder('~/tutorial/recorder/recording01.log')
```

### Capture data

It is time to set the ego vehicle free. It could be manually controlled using `/PythonAPI/examples/manual_control.py`. However, the script enables the autopilot mode. The [Traffic Manager](adv_traffic_manager.md) will make it roam around the city automatically. 

Create a loop to prevent the script from finishing until the user commands via terminal. The recorder will continue until then. Let the simulation run for a while, depending on the amount of data desired.  

```py
# --------------
# Capture data
# --------------
ego_vehicle.set_autopilot(True)
print('\nEgo autopilot enabled')

while True:
    world_snapshot = world.wait_for_tick()
```

!!! Note
    To avoid rendering and save up computational cost, enable [__no rendering mode__](adv_rendering_options.md#no-rendering-mode). The script `/PythonAPI/examples/no_rendering_mode.py` does this while creating a simple aerial view.  

### Stop recording 

A timeout can be added to the script. Right now, use `Ctrl+C` or quit the terminal to finish it. The script will stop the recorder, destroy the sensor and the ego vehicle, and finish.  

```py
finally:
# --------------
# Stop recording
# --------------
client.stop_recorder()
if ego_vehicle is not None:
    if ego_cam is not None:
        print('\nDestroying the RGB camera in tutorial')
        ego_cam.stop()
        ego_cam.destroy()
    print('\nDestroying ego vehicle in tutorial')
    ego_vehicle.destroy()
```

---
## Set advanced sensors

Now that a simulation has been recorded sucessfully, it is time to play with it. One of the best ways to do so is adding new sensors to gather new data. The script __tutorial_replay.py__ contains definitions of more sensors. They work in the same way as the basic ones, but their comprehension may be a bit harder.

### Depth camera

The [depth camera](ref_sensors.md#depth-camera) generates shot that maps every pixel in a grayscale depth map. However, the output is not directly this. It originally maps the depth buffer of the camera using a RGB color space, but this has to be translated to a grayscale to be comprehensible.  

In order to do this, simply save the image as previously done with the RGB camera, but this time, apply a [carla.ColorConverter](python_api.md#carla.ColorConverter) to it. There are two conversions available for depth cameras.  

* __carla.ColorConverter.Depth__ translates the original depth with milimetric precision.  
* __carla.ColorConverter.LogarithmicDepth__ also has milimetric granularity, but provides better results in close distances and a little worse for further elements.  

The attributes for the depth camera only set elements previously stated in the RGB camera: `fov`, `image_size_x`, `image_size_y` and `sensor_tick`. The script sets this sensor to match the previous RGB camera used. 

```py
# --------------
# Add a Depth camera to ego vehicle. 
# --------------
depth_cam = None
depth_bp = world.get_blueprint_library().find('sensor.camera.depth')
depth_location = carla.Location(2,0,1)
depth_rotation = carla.Rotation(0,180,0)
depth_transform = carla.Transform(depth_location,depth_rotation)
depth_cam = world.spawn_actor(depth_bp,depth_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.SpringArm)
# This time, a color converter is applied to the image, to get the semantic segmentation view
depth_cam.listen(lambda image: image.save_to_disk('tutorial/new_depth_output/%.6d.png' % image.frame,carla.ColorConverter.LogarithmicDepth))
```

### Semantic segmentation camera

The [semantic segmentation camera](ref_sensors.md#semantic-segmentation-camera) renders elements in scene with a different color depending on how these have been tagged. The tags are created by the simulator depending on the path of the asset used for spawning. For example, meshes stored in `Unreal/CarlaUE4/Content/Static/Pedestrians` are tagged as `Pedestrian`.  

The output is an image, as any camera, but each pixel contains the tag information encoded in the red channel. This original image must be converted, as it happened with the depth camera, using the __ColorConverter.CityScapesPalette__. New tags can be created, read more in the [semantic segmentation camera reference](ref_sensors.md#semantic-segmentation-camera).  

The attributes available for this camera are exactly the same as the depth camera. The script also sets this to match the original RGB camera. 

```py
# --------------
# Add a new semantic segmentation camera to my ego
# --------------
sem_cam = None
sem_bp = world.get_blueprint_library().find('sensor.camera.semantic_segmentation')
sem_bp.set_attribute("image_size_x",str(1920))
sem_bp.set_attribute("image_size_y",str(1080))
sem_bp.set_attribute("fov",str(105))
sem_location = carla.Location(2,0,1)
sem_rotation = carla.Rotation(0,180,0)
sem_transform = carla.Transform(sem_location,sem_rotation)
sem_cam = world.spawn_actor(sem_bp,sem_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.SpringArm)
# This time, a color converter is applied to the image, to get the semantic segmentation view
sem_cam.listen(lambda image: image.save_to_disk('tutorial/new_sem_output/%.6d.png' % image.frame,carla.ColorConverter.CityScapesPalette))
```

### LIDAR raycast sensor

The [LIDAR sensor](ref_sensors.md#lidar-raycast-sensor) simulates a rotating LIDAR. It creates a cloud of points that maps the scene in 3D. The LIDAR contains a set of lasers that rotate at a certain frequency. The lasers raycast the distance to impact, and store every shot as one single point.  

The way the array of lasers is disposed can be set using different sensor attributes. 

* `upper_fov` and `lower_fov` the angle of the highest and the lowest laser respectively.
* `channels` sets the amount of lasers to be used. These are distributed along the desired `fov`. 

The key attributes however, are the ones that set the way this points are calculated. This can be used to calculate the amount of points that each laser calculates every step: `points_per_second / (FPS * channels)`.  

* `range` determines the maximum distance to capture.  
* `points_per_second` is the amount of points that will be obtained every second. This quantity is divided between the amount of `channels`.  
* `rotation_frequency` is the amount of times the LIDAR will rotate every second. 

The point cloud output is described as a [carla.LidarMeasurement]can be iterated as a list of [carla.Location] or saved to a _.ply_ standart file format. The script __tutorial_replay.py__ sets the LIDAR render highly-detailed static images of the scene on every step.

```py
# --------------
# Add a new LIDAR sensor to my ego
# --------------
lidar_cam = None
lidar_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast')
lidar_bp.set_attribute('channels',str(50))
lidar_bp.set_attribute('points_per_second',str(900000))
lidar_bp.set_attribute('rotation_frequency',str(20))
lidar_bp.set_attribute('range',str(20))
lidar_location = carla.Location(0,0,2)
lidar_rotation = carla.Rotation(0,0,0)
lidar_transform = carla.Transform(lidar_location,lidar_rotation)
lidar_sen = world.spawn_actor(lidar_bp,lidar_transform,attach_to=ego_vehicle,attachment_type=carla.AttachmentType.SpringArm)
lidar_sen.listen(lambda point_cloud: point_cloud.save_to_disk('tutorial/new_lidar_output/%.6d.ply' % point_cloud.frame))
```

The output can be visualized using __Meshlab__.
__1.__ Install [Meshlab](http://www.meshlab.net/#download).
```sh
sudo apt-get update -y
sudo apt-get install -y meshlab
```
__2.__ Open Meshlab.
```sh
meshlab
```
__3.__ Open one of the _.ply_ files. `File > Import mesh...` 


### Radar sensor

The [radar sensor](ref_sensors.md#radar-sensor) is similar to de LIDAR. It creates a conic view and shoots lasers that raycast their impacts. This time the output is a [carla.RadarMeasurement]. These contain a list of the [carla.RadarDetection] detected by the lasers instead of points in space. The detections now contain coordinates regarding the sensor, `azimuth`, `altitude`, `sensor` and `velocity`. 

The radar is placed on the hood, and rotated a bit upwards. That way, the output will map the front view of the car. 

The attributes of this sensor mostly set the way the lasers are located.

* `horizontal_fov` and `vertical_fov` determine the amplitude of the conic view.
* `channels` sets the amount of lasers to be used. These are distributed along the desired `fov`. 
* `range` is the maximum distance for the lasers to raycast. 
* `points_per_second` sets the the amount of points to be captured, that will be divided between the channels stated. 

For the sake of this tutorial, the `horizontal_fov` is incremented, and the `vertical_fov` diminished. The area of interest is specially the height were vehicles and walkers usually move on. Not much will be going on upwards. The `range` is also changed from 100m to 10m, in order to retrieve data only right ahead of the vehicle. 

The code for this sensor includes this time a more complex callback for the listen method. It will draw the points captured by the radar on the fly. The points will be colored depending on their velocity regarding the ego vehicle. 
* __Blue__ for points approaching the vehicle.  
* __Read__ for points moving away from it. 
* __White__ for points static regarding the ego vehicle, meaning that both move at the same velocity. 

This is an example of the real capabilities of this function. Getting 

```py
# --------------
# Add a new radar sensor to my ego
# --------------
rad_cam = None
rad_bp = world.get_blueprint_library().find('sensor.other.radar')
rad_bp.set_attribute('horizontal_fov', str(35))
rad_bp.set_attribute('vertical_fov', str(20))
rad_bp.set_attribute('range', str(20))
rad_location = carla.Location(x=2.0, z=1.0)
rad_rotation = carla.Rotation(pitch=5)
rad_transform = carla.Transform(rad_location,rad_rotation)
rad_ego = world.spawn_actor(rad_bp,rad_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def rad_callback(radar_data):
    velocity_range = 7.5 # m/s
    current_rot = radar_data.transform.rotation
    for detect in radar_data:
        azi = math.degrees(detect.azimuth)
        alt = math.degrees(detect.altitude)
        # The 0.25 adjusts a bit the distance so the dots can
        # be properly seen
        fw_vec = carla.Vector3D(x=detect.depth - 0.25)
        carla.Transform(
            carla.Location(),
            carla.Rotation(
                pitch=current_rot.pitch + alt,
                yaw=current_rot.yaw + azi,
                roll=current_rot.roll)).transform(fw_vec)

        def clamp(min_v, max_v, value):
            return max(min_v, min(value, max_v))

        norm_velocity = detect.velocity / velocity_range # range [-1, 1]
        r = int(clamp(0.0, 1.0, 1.0 - norm_velocity) * 255.0)
        g = int(clamp(0.0, 1.0, 1.0 - abs(norm_velocity)) * 255.0)
        b = int(abs(clamp(- 1.0, 0.0, - 1.0 - norm_velocity)) * 255.0)
        print("I got here")
        world.debug.draw_point(
            radar_data.transform.location + fw_vec,
            size=0.075,
            life_time=0.06,
            persistent_lines=False,
            color=carla.Color(r, g, b))
rad_ego.listen(lambda radar_data: rad_callback(radar_data))
```

---
## Exploit the recording

So far, a simulation has been set running. An ego vehicle roamed around for a while while retrieving data, and the whole simulation has been recorded. Make sure to have the data retrieved by the sensor and the log of the recording. Look inside the folders detailed in the __tutorial_ego.py__.  

Close the simulation and any script runnning. It is time to dive into the last script, __tutorial_replay.py__. This script is subject to change, so it contains different segments of code commented for different functionalities.  

### Query the events

The different queries are detailed in the [__recorder documentation__](adv_recorder.md). In summary, they retrieve different things.  

* A log of the most important events in the recording or in every frame. 
* A log of the actors blocked. Those that do not move a minimum distance in a certain time. 
* A log of the collisions registered by [collision sensors](ref_sensors.md#collision-detector). 

Run a new simulation. 

```sh
./CarlaUE4.sh
```

Use the queries to study the recording. Find moments of remarkable interest. The file info is also useful to identify the ego vehicle, or any actor, with its ID.  

```py
# --------------
# Query the recording
# --------------
# Show only the most important events in the recording.  
print(client.show_recorder_file_info("~/tutorial/recorder/recording01.log",False))
# Show actors not moving 1 meter in 10 seconds.  
print(client.show_recorder_actors_blocked("~/tutorial/recorder/recording01.log",10,1))
# Filter collisions between vehicles 'v' and 'a' any other type of actor.  
print(client.show_recorder_collisions("~/tutorial/recorder/recording01.log",v,a))
```

!!! Note
    Getting detailed file info for every frame can be overwhelming. Use it after other queries to know where to look at. 

### Choose a fragment

After the queries, it is time to choose which fragment or fragments of the simulation are interesting. The method allows to choose the beginning and ending point of the playback, and an actor to follow. 

Use this time to investigate. Play different fragments, follow different actors, even play the whole recording and roam around with a free spectator view. Make sure to find the spotlights.  

```py
# --------------
# Reenact a fragment of the recording
# --------------
client.replay_file("~/tutorial/recorder/recording01.log",45,10,0) #310=ego,  237=blocked at 53 for 42
# Set the ego vehicle, as this will be needed for the rest 
ego_vehicle = world.get_actor(310)
```

!!! Note
    Set the actor_id to `0` to set the spectator free and not follow any other actor. 

### Add new sensors

The recorder will recreate in this simulation, the exact same conditions as the original. That ensures consistent data within different playbacks. Choose any other sensor and spawn it attached to the ego vehicle. 

The process is exactly the same as before, it only changes depending on the specific needs of the sensor. Take a look at the [sensor reference](ref_sensors.md) The script __tutorial_replay.py__ provides different examples that have been thoroughly explained in the [__Add advanced sensors__](#add-advanced-sensors) section. All of them are disabled by default. Enable the ones desired, and make sure to modify the output path when corresponding. 

### Change conditions

The recording will recreate the original weather conditions. However, this can be altered at will. This is really interesting to compare how does it affect data, while mantaining the rest of events the same.  

Get the current weather and modify it freely. Remember that [carla.WeatherParameters](python_api.md#carla.WeatherParameters) has some presets available. The script will change the environment to a foggy sunset. 

```py
# --------------
# Change weather for playback
# --------------
weather = world.get_weather()
weather.sun_altitude_angle = -30
weather.fog_density = 65
weather.fog_distance = 10
world.set_weather(weather)
```

### Reenact the simulation

Modify the __tutorial_replay.py__ script at will. Once everything is ready, open a terminal and run CARLA. Open another terminal and run the script. 

The recorder will play the desired fragment and then the simulation will go on. Walkers will stop, but vehicles will continue their way around the city. This may be important to recreate different scenarios with new conditions, and get new results.  

```sh
# Terminal 01
cd /opt/carla/bin
./CarlaUE4.sh
```
```sh
# Terminal 02
python tuto_replay.py
```

---
## Tutorial scripts

Hereunder are the two scripts gathering the fragments of code for thist tutorial. Most of the code is commented, as it is meant to be modified to fit specific purposes.

<details>
<summary><b>tutorial_ego.py</b> </summary>

```py

```
</details>
<br>
<details>
<summary><b>tutorial_replay.py</b></summary>

```py

```
</details>
<br>

---
That is a wrap on how to properly retrieve data from the simulation. Make sure to play around, change the conditions of the simulator, experiment with sensor settings. The possibilities are endless. 


Visit the forum to post any doubts or suggestions that have come to mind during this reading.  

<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="CARLA forum">
CARLA forum</a>
</p>
</div>
</div>