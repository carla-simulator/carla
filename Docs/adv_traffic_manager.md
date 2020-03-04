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

The Traffic Manager, TM for short, is a module built on top of the CARLA API in C++ that is in charge of controlling vehicles inside the simulation. Its goal is to populate the simulation with realistic urban traffic conditions allowing users to customize some behaviours, for example to set specific learning circumstances. To do so, every TM controls vehicles registered to it by setting autopilot to true, and is accounting for the rest by considering them unregistered. 

#### Structured design
The TM is built on the client-side of the CARLA architecture, replacing the former server-side autopilot after version 0.9.7. The execution flow is divided in __stages__ with independent operations and goals, to facilitate the development of phase-related functionalities and data structures, while also improving computational efficiency. Each stage runs on a different thread and communication with the rest is managed through messages facilitating synchronous messaging between the different stages, so the information flows only in one direction.  

#### User customization
Users need to be granted some control by setting parameters that allow, force or encourage certain behaviours. Thus, users can change the traffic behaviour as they prefer, both online and offline. For example they could allow a car to ignore the speed limits or force a lane change. Being able to play around with behaviours is a must when trying to simulate reality, specially to train driving systems under specific and atypical circumstances. 

---
## How does it work?

#### Architecture
The following diagram is a summary of the internal architecture of the Traffic Manager. Blue bodies are the different stages, green ones are additional modules needed during the different stages and the arrows represent communication managed by messenger classes between the different elements.  
The inner structure of the TM can be easily translated to code. Each relevant element has its equivalent in the C++ code (.cpp files) found inside `LibCarla/source/carla/trafficmanager`. 

<div style="text-align:center">
<img src="../img/traffic_manager_diagram.png">
</div>

#### Stages

__1. Localization Stage:__ the TM stores a list of waypoints ahead for each vehicle to follow. The buffer list of waypoints is is updated each iteration and deleted when lane changes are applied, to create a new one with the corresponding trajectory. The amount of waypoints contained in said list varies depending on the vehicle's speed, being greater the faster it goes. The localization stage contains a __spacial hashing__ which summarizes the position for every car __registered to the Traffic Manager in a world grid__. This is a way to roughly predict possible collisions and create a list of overlapping actors for every vehicle that will be later used by the next stage.  

* __Related .cpp files:__ `LocalizationStage.cpp` and `LocalizationUtils.cpp`.  

__2. Collision Stage:__ checks possible collisions for every vehicle. For each pairs of overlapping actors in the list contained by the localization stage, it extends a __geodesic boundary__. These are extended bounding bodies that represent the vehicle along its trajectory. Collisions between two geodesic boundaries are communicated to the __Motion planner stage__ that will manage them.  

* __Related .cpp files:__ `CollisionStage.cpp`.  

__3. Traffic Light Stage:__ manages some general traffic regulations, mainly priority at junctions. Whenever there is a traffic light, if red, a __traffic hazard__ is set to true. Non signaled junctions are managed with a ticket system. When coming closer to these, a __geodesic boundary__ is extended through the intersection along the vehicle's trajectory. If there is another vehicle inside of it, the parent waits for the junction to be free.  

* __Related .cpp files:__ `TrafficLightStage.cpp`.  

__4. Motion Planner Stage:__ aggregates all the information gathered through the previous stages. Their guidance is adjusted with actuation signals provided by a PID controller, this stage makes decisions on how to move the vehicles. It computes all the commands needed for every vehicle and then send these to the final stage. For example, when facing a __traffic hazard__, this stage will compute the brake needed for said vehicle and communicate it to the apply control stage.  

* __Related .cpp files:__ `MotionPlannerStage.cpp`.  

__5. Apply Control Stage:__ receives actuation signals (such as throttle, brake, steer) from the Motion Planner stage and commands these to the simulator in batches to control every vehicles' movement. It mainly uses the __apply_batch()__ method in [carla.Client](../python_api/#carla.Client) and different [carla.VehicleControl](../python_api/#carla.Client) for the registered vehicles.  

* __Related .cpp files:__ `BatchControlStage.cpp`.  

#### Additional modules

__Cached map:__ in order to ease computation costs during the localization stage, the map is divided into a grid of waypoints, so that these are not calculated every time. These waypoints are not the usual _carla.Waypoint_ accessible from the API but a specific data structure designed according to the needs of the TM. The grid of waypoints representing the road map is at the same time divided into different sections, each of them representing little portions of the road, for instance a junction. These sections are numbered and connected so that it is easier to locate a vehicle on the map and to identify which vehicle are close to it.  

* __Related .cpp files:__ `InMemoryMap.cpp` and `SimpleWaypoint.cpp`.  

__PID controller:__ the TM module contains its own PID controller to regulate throttle, brake and steering according to an intended result. The way this adjustment is made depends on the specific parametrization of the controller, which can be modified if the desired behaviour is different. Read more about [PID compensation](https://commons.wikimedia.org/wiki/File:PID_Compensation_Animated.gif) to learn how to do it.  

* __Related .cpp files:__ `PIDController.cpp`.  

---
## Using the Traffic Manager 

#### General considerations

First of all there are some general behaviour patterns the TM will generate that should be understood beforehand. These statements inherent to the way the TM is implemented:  

* __Vehicles are not goal-oriented__ they follow a road and whenever a junction appears, choose a path randomly. Their path is endless and thus, will never stop roaming around the city.
* __Vehicles' intended speed is 70% their current speed limit:__ unless any other behaviour is set. 
* __Lane change is currently disabled:__ it can be enabled through the API, though right now some issues may happen. Disabling lane changes will also ban overtakings between vehicles in the same lane. 
* __Junction priority does not follow traffic regulations:__ the TM has a ticket system to be used while junction complexity is solved. This may cause some issues such as a vehicle inside a roundabout yielding to a vehicle trying to get in. 

The TM provides with a set of possibilities that decide how the traffic is conducted or set specific rules for a vehicle. Thus, the user can aim for different behaviours in both a general and specific sense. All the methods accessible from the Python API are listed in the [documentation](../python_api/#carla.TrafficManager). However, here is a brief summary of what the current possibilities are. 

<table style="width:100%">
  <col width="150">
  <tr>
    <td><b> TM creation: </b></td>
    <td> <li><br>1. Get a TM instance for a client.</li> </td>
  </tr>
  <tr>
    <td><b>Safety conditions: </b></td>
    <td><br> 
    1. Set a minimum distance between stopped vehicles (for a vehicle or all of them). This will affect the minimum moving distance. <br>
    2. Set an intended speed regarding current speed limitation (for a vehicle or all of them). <br>
    3. Reset traffic lights. 
    </td>
  </tr>
  <tr>
    <td><b>Collision managing: </b></td>
    <td><br>
    1. Enable/Disable collisions between a vehicle and a specific actor.  <br>
    2. Make a vehicle ignore all the other vehicles. <br>
    3. Make a vehicle ignore all the walkers.  <br>
    4. Make a vehicle ignore all the traffic lights. 
    </td>
  </tr>
  <tr>
    <td><b>Lane changes: </b></td>
    <td><br>
    1. Force a lane change disregarding possible collisions. <br>
    2. Enable/Disable lane changes for a vehicle.  
    </td>
  </tr>
</table>
<br>

By default, vehicles will not ignore any actor, drive at their current speed limit and leave 1 meter of safety distance between them.  
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

The script `spawn_npc.py` in `/PythonAPI/examples` creates a TM instance in the port passed as argument and registers every vehicle spawned to it by setting the autopilot to True on a batch.
```py
traffic_manager = client.get_trafficmanager(args.tm_port)
...
batch.append(SpawnActor(blueprint, transform).then(SetAutopilot(FutureActor, True)))
...
traffic_manager.global_percentage_speed_difference(30.0)
```

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
However, it is important that when shutting down a TM, the vehicles registered to it are destroyed. Otherwise, they will stop at place, as no one will be conducting them. The script `spawn_npc.py` does this automatically. 

!!! Warning 
    Shutting down a __TM-Server__ will shut down the __TM-Clients__ connecting to it. To learn the difference between a __TM-Server__ and a __TM-Client__ read the following section. 

---
## Multiclient and multiTM management

#### Different Traffic Manager definitions 

When working with different clients containing different TM, understanding inner implementation of the TM in the client-server architecture becomes specially relevant. There is one ruling these scenarios: __the port is the key__.  

A client creates a TM by communicating with the server and passing the intended port to be used for said purpose. The port can either be stated or not, using the default as `8000`.  

```py 
tm01 = client01.get_trafficmanager() # tm01 --> tm01 (p=8000)
```
```py
tm02 = client02.get_trafficmanager(5000) # tm02(p=5000) --> tm02 (p=5000)
```

The previous code creates two different TM, as both have different ports and none of them was previously occupied by another TM. These are __TM-Servers__: instances of the TM created on free ports.  

However, if a client tries to create a TM using a port that is already assigned to another TM, __it will not create its own but connect to the one already existing__. These are __TM-Clients__: instances of the TM created in an occupied port, which connect to the previous existing one.  

```py 
tm03 = client03.get_trafficmanager() # tm03 --> tm01 (p=8000). 
```
```py
tm04 = client04.get_trafficmanager(5000) # tm04(p=5000) --> tm02 (p=5000)
```

!!! Important
    Note how the default creation of a TM uses always `port=8000`, and so, only the first time a __TM-Server__ is created. The rest will be __TM-Clients__ connecting to it.

The omniscient CARLA server keeps register of all these instances internally and everytime a TM is created, it stores the port and also the client IP that links to it. For said reason, TM are also accessible using both the ip and the port, but this case will return an error in case any of the pair is wrong.  

```py 
tm05 = client05.get_trafficmanager(6000) # tm05(ip=tm02,p=6000) --> ERROR, wrong IP. 
```
```py
tm06 = client06.get_trafficmanager(5000) # tm06(ip=tm02,p=5000) --> tm02 (p=5000)
```

!!! Note
    Right now there is no way to check which TM have been created so far. A connection will be attempted when trying to create an instance.  

#### Multiclient VS MultiTM

Based on the different definitions for a TM, successfully creating a TM can lead to two different results:  

* __TM-Server:__ when the port is free. This type of TM is in charge of its own logic, managed in `TrafficManagerLocal.cpp`.  

* __TM-Client:__ when the port is occupied. This type of TM is not in charge of its own logic. It gets the information from a __TM-Server__, the one that is actually in the intended port. They retrieve the information from the original TM using `TrafficManagerRemote.cpp`.

That creates a clear distinction between having multiple clients and multiple Traffic Managers running: 

* __Multiclient scenario:__ when there is only one TM-Server, but there are other TM-Clients created with the same port definition that are actually connecting to said TM-Server.  

* __MultiTM scenario:__ when there are different TM-Server, created with different port definitions.  

!!! Note 
    The script `TrafficManager.cpp` acts as a central hub managing all the different TM instances. 

---
## Other considerations

The TM is a module constantly envolving and trying to adapt the range of possibilities that it presents. For instance, in order to get more realistic behaviours we can have many clients with different TM in charge of sets of vehicles with specific and distinct behaviours. This range of possibilities also makes for a lot of different configurations that can get really complex and specific. For such reason, here are listed of considerations that should be taken into account when working with the TM as it is by the time CARLA 0.9.8 is released: 

#### FPS limitations

The TM stops working properly in asynchronous mode when the simulation is under 20fps. Below that rate, the server is going much faster than the client containing the TM and behaviours cannot be simulated properly. For said reason, under these circumstances it is recommended to work in __synchronous mode__.  

!!! Important
    The FPS limitations are specially relevant when working in the night mode. 

#### Synchronous mode

When different clients work in __synchronous mode__ with the server, the server waits for a tick that may come from any client. If more than one client ticks, the synchrony will fail, as the server will move forward on every tick.  
For said reason, it is important to have one leading client while the rest remain silent. This is specially relevant when working in sync. with the __scenario runner__, which runs a TM. In this case, the TM will be subordinated to the scenario runner and wait for it. 

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
