# 1st. World and client

The client and the world are two of the fundamentals of CARLA, a necessary abstraction to operate the simulation and its actors.  

This tutorial goes from defining the basics and creation of these elements, to describing their possibilities. If any doubt or issue arises during the reading, the [CARLA forum](https://github.com/carla-simulator/carla/discussions/) is there to solve them.  

*   [__The client__](#the-client)  
	*   [Client creation](#client-creation)  
	*   [World connection](#world-connection)  
	*   [Other client utilities](#other-client-utilities)  
*   [__The world__](#the-world)  
	*   [Actors](#actors)  
	*   [Weather](#weather)  
	*   [Lights](#lights)  
	*   [Debugging](#debugging)  
	*   [World snapshots](#world-snapshots)  
	*   [World settings](#world-settings)  

---
## The client

Clients are one of the main elements in the CARLA architecture. They connect to the server, retrieve information, and command changes. That is done via scripts. The client identifies itself, and connects to the world to then operate with the simulation.  

Besides that, clients are able to access advanced CARLA modules, features, and apply command batches. Only command batches will be covered in this section. These are useful for basic things such as spawning lots of actors. The rest of features are more complex, and they will be addressed in their respective pages in __Advanced steps__.  

Take a look at [__carla.Client__](python_api.md#carla.Client) in the Python API reference to learn on specific methods and variables of the class. 


### Client creation

Two things are needed. The __IP__ address identifying it, and __two TCP ports__ to communicate with the server. An optional third parameter sets the amount of working threads. By default this is set to all (`0`). The [carla.Client](python_api.md#carla.Client.__init__) in the Python API reference contains a snipet that shows how to parse these as arguments when running the script. 

```py
client = carla.Client('localhost', 2000)
```
By default, CARLA uses local host IP, and port 2000 to connect but these can be changed at will. The second port will always be `n+1`, 2001 in this case.  

Once the client is created, set its __time-out__. This limits all networking operations so that these don't block the client forever. An error will be returned if connection fails. 

```py
client.set_timeout(10.0) # seconds
```

It is possible to have many clients connected, as it is common to have more than one script running at a time. Working in a multiclient scheme with advanced CARLA features, such as the traffic manager, is bound to make communication more complex.  

!!! Note
    Client and server have different `libcarla` modules. If the versions differ, issues may arise. This can be checked using the `get_client_version()` and `get_server_version()` methods. 

### World connection

A client can connect and retrieve the current world fairly easily. 

```py
world = client.get_world()
```

The client can also get a list of available maps to change the current one. This will destroy the current world and create a new one.
```py
print(client.get_available_maps())
...
world = client.load_world('Town01')
# client.reload_world() creates a new instance of the world with the same map. 
```

Every world object has an `id` or episode. Everytime the client calls for `load_world()` or `reload_world()` the previous one is destroyed. A new one is created from scratch with a new episode. Unreal Engine is not rebooted in the process. 

### Using commands

__Commands__ are adaptations of some of the most-common CARLA methods, that can be applied in batches. For instance, the [command.SetAutopilot](python_api.md#command.SetAutopilot) is equivalent to [Vehicle.set_autopilot()](python_api.md#carla.Vehicle.set_autopilot), enables the autopilot for a vehicle. However, using the methods [Client.apply_batch](python_api.md#carla.Client.apply_batch) or [Client.apply_batch_sync()](python_api.md#carla.Client.apply_batch_sync), a list of commands can be applied in one single simulation step. This becomes extremely useful for methods that are usually applied to even hundreds of elements.  

The following example uses a batch to destroy a list of vehicles all at once.  

```py
client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])
```

All the commands available are listed in the [latest section](python_api.md#command.ApplyAngularVelocity) of the Python API reference.  

### Other client utilities

The main purpose of the client object is to get or change the world, and apply commands. However, it also provides access to some additional features.  

*   __Traffic manager.__ This module is in charge of every vehicle set to autopilot to recreate urban traffic.  
*   __[Recorder](adv_recorder.md).__ Allows to reenact a previous simulation. Uses [snapshots](core_world.md#world-snapshots) summarizing the simulation state per frame.  

---
## The world

The major ruler of the simulation. Its instance should be retrieved by the client. It does not contain the model of the world itself, that is part of the [Map](core_map.md) class. Instead, most of the information, and general settings can be accessed from this class.

*   Actors in the simulation and the spectator.  
*   Blueprint library.  
*   Map.  
*   Simulation settings.  
*   Snapshots.  
*   Weather and light manager.  

Some of its most important methods are _getters_, precisely to retrieve information or instances of these elements. Take a look at [carla.World](python_api.md#carla.World) to learn more about it.  

### Actors

The world has different methods related with actors that allow for different functionalities.  

*   Spawn actors (but not destroy them). 
*   Get every actor on scene, or find one in particular.  
*   Access the blueprint library.  
*   Access the spectator actor, the simulation's point of view.  
*   Retrieve a random location that is fitting to spawn an actor.  

Spawning will be explained in [2nd. Actors and blueprints](core_actors.md). It requires some understanding on the blueprint library, attributes, etc.  

### Weather

The weather is not a class on its own, but a set of parameters accessible from the world. The parametrization includes sun orientation, cloudiness, wind, fog, and much more. The helper class [carla.WeatherParameters](python_api.md#carla.WeatherParameters) is used to define a custom weather.  
```py
weather = carla.WeatherParameters(
    cloudiness=80.0,
    precipitation=30.0,
    sun_altitude_angle=70.0)

world.set_weather(weather)

print(world.get_weather())
```

There are some weather presets that can be directly applied to the world. These are listed in [carla.WeatherParameters](python_api.md#carla.WeatherParameters) and accessible as an enum.  

```py
world.set_weather(carla.WeatherParameters.WetCloudySunset)
```

The weather can also be customized using two scripts provided by CARLA. 

*   __`environment.py`__ *(in `PythonAPI/util`)* — Provides access to weather and light parameters so that these can be changed in real time.  

<details>
<summary> Optional arguments in <b>environment.py</b> </summary>

```sh
  -h, --help            show this help message and exit
  --host H              IP of the host server (default: 127.0.0.1)
  -p P, --port P        TCP port to listen to (default: 2000)
  --sun SUN             Sun position presets [sunset | day | night]
  --weather WEATHER     Weather condition presets [clear | overcast | rain]
  --altitude A, -alt A  Sun altitude [-90.0, 90.0]
  --azimuth A, -azm A   Sun azimuth [0.0, 360.0]
  --clouds C, -c C      Clouds amount [0.0, 100.0]
  --rain R, -r R        Rain amount [0.0, 100.0]
  --puddles Pd, -pd Pd  Puddles amount [0.0, 100.0]
  --wind W, -w W        Wind intensity [0.0, 100.0]
  --fog F, -f F         Fog intensity [0.0, 100.0]
  --fogdist Fd, -fd Fd  Fog Distance [0.0, inf)
  --wetness Wet, -wet Wet
                        Wetness intensity [0.0, 100.0]
```
</details><br>

*   __`dynamic_weather.py`__ *(in `PythonAPI/examples`)* — Enables a particular weather cycle prepared by developers for each CARLA map.  

<details>
<summary> Optional arguments in <b>dynamic_weather.py</b> </summary>

```sh
  -h, --help            show this help message and exit
  --host H              IP of the host server (default: 127.0.0.1)
  -p P, --port P        TCP port to listen to (default: 2000)
  -s FACTOR, --speed FACTOR
                        rate at which the weather changes (default: 1.0)
```
</details><br>

!!! Note
    Changes in the weather do not affect physics. They are only visuals that can be captured by the camera sensors. 

__Night mode starts when sun_altitude_angle < 0__, which is considered sunset. This is when lights become especially relevant.  

### Lights

*   __Street lights__ automatically turn on when the simulation enters night mode. The lights are placed by the developers of the map, and accessible as [__carla.Light__](python_api.md#carla.Light) objects. Properties such as color and intensity can be changed at will. The variable __light_state__ of type [__carla.LightState__](python_api.md#carla.LightState) allows setting all of these in one call.  
Street lights are categorized using their attribute __light_group__, of type [__carla.LightGroup__](python_api.md#carla.LightGroup). This allows to classify lights as street lights, building lights... An instance of [__carla.LightManager__](python_api.md#carla.LightManager) can be retrieved to handle groups of lights in one call.  

```py
# Get the light manager and lights
lmanager = world.get_lightmanager()
mylights = lmanager.get_all_lights()

# Custom a specific light
light01 = mylights[0]
light01.turn_on()
light01.set_intensity(100.0)
state01 = carla.LightState(200.0,red,carla.LightGroup.Building,True)
light01.set_light_state(state01)

# Custom a group of lights
my_lights = lmanager.get_light_group(carla.LightGroup.Building)
lmanager.turn_on(my_lights)
lmanager.set_color(my_lights,carla.Color(255,0,0))
lmanager.set_intensities(my_lights,list_of_intensities)
```

* __Vehicle lights__ have to be turned on/off by the user. Each vehicle has a set of lights listed in [__carla.VehicleLightState__](python_api.md#carla.VehicleLightState). So far, not all vehicles have lights integrated. Here is a list of those that are available by the time of writing.  
	*   __Bikes.__ All of them have a front and back position light.  
	*   __Motorcycles.__ Yamaha and Harley Davidson models.  
	*   __Cars.__ Audi TT, Chevrolet, Dodge (the police car), Etron, Lincoln, Mustang, Tesla 3S, Wolkswagen T2 and the new guests coming to CARLA.  

The lights of a vehicle can be retrieved and updated anytime using the methods [carla.Vehicle.get_light_state](python_api.md#carla.Vehicle.get_light_state) and [carla.Vehicle.set_light_state](#python_api.md#carla.Vehicle.set_light_state). These use binary operations to customize the light setting.  

```py
# Turn on position lights
current_lights = carla.VehicleLightState.NONE
current_lights |= carla.VehicleLightState.Position
vehicle.set_light_state(current_lights)
```

!!! Note
    Lights can also be set in real time using the `environment.py` described in the [weather](#weather) section.  

### Debugging

World objects have a [carla.DebugHelper](python_api.md#carla.DebugHelper) object as a public attribute. It allows for different shapes to be drawn during the simulation. These are used to  trace the events happening. The following example would draw a red box at an actor's location and rotation. 

```py
debug = world.debug
debug.draw_box(carla.BoundingBox(actor_snapshot.get_transform().location,carla.Vector3D(0.5,0.5,2)),actor_snapshot.get_transform().rotation, 0.05, carla.Color(255,0,0,0),0)
```

This example is extended in a snipet in [carla.DebugHelper](python_api.md#carla.DebugHelper.draw_box) that shows how to draw boxes for every actor in a world snapshot. 

### World snapshots

Contains the state of every actor in the simulation at a single frame. A sort of still image of the world with a time reference. The information comes from the same simulation step, even in asynchronous mode.  

```py
# Retrieve a snapshot of the world at current frame.
world_snapshot = world.get_snapshot()
```

A [carla.WorldSnapshot](python_api.md#carla.WorldSnapshot) contains a [carla.Timestamp](python_api.md#carla.Timestamp) and a list of [carla.ActorSnapshot](python_api.md#carla.ActorSnapshot). Actor snapshots can be searched using the `id` of an actor. A snapshot lists the `id` of the actors appearing in it.  

```py
timestamp = world_snapshot.timestamp # Get the time reference 

for actor_snapshot in world_snapshot: # Get the actor and the snapshot information
    actual_actor = world.get_actor(actor_snapshot.id)
    actor_snapshot.get_transform()
    actor_snapshot.get_velocity()
    actor_snapshot.get_angular_velocity()
    actor_snapshot.get_acceleration()  

actor_snapshot = world_snapshot.find(actual_actor.id) # Get an actor's snapshot
```

### World settings

The world has access to some advanced configurations for the simulation. These determine rendering conditions, simulation time-steps, and synchrony between clients and server. They are accessible from the helper class [carla.WorldSettings](python_api.md#carla.WorldSettings).  

For the time being, default CARLA runs with the best graphics quality, a variable time-step, and asynchronously. To dive further in this matters take a look at the __Advanced steps__ section. The pages on [synchrony and time-step](adv_synchrony_timestep.md), and [rendering options](adv_rendering_options.md) could be a great starting point.

---
That is a wrap on the world and client objects. The next step takes a closer look into actors and blueprints to give life to the simulation.  

Keep reading to learn more. Visit the forum to post any doubts or suggestions that have come to mind during this reading.  

<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA forum">
CARLA forum</a>
</p>
</div>
<div class="build-buttons">
<p>
<a href="../core_actors" target="_blank" class="btn btn-neutral" title="2nd. Actors and blueprints">
2nd. Actors and blueprints</a>
</p>
</div>
</div>
