<h1>Traffic manager</h1>
  * [__What is it?__](#what-is-it)  
  * [__How does it work?__](#how-does-it-work)  
	* Architecture
	* Stages
  * [__Using the traffic manager__](#using-the-traffic-manager)
	* Parameters
	* Creating a traffic manager
	* Setting the traffic manager
  * [__Other considerations__](#other-considerations)
	* FPS limitations  
	* Multiclient management  
  * [__Summary__](#summary)

---------------
##What is it? 

The traffic manager is a module build on top of the CARLA API that handles any group of vehicles in order to simulate specific behaviours. This module manages every [carla.Vehicle]() that has been set to autopilot mode. Its main goal is to improve the traffic around the city so that the simulation runs under realistic conditions. While doing so, another goal is to give the users the option to customize the simulated traffic like never before.
To achieve that, the traffic manager has to follow two ground principles:  

<h4>Structured design</h4>
The traffic manager remains on the client-side of the CARLA architecture, in order to save the server some work and don't interfere with the simulation performance. For said reason, the flow of information is divided in __stages__ that help improving computacional efficiency and provide stage-related data structures that vehicles can share to commmunicate. This clear distintion between different sub-modules inside the traffic manager that are accessed independently using different is central to the whole manager. It avoids potential bottlenecks in managing collisions and allows negotiation between cars. 

<h4>User customization</h4>  
Users need to be granted some control by setting parameters that allow, force or encourage certain behaviours. Thus, users can change the traffic behaviour as they prefer, both online and offline. For example they could allow a car to ignore the speed limits or force a lane change. Being able to play around with behaviours is a must when trying to simulate reality, specially to train driving systems under specific and atypical circumstances. 

* _Having many traffic managers, which gets the vehicle when autopilot is set to true?_

---------------
##How does it work?

<h4>Architecture</h4>
The trafic manager is a module build on top of the CARLA API in C++. The inner structure is divided in stages can be somehow visualized through code, they have their equivalent in the c++ code (.cpp files) foun inside `/libcarla`. There are other inner parts of the traffic manager, such as as setting up the map, but they are managed internally and mostly only done once, so these are not as fundamental to understand the whole.  
Each stage class runs on a different thread and has its independent operations and goals. Communication with the rest is managed through messages __facilitating synchronous communication__. The information flows only in one direction. In this manner, for each stage or class there are methods and tools that can be located in one or many stages with a very specific structure and purpose. 
The following diagram is a summary of the internal architecture of the traffic manager. Main bodies are the different stages, blue arrows represent messenger classes that facilitate __asynchronous communication?__, __black arrows represent non-concurrent data flow__. 

<div style="text-align:center">
<img src="../img/traffic_manager_diagram.png">
</div>

* _What does it mean facilitating synchrony? Doesn't it work asynchronous mode? It does._


<h4>Stages</h4>
1. __Localization Stage:__ For each vehicle subscribed to the traffic manager, maintains a list of waypoints ahead it to follow. The buffer list of waypoints is not created every iteration but updated. The amount of waypoints contained in said list varies depending on the vehicle's speed, being greater the faster it goes. The localization stage contains a __spacial hashing__ which summarizes the position for every car __subscribed to the traffic manager in a world grid__. This is a way to roughly predict possible collisions and create a __list, touple?__ of overlapping actors that will be later used by the next stage.   

!!! Important
    The localization stage also manages lane change decisions by deleting the previous buffer of waypoints and creating a new list with the corresponding trajectory. This feature has been temporarily disabled until some bugs are fixed.  

2. __Collision Stage:__ Checks possible collisions for every vehicle along its trajectory. For each pairs of overlapping actors in the list contained by the localization stage, it extends a __geodesic boundary__. These are extended bounding bodies that represent the vehicle along its trajectory. If two geodesic boundaries collide, a __collision hazard__ is set to true for them. The hazards will later be managed by the __Motion planer stage__.  

3. __Traffic Light Stage:__ Manages some general traffic regulations, mainly priority at junctions. Whenever there is a traffic light, if red, a __traffic hazard__ is set to true. Non signaled junctions are managed with a ticket system. When coming closer to these, a __geodesic boundary__ is created and, if there is another vehicle inside of it, the parent one shall wait some time before continuing its way.  

4. __Motion Planner Stage:__ Aggregates all the information gathered through the previous stages. With their guidance adjusted with actuation signals provided by a PID controller, this stage makes decisions on how to move the vehicles. It computes all the commands needed for every vehicle and then send these to the final stage. For example, when facing a __traffic hazard__, this stage will compute the brake needed for said vehicle and communicate it to the apply control stage. 

5. __Apply Control Stage:__ Receives actuation signals (such as throttle, brake, steer) from the Motion planner stage and commands these to the simulator in batches to control every vehicles' movement. It mainly uses the __apply_batch()__ method in [carla.Client](../python_api/#carla.Client).  

---------------
##Using the traffic manager 

First of all there are some general behaviour patterns the traffic manager will generate that should be understood beforehand. These statements inherent to the way the traffic manager is implemented:  

* __Vehicles are not goal-oriented__ They follow a road and whenever a junction appears, choose a path randomly.
* __Vehicles' intended speed is their current speed limit:__ Unless any other behaviour is set. Remember that the default maximum speed is 30km/h when spawned. 
* __Lane change is currently disabled:__ It can be enabled through the API, though right now some issues may happen. Disabling lane changes will also ban overtakings between vehicles in the same lane. 
* __Vehicles will never stop:__ Unless something unintended occurs. 
* __Junction priority does not follow traffic regulations:__ The traffic manager has a ticket system to be used while junction complexity is solved. This may cause some issues such as a vehicle inside a roundabout yielding to a vehicle trying to get in. 

<h4>Creating a traffic manager</h4>

Traffic manager is contained inside the libcarla, so an instance of it can be received to then be accessed and configured. The traffic manager needs a client to run on, so when getting an instance, the container client is needed:  

```python
tm = client.GetTrafficManager(client)
```

Now the traffic manager needs some vehicles to be in charge of. In order to do so, we can provide a specific list of actors that should be subscribed to it. 

```python
tm.register_vehicles(vehicles_list)
``` 
__What happens if I add an actor that is not a vehicle such as walker, traffic sign or sensor?__

The vehicles will not start running yet. The autopilot needs to be turned on: 

```python
 for v in vehicles_list:
     v.set_autopilot(True)
```

Alternatively, if the autopilot is turned on for any vehicle, it will be automatically registered. In case no traffic manager currently exists, an instance will be created __where?__. This is the common behaviour when spawning many vehicles at once via batch command.  
The reason to register vehicles with the autopilot off is to __make the manager aware of them?__

```python
v = world.spawn_actor(car_blueprint,transform)
v.set_autopilot(True)
``` 

__What happens if there are many traffic managers?__  

__How can I check if there are traffic managers running and where?__  

__What is with spawn_actor and tm_spawn_actor?__  

__Is this the right way to use traffic manager?__  


<h4>Setting a traffic manager</h4>

The traffic manager provides with a set of possibilities that can interfere with how the traffic is conducted or set specific rules for a vehicle. Thus, the user can aim for different behaviours in both a general and specific sense. All the methods accessible from the Python API are listed in the [documentation](../python_api/#carla.TrafficManager). However, here is a brief summary of what the current possibilities are. 

__Traffic manager fundamental methods:__  

* Get a traffic manager instance from a client.  
* Register/Unregister vehicles from the traffic manager's lists.  

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

The following example creates an instance of the traffic manager in default port `8000` (in case there is none) and sets a dangerous behaviour for a specific car that will ignore all traffic lights, leave no safety distance with the rest and drive at 120% its current speed limit. 

```python
tm = client.GetTrafficManager(client)
tm.register_vehicles(my_vehicles)
danger_car = my_vehicles[0]
tm.ignore_lights_perc(danger_car,100)
tm.set_distance_to_leading_vehicle(danger_car,0)
tm.set_vehicle_max_speed_difference(danger_car,-20)
``` 

Now, here is an example that registers that same list of vehicles but instead is set to conduct them with a moderate behaviour. The vehicles will drive at 80% their current speed limit, leaving at least 5 meters between them and never perform a lane change.
```python
tm = client.GetTrafficManager(client)
tm.register_vehicles(my_vehicles)
danger_car = my_vehicles[0]
tm.set_global_distance_to_leading_vehicle(5)
tm.set_global_max_speed_difference(80)
for v in my_vehicles: 
  tm.set_auto_lane_change(v,False)
``` 

!!! Important 
    Lane changes are currently disabled in the traffic manager due to unintended collisions causing jams. As long as this issues are not fixed, vehicles will remain in the lane they are spawned and the methods to set lane changes will be disabled. 


<h4>Stopping a traffic manager</h4>

> Destroy?
> What happens? 

---------------
##Other considerations

The traffic manager is a module constantly envolving and trying to adapt the range of possibilities that it presents. For instance, in order to get more realistic behaviours we can have many clients with different traffic managers in charge of sets of vehicles with specific and distinct behaviours. This range of possibilities also makes for a lot of different configurations that can get really complex and specific. For such reason, here are listed of considerations that should be taken into account when working with the traffic manager as it is by the time CARLA 0.9.8 is released: 

<h4>FPS limitations</h4>

The traffic manager stops working properly when the simulation is under 20fps. Below that rate, the server is going much faster than the client containing the traffic manager and behaviours cannot be simulated properly. This is specially relevant when working in the night mode __table with relation between cars+night mode+city to fps?__
For said reason, under these circumstances it is recommended to work in __synchronous mode__.  

!!! Warning
    Right now there is an issue when different clients work in __synchronous mode__ with the server, as the simulation waits for a tick that could come from any client, thus breaking the synchrony with the rest. This issue is specially relevant when working in synchronous with the __scenario runner__, which runs a traffic manager. For said reason, right know the traffic manager will be subordinated to the scenario runner and wait for it. 

<h4>Multiclient management</h4>

When working with different clients containing different traffic managers, understanding inner implementation of the traffic manager in the client-server architecture becomes specially relevant. There is one ruling these scenarios: __the port is the key__.  

A client (cl) creates a traffic manager (tm) by communicating with the server (s) and passing the intended port to be used for said purpose. The port can either be stated or not, using the default as `8000`.  

`cl_01 --> s --> tm_01 (p=8000)`  
`cl_02(p=5000) --> s --> tm_02 (p=5000)`  

If a client tries to create a traffic manager using a port that is already assigned to another traffic manager, it will not create its own but connect to the one already existing. The reason for this is that __there cannot be two traffic managers sharing the same port__. This is specially relevant when using the default creation, as this will only create a traffic manager in the client if port `8000` has not been assigned yet. Otherwise, it  will connect with the traffic manager created (in this case) by `cl_01` in port `8000`.  

`cl_03 --> s --> tm_01 (p=8000)`  
`cl_04(p=5000) --> s --> tm_02 (p=5000)`  
`cl_05(p=6000) --> s --> tm_03 (p=6000)`  

The omniscient server keeps register of all these things internally and everytime a traffic manager is created, it stores the port and also the client ip that links to it. For said reason, traffic managers are also accessible using both the ip and the port, but this case will return an error in case any of the pair is wrong.  

`cl_06(ip=cl_02,p=6000) --> s --> ERROR`  
`cl_07(ip=cl02,p=5000) --> s --> tm_02 (p=5000)`  


__IS THERE ANYTHING ELSE WORTH MENTIONING?? ANYTHING ELSE THAT BREAKS IT??__


---------------
## Summary


The traffic manager is one of the most complex features in CARLA and so, one that is prone to all kind of unexpected and really specific issues. The CARLA forum is open to everybody to post any doubts or suggestions, and it is the best way to keep track of issues and help the CARLA community to become greater. Feel free to login and join the community. 

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</p>
</div>
