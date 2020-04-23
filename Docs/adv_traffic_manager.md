# Traffic Manager
*   [__What is it?__](#what-is-it)  
*   [__How does it work?__](#how-does-it-work)  
	*   [Architecture](#architecture)  
	*   [Stages](#stages)  
*   [__Using the Traffic Manager__](#using-the-traffic-manager)  
	*   [General considerations](#general-considerations)  
	*   [Creating a Traffic Manager](#creating-a-traffic-manager)  
	*   [Setting a Traffic Manager](#setting-a-traffic-manager)  
	*   [Stopping a Traffic Manager](#stopping-a-traffic-manager)  
*   [__Hybrid physics mode__](#hybrid-physics-mode)  
*   [__Running multiple Traffic Managers__](#running-multiple-traffic-managers)  
	*   [Definitions](#definitions)  
	*   [Multiclient](#multiclient)  
	*   [MultiTM](#multitm)  
	*   [Multisimulation](#multisimulation)  
*   [__Other considerations__](#other-considerations)  
	*   [FPS limitations](#fps-limitations)  
	*   [Synchronous mode](#synchronous-mode)  
*   [__Summary__](#summary)  

---
## What is it?

The Traffic Manager, TM for short, is the module in charge of controlling vehicles inside the simulation. It is built on top of the CARLA API in C++. Its goal is to populate the simulation with realistic urban traffic conditions. Users can customize some behaviours, for example to set specific learning circumstances. Every TM controls vehicles registered to it by setting autopilot to true, and is accounting for the rest by considering them unregistered. 

### Structured design
The TM is built on the client-side of the CARLA architecture. It replaces the server-side autopilot. The execution flow is divided in __stages__ with independent operations and goals. This facilitates the development of phase-related functionalities and data structures, while also improving computational efficiency. Each stage runs on a different thread. Communication with the rest is managed through synchronous messaging between the stages.The information flows only in one direction.  

### User customization
Users must have some control over the traffic flow by setting parameters that allow, force or encourage specific behaviours. Users can change the traffic behaviour as they prefer, both online and offline. For example they could allow a car to ignore the speed limits or force a lane change. Being able to play around with behaviours is a must when trying to simulate reality. It is necessary to train driving systems under specific and atypical circumstances. 

---
## How does it work?

### Architecture
The following diagram is a summary of the internal architecture of the Traffic Manager. Blue bodies represent the different stages. Green ones are additional modules that work with these. The arrows represent communication between elements managed by messenger classes.  

The inner structure of the TM can be easily translated to code. Each relevant element has its equivalent in the C++ code (.cpp files) inside `LibCarla/source/carla/trafficmanager`. 

<div style="text-align:center">
<img src="../img/traffic_manager_diagram.png">
</div>

### Stages

__1. Localization Stage:__ the TM stores a list of waypoints ahead for each vehicle to follow. The list of waypoints is updated each iteration, changing according to the decisions taken during the stage, such as lane changes, to modify the vehicle's trajectory. The amount of waypoints stored depends on the vehicle's speed, being greater the faster it goes. This stage contains a __spatial hashing__ which saves the position for every car __registered to the Traffic Manager in a world grid__. This is a way to roughly predict possible collisions and create a list of overlapping actors for every vehicle that will be later used by the next stage.  

* __Related .cpp files:__ `LocalizationStage.cpp` and `LocalizationUtils.cpp`.  

__2. Collision Stage:__ checks possible collisions for every vehicle. For each pairs of overlapping actors in the list stored by the Localization stage, it extends a __geodesic boundary__. These are extended bounding boxes that represent the vehicle along its trajectory. This stage determines which vehicle has priority and communicates the result to the __Motion Planner stage__.  

* __Related .cpp files:__ `CollisionStage.cpp`.  

__3. Traffic Light Stage:__ manages some general traffic regulations, mainly priority at junctions. A _traffic hazard__ is set to true whenever a yellow or red traffic light is detected. Non-signalized junctions are managed with a priority system. When approaching to these, a __geodesic boundary__ is extended through the intersection along the intended trajectory. The vehicle will wait for the junction to be free if another vehicle is detected inside of the geodesic boundary.  

* __Related .cpp files:__ `TrafficLightStage.cpp`.  

__4. Motion Planner Stage:__ aggregates all the information from the previous stages and makes decisions on how to move the vehicles. It is asisted by a PID controller to adjust the resulting behaviour. After computing all the commands needed for every vehicle, these are sent to the final stage. For example, when facing a __traffic hazard__, this stage will compute the brake needed for said vehicle and communicates it to the Apply Control stage.  

* __Related .cpp files:__ `MotionPlannerStage.cpp`.  

__5. Apply Control Stage:__ receives actuation signals, such as throttle, brake, steer, from the Motion Planner stage and sends these to the simulator in batches to control every vehicles' movement. Using the __apply_batch()__ method in [carla.Client](../python_api/#carla.Client) and different [carla.VehicleControl](../python_api/#carla.VehicleControl) for the registered vehicles.  

* __Related .cpp files:__ `BatchControlStage.cpp`.  

### Additional modules

__Cached map:__ in order to increase computational efficiency during the Localization stage, the map is discretized and cached as a grid of waypoints. These are included in a specific data structure designed to hold more information, such as links between them. The grids allow to easily connect the map, each of them representing sections of a road or a whole junction, by also having an ID that is used to quickly identify vehicles in nearby areas.  

* __Related .cpp files:__ `InMemoryMap.cpp` and `SimpleWaypoint.cpp`.  

__PID controller:__ the TM module uses a PID controller to regulate throttle, brake and steering according to a target value. The way this adjustment is made depends on the specific parametrization of the controller, which can be modified if the desired behaviour is different. Read more about [PID compensation](https://commons.wikimedia.org/wiki/File:PID_Compensation_Animated.gif) to learn how to do it.  

* __Related .cpp files:__ `PIDController.cpp`.  

---
## Using the Traffic Manager 

### General considerations

First of all there are some general behaviour patterns the TM will generate that should be understood beforehand. These statements are inherent to the way the TM is implemented:  

* __Vehicles are not goal-oriented__ they follow a trajectory and whenever approaching a junction, choose a path randomly. Their path is endless, and will never stop roaming around the city.  
* __Vehicles' target speed is 70% their current speed limit:__ unless any other value is set. 
* __Junction priority does not follow traffic regulations:__ the TM has a priority system to be used while junction complexity is solved. This may cause some issues such as a vehicle inside a roundabout yielding to a vehicle trying to get in. 

The TM provides a set of possibilities so the user can establish specific behaviours. All the methods accessible from the Python API are listed in the [documentation](../python_api/#carla.TrafficManager). However, here is a brief summary of what the current possibilities are. 

<table class ="defTable">
<tbody>
<td><b>General:</b> </td>
<td><br>
    <b>1.</b> Use a carla.Client to create a TM instance connected to a port.<br>
    <b>2.</b> Retrieve the port where a TM is connected.</td>
<tr>
<td><b>Safety conditions:</b> </td>
<td><br>
    <b>1.</b> Set a minimum distance between stopped vehicles (for a vehicle or all of them). This will affect the minimum moving distance. <br>
    <b>2.</b> Set an intended speed regarding current speed limitation (for a vehicle or all of them). <br>
    <b>3.</b> Reset traffic lights. 
</td>
<tr>
<td><b>Collision managing:</b> </td>
<td><br>
    <b>1.</b> Enable/Disable collisions between a vehicle and a specific actor.  <br>
    <b>2.</b> Make a vehicle ignore all the other vehicles. <br>
    <b>3.</b> Make a vehicle ignore all the walkers.  <br>
    <b>4.</b> Make a vehicle ignore all the traffic lights. 
</td>
<tr>
<td><b>Lane changes:</b> </td>
<td><br>
    <b>1.</b> Force a lane change disregarding possible collisions. <br>
    <b>2.</b> Enable/Disable lane changes for a vehicle.
</td>
<tr>
<td><b>Hybrid physics mode:</b> </td>
<td><br>
    <b>1.</b> Enable/Disable the hybrid physics mode.  <br>
    <b>2.</b> Change the radius where physics are enabled.  
</td>
</tbody>
</table>
<br>

### Creating a Traffic Manager

A TM instance can be created by any [carla.Client](python_api.md#carla.Client) specifying the port that will be used. The default port is `8000`.  

```python
tm = client.get_trafficmanager(port)
```

Now the TM needs some vehicles to be in charge of. In order to do so, enable the autopilot mode for the set of vehicles to be managed. Retrieve the port of the TM object that has been created. If no port is provided, it will try to connect to a TM in the default port, `8000`. If the TM does not exist, it will create it.  

```python
tm_port = tm.get_port()
 for v in vehicles_list:
     v.set_autopilot(True,tm_port)
```
!!! Note 
    In multiclient situations, creating or connecting to a TM is not that straightforward. Take a look into the [other considerations](#other-considerations) section to learn more about this. 

The script `spawn_npc.py` in `/PythonAPI/examples` creates a TM instance in the port passed as argument and registers every vehicle spawned to it by setting the autopilot to True on a batch.

```py
traffic_manager = client.get_trafficmanager(args.tm-port)
tm_port = traffic_manager.get_port()
...
batch.append(SpawnActor(blueprint, transform).then(SetAutopilot(FutureActor, True,tm_port)))
...
traffic_manager.global_percentage_speed_difference(30.0)
```

### Setting a Traffic Manager

The following example creates an instance of the TM and sets a dangerous behaviour for a specific car that will ignore all traffic lights, leave no safety distance with the rest and drive at 120% its current speed limit. 

```python
tm = client.get_trafficmanager(port)
tm_port = tm.get_port()
for v in my_vehicles:
  v.set_autopilot(True,tm_port)
danger_car = my_vehicles[0]
tm.ignore_lights_percentage(danger_car,100)
tm.distance_to_leading_vehicle(danger_car,0)
tm.vehicle_percentage_speed_difference(danger_car,-20)
``` 

Now, here is an example that registers that same list of vehicles but instead is set to conduct them with a moderate behaviour. The vehicles will drive at 80% their current speed limit, leaving at least 5 meters between them and never perform a lane change.
```python
tm = client.get_trafficmanager(port)
tm_port = tm.get_port()
for v in my_vehicles:
  v.set_autopilot(True,tm_port)
danger_car = my_vehicles[0]
tm.global_distance_to_leading_vehicle(5)
tm.global_percentage_speed_difference(80)
for v in my_vehicles: 
  tm.auto_lane_change(v,False)
``` 

### Stopping a Traffic Manager

The TM is not an actor that needs to be destroyed, it will stop when the corresponding client does so. This is automatically managed by the API so the user does not have to take care of it.  
However, it is important that when shutting down a TM, the vehicles registered to it are destroyed. Otherwise, they will stop at place, as no one will be conducting them. The script `spawn_npc.py` does this automatically. 

!!! Warning 
    Shutting down a __TM-Server__ will shut down the __TM-Clients__ connecting to it. To learn the difference between a __TM-Server__ and a __TM-Client__ read about [multiclient and multiTM](#multiclient-and-multitm-management). 

---
## Hybrid physics mode

In hybrid mode, either all vehicle physics can be disabled, or enabled only in a radius around an ego vehicle with the tag `hero`. This feature removes the vehicle physics bottleneck from the simulator. Since vehicle physics are not active, all cars move by teleportation. This feature relies on [Actor.set_simulate_physics()](https://carla.readthedocs.io/en/latest/python_api/#carla.Actor.set_simulate_physics). However, not all the physics are disregarded. Basic calculations for a linear acceleration are maintained. By doing so, the position update, and vehicle speed still look realistic. That guarantees that when a vehicle enables or disables its physics, the transition is fluid.  

The hybrid mode is disabled by default. There are two ways to enable it.  

*   [__TrafficManager.set_hybrid_phisics_mode(True)__](https://carla.readthedocs.io/en/latest/python_api/#carla.TrafficManager.set_hybrid_physics_mode) — This method will enable the hybrid mode for the Traffic Manager object calling it.  
*   __Running `spawn_npc.py` with the flag `--hybrid`__ — The vehicles spawned will be registered to a Traffic Manager stated inside the script, and this will run with the hybrid physics on.  

The are two parameters ruling the hybrid mode. One is the __radius__ that states the proximity area around any ego vehicle where physics are enabled. The other is the __vehicle__ with , that will act as center of this radius.  

*   __Radius__ *(default = 70 meters)* — States the proximity area around the ego vehicle where physics are enabled. The value be changed with [traffic_manager.set_hybrid_mode_radius(r)](https://carla.readthedocs.io/en/latest/python_api/#carla.TrafficManager.set_hybrid_mode_radius).  
*   __Ego vehicle__ — A vehicle tagged with `role_name='hero'` that will act of the radius.
	*   __If there is none,__ all the vehicles will disable physics.
	*   __If there are many,__ the radius will be considered for all of them. That will create different areas of influence with physics enabled.  

The following example shows how the physics are enabled and disabled when hybrid mode is on. The __ego vehicle__ is tagged with a __red square__. Vehicles with __physics disabled__ are tagged with a __blue square__. When inside the area of influence stated by the radius, __physics are enabled and the tag becomes green__.

![Welcome to CARLA](img/tm_hybrid.gif)


---
## Running multiple Traffic Managers

### Definitions

When working with different clients containing different TM, understanding inner implementation of the TM in the client-server architecture becomes specially relevant. There is one ruling these scenarios: __the port is the key__.  

A client creates a TM by communicating with the server and passing the intended port to be used for said purpose. The port can either be stated or not, using the default as `8000`.  


* __TM-Server__ — The port is free. This type of TM is in charge of its own logic, managed in `TrafficManagerLocal.cpp`. The following code creates two TM-Servers. Each one connects to a different port, not previously used. 

```py 
tm01 = client01.get_trafficmanager() # tm01 --> tm01 (p=8000)
```
```py
tm02 = client02.get_trafficmanager(5000) # tm02(p=5000) --> tm02 (p=5000)
```

* __TM-Client__ — The port is occupied by another TM. This instances are not in charge of their own logic. Instead, they ask for changes in the parameters of the __TM-Server__ they are connected to in  `TrafficManagerRemote.cpp`. The following code creates two TM-Clients, that connect with the TM-Servers previously created. 

```py 
tm03 = client03.get_trafficmanager() # tm03 --> tm01 (p=8000). 
```
```py
tm04 = client04.get_trafficmanager(5000) # tm04(p=5000) --> tm02 (p=5000)
```

!!! Important
    Note how the default creation of a TM uses always `port=8000`, and so, only the first time a __TM-Server__ is created. The rest will be __TM-Clients__ connecting to it.

The CARLA server keeps register of all the TM instances internally by storing the port and also the client IP (hidden to the user) that link to them. Right now there is no way to check the TM instances that have been created so far. A connection will always be attempted when trying to create an instance and it will either create a new __TM-Server__ or a __TM-Client__.  

!!! Note 
    The class `TrafficManager.cpp` acts as a central hub managing all the different TM instances.  

### Multiclient 

More than one TM instances created with the same port. The first will be a TM-Server. The rest will be TM-Clients connecting to it.  
```py
terminal 1: ./CarlaUE4.sh -carla-rpc-port=4000
terminal 2: python3 spawn_npc.py --port 4000 --tm-port 4050 # TM-Server
terminal 3: python3 spawn_npc.py --port 4000 --tm-port 4050 # TM-Client
```

### MultiTM

Different TM instances with different ports assigned.  
```py
terminal 1: ./CarlaUE4.sh -carla-rpc-port=4000
terminal 2: python3 spawn_npc.py --port 4000 --tm-port 4050 # TM-Server A
terminal 3: python3 spawn_npc.py --port 4000 --tm-port 4550 # TM-Server B
```

### Multisimulation 

Multisimulation is when there are more than one CARLA server running at the same time. The TM declaration is not relevant. As long as the computational power allows for it, the TM can run multiple simulations at a time without any problems.  

```py
terminal 1: ./CarlaUE4.sh -carla-rpc-port=4000 # simulation A 
terminal 2: ./CarlaUE4.sh -carla-rpc-port=5000 # simulation B
terminal 3: python3 spawn_npc.py --port 4000 --tm-port 4050 # TM-Server A connected to simulation A
terminal 4: python3 spawn_npc.py --port 5000 --tm-port 5050 # TM-Server B connected to simulation B
```

The concept of multisimulation is independent from the Traffic Manager itself. The example above runs two CARLA simulations in parallel, A and B. In each of them, a TM-Server is created independently from the other. Simulation A could run a Multiclient TM while simulation B is running a MultiTM, or no TM at all.  

The only possible issue arising from this is a client trying to connect to an already existing TM which is not running on the selected simulation. In case this happens, an error message will appear and the connection will be aborted, to prevent interferences between simulations.  


---
## Other considerations

The TM is a module constantly evolving and trying to adapt the range of possibilities that it presents. For instance, in order to get more realistic behaviours we can have many clients with different TM in charge of sets of vehicles with specific and distinct behaviours. This range of possibilities also makes for a lot of different configurations that can get really complex and specific. For such reason, here are listed of considerations that should be taken into account when working with the TM as it is by the time of writing.  

### FPS limitations

The TM stops working properly in asynchronous mode when the simulation is under 20fps. Below that rate, the server is going much faster than the client containing the TM and behaviours cannot be simulated properly. For said reason, under these circumstances it is recommended to work in __synchronous mode__.  

!!! Important
    The FPS limitations are specially relevant when working in the night mode. 

### Synchronous mode

TM-Clients cannot tick the CARLA server in synchronous mode, __only a TM-Server can call for a tick__.  
If more than one TM-Server ticks, the synchrony will fail, as the server will move forward on every tick. This is specially relevant when working with the __ScenarioRunner__, which runs a TM. In this case, the TM will be subordinated to the ScenarioRunner and wait for it. 

!!! Warning
    Disable the synchronous mode in the script doing the ticks before it finishes. Otherwise, the server will be blocked, waiting forever for a tick.  

---
## Summary


The Traffic Manager is one of the most complex features in CARLA and so, one that is prone to all kind of unexpected and really specific issues. The CARLA forum is open to everybody to post any doubts or suggestions, and it is the best way to keep track of issues and help the CARLA community to become greater. Feel free to login and join the community. 

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</p>
</div>
