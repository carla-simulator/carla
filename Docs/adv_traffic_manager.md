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
The TM is built on the client-side of the CARLA architecture, replacing the former server-side autopilot after version 0.9.7. The execution flow is divided in __stages__ with independent operations and goals, to facilitate the development of phase-related functionalities and data structures, while also improving computational efficiency. Each stage runs on a different thread and communication with the rest is managed through synchronous messaging between the stages, so the information flows only in one direction.  

#### User customization
Users must have some control over the traffic flow by setting parameters that allow, force or encourage specific behaviours. Thus, users can change the traffic behaviour as they prefer, both online and offline. For example they could allow a car to ignore the speed limits or force a lane change. Being able to play around with behaviours is a must when trying to simulate reality, specially to train driving systems under specific and atypical circumstances. 

---
## How does it work?

#### Architecture
The following diagram is a summary of the internal architecture of the Traffic Manager. Blue bodies represent the different stages, green ones are additional modules that work with these and the arrows represent communication between elements managed by messenger classes.  
The inner structure of the TM can be easily translated to code. Each relevant element has its equivalent in the C++ code (.cpp files) found inside `LibCarla/source/carla/trafficmanager`. 

<div style="text-align:center">
<img src="../img/traffic_manager_diagram.png">
</div>

#### Stages

__1. Localization Stage:__ the TM stores a list of waypoints ahead for each vehicle to follow. The list of waypoints is updated each iteration, changing according to the decisions taken during the stage, such as lane changes, to modify the vehicle's trajectory. The amount of waypoints stored depends on the vehicle's speed, being greater the faster it goes. This stage contains a __spatial hashing__ which saves the position for every car __registered to the Traffic Manager in a world grid__. This is a way to roughly predict possible collisions and create a list of overlapping actors for every vehicle that will be later used by the next stage.  

* __Related .cpp files:__ `LocalizationStage.cpp` and `LocalizationUtils.cpp`.  

__2. Collision Stage:__ checks possible collisions for every vehicle. For each pairs of overlapping actors in the list stored by the Localization stage, it extends a __geodesic boundary__. These are extended bounding boxes that represent the vehicle along its trajectory. This stage determines which vehicle has priority and communicates the result to the __Motion Planner stage__.  

* __Related .cpp files:__ `CollisionStage.cpp`.  

__3. Traffic Light Stage:__ manages some general traffic regulations, mainly priority at junctions. A _traffic hazard__ is set to true whenever a yellow or red traffic light is detected. Non-signalized junctions are managed with a priority system. When approaching to these, a __geodesic boundary__ is extended through the intersection along the intended trajectory. The vehicle will wait for the junction to be free if another vehicle is detected inside of the geodesic boundary.  

* __Related .cpp files:__ `TrafficLightStage.cpp`.  

__4. Motion Planner Stage:__ aggregates all the information from the previous stages and makes decisions on how to move the vehicles. It is asisted by a PID controller to adjust the resulting behaviour. After computing all the commands needed for every vehicle, these are sent to the final stage. For example, when facing a __traffic hazard__, this stage will compute the brake needed for said vehicle and communicates it to the Apply Control stage.  

* __Related .cpp files:__ `MotionPlannerStage.cpp`.  

__5. Apply Control Stage:__ receives actuation signals, such as throttle, brake, steer, from the Motion Planner stage and sends these to the simulator in batches to control every vehicles' movement. Using the __apply_batch()__ method in [carla.Client](../python_api/#carla.Client) and different [carla.VehicleControl](../python_api/#carla.Client) for the registered vehicles.  

* __Related .cpp files:__ `BatchControlStage.cpp`.  

#### Additional modules

__Cached map:__ in order to increase computational efficiency during the Localization stage, the map is discretized and cached as a grid of waypoints. These are included in a specific data structure designed to hold more information, such as links between them. The grids allow to easily connect the map, each of them representing sections of a road or a whole junction, by also having an ID that is used to quickly identify vehicles in nearby areas.  

* __Related .cpp files:__ `InMemoryMap.cpp` and `SimpleWaypoint.cpp`.  

__PID controller:__ the TM module uses a PID controller to regulate throttle, brake and steering according to a target value. The way this adjustment is made depends on the specific parametrization of the controller, which can be modified if the desired behaviour is different. Read more about [PID compensation](https://commons.wikimedia.org/wiki/File:PID_Compensation_Animated.gif) to learn how to do it.  

* __Related .cpp files:__ `PIDController.cpp`.  

---
## Using the Traffic Manager 

#### General considerations

First of all there are some general behaviour patterns the TM will generate that should be understood beforehand. These statements are inherent to the way the TM is implemented:  

* __Vehicles are not goal-oriented__ they follow a trajectory and whenever approaching a junction, choose a path randomly. Their path is endless, and will never stop roaming around the city.  
* __Vehicles' target speed is 70% their current speed limit:__ unless any other value is set. 
* __Junction priority does not follow traffic regulations:__ the TM has a priority system to be used while junction complexity is solved. This may cause some issues such as a vehicle inside a roundabout yielding to a vehicle trying to get in. 

The TM provides a set of possibilities so the user can establish specific behaviours. All the methods accessible from the Python API are listed in the [documentation](../python_api/#carla.TrafficManager). However, here is a brief summary of what the current possibilities are. 

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

#### Creating a Traffic Manager

A TM instance can be created by any [carla.Client](python_api.md#carla.Client) specifying the port that will be used. The default port is `8000`.  

```python
tm = client.get_trafficmanager(port)
```

Now the TM needs some vehicles to be in charge of. In order to do so, enable the autopilot mode for the set of vehicles to be managed. In case the client is not connected to any TM, an instance will be created with default presets.

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

The CARLA server keeps register of all the TM instances internally by storing the port and also the client IP (hidden to the user) that link to them. Right now there is no way to check the TM instances that have been created so far. A connection will always be attempted when trying to create an instance and it will either create a new __TM-Server__ or a __TM-Client__.  

#### Multiclient VS MultiTM

Based on the different definitions for a TM, successfully creating a TM can lead to two different results:  

* __TM-Server:__ when the port is free. This type of TM is in charge of its own logic, managed in `TrafficManagerLocal.cpp`.  

* __TM-Client:__ when the port is occupied. It is not in charge of its own logic. Instead, it asks for changes in the parameters of the __TM-Server__ it is connected to in  `TrafficManagerRemote.cpp`.

That creates a clear distinction between having multiple clients and multiple Traffic Managers running: 

* __Multiclient scenario:__ when there is only one TM-Server, but there are other TM-Clients created with the same port definition that are actually connecting to said TM-Server.  

* __MultiTM scenario:__ when there are different TM-Server, created with different port definitions.  

!!! Note 
    The class `TrafficManager.cpp` acts as a central hub managing all the different TM instances. 

---
## Other considerations

The TM is a module constantly evolving and trying to adapt the range of possibilities that it presents. For instance, in order to get more realistic behaviours we can have many clients with different TM in charge of sets of vehicles with specific and distinct behaviours. This range of possibilities also makes for a lot of different configurations that can get really complex and specific. For such reason, here are listed of considerations that should be taken into account when working with the TM as it is by the time CARLA 0.9.8 is released: 

#### FPS limitations

The TM stops working properly in asynchronous mode when the simulation is under 20fps. Below that rate, the server is going much faster than the client containing the TM and behaviours cannot be simulated properly. For said reason, under these circumstances it is recommended to work in __synchronous mode__.  

!!! Important
    The FPS limitations are specially relevant when working in the night mode. 

#### Synchronous mode

TM-Clients cannot tick the CARLA server in synchronous mode, __only a TM-Server can call for a tick__.  
If more than one TM-Server ticks, the synchrony will fail, as the server will move forward on every tick. This is specially relevant when working with the __ScenarioRunner__, which runs a TM. In this case, the TM will be subordinated to the ScenarioRunner and wait for it. 

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
