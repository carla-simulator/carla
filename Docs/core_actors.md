# 2nd. Actors and blueprints

The actors in CARLA include almost everything playing a role the simulation. That includes not only vehicles and walkers but also sensors, traffic signs, traffic lights and the spectator, the camera providing the simulation's point of view. They are crucial, and so it is to have fully understanding on how to operate on them.  
This section will cover the basics: from spawning up to destruction and their different types. However, the possibilities they present are almost endless. This is the first step to then experiment, take a look at the __How to's__ in this documentation and share doubts and ideas in the [CARLA forum](https://forum.carla.org/).  

  * [__Blueprints__](#blueprints)  
	* Managing the blueprint library  
  * [__Actor life cycle__](#actor-life-cycle):  
	* Spawning  
	* Handling  
	* Destruction  
  * [__Types of actors__](#types-of-actors):
	* Sensors  
	* Spectator  
    * Traffic signs and traffic lights
    * Vehicles
    * Walkers

---
## Blueprints

This layouts allow the user to smoothly add new actors into the simulation. They basically are already-made models with a series of attributes listed, some of which are modifiable and others are not: vehicle color, amount of channels in a lidar sensor, _fov_ in a camera, a walker's speed. All of these can be changed at will. All the available blueprints are listed in the [blueprint library](bp_library.md) with their attributes and a tag to identify which can be set by the user.  

#### Managing the blueprint library

There is a [carla.BlueprintLibrary](python_api.md#carla.BlueprintLibrary) class containing a list of [carla.ActorBlueprint](python_api.md#carla.ActorBlueprint) elements. It is the world object who can provide access to an instance of it:
```py
blueprint_library = world.get_blueprint_library()
```
Each blueprints has its own ID, useful to identify it and the actors spawned using it. The library can be read to find a certain ID, choose randomly or filter results using a [wildcard pattern](https://tldp.org/LDP/GNU-Linux-Tools-Summary/html/x11655.htm):

```py
# Find a specific blueprint.
collision_sensor_bp = blueprint_library.find('sensor.other.collision')
# Choose a vehicle blueprint at random.
vehicle_bp = random.choice(blueprint_library.filter('vehicle.*.*'))
```

Besides that, each [carla.ActorBlueprint](python_api.md#carla.ActorBlueprint) has a series of [carla.ActorAttribute](python_api.md#carla.ActorAttribute) that can be _get_, _set_, and checked if existing: 
```py
is_bike = [vehicle.get_attribute('number_of_wheels') == 2]
if(is_bike)
    vehicle.set_attribute('color', '255,0,0')
```
!!! Note
    Some of the attributes cannot be modified. Check it out in the [blueprint library](bp_library.md).

Attributes have a helper class [carla.ActorAttributeType](python_api.md#carla.ActorAttributeType) which defines possible types as enums. Also, modifiable attributes come with a __list of recommended values__: 

```py
for attr in blueprint:
    if attr.is_modifiable:
        blueprint.set_attribute(attr.id, random.choice(attr.recommended_values))
```
!!! Note
    Users can create their own vehicles, take a look at the tutorials in __How to... (content)__ to learn on that. Contributors can [add their new content to CARLA](tuto_D_contribute_assets.md). 

---
## Actor life cycle  

!!! Important
    All along this section, many different functions and methods regarding actors will be covered. The Python API provides for __[commands](python_api.md#command.SpawnActor)__ to apply batches of this common functions (such as spawning or destroying actors) in just one frame. 

#### Spawning

The world object is responsible of spawning actors and keeping track of those who are currently on scene. Besides a blueprint, only a [carla.Transform](python_api.md#carla.Transform), which basically defines a location and rotation for the actor. 

```py
transform = Transform(Location(x=230, y=195, z=40), Rotation(yaw=180))
actor = world.spawn_actor(blueprint, transform)
```

In case of collision at the specified location, the actor will not spawn. This may happen when trying to spawn inside a static object, but also if there is another actor currently at said point. The world has two different methods to spawn actors: [`spawn_actor()`](python_api.md#carla.World.spawn_actor) and [`try_spawn_actor()`](python_api.md#carla.World.try_spawn_actor).
The former will raise an exception if the actor could not be spawned, the later will return `None` instead.

To try to avoid this, the world can ask the map for a list of recommended transforms to act as spawning points __for vehicles__: 

```py
spawn_points = world.get_map().get_spawn_points()
```

Quite the same works goes __for walkers__, but this time the world can get a random point on a sidewalk (this same method is used to set goal locations for walkers):

```py
spawn_point = carla.Transform()
spawn_point.location = world.get_random_location_from_navigation()
```

Finally, an actor can be attached to another one when spawned, meaning it will follow the parent object around. This is specially useful for sensors. The attachment can be rigid or smooth, as defined by the helper class [carla.AttachmentType](python_api.md#carla.AttachmentType).  
The next example attaches a camera rigidly to a vehicle, so their relative position remains fixed. 

```py
camera = world.spawn_actor(camera_bp, relative_transform, attach_to=my_vehicle)
```
!!! Note
    When spawning attached actors, the transform provided must be relative to the parent actor. 

Once spawned, the world object adds the actors to a list, keeping track of the current state of the simulation. This list can be iterated on or searched easily: 
```py
actor_list = world.get_actors()
# Find an actor by id.
actor = actor_list.find(id)
# Print the location of all the speed limit signs in the world.
for speed_sign in actor_list.filter('traffic.speed_limit.*'):
    print(speed_sign.get_location())
```

#### Handling

Once an actor si spawned, handling is quite straightforward. The [carla.Actor](python_api.md#carla.Actor) class mostly consists of _get()_ and _set()_ methods to manage the actors around the map:

```py
location = actor.get_location()
location.z += 10.0
actor.set_location(location)
print(actor.get_acceleration())
print(actor.get_velocity())
```

The actor's physics can be disabled to freeze it in place. 

```py
actor.set_simulate_physics(False)
```
Besides that, actors also have tags provided by their blueprints that are mostly useful for semantic segmentation sensors, though this will be covered later on this documentation. 

!!! Important
    Most of the methods send requests to the simulator asynchronously. The simulator queues these, but has a limited amount of time each update to parse them. Flooding the simulator with lots of "set" methods will accumulate a significant lag.


#### Destruction

To remove an actor from the simulation, an actor can get rid of itself and notify if successful doing so:

```py
destroyed_sucessfully = actor.destroy()
```

Actors are not destroyed when the Python script finishes, they remain and the world keeps track of them until they are explicitly destroyed.

!!! Important
    Destroying an actor blocks the simulator until the process finishes. 

---
## Types of actors  
#### Sensors

Sensors are actors that produce a stream of data. They are so important and vast that they will be properly written about on their own section: [4th. Sensors and data](core_sensors.md). 
So far, let's just take a look at a common sensor spawning routine: 

```py
camera_bp = blueprint_library.find('sensor.camera.rgb')
camera = world.spawn_actor(camera_bp, relative_transform, attach_to=my_vehicle)
camera.listen(lambda image: image.save_to_disk('output/%06d.png' % image.frame))
```
This example spawns a camera sensor, attaches it to a vehicle, and tellds the camera to save to disk each of the images that is going to generate. Three main highlights here:  

* They have blueprints too. Each sensor works differently so setting attributes is especially important.  
* Most of the sensors will be attached to a vehicle to gather information on its surroundings. 
* Sensors __listen__ to data. When receiving it, they call a function described with __[Lambda expressions](https://docs.python.org/3/reference/expressions.html)__, so it is advisable to learn on that beforehand <small>(6.13 in the link provided)</small>. 

#### Spectator

This unique actor is the one element placed by Unreal Engine to provide an in-game point of view. It can be used to move the view of the simulator window.

#### Traffic signs and traffic lights

So far, CARLA only is aware of some signs: stop, yield and speed limit, described with [carla.TrafficSign](python_api.md#carla.TrafficSign). Traffic lights, which are considered an inherited class named [carla.TrafficLight](python_api.md#carla.TrafficLight). __These cannot be found on the blueprint library__ and thus, cannot be spawned. They set traffic conditions and so, they are mindfully placed by developers. 

Traffic signs are not defined in the road map itself (more on that in the following page). Instead, they have a [carla.BoundingBox](python_api.md#carla.BoundingBox) that triggers when a vehicle is inside of it and thus, affected by the traffic sign. 
```py
#Get the traffic light affecting a vehicle
if vehicle_actor.is_at_traffic_light():
    traffic_light = vehicle_actor.get_traffic_light()
``` 
!!! Note
    Traffic lights will only affect a vehicle if the light is red. 

Regarding traffic lights, they are found in junctions so they belong to a `group` and identify with a `pole` number inside the group.  
A group of traffic lights follows a cycle in which a pole is set to green, yellow and then red while the rest remain frozen in red. After spending a certain amount of seconds per state (including red, meaning there is a period of time in which all lights are red), the next pole starts its cycle and the previous one is frozen with the rest.  
State and time per state can be easily accessed using _get()_ and _set()_ methods described in said class. Possible states are described with [carla.TrafficLightState](python_api.md#carla.TrafficLightState) as a series of enum values. 
```py
#Change a red traffic light to green
if traffic_light.get_state() == carla.TrafficLightState.Red:
    traffic_light.set_state(carla.TrafficLightState.Green)
``` 

  * **Speed limit signs** with the speed codified in their type_id.

#### Vehicles

[carla.Vehicle](python_api.md#carla.Vehicle) are a special type of actor that provide some better physics control. This is achieved applying four types of different controls: 

* __[carla.VehicleControl](python_api.md#carla.VehiclePhysicsControl)__: provides input for driving commands such as throttle, steering, brake, etc. 
```py
vehicle.apply_control(carla.VehicleControl(throttle=1.0, steer=-1.0))
```
* __[carla.VehiclePhysicsControl](python_api.md#carla.VehiclePhysicsControl)__: defines many physical attributes of the vehicle from mass and drag coefficient up to torque, maximum rpm, clutch strenght and many more. These controller contains within two more controllers. One is for the gears, [carla.GearPhysicsControl](python_api.md#carla.GearPhysicsControl), the other is a list of [carla.WheelPhysicsControl](python_api.md#carla.WheelPhysicsControl) that provide specific control over the different wheels of the vehicle for a realistic result. 

```py
vehicle.apply_physics_control(carla.VehiclePhysicsControl(max_rpm = 5000.0, center_of_mass = carla.Vector3D(0.0, 0.0, 0.0), torque_curve=[[0,400],[5000,400]]))
```

Vehicles can be set to an __autopilot mode__ which will subscribe them to the Traffic manager, an advanced CARLA module that needs its own section to be fully comprehended. For now, it is enough to know that it conducts all the vehicles set to autopilot in order to simulate real urban conditions: 

```py
vehicle.set_autopilot(True)
```
!!! Note
    The traffic manager module is hard-coded, not based on machine learning.  

Finally, vehicles also have a [carla.BoundingBox](python_api.md#carla.BoundingBox) that encapsulates them and is used for collision detection: 

```py
box = vehicle.bounding_box
print(box.location)         # Location relative to the vehicle.
print(box.extent)           # XYZ half-box extents in meters.
```

#### Walkers

[carla.Walker](python_api.md#carla.Walker) are moving actors and so, work in quite a similar way as vehicles do. Control over them is provided by controllers:  

* __[carla.WalkerControl](python_api.md#carla.Walker)__: to move the pedestrian around with a certain direction and speed. It also allows them to jump. 
* __[carla.WalkerBoneControl](python_api.md#carla.Walker)__: provides control over the specific bones of the 3D model. The skeleton structure and how to control it is summarized in this __[How to](tuto_G_control_walker_skeletons.md)__. 

Walkers can be AI controlled. They do not have an autopilot mode, but there is another actor, [carla.WalkerAIController](python_api.md#carla.WalkerAIController) that, when spawned attached to a walker, can move them around: 

```py
walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
world.SpawnActor(walker_controller_bp, carla.Transform(), parent_walker)
```
The AI controller is bodiless, so it will not appear on scene. Passing location (0,0,0) relative to its parent will not cause a collision.  
For the walkers to start wandering around, each AI controller has to be initialized, set a goal and optionally a speed. Stopping the controller works in the same manner:

```py
ai_controller.start()
ai_controller.go_to_location(world.get_random_location_from_navigation())
ai_controller.set_max_speed(1 + random.random())  # Between 1 and 2 m/s (default is 1.4 m/s).
...
ai_controller.stop()
```
When a walker reaches the target location, they will automatically walk to another random point. If the target point is not reachable, then they reach the closest point from the are where they are.  

For a more advanced reference on how to use this, take a look at [this recipe](ref_code_recipes.md#walker-batch-recipe) where a lot of walkers is spawned and set to wander around using batches. 

!!! Note
    To **destroy the pedestrians**, the AI controller needs to be stopped first and then, both actor and controller should be destroyed. 

---
That is a wrap as regarding actors in CARLA.  
The next step should be learning more about the map, roads and traffic in CARLA. Keep reading to learn more or visit the forum to post any doubts or suggestions that have come to mind during this reading: 
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
<a href="../core_map" target="_blank" class="btn btn-neutral" title="3rd. Maps and navigation">
3rd. Maps and navigation</a>
</p>
</div>
</div>