<h1>Python API tutorial</h1>

In this tutorial we introduce the basic concepts of the CARLA Python API, as
well as an overview of its most important functionalities. The reference of all
classes and methods available can be found at
[Python API reference](python_api.md).

!!! note
    **This document applies only to the latest development version**. <br>
    The API has been significantly changed in the latest versions starting at
    0.9.0. We commonly refer to the new API as **0.9.X API** as opposed to
    the previous **0.8.X API**.

First of all, we need to introduce a few core concepts:

  - **Actor:** Actor is anything that plays a role in the simulation and can be
    moved around, examples of actors are vehicles, pedestrians, and sensors.
  - **Blueprint:** Before spawning an actor you need to specify its attributes,
    and that's what blueprints are for. We provide a blueprint library with
    the definitions of all the actors available.
  - **World:** The world represents the currently loaded map and contains the
    functions for converting a blueprint into a living actor, among other. It
    also provides access to the road map and functions to change the weather
    conditions.

#### Connecting and retrieving the world

To connect to a simulator we need to create a "Client" object, to do so we need
to provide the IP address and port of a running instance of the simulator

```py
client = carla.Client('localhost', 2000)
```

The first recommended thing to do right after creating a client instance is
setting its time-out. This time-out sets a time limit to all networking
operations, if the time-out is not set networking operations may block forever

```py
client.set_timeout(10.0) # seconds
```

Once we have the client configured we can directly retrieve the world

```py
world = client.get_world()
```

Typically we won't need the client object anymore, all the objects created by
the world will connect to the IP and port provided if they need to. These
operations are usually done in the background and are transparent to the user.

#### Blueprints

A blueprint contains the information necessary to create a new actor. For
instance, if the blueprint defines a car, we can change its color here, if it
defines a lidar, we can decide here how many channels the lidar will have. A
blueprints also has an ID that uniquely identifies it and all the actor
instances created with it. Examples of IDs are "vehicle.nissan.patrol" or
"sensor.camera.depth".

The list of all available blueprints is kept in the **blueprint library**

```py
blueprint_library = world.get_blueprint_library()
```

The library allows us to find specific blueprints by ID, filter them with
wildcards, or just choosing one at random

```py
# Find specific blueprint.
collision_sensor_bp = blueprint_library.find('sensor.other.collision')
# Chose a vehicle blueprint at random.
vehicle_bp = random.choice(blueprint_library.filter('vehicle.bmw.*'))
```

Some of the attributes of the blueprints can be modified while some other are
just read-only. For instance, we cannot modify the number of wheels of a vehicle
but we can change its color

```py
vehicles = blueprint_library.filter('vehicle.*')
bikes = [x for x in vehicles if int(x.get_attribute('number_of_wheels')) == 2]
for bike in bikes:
    bike.set_attribute('color', '255,0,0')
```

Modifiable attributes also come with a list of recommended values

```py
for attr in blueprint:
    if attr.is_modifiable:
        blueprint.set_attribute(attr.id, random.choice(attr.recommended_values))
```

The blueprint system has been designed to ease contributors adding their custom
actors directly in Unreal Editor, we'll add a tutorial on this soon, stay tuned!

#### Spawning actors

Once we have the blueprint set up, spawning an actor is pretty straightforward

```py
transform = Transform(Location(x=230, y=195, z=40), Rotation(yaw=180))
actor = world.spawn_actor(blueprint, transform)
```

The spawn actor function comes in two flavours, `spawn_actor` and
`try_spawn_actor`. The former will raise an exception if the actor could not be
spawned, the later will return `None` instead. The most typical cause of
failure is collision at spawn point, meaning the actor does not fit at the spot
we chose; probably another vehicle is in that spot or we tried to spawn into a
static object.

To ease the task of finding a spawn location, each map provides a list of
recommended transforms

```py
spawn_points = world.get_map().get_spawn_points()
```

We'll add more on the map object later in this tutorial.

Finally, the spawn functions have an optional argument that controls whether the
actor is going to be attached to another actor. This is specially useful for
sensors. In the next example, the camera remains rigidly attached to our vehicle
during the rest of the simulation

```py
camera = world.spawn_actor(camera_bp, relative_transform, attach_to=my_vehicle)
```

Note that in this case, the transform provided is treated relative to the parent
actor.

#### Handling actors

Once we have an actor alive in the world, we can move this actor around and
check its dynamic properties

```py
location = actor.get_location()
location.z += 10.0
actor.set_location(location)
print(actor.get_acceleration())
print(actor.get_velocity())
```

We can even freeze an actor by disabling its physics simulation

```py
actor.set_simulate_physics(False)
```

And once we get tired of an actor we can remove it from the simulation with

```py
actor.destroy()
```

Note that actors are not cleaned up automatically when the Python script
finishes, if we want to get rid of them we need to explicitly destroy them.

!!! important
    **Known issue:** To improve performance, most of the methods send requests
    to the simulator asynchronously. The simulator queues each of these
    requests, but only has a limited amount of time each update to parse them.
    If we flood the simulator by calling "set" methods too often, e.g.
    set_transform, the requests will accumulate a significant lag.

#### Vehicles

Vehicles are a special type of actor that provide a few extra methods. Apart
from the handling methods common to all actors, vehicles can also be controlled
by providing throttle, break, and steer values

```py
vehicle.apply_control(carla.VehicleControl(throttle=1.0, steer=-1.0))
```

These are all the parameters of the `VehicleControl` object and their default
values

```py
carla.VehicleControl(
    throttle = 0.0
    steer = 0.0
    brake = 0.0
    hand_brake = False
    reverse = False
    manual_gear_shift = False
    gear = 0)
```

Also, physics control properties can be tuned for vehicles and its wheels
```py
vehicle.apply_physics_control(carla.VehiclePhysicsControl(max_rpm = 5000.0, center_of_mass = carla.Vector3D(0.0, 0.0, 0.0), torque_curve=[[0,400],[5000,400]]))
```

These properties are controlled through a `VehiclePhysicsControl` object, which also contains a property to control each wheel's physics through a `WheelPhysicsControl` object.

```py
carla.VehiclePhysicsControl(
    torque_curve,
    max_rpm,
    moi,
    damping_rate_full_throttle,
    damping_rate_zero_throttle_clutch_engaged,
    damping_rate_zero_throttle_clutch_disengaged,
    use_gear_autobox,
    gear_switch_time,
    clutch_strength,
    mass,
    drag_coefficient,
    center_of_mass,
    steering_curve,
    wheels)
```
Where:
- *torque_curve*: Curve that indicates the torque measured in Nm for a specific revolutions per minute of the vehicle's engine
- *max_rpm*: The maximum revolutions per minute of the vehicle's engine
- *moi*: The moment of inertia of the vehicle's engine
- *damping_rate_full_throttle*: Damping rate when the throttle is maximum.
- *damping_rate_zero_throttle_clutch_engaged*: Damping rate when the thottle is zero with clutch engaged
- *damping_rate_zero_throttle_clutch_disengaged*: Damping rate when the thottle is zero with clutch disengaged

- *use_gear_autobox*: If true, the vehicle will have automatic transmission
- *gear_switch_time*: Switching time between gears
- *clutch_strength*: The clutch strength of the vehicle. Measured in Kgm^2/s

- *mass*: The mass of the vehicle measured in Kg
- *drag_coefficient*: Drag coefficient of the vehicle's chassis
- *center_of_mass*: The center of mass of the vehicle
- *steering_curve*: Curve that indicates the maximum steering for a specific forward speed
- *wheels*: List of `WheelPhysicsControl` objects.

```py
carla.WheelPhysicsControl(
    tire_friction,
    damping_rate,
    max_steer_angle,
    radius,
    max_brake_torque,
    max_handbrake_torque,
    position)
```
Where:
- *tire_friction*: Scalar value that indicates the friction of the wheel.
- *damping_rate*: The damping rate of the wheel.
- *max_steer_angle*: The maximum angle in degrees that the wheel can steer.
- *radius*: The radius of the wheel in centimeters.
- *max_brake_torque*: The maximum brake torque in Nm.
- *max_handbrake_torque*: The maximum handbrake torque in Nm.
- *position*: The position of the wheel.


Our vehicles also come with a handy autopilot

```py
vehicle.set_autopilot(True)
```

As has been a common misconception, we need to clarify that this autopilot
control is purely hard-coded into the simulator and it's not based at all in
machine learning techniques.

Finally, vehicles also have a bounding box that encapsulates them

```py
box = vehicle.bounding_box
print(box.location)         # Location relative to the vehicle.
print(box.extent)           # XYZ half-box extents in meters.
```

#### Sensors

Sensors are actors that produce a stream of data. Sensors are such a key
component of CARLA that they deserve their own documentation page, so here we'll
limit ourselves to show a small example of how sensors work

```py
camera_bp = blueprint_library.find('sensor.camera.rgb')
camera = world.spawn_actor(camera_bp, relative_transform, attach_to=my_vehicle)
camera.listen(lambda image: image.save_to_disk('output/%06d.png' % image.frame))
```

In this example we have attached a camera to a vehicle, and told the camera to
save to disk each of the images that are going to be generated.

The full list of sensors and their measurement is explained in
[Cameras and sensors](cameras_and_sensors.md).

#### Other actors

Apart from vehicles and sensors, there are a few other actors in the world. The
full list can be requested to the world with

```py
actor_list = world.get_actors()
```

The actor list object returned has functions for finding, filtering, and
iterating actors

```py
# Find an actor by id.
actor = actor_list.find(id)
# Print the location of all the speed limit signs in the world.
for speed_sign in actor_list.filter('traffic.speed_limit.*'):
    print(speed_sign.get_location())
```

Among the actors you can find in this list are

  * **Traffic lights** with a `state` property to check the light's current state.
  * **Speed limit signs** with the speed codified in their type_id.
  * The **Spectator** actor that can be used to move the view of the simulator window.

#### Changing the weather

The lighting and weather conditions can be requested and changed with the world
object

```py
weather = carla.WeatherParameters(
    cloudyness=80.0,
    precipitation=30.0,
    sun_altitude_angle=70.0)

world.set_weather(weather)

print(world.get_weather())
```

For convenience, we also provided a list of predefined weather presets that can
be directly applied to the world

```py
world.set_weather(carla.WeatherParameters.WetCloudySunset)
```

The full list of presets can be found in the
[WeatherParameters reference](python_api.md#carlaweatherparameters).

#### Map and waypoints

One of the key features of CARLA is that our roads are fully annotated. All our
maps come accompanied by [OpenDrive](http://www.opendrive.org/) files that
defines the road layout. Furthermore, we provide a higher level API for querying
and navigating this information.

These objects were a recent addition to our API and are still in heavy
development, we hope to make them much more powerful soon.

Let's start by getting the map of the current world

```py
map = world.get_map()
```

For starters, the map has a `name` attribute that matches the name of the
currently loaded city, e.g. Town01. And, as we've seen before, we can also ask
the map to provide a list of recommended locations for spawning vehicles,
`map.get_spawn_points()`.

However, the real power of this map API comes apparent when we introduce
waypoints. We can tell the map to give us a waypoint on the road closest to our
vehicle

```py
waypoint = map.get_waypoint(vehicle.get_location())
```

This waypoint's `transform` is located on a drivable lane, and it's oriented
according to the road direction at that point.

Waypoints also have function to query the "next" waypoints; this method returns
a list of waypoints at a certain distance that can be accessed from this
waypoint following the traffic rules. In other words, if a vehicle is placed in
this waypoint, give me the list of posible locations that this vehicle can drive
to. Let's see a practical example

```py
# Retrieve the closest waypoint.
waypoint = map.get_waypoint(vehicle.get_location())

# Disable physics, in this example we're just teleporting the vehicle.
vehicle.set_simulate_physics(False)

while True:
    # Find next waypoint 2 meters ahead.
    waypoint = random.choice(waypoint.next(2.0))
    # Teleport the vehicle.
    vehicle.set_transform(waypoint.transform)
```

The map object also provides methods for generating in bulk waypoints all over
the map at an approximated distance between them

```py
waypoint_list = map.generate_waypoints(2.0)
```

For routing purposes, it is also possible to retrieve a topology graph of the
roads

```py
waypoint_tuple_list = map.get_topology()
```

this method returns a list of pairs (tuples) of waypoints, for each pair, the
first element connects with the second one. Only the minimal set of waypoints to
define the topology are generated by this method, only a waypoint for each lane
for each road segment in the map.

Finally, to allow access to the whole road information, the map object can be
converted to OpenDrive format, and saved to disk as such.

#### Recording and Replaying system

CARLA includes now a recording and replaying API, that allows to record a simulation in a file and later replay that simulation. The file is written on server side only, and it includes which **actors are created or destroyed** in the simulation, the **state of the traffic lights** and the **position/orientation** of all vehicles and walkers.

All data is written in a binary file on the server. We can use filenames with or without a path. If we specify a filename without any of '\\', '/' or ':' characters, then it is considered to be only a filename and will be saved on folder **CarlaUE4/Saved**. If we use any of the previous characters then the filename will be considered as an absolute filename with path (for example: '/home/carla/recording01.log' or 'c:\\records\\recording01.log').

As estimation, a simulation with about 150 actors (50 traffic lights, 100 vehicles) for 1h of recording takes around 200 Mb in size.

To start recording we only need to supply a file name:

```py
client.start_recorder("recording01.log")
```

To stop the recording, we need to call:

```py
client.stop_recorder()
```

At any point we can replay a simulation, specifying the filename:

```py
client.replay_file("recording01.log")
```
The replayer will create and destroy all actors that were recorded, and move all actors and setting the traffic lights as they were working at that moment.

When replaying we have some other options that we can use, the full API call is:

```py
client.replay_file("recording01.log", start, duration, camera)
```
* **start**: time we want to start the simulation.
  * If the value is positive, it means the number of seconds from the beginning.
  Ex: a value of 10 will start the simulation at second 10.
  * If the value is negative, it means the number of seconds from the end.
  Ex: a value of -10 will replay only the last 10 seconds of the simulation.
* **duration**: we can say how many seconds we want to play. If the simulation has not reached the end, then all actors will have autopilot enabled automatically. The intention here is to allow for replaying a piece of a simulation and then let all actors start driving in autopilot again.
* **camera**: we can specify the Id of an actor and then the camera will follow that actor while replaying. Continue reading to know which Id has an actor.

We can specify the time factor (speed) for the replayer at any moment, using the next API:

```py
client.set_replayer_time_factor(2.0)
```
A value greater than 1.0 will play in fast motion, and a value below 1.0 will play in slow motion, being 1.0 the default value for normal playback.
As a performance trick, with values over 2.0 the interpolation of positions is disabled.

The call of this API will not stop the replayer in course, it will change just the speed, so you can change that several times while the replayer is running.

We can know details about a recorded simulation, using this API:

```py
client.show_recorder_file_info("recording01.log")
```

The output result is something like this:

```
Version: 1
Map: Town05
Date: 02/21/19 10:46:20

Frame 1 at 0 seconds
 Create 2190: spectator (0) at (-260, -200, 382.001)
 Create 2191: traffic.traffic_light (3) at (4255, 10020, 0)
 Create 2192: traffic.traffic_light (3) at (4025, 7860, 0)
 Create 2193: traffic.traffic_light (3) at (1860, 7975, 0)
 Create 2194: traffic.traffic_light (3) at (1915, 10170, 0)
 ...
 Create 2258: traffic.speed_limit.90 (0) at (21651.7, -1347.59, 15)
 Create 2259: traffic.speed_limit.90 (0) at (5357, 21457.1, 15)
 Create 2260: traffic.speed_limit.90 (0) at (858, 18176.7, 15)
Frame 2 at 0.0254253 seconds
 Create 2276: vehicle.mini.cooperst (1) at (4347.63, -8409.51, 120)
  number_of_wheels = 4
  object_type =
  color = 255,241,0
  role_name = autopilot
Frame 4 at 0.0758538 seconds
 Create 2277: vehicle.diamondback.century (1) at (4017.26, 14489.8, 123.86)
  number_of_wheels = 2
  object_type =
  color = 50,96,242
  role_name = autopilot
Frame 6 at 0.122666 seconds
 Create 2278: vehicle.seat.leon (1) at (3508.17, 7611.85, 120.002)
  number_of_wheels = 4
  object_type =
  color = 237,237,237
  role_name = autopilot
Frame 8 at 0.171718 seconds
 Create 2279: vehicle.diamondback.century (1) at (3160, 3020.07, 120.002)
  number_of_wheels = 2
  object_type =
  color = 50,96,242
  role_name = autopilot
Frame 10 at 0.219568 seconds
 Create 2280: vehicle.bmw.grandtourer (1) at (-5405.99, 3489.52, 125.545)
  number_of_wheels = 4
  object_type =
  color = 0,0,0
  role_name = autopilot
Frame 2350 at 60.2805 seconds
 Destroy 2276
Frame 2351 at 60.3057 seconds
 Destroy 2277
Frame 2352 at 60.3293 seconds
 Destroy 2278
Frame 2353 at 60.3531 seconds
 Destroy 2279
Frame 2354 at 60.3753 seconds
 Destroy 2280

Frames: 2354
Duration: 60.3753 seconds
```
From here we know the **date** and the **map** where the simulation was recorded.
Then for each frame that has an event (create or destroy an actor, collisions) it shows that info. For creating actors we see the **Id** it has and some info about the actor to create. This is the **id** we need to specify in the **camera** option when replaying if we want to follow that actor during the replay.
At the end we can see the **total time** of the recording and also the number of **frames** that were recorded.

In simulations whith a **hero actor** the collisions are automatically saved, so we can query a recorded file to see if any **hero actor** had collisions with some other actor. Currently the actor types we can use in the query are these:

* **h** = Hero
* **v** = Vehicle
* **w** = Walker
* **t** = Traffic light
* **o** = Other
* **a** = Any

The collision query needs to know the type of actors involved in the collision. If we don't care we can specify **a** (any) for both. These are some examples:

* **a** **a**: will show all collisions recorded
* **v** **v**: will show all collisions between vehicles
* **v** **t**: will show all collisions between a vehicle and a traffic light
* **v** **w**: will show all collisions between a vehicle and a walker
* **v** **o**: will show all collisions between a vehicle and other actor, like static meshes
* **h** **w**: will show all collisions between a hero and a walker

Currently only **hero actors** record the collisions, so first actor will be a hero always.

The API for querying the collisions is:

```py
client.show_recorder_collisions("recording01.log", "a", "a")
```

The output is something similar to this:

```
Version: 1
Map: Town05
Date: 02/19/19 15:36:08

    Time  Types     Id Actor 1                                 Id Actor 2
      16   v v     122 vehicle.yamaha.yzf                     118 vehicle.dodge_charger.police
      27   v o     122 vehicle.yamaha.yzf                       0

Frames: 790
Duration: 46 seconds
```

We can see there for each collision the **time** when happened, the **type** of the actors involved, and the **id and description** of each actor.

So, if we want to see what happened on that recording for the first collision where the hero actor was colliding with a vehicle, we could use this API:

```py
client.replay_file("col2.log", 13, 0, 122)
```
We have started the replayer just a bit before the time of the collision, so we can see how it happened.
Also, a value of 0 for the **duration** means to replay all the file (it is the default value).

We can see something like this then:

![collision](img/collision1.gif)

There is another API to get information about actors that has been blocked by something and can not follow its way. That could be good to find incidences in the simulation. The API is:

```py
client.show_recorder_actors_blocked("recording01.log", min_time, min_distance)
```

The parameters are:
* **min_time**: the minimum time that an actor needs to be stopped to be considered as blocked (in seconds).
* **min_distance**: the minimum distance to consider an actor to be stopped (in cm).

So, if we want to know which actor is stopped (moving less than 1 meter during 60 seconds), we could use something like:

```py
client.show_recorder_actors_blocked("col3.log", 60, 100)
```

The result can be something like (it is sorted by the duration):

```
Version: 1
Map: Town05
Date: 02/19/19 15:45:01

    Time     Id Actor                                 Duration
      36    173 vehicle.nissan.patrol                      336
      75    104 vehicle.dodge_charger.police               295
      75    214 vehicle.chevrolet.impala                   295
     234     76 vehicle.nissan.micra                       134
     241    162 vehicle.audi.a2                            128
     302    143 vehicle.bmw.grandtourer                     67
     303    133 vehicle.nissan.micra                        67
     303    167 vehicle.audi.a2                             66
     302     80 vehicle.nissan.micra                        67

Frames: 6985
Duration: 374 seconds
```

This lines tell us when an actor was stopped for at least the minimum time specified.
For example the 6th line, the actor 143, at time 302 seconds, was stopped for 67 seconds.

We could check what happened that time with the next API command:

```py
client.replay_file("col3.log", 302, 0, 143)
```

![actor blocked](img/actor_blocked1.png)

We see there is some mess there that actually blocks the actor (red vehicle in the image).
We can check also another actor with:

```py
client.replay_file("col3.log", 75, 0, 104)
```

![actor blocked](img/actor_blocked2.png)

We can see it is the same incidence but from another actor involved (police car).

The result is sorted by duration, so the actor that is blocked for more time comes first. We could check the first line, with Id 173 at time 36 seconds it get stopped for 336 seconds. We could check how it arrived to that situation replaying a few seconds before time 36.

```py
client.replay_file("col3.log", 34, 0, 173)
```

![accident](img/accident.gif)

We can see then the responsible of the incident.

#### Sample PY scripts to use with the recording / replaying system

There are some scripts you could use:

* **start_recording.py**: this will start recording, and optionally you can spawn several actors and define how much time you want to record.
  * **-f**: filename of write
  * **-n**: vehicles to spawn (optional, 10 by default)
  * **-t**: duration of the recording (optional)
* **start_replaying.py**: this will start a replay of a file. We can define the starting time, duration and also an actor to follow.
  * **-f**: filename of write
  * **-s**: starting time (optional, by default from start)
  * **-d**: duration (optional, by default all)
  * **-c**: actor to follow (id) (optional)
* **show_recorder_collisions.py**: this will show all the collisions hapenned while recording (currently only involved by hero actors).
  * **-f**: filename of write
  * **-t**: two letters definning the types of the actors involved, for example: -t aa
    * **h** = Hero
    * **v** = Vehicle
    * **w** = Walker
    * **t** = Traffic light
    * **o** = Other
    * **a** = Any
* **show_recorder_actors_blocked.py**: this will show all the actors that are blocked (stopped) in the recorder. We can define the time and distance to be considered as blocked.
  * **-f**: filename of write
  * **-t**: minimum seconds stopped to be considered as blocked (optional)
  * **-d**: minimum distance to be considered stopped (optional)


