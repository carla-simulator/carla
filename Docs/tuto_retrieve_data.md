# Retrieve simulation data

Learning an efficient way to retrieve simulation data is essential in CARLA. This tutorial is advised for both, newcomers and more experienced users. It starts from the very beginning, creating a simulation with custom conditions, and spawning an ego vehicle with a sensor. Then it gradually dives into the recorder, how to query it, manage other sensors, and much more. 

* [__Overview__](#overview)  
* [__Set the simulation__](#set-the-simulation)  
	* Map setting
	* Weather setting
	* AI actor setting
* [__Set the ego vehicle__](#set-the-ego-vehicle)  
	* Spawn the ego vehicle
	* Spawn a sensor
	* Place the spectator
* [__Record and retrieve data__](#record-and-retrieve-data)  
	* Start recording
	* Capture data
	* Stop recording
* [__Exploit the recording__](#exploit-the-recording)  
	* Query the recording
	* Reenact 
	* Adding new sensors
	* Changing conditions

---
## Overview

There are some common mistakes in the process of retrieving simulation data, such as flooding the simulator with sensors, storing useless data, or trying too hard to find a specific event. However, there is a proper path to follow in order to get a simulation ready, so that data can be replicated, examined and altered at will.  

This tutorial is all about making things simple and going straight to the point. Here is a list of the scripts that will be used. All of them are already provided in CARLA.  

* __config.py__ to choose the town map.  
	* `carla/PythonAPI/utils/config.py`
* __dynamic_weather.py__ to create interesting weather conditions.  
	* `carla/PythonAPI/examples/config.py`
* __spawn_npc.py__ to spawn some AI controlled vehicles and walkers.  
	* `carla/PythonAPI/examples/config.py`
* __tutorial_ego.py__ to spawn an ego vehicle with a sensor attached, and let it roam around while recording.  
	* ` `
* __tutorial_replay.py__ to reenact the recording and play with it.  
	* ` `

!!! Important
    This tutorial uses the [__CARLA 0.9.8 deb package__](start_quickstart.md). There may be changes depending on your CARLA version and installation, specially regarding paths.

---
## Set the simulation

The first thing to do is set the simulation ready to a desired environment.  

Run CARLA. 

```sh
cd /opt/carla/bin
./CarlaUE.sh
```

### Map setting

Choose a map for the simulation to run. Take a look at the [map documentation](core_map.md#carla-maps) to learn more about their specific attributes. For the sake of this tutorial, __Town07__ is chosen due to the rural setting. 

Open a new terminal. Change the map using the __config.py__ script. 

```
cd /opt/carla/PythonAPI/utils
./config.py --map Town07
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


### AI actor setting

Now let's spawn some life into the town. 

Open a new terminal, and run __spawn_npc.py__ to spawn vehicles and walkers. Let's just spawn 50 vehicles and the same amount of walkers. 

```sh
cd /opt/carla/PythonAPI/examples
./spawn_npc.py -n 50 --safe
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

The ego vehicle is usually the main actor of the simulation. Use any of the vehicle blueprints in the library. To differenciate it from the rest of vehicles, the attribute `role_name` is set to `ego`. There are other attributes that can be set, some with recommended values. In this case, the color will be changed at random to one of these. 

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

### Spawn a sensor

Use the library to find a sensor blueprint. This tutorial will first use the [__RGB camera__](ref_sensors.md#rgb-camera), which generates retrieves realistic shots of the scene.  

The sensor will be attached to the ego vehicle at spawning. That means that its transform is __relative to its parent__. The script locates the camera in the hood of the car, and pointing forward. It will capture the front view of the car. The `AttachmentType.SpringArm` will make that the camera position updates smooth, with little eases regarding its parent.  

The key element is the `listen()` method. The [__lambda__](https://www.w3schools.com/python/python_lambda.asp) method will be called each time the sensor listens for data in the simulation. The argument `image` is the sensor data retrieved, in this case, a [carla.Image](python_api.md#carla.Image). The script calls for it, and saves the image to disk. The path can be altered at will, and the name of each file is based on the simulation frame stated in the image.  

```py
# --------------
# Spawn attached RGB camera
# --------------
cam_bp = world.get_blueprint_library().find('sensor.camera.rgb')
cam_location = carla.Location(2,0,1)
ego_forward = ego_vehicle.get_transform().get_forward_vector()
cam_rotation = carla.Rotation(0,180,0)
cam_transform = carla.Transform(cam_location,cam_rotation)
ego_cam = world.spawn_actor(cam_bp,cam_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.SpringArm)
ego_cam.listen(lambda image: image.save_to_disk('/home/user/Desktop/tutorial/output/%.6d.png' % image.frame))
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
## Record and retrieve data

### Start recording

The [__recorder__](adv_recorder.md) can be started at anytime. The script does it at the very beginnning, in order to capture everything, including the spawning of the first actors. If no path is detailed, the log will be saved into `CarlaUE4/Saved`. 

```py
# --------------
# Start recording
# --------------
client.start_recorder('/home/user/Desktop/tutorial/recorder/recording01.log')
```

### Capture data

It is time to set the ego vehicle free. It could be manually controlled using `/PythonAPI/examples/manual_control.py`. However, the script uses the [__Traffic manager__](adv_traffic_manager.md) to move the vehicle around the map automatically, as the rest of vehicles. This saves the user having to worry about driving according to traffic regulations.  

Enable the autopilot mode. As other autopilot vehicles have been previously spawned using __spawn_npc.py__, this script will client a __TM-Client__ that will connect to the __TM-Server__ created by __spawn_npc.py__. This is transparent to the user, but noted here for the sake of understanding. The Traffic Manager documentation dives deep into this matter. 

Creating a loop will prevent the script from finishing until the user commands via terminal. The recorder will continue until then. Let the simulation run for a while, depending on the amount of data desired.  

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

Use `Ctrl+C` or quit the terminal. The script will stop the recorder, destroy the sensor and the ego vehicle, and finish.  

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
## Exploit the recording

So far, a simulation has been set running. An ego vehicle roamed around for a while while retrieving data, and the whole simulation has been recorded. Make sure to have the data retrieved by the sensor and the log of the recording. Look inside the folders detailed in the __tutorial_ego.py__.  

Close the simulation and any script runnning. It is time to dive into the last script, __tutorial_replay.py__. This script is subject to change, so it contains different segments of code commented for different functionalities.  

### Query the events

The different queries are detaile in the [__recorder documentation__](adv_recorder.md). In summary, they retrieve different things.  

* A log of the most important events in the recording or in every frame. 
* A log of the actors blocked. Those that do not move a minimum distance in a certain time. 
* A log of the collisions registered by [collision sensors](ref_sensors.md#collision-detector). 

Use these to study the recording and find moments of remarkable interest. The file info is also useful to identify the ego vehicle, or any actor, with its ID.  

```py
# --------------
# Query the recording
# --------------
# Show only the most important events in the recording.  
print(client.show_recorder_file_info("/home/adas/Desktop/tutorial/recorder/recording01.log",False))
# Show actors not moving 1 meter in 10 seconds.  
print(client.show_recorder_actors_blocked("/home/adas/Desktop/tutorial/recorder/recording01.log",10,1))
# Show collisions between any type of actor.  
print(client.show_recorder_collisions("/home/adas/Desktop/tutorial/recorder/recording01.log",v,a))
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
client.replay_file("/home/adas/Desktop/tutorial/recorder/recording01.log",45,10,0) #310=ego,  237=blocked at 53 for 42
# Set the ego vehicle, as this will be needed for the rest 
ego_vehicle = world.get_actor(310)
```

!!! Note
    Set the actor_id to `0` to set the spectator free and not follow any other actor. 

### Add new sensors

It is time to retrieve more data, as much as desired. Choose any other sensor and spawn it attached to the ego vehicle. The recording ensures the same conditions as the original simulation, so the information will be consistent between different playbacks.  

The process is exactly the same as before, it only changes depending on the specific needs of teh sensor. Take a look at the [sensor reference]. The script provides as example a semantic segmentation camera. 

```py
# --------------
# Add a new semantic sensor to my ego
# --------------
sem_cam = None
sem_bp = world.get_blueprint_library().find('sensor.camera.semantic_segmentation')
sem_location = carla.Location(2,0,1)
sem_forward = ego_vehicle.get_transform().get_forward_vector()
sem_rotation = carla.Rotation(0,180,0)
sem_transform = carla.Transform(sem_location,sem_rotation)
sem_cam = world.spawn_actor(sem_bp,sem_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.SpringArm)
# This time, a color converter is applied to the image, to get the semantic segmentation view
sem_cam.listen(lambda image: image.save_to_disk('/home/user/Desktop/tutorial/rec_sem_output/%.6d.png' % image.frame,carla.ColorConverter.CityScapesPalette))
```

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