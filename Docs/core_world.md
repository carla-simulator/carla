# 1st. World and client

This is bound to be one of the first topics to learn about when entering CARLA. The client and the world are two of the fundamentals of CARLA, a necessary abstraction to operate the simulation and its actors.  
This tutorial goes from defining the basics and creation of these elements to describing their possibilities without entering into higher complex matters. If any doubt or issue arises during the reading, the [CARLA forum](forum.carla.org/) is there to solve them.  

  * [__The client__](#the-client):
	* Client creation
	* World connection 
	* Other client utilities
  * [__The world__](#the-world):
	* World life cycle   
	* Get() from the world
	* Weather 
	* World snapshots 
	* Settings  

---
## The client

Clients are one of the main elements in the CARLA architecture. Using these, the user can connect to the server, retrieve information from the simulation and command changes. That is done via scripts where the client identifies itself and connects to the world to then operate with the simulation.  
Besides that, the client is also able to access other CARLA modules, features and apply command batches. Command batches are relevant to this moment of the documentation, as they are useful as soon as spawning actors is required. The rest though are more advanced parts of CARLA and they will not be covered yet in this section.  
The __carla.Client__ class is explained thoroughly in the [PythonAPI reference](python_api.md#carla.Client). 


#### Client creation

Two things are needed: The IP address identifying it and two TCP ports the client will be using to communicate with the server. There is an optional third parameter, an `int` to set the working threads that by default is set to all (`0`). [This code recipe](ref_code_recipes.md#parse-client-creation-arguments) shows how to parse these as arguments when running the script. 

```py
client = carla.Client('localhost', 2000)
```
By default, CARLA uses local host and port 2000 to connect but these can be changed at will. The second port will always be `n+1` (in this case, 2001).  

Once the client is created, set its time-out. This limits all networking operations so that these don't block forever the client but return an error instead if connection fails. 

```py
client.set_timeout(10.0) # seconds
```

It is possible to have many clients connected, as it is common to have more than one script running at a time. Just take note that working in a multiclient scheme with more advanced CARLA features such as the traffic manager or the synchronous mode is bound to make communication more complex.  

!!! Note
    Client and server have different `libcarla` modules. If the versions differ due to different origin commits, issues may arise. This will not normally the case, but it can be checked using the `get_client_version()` and `get_server_version()` methods. 

#### World connection

Being the simulation running, a configured client can connect and retrieve the current world easily: 

```py
world = client.get_world()
```
Using `reload_world()` the client creates a new instance of the world with the same map. Kind of a reboot method.  
The client can also get a list of available maps to change the current one. This will destroy the current world and create a new one.
```py
print(client.get_available_maps())
...
world = client.load_world('Town01')
```

Every world object has an `id` or episode. Everytime the client calls for `load_world()` or `reload_world()` the previous one is destroyed and the new one is created from from scratch without rebooting Unreal Engine, so this episode will change. 

#### Other client utilities

The main purpose of the client object is to get or change the world and many times, it is no longer used after that. However, this object is in charge of two other main tasks: accessing to advanced CARLA features and applying command batches.  
The list of features that are accessed from the client object are:  

* __Traffic manager:__ this module is in charge of every vehicle set to autopilot to recreate an urban environment. 
* __[Recorder](adv_recorder.md):__ allows to reenact a previous simulation using the information stored in the [snapshots]() summarizing the simulation state per frame. 

As far as batches are concerned, the latest sections in the Python API describe the [available commands](python_api.md#command.ApplyAngularVelocity). These are common functions that have been prepared to be executed in batches or lots so that they are applied during the same step of the simulation.  
The following example would destroy all the vehicles contained in `vehicles_list` at once: 
```py
client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])
```

The method `apply_batch_sync()` is only available when running CARLA in [synchronous mode]() and allows to return a __command.Response__ per command applied.

---
## The world

This class acts as the major ruler of the simulation and its instance should be retrieved by the client. It does not contain the model of the world itself (that is part of the [Map](core_map.md) class), but rather is an anchor for the simulation. Most of the information and general settings can be accessed from this class, for example: 

* Actors and the spectator. 
* Blueprint library. 
* Map. 
* Settings. 
* Snapshots. 

In fact, some of the most important methods of this class are the _getters_. They summarize all the information the world has access to. More explicit information regarding the World class can be found in the [Python API reference](python_api.md#carla.World).

#### Actors

The world has different methods related with actors that allow it to:  

* Spawn actors (but not destroy them). 
* Get every actor on scene or find one in particular.  
* Access the blueprint library used for spawning these.  
* Access the spectator actor that manages the simulation's point of view.  
* Retrieve a random location that is fitting to spawn an actor.  

Explanations on spawning will be conducted in the second step of this guide: [2nd. Actors and blueprints](core_actors.md), as it requires some understanding on the blueprint library, attributes, etc. Keep reading or visit the [Python API reference](python_api.md) to learn more about this matter. 

#### Weather

The weather is not a class on its own, but a world setting. However, there is a helper class named [carla.WeatherParameters](python_api.md#carla.WeatherParameters) that allows to define a series of visual characteristics such as sun orientation, cloudiness, lightning, wind and much more. The changes can then be applied using the world as the following example does:  
```py
weather = carla.WeatherParameters(
    cloudiness=80.0,
    precipitation=30.0,
    sun_altitude_angle=70.0)

world.set_weather(weather)

print(world.get_weather())
```

For convenience, there are a series of weather presets that can be directly applied to the world. These are listed in the [Python API reference](python_api.md#carla.WeatherParameters) with all the information regarding the class and are quite straightforward to use: 

```py
world.set_weather(carla.WeatherParameters.WetCloudySunset)
```

!!! Note
    Changes in the weather do not affect physics. They are only visuals that can be captured by the camera sensors. 

#### Debugging

World objects have a public attribute that defines a [carla.DebugHelper](python_api.md#carla.DebugHelper) object. It allows for different shapes to be drawn during the simulation in order to trace the events happening. The following example would access the attribute to draw a red box at an actor's location and rotation. 

```py
debug = world.debug
debug.draw_box(carla.BoundingBox(actor_snapshot.get_transform().location,carla.Vector3D(0.5,0.5,2)),actor_snapshot.get_transform().rotation, 0.05, carla.Color(255,0,0,0),0)
```

This example is extended in this [code recipe](ref_code_recipes.md#debug-bounding-box-recipe) to draw boxes for every actor in a world snapshot. Take a look at it and at the Python API reference to learn more about this.  

#### World snapshots

Contains the state of every actor in the simulation at a single frame, a sort of still image of the world with a time reference. This feature makes sure that all the information contained comes from the same simulation step without the need of using synchronous mode. 

```py
# Retrieve a snapshot of the world at current frame.
world_snapshot = world.get_snapshot()
```

The [carla.WorldSnapshot](python_api.md#carla.WorldSnapshot) contains a [carla.Timestamp](python_api.md#carla.Timestamp) and a list of [carla.ActorSnapshot](python_api.md#carla.ActorSnapshot). Actor snapshots can be searched using the `id` of an actor and the other way round, the actor regarding a snapshot is facilitated by the `id` in the actor snapshot. 

```py
timestamp = world_snapshot.timestamp #Get the time reference 

for actor_snapshot in world_snapshot: #Get the actor and the snapshot information
    actual_actor = world.get_actor(actor_snapshot.id)
    actor_snapshot.get_transform()
    actor_snapshot.get_velocity()
    actor_snapshot.get_angular_velocity()
    actor_snapshot.get_acceleration()  

actor_snapshot = world_snapshot.find(actual_actor.id) #Get an actor's snapshot
```

#### World settings

The world also has access to some advanced configurations for the simulation that determine rendering conditions, steps in the simulation time and synchrony between clients and server. These are advanced concepts that do better if untouched by newcomers.  
For the time being let's say that CARLA by default runs in with its best quality, with a variable time-step and asynchronously. The helper class is [carla.WorldSettings](python_api.md#carla.WorldSettings). To dive further in this matters take a look at the __Advanced steps__ section of the documentation and read about [synchrony and time-step](adv_synchrony_timestep.md) or [rendering_options.md](adv_rendering_options.md).

---
That is a wrap on the world and client objects, the very first steps in CARLA.  
The next step should be learning more about actors and blueprints to give life to the simulation. Keep reading to learn more or visit the forum to post any doubts or suggestions that have come to mind during this reading: 
<div text-align: center>
<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="forum.carla.org" target="_blank" class="btn btn-neutral" title="CARLA forum">
CARLA forum</a>
</p>
</div>
<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="../core_actors" target="_blank" class="btn btn-neutral" title="2nd. Actors and blueprints">
2nd. Actors and blueprints</a>
</p>
</div>
</div>
