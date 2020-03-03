# Traffic Manager
  * [__What is it?__](#what-is-it)  
  * [__How does it work?__](#how-does-it-work)  
	* Architecture
	* Stages
  * [__Using the Traffic Manager__](#using-the-traffic-manager)
	* Parameters
	* Creating a Traffic Manager
	* Setting the Traffic Manager
  * [__Other considerations__](#other-considerations)
	* FPS limitations  
	* Multiclient management  
  * [__Summary__](#summary)

---
## What is it?

The Traffic Manager (TM for short) is a module built on top of the CARLA API in C++ that handles any group of vehicles set to autopilot mode. The goal is to populate the simulation with realistic urban traffic conditions and give the chance to user to customize some behaviours, for example to set specific learning circumstances. To do so, every TM controls vehicles registered to it by setting autopilot to true, and is accounting for the rest by considering them unregistered. 
The Traffic Manager is based on two ground principles:  

#### Structured design
The TM is built on the client-side of the CARLA architecture, in order to save the server some work and not interfere with the simulation performance. For said reason, the flow of execution is divided in __stages__ that help improving computational efficiency and provide stage-related data structures that vehicles can share to communicate. This clear distintion between different sub-modules inside the TM that are accessed independently is central to the whole manager. It avoids potential bottlenecks in managing collisions and allows negotiation between cars. 

#### User customization
Users need to be granted some control by setting parameters that allow, force or encourage certain behaviours. Thus, users can change the traffic behaviour as they prefer, both online and offline. For example they could allow a car to ignore the speed limits or force a lane change. Being able to play around with behaviours is a must when trying to simulate reality, specially to train driving systems under specific and atypical circumstances. 

---
## How does it work?

#### Architecture
The inner structure is divided in stages can be somehow visualized through code, they have their equivalent in the c++ code (.cpp files) foun inside `/libcarla`. There are other inner parts of the TM, such as as setting up the map, but they are managed internally and mostly only done once, so these are not as fundamental to understand the whole.  
Each stage class runs on a different thread and has its independent operations and goals. Communication with the rest is managed through messages facilitating synchronous messaging between the different stages. The information flows only in one direction. In this way, for each stage or class there are methods and tools that can be located in one or many stages with a very specific structure and purpose. 
The following diagram is a summary of the internal architecture of the Traffic Manager. Main bodies are the different stages, blue arrows represent messenger classes that facilitate __asynchronous communication?__, __black arrows represent non-concurrent data flow__. 

<div style="text-align:center">
<img src="../img/traffic_manager_diagram.png">
</div>

#### Stages

__1. Localization Stage:__ For each vehicle registered to the TM, maintains a list of waypoints ahead it to follow. The buffer list of waypoints is not created every iteration but updated. The buffer is deleted though when lane changes are applied to create a new one with the corresponding trajectory. The amount of waypoints contained in said list varies depending on the vehicle's speed, being greater the faster it goes. The localization stage contains a __spacial hashing__ which summarizes the position for every car __registered to the Traffic Manager in a world grid__. This is a way to roughly predict possible collisions and create a list of overlapping actors for every vehicle that will be later used by the next stage.  

* __Related .cpp files:__ `LocalizationStage.cpp`.  

__2. Collision Stage:__ Checks possible collisions for every vehicle. For each pairs of overlapping actors in the list contained by the localization stage, it extends a __geodesic boundary__. These are extended bounding bodies that represent the vehicle along its trajectory. Collisions between two geodesic boundaries are communicated to the __Motion planner stage__ that will manage them.  

* __Related .cpp files:__ `CollisionStage.cpp`.  

__3. Traffic Light Stage:__ Manages some general traffic regulations, mainly priority at junctions. Whenever there is a traffic light, if red, a __traffic hazard__ is set to true. Non signaled junctions are managed with a ticket system. When coming closer to these, a __geodesic boundary__ is extended through the intersection along the vehicle's trajectory. If there is another vehicle inside of it, the parent waits for the junction to be free.  

* __Related .cpp files:__ `TrafficLightStage.cpp`.  

__4. Motion Planner Stage:__ Aggregates all the information gathered through the previous stages. Their guidance is adjusted with actuation signals provided by a PID controller, this stage makes decisions on how to move the vehicles. It computes all the commands needed for every vehicle and then send these to the final stage. For example, when facing a __traffic hazard__, this stage will compute the brake needed for said vehicle and communicate it to the apply control stage.  

* __Related .cpp files:__ `MotionPlannerStage.cpp`.  

__5. Apply Control Stage:__ Receives actuation signals (such as throttle, brake, steer) from the Motion planner stage and commands these to the simulator in batches to control every vehicles' movement. It mainly uses the __apply_batch()__ method in [carla.Client](../python_api/#carla.Client) and different [carla.VehicleControl](../python_api/#carla.Client) for the registered vehicles.  

* __Related .cpp files:__ `BatchControlStage.cpp`.  

## Using the Traffic Manager 

First of all there are some general behaviour patterns the TM will generate that should be understood beforehand. These statements inherent to the way the TM is implemented:  

* __Vehicles are not goal-oriented__ they follow a road and whenever a junction appears, choose a path randomly. Their path is endless and thus, will never stop roaming around the city.
* __Vehicles' intended speed is 70% their current speed limit:__ unless any other behaviour is set. 
* __Lane change is currently disabled:__ it can be enabled through the API, though right now some issues may happen. Disabling lane changes will also ban overtakings between vehicles in the same lane. 
* __Junction priority does not follow traffic regulations:__ the TM has a ticket system to be used while junction complexity is solved. This may cause some issues such as a vehicle inside a roundabout yielding to a vehicle trying to get in. 

The Traffic Manager provides with a set of possibilities that can interfere with how the traffic is conducted or set specific rules for a vehicle. Thus, the user can aim for different behaviours in both a general and specific sense. All the methods accessible from the Python API are listed in the [documentation](../python_api/#carla.TrafficManager). However, here is a brief summary of what the current possibilities are. 

__Traffic Manager fundamental methods:__  

* Get a TM instance from a client.  
* Register/Unregister vehicles from the TM's lists.  

__Safety conditions:__  

* Set a minimum distance between stopped vehicles to be used for a vehicle or all of them. This will also affect the minimum moving distance. 
* Set an intended speed regarding current speed limitation for a specific vehicle or all of them. 
* Reset traffic lights. 

__Collision managing:__  

* Enable/Disable collisions between a vehicle and a specific actor. 
* Make a vehicle ignore all the other vehicles.
* Make a vehicle ignore all the walkers. 
* Make a vehicle ignore all the traffic lights. 

__Lane changes:__  

* Force a lane change disregarding possible collisions. 
* Enable/Disable lane changes for a vehicle.  

By default, vehicles will not ignore any Actor, drive at their current speed limit and leave 1 meter of safety distance between them.  
All of these properties can be easily managed as any other _set()_ method would. 

#### Creating a Traffic Manager

A TM instance can be created in any [carla.Client](python_api.md#carla.Client). To get it, two elements are needed: the container client and the port that will be used to connect with it. Default port is `8000`.  

```python
tm = client.get_trafficmanager(port)
```

Now the TM needs some vehicles to be in charge of. In order to do so, enable the autopilot mode for every vehicle it is in charge of. In case the client is not connected to any TM, an instance will be created with default presets.

```python
 for v in vehicles_list:
     v.set_autopilot(True)
```
!!! Note 
    In multiclient situations, creating or connecting to a TM is not that straightforward. Take a look into the [other considerations](#other-considerations) section to learn more about this. 

There is a script in `/PythonAPI/examples` named as `spawn_npc` that connects to the TM automatically when spawning vehicles:  

* `spawn_npc`: does not allow the user access to the TM and so, runs it with default conditions. There is no explicit attempt to create an instance of the TM with a specific port, so when the autopilot is set to __True__, it tries to connect with a TM in default port `8000` and creates it in said client if it does not exist. 
```sh
...
batch.append(SpawnActor(blueprint, transform).then(SetAutopilot(FutureActor, True)))
```

Right now there is no way to check which Traffic Managers have been created so far. A connection will be attempted when trying to get the instance and if there is no TM using 

#### Setting a Traffic Manager

The following example creates an instance of the TM and sets a dangerous behaviour for a specific car that will ignore all traffic lights, leave no safety distance with the rest and drive at 120% its current speed limit. 

```python
tm = client.get_trafficmanager(port)
for v in my_vehicles:
  v.set_autopilot(True)
danger_car = my_vehicles[0]
tm.ignore_lights_percentage(danger_car,100)
tm.distance_to_leading_vehicle(danger_car,0)
tm.vehicle_percentage_speed_difference(danger_car,-20)
``` 

Now, here is an example that registers that same list of vehicles but instead is set to conduct them with a moderate behaviour. The vehicles will drive at 80% their current speed limit, leaving at least 5 meters between them and never perform a lane change.
```python
tm = client.get_trafficmanager(port)
for v in my_vehicles:
  v.set_autopilot(True)
danger_car = my_vehicles[0]
tm.global_distance_to_leading_vehicle(5)
tm.global_percentage_speed_difference(80)
for v in my_vehicles: 
  tm.auto_lane_change(v,False)
``` 

!!! Important 
    Lane changes are currently disabled in the TM due to unintended collisions causing jams. As long as this issues are not fixed, vehicles will remain in the lane they are spawned and the methods to set lane changes will be disabled. 


#### Stopping a Traffic Manager

The TM is not an actor that needs to be destroyed, it will stop when the corresponding client does so. This is automatically managed by the API so the user does not have to take care of it.

---
## Other considerations

The TM is a module constantly envolving and trying to adapt the range of possibilities that it presents. For instance, in order to get more realistic behaviours we can have many clients with different TM in charge of sets of vehicles with specific and distinct behaviours. This range of possibilities also makes for a lot of different configurations that can get really complex and specific. For such reason, here are listed of considerations that should be taken into account when working with the TM as it is by the time CARLA 0.9.8 is released: 

#### FPS limitations

The TM stops working properly in asynchronous mode when the simulation is under 20fps. Below that rate, the server is going much faster than the client containing the TM and behaviours cannot be simulated properly. This is specially relevant when working in the night mode __table with relation between cars+night mode+city to fps?__
For said reason, under these circumstances it is recommended to work in __synchronous mode__.  

> how to run it in synchronous mode. 

When different clients work in __synchronous mode__ with the server, the server waits for a tick that may come from any client. If more than one client ticks, the synchrony will fail, as the server will move forward on every tick. For said reason, it is important to have one leading client while the rest remain silent. This is specially relevant when working in sync. with the __scenario runner__, which runs a TM. In this case, the TM will be subordinated to the scenario runner and wait for it. 

#### Multiclient management

When working with different clients containing different TM, understanding inner implementation of the TM in the client-server architecture becomes specially relevant. There is one ruling these scenarios: __the port is the key__.  

A client (cl) creates a Traffic Manager (tm) by communicating with the server (s) and passing the intended port to be used for said purpose. The port can either be stated or not, using the default as `8000`.  

`cl_01 --> s --> tm_01 (p=8000)`  
`cl_02(p=5000) --> s --> tm_02 (p=5000)`  

If a client tries to create a TM using a port that is already assigned to another TM, it will not create its own but connect to the one already existing. The reason for this is that __there cannot be two Traffic Managers sharing the same port__. This is specially relevant when using the default creation, as this will only create a TM in the client if port `8000` has not been assigned yet. Otherwise, it  will connect with the TM created (in this case) by `cl_01` in port `8000`.  

`cl_03 --> s --> tm_01 (p=8000)`  
`cl_04(p=5000) --> s --> tm_02 (p=5000)`  
`cl_05(p=6000) --> s --> tm_03 (p=6000)`  

The omniscient server keeps register of all these things internally and everytime a TM is created, it stores the port and also the client ip that links to it. For said reason, Traffic Managers are also accessible using both the ip and the port, but this case will return an error in case any of the pair is wrong.  

`cl_06(ip=cl_02,p=6000) --> s --> ERROR`  
`cl_07(ip=cl02,p=5000) --> s --> tm_02 (p=5000)`  


__Anything else worth mentioning?? Other ways to break it??__

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
