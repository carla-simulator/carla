# Traffic Manager

- [__What is the Traffic Manager?__](#what-is-the-traffic-manager)
	- [Structured design](#structured-design)
	- [User customization](#user-customization)
- [__Architecture__](#architecture)
	- [Overview](#overview)
	- [ALSM](#alsm)
	- [Vehicle registry](#vehicle-registry)
	- [Simulation state](#simulation-state)
	- [Control loop](#control-loop)
	- [In-Memory Map](#in-memory-map)
	- [PBVT](#pbvt)
	- [PID controller](#pid-controller)
	- [Command array](#command-array)
	- [Stages of the Control Loop](#stages-of-the-control-loop)
- [__Using the Traffic Manager__](#using-the-traffic-manager)
	- [General considerations](#general-considerations)
	- [Creating a Traffic Manager](#creating-a-traffic-manager)
	- [Configuring autopilot behaviour](#configuring-autopilot-behaviour)
	- [Stopping a Traffic Manager](#stopping-a-traffic-manager)
- [__Deterministic mode__](#deterministic-mode)
- [__Hybrid physics mode__](#hybrid-physics-mode)
- [__Running multiple Traffic Managers__](#running-multiple-traffic-managers)
	- [Definitions](#definitions)
	- [Multiclient](#multiclient)
	- [MultiTM](#multitm)
	- [Multisimulation](#multisimulation)
- [__Synchronous mode__](#synchronous-mode)

---
## What is the Traffic Manager?

The Traffic Manager (TM) is the module that controls vehicles in autopilot mode in a simulation. Its goal is to populate a simulation with realistic urban traffic conditions. Users can customize some behaviours, for example to set specific learning circumstances.

### Structured design

TM is built on CARLA's client side. It replaces the server-side autopilot. The execution flow is divided into __stages__, each with independent operations and goals. This facilitates the development of phase-related functionalities and data structures, whilst improving computational efficiency. Each stage runs on a different thread. Communication with the rest (of what) is managed through synchronous messaging between stages. Information flows in one direction.

### User customization
Users have some control over the traffic flow by setting parameters that allow, force or encourage specific behaviours. Users can change the traffic behaviour as they prefer, both online and offline. For example they could allow a car to ignore the speed limits or force a lane change. Being able to play around with behaviours is indispensible when trying to simulate reality. Driving systems need to be trained under specific and atypical circumstances.

---
## Architecture

### Overview

![Architecture](img/tm_2_architecture.jpg)

The above diagram is a representation of the internal architecture of the TM. The C++ code for each component can be found in `LibCarla/source/carla/trafficmanager`. Each component is explained in detail in the following sections. A simplified overview of the logic is as follows: 

__1. Store and update the current state of the simulation.__

- The [Agent Lifecycle & State Management](#alsm) (ALSM) scans the world to keep track of all the vehicles and walkers present and to clean up entries for those that no longer exist. All the data is retrieved from the server and is passed through several [stages](#stages). (In such way, calls to the server are isolated in the ALSM, and this information can be easily accessible onwards. (Does this mean that the ALSM is the only component that makes calls to the server in order to make the information accessible to the other components?)) 
- The [vehicle registry](#vehicle-registry) contains an array of vehicles on autopilot (controlled by the TM), and a list of pedestrians and vehicles not on autopilot (not controlled by the TM).
- The [simulation state](#simulation-state) is a cache store of the position, velocity and additional information of all the vehicles and pedestrians in the simulation.

__2. Calculate the movement of every autopilot vehicle.__

The TM generates viable commands for all vehicles in the [vehicle registry](#vehicle-registry) according to the [simulation state](#simulation-state). Calculations for each vehicle are done separately. These calculations are divided into different [stages](#stages). The [control loop](#control-loop) makes sure that all calculations are consistent by creating __synchronization barriers__ between stages. No vehicle moves to the next stage before calculations are finished for all vehicles in the current stage. Each vehicle goes through the following stages:

>__2.1 - [Localization Stage](#stage-1-localization-stage).__

>Vehicles in autopilot do not have predefined routes. Instead, paths are created dynamically using a list of nearby waypoints collected from the [In-Memory Map](#in-memory-map), a simplification of the simulation map as a grid of waypoints. Directions at junctions are chosen randomly. Each vehicles's path is stored by the [Path Buffers & Vehicle Tracking](#pbvt) (PBVT) component for easy access and modification in future stages. (There is an explanation here that belongs better in the section above. What is it that actually happens in the localization stage?)

>__2.2 - [Collision Stage](#stage-2-collision-stage).__

>Bounding boxes are extended over each vehicles's path to identify and navigate potential collision hazards.

>__2.3 - [Traffic Light Stage](#stage-3-traffic-light-stage).__

>Similar to the Collision Stage, potential hazards that affect each vehicle's path due to traffic light influence, stop signs, and junction priority are identified.

>__2.4 - [Motion Planner Stage](#stage-4-motion-planner-stage).__

>Vehicle movement is computed based on the defined path. A [PID controller](#pid-controller) determines how to reach the target waypoints. This is then translated into a CARLA command for application in the next step.

__3. Apply the commands in the simulation.__

Commands generated in the previous step are collected into the [command array](#command-array) and sent to the CARLA server in a batch to be applied in the same frame.

The following sections will explain each component and stage in the TM logic described above in more detail.

### ALSM

ALSM stands for __Agent Lifecycle and State Management__. It is the first step in the TM logic cycle and provides context of the current state of the simulation.

The ALSM component:

- Scans the world to keep track of all vehicles and pedestrians, their positions and velocities. If physics are enabled, the velocity is retrieved by [Vehicle.get_velocity()](python_api.md#carla.Vehicle). Otherwise, the velocity is calculated using the history of position updates over time.
- Stores the position, velocity and additional information (traffic light influence, bounding boxes, etc) of every vehicle and pedestrian in the [simulation state](#simulation-state) component.
- Updates the list of TM controlled vehicles in the [vehicle registry](#vehicle-registry).
- Updates entries in the [control loop](#control-loop) and [PBVT](#pbvt) components to match the vehicle registry.

__Related .cpp files:__ `ALSM.h`, `ALSM.cpp`.

### Vehicle registry

The vehicle registry keeps track of all vehicles and pedestrians in the simulation.

The vehicle registry:

- The [ALSM](#alsm) scans the world and passes the vehicle registry an updated list of vehicles and pedestrians.
- Vehicles registered to the TM are stored in a separate array for iteration during the [control loop](#control-loop).

__Related .cpp files:__ `MotionPlannerStage.cpp`.

### Simulation state

The simulation state stores information about all vehicles in the simulation for easy access and modifcation in later stages.

The simulation state:

- Receives the current state of all vehicles and pedestrians in the simulation from the [ALSM](#alsm). This includes position, velocity, traffic light influence, traffic light state, etc.
- Stores all information in cache so no additional calls to the server are needed during the [control loop](#control-loop).

__Related .cpp files:__ `SimulationState.cpp`, `SimulationState.h`.

### Control loop

The control loop manages the calculations of the next command for all vehicles controlled by the TM so they can be performed synchronously. The control loop consists of four different [stages](#stages); localization, collision, traffic light and motion planner.

The control loop:

- Receives an array of TM controlled vehicles from the [vehicle registry](#vehicle-registry).
- Performs calculations for each vehicle separately by looping over the array.
- Calculations are divided into a series of [stages](#stages).
- Creates synchronization barriers between stages to guarantee consistency. Calculations for all vehicles are finished before any move to the next stage, ensuring all vehicles are updated in the same frame.
- Coordinates the transition between [stages](#stages) so all calculations are done in sync.
- When the last stage of the control loop ([Motion Planner Stage](#stage-4-motion-planner-stage)) finishes, the [command array](#command-array) is sent to the server so there are no frame delays between the calculations of the control loop, and the commands being applied.

__Related .cpp files:__ `TrafficManagerLocal.cpp`.

### In-Memory Map

The In-Memory Map is a helper module contained within the [PBVT](#pbvt) and is used during the [Localization Stage](#stage-1-localization-stage).  

The In-Memory Map:

- Converts the map into a grid of discrete waypoints.
- Includes waypoints in a specific data structure with more information to connect waypoints and identify roads, junctions, etc.
- Identifies these structures with an ID used to quickly locate vehicles in nearby areas.

__Related .cpp files:__ `InMemoryMap.cpp` and `SimpleWaypoint.cpp`.

### PBVT

PBVT stands for __Path Buffer and Vehicle Tracking__. The PBVT is a data structure that contains the expected path for every vehicle and allows easy access to data during the [control loop](#control-loop).

The PBVT:

- Contains a map of deque objects with one entry per vehicle.
- Xontains a set of waypoints for each vehicle describing its current location and near-future path.
- Contains the [In-Memory Map](#in-memory-map) that will be used by the [Localization Stage](#stage-1-localization-stage) to relate every vehicle to the nearest waypoint and possible overlapping paths.

### PID controller

The PID controller is a helper module that performs calculations during the [Motion Planner Stage](#stage-4-motion-planner-stage).

The PID controller:

- Estimates the throttle, brake and steering input needed to reach a target value using the information gathered by the [Motion Planner Stage](#stage-4-motion-planner-stage).
- Makes adjustments depending on the specific parameterization of the controller. Parameters can be modified if desired. Read more about [PID controllers](https://en.wikipedia.org/wiki/PID_controller) to learn how to make modifications.

__Related .cpp files:__ `PIDController.cpp`.
### Command Array

The Command Array represents the last step in the TM logic cycle. It receives commands for all the registered vehicles and applies them.

The Command Array:

- Receives a series of [carla.VehicleControl](python_api.md#carla.VehicleControl) from the [Motion Planner Stage](#stage-4-motion-planner-stage).
- Batches all commands to be applied during the same frame.
- Sends the batch to the CARLA server calling either __apply_batch()__ or __apply_batch_synch()__ in [carla.Client](../python_api/#carla.Client) depending if the simulation is running in asynchronous or synchronous mode respectively.

__Related .cpp files:__ `TrafficManagerLocal.cpp`.

### Stages of the Control Loop

##### Stage 1- Localization Stage

The Localization Stage defines a near-future path for vehicles controlled by the TM.

The Localization Stage:

- Obtains the position and velocity of all vehicles from the [simulation state](#simulation-state).
- Uses the [In-Memory Map](#in-memory-map) to relate every vehicle with a list of waypoints that describes its current location and near-future path according to its trajectory. The faster the vehicle goes, the larger the list will be.
- Updates the path according to planning decisions such as lane changes, speed limit, distance to leading vehicle parameterization, etc.
- Stores the path for all vehicles in the [PBVT](#pbvt) module.
- Compares paths with each other to estimate possible collision situations. Results are passed to the Colllision Stage.

__Related .cpp files:__ `LocalizationStage.cpp` and `LocalizationUtils.cpp`.

##### Stage 2- Collision Stage

The Collision Stage triggers collision hazards.

The Collision Stage:

- Receives a list vehicle pairs whose paths could potentially overlap from the Localization Stage.
- Extends bounding boxes along the path ahead (geodesic boundaries) for each vehicle pair to check if they actually overlap and determine wether the risk of collision is real.
- Sends hazards for all possible collisions to the [Motion Planner Stage](#stage-4-motion-planner-stage) to modify the path accordingly.

__Related .cpp files:__ `CollisionStage.cpp`.

##### Stage 3- Traffic Light Stage

The Traffic Light stage triggers hazards due to traffic regulators such as traffic lights, stop signs, and priority at junctions.

The Traffic Light stage:

- Sets a traffic hazard if a vehicle is under the influence of a yellow or red traffic light or a stop sign.
- Extends a bounding box along a vehicle's path if it is in an unsignaled junction. Vehicles with overlapping paths follow a "First-In First-Out" order to move and wait-times are set to a fixed value.

__Related .cpp files:__ `TrafficLightStage.cpp`.

##### Stage 4- Motion Planner Stage

The Motion Planner Stage generates the CARLA commands to be applied to vehicles.

The Motion Planner Stage:

- Gathers a vehicle's position and velocity ([simulation state](#simulation-state)), path ([PBVT](#pbvt)) and hazards ([Collision Stage](#stage-2-collision-stage) and [Traffic Light Stage](#stage-3-traffic-light-stage)).
- Makes high-level decisions about how a vehicle should move, for example, computing the brake needed to prevent a collision hazard. A [PID controller](#pid-controller) is used to estimate behaviors according to target values.
- Translates the desired movement to a [carla.VehicleControl](python_api.md#carla.VehicleControl) that can be applied to the vehicle.
- Sends the resulting CARLA commands to the [Command Array](#command-array).

__Related .cpp files:__ `MotionPlannerStage.cpp`.

---
## Using the Traffic Manager

### General considerations

The TM implements general behaviour patterns that must be taken into consideration when you set vehicles to autopilot:

- __Vehicles are not goal-oriented,__ they follow a dynamically produced trajectory and choose a path randomly when approaching a junction. Their path is endless.
- __Vehicles' target speed is 70% their current speed limit,__ unless any other value is set.
- __Junction priority does not follow traffic regulations.__ The TM uses its own priority system at junctions. The resolution of this restriction is a work in progress. In the meantime, some issues may arise, for example, vehicles inside a roundabout yielding to a vehicle trying to get in.

TM behaviour can be adjusted through the Python API. For specific methods, see the TM section of the Python API [documentation](../python_api/#carla.TrafficManager). Below is a general summary of what is possible through the API:

| Topic | Description |
| ----- | ----------- |
| **General:** | - Create a TM instance connected to a port. <br> - Retrieve the port where a TM is connected. |
| **Safety conditions:** | - Set a minimum distance between stopped vehicles (for a single vehicle or for all vehicles). This will affect the minimum moving distance. <br> - Set a desired speed as a percentage of the current speed limit (for a single vehicle or for all vehicles). <br> - Reset traffic lights. |
| **Collision  managing:** | - Enable/Disable collisions between a vehicle and a specific actor. <br> - Make a vehicle ignore all other vehicles. <br> - Make a vehicle ignore all walkers. <br> - Make a vehicle ignore all traffic lights. |
| **Lane  changes:** | - Force a lane change, ignoring possible collisions. <br> - Enable/Disable lane changes for a vehicle. |
| **Hybrid physics mode:** | - Enable/Disable hybrid physics mode. <br> - Change the radius in which physics is enabled. |


### Creating a Traffic Manager

A TM instance is created by a [`carla.Client`](python_api.md#carla.Client), passing the port to be used. The default port is `8000`.

To create a TM instance:

```python
tm = client.get_trafficmanager(port)
```

To enable autopilot for a set of vehicles, retrieve the port of the TM instance and set `set_autopilot` to `True`, passing the TM port at the same time. If no port is provided, it will try to connect to a TM in the default port (`8000`). If the TM does not exist, it will create one:

```python
tm_port = tm.get_port()
 for v in vehicles_list:
     v.set_autopilot(True,tm_port)
```
!!! Note 
    Creating or connecting to a TM in multi-client situations is different from the above example. Learn more in the section [*Running multiple Traffic Managers*](#running-multiple-traffic-managers).

The `generate_traffic.py` script in `/PythonAPI/examples` provides an example of how to create a TM instance using a port passed as a script argument and register every vehicle spawned to it by setting the autopilot to `True` in a batch:

```py
traffic_manager = client.get_trafficmanager(args.tm-port)
tm_port = traffic_manager.get_port()
...
batch.append(SpawnActor(blueprint, transform).then(SetAutopilot(FutureActor, True,tm_port)))
...
traffic_manager.global_percentage_speed_difference(30.0)
```

### Configuring autopilot behaviour

The following example creates a TM instance and configures dangerous behaviour for a specific vehicle so it will ignore all traffic lights, leave no safety distance from other vehicles and drive at 120% the current speed limit:

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

The example below sets the same list of vehicles to autopilot, but instead configures them with moderate driving behaviour. The vehicles drive at 80% the current speed limit, leaving at least 5 meters between themselves and other vehicles, and never perform lane changes:

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

The TM is not an actor that needs to be destroyed, it will stop when the client that created it stops. This is automatically managed by the API, the user does not have to do anything. However, when shutting down a TM, the user must destroy the vehicles controlled by it, otherwise they will remain immobile on the map. The script `generate_traffic.py` does this automatically:

```py
client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])
```

!!! Warning 
    Shutting down a __TM-Server__ will shut down the __TM-Clients__ connecting to it. To learn the difference between a __TM-Server__ and a __TM-Client__ read about [*Running multiple Traffic Managers*](#running-multiple-traffic-managers). 

---
## Deterministic mode

In deterministic mode, the TM will produce the same results and behaviours under the same conditions. Do not mistake determinism with the recorder. While the recorder allows you to store the log of a simulation to play it back, determinism ensures that the TM will always have the same output over different executions of a script as long as the same conditions are maintained.

Deterministic mode is available __in synchronous mode only__. In asynchronous mode, there is less control over the simulation and determinism cannot be achieved. Read more in the section [__TM in synchronous mode__](#synchronous-mode) before starting.

To enable deterministic mode, use the following method:

```py
my_tm.set_random_device_seed(seed_value)
``` 

`seed_value` is an `int` number from which random numbers will be generated. The value itself is not relevant, but the same value will always result in the same output. Two simulations, with the same conditions, that use the same seed value, will be deterministic.

Deterministic mode can be tested in the `generate_traffic.py` example script by passing a seed value as an argument. The following example sets the seed to an arbitrary value of `9`:

```sh
cd PythonAPI/examples
python3 generate_traffic.py -n 50 --sync --seed 9
```

!!! Warning
    Make sure to set both the world and the TM to synchronous mode before enabling deterministic mode. 

---
## Hybrid physics mode

Hybrid mode allows users to disable most physics calculations for all autopilot vehicles, or for autopilot vehicles outside of a certain radius of a vehicle tagged with `hero`. This removes the vehicle physics bottleneck from a simulation. Vehicles whose physics are disabled will move by teleportation. Basic calculations for linear acceleration are maintained to ensure position updates and vehicle speed remain realistic and the toggling physics calculations on vehicles is fluid. 

Hybrid mode uses the [`Actor.set_simulate_physics()`](https://carla.readthedocs.io/en/latest/python_api/#carla.Actor.set_simulate_physics) method to toggle physics calculations. It is disabled by default. There are two options to enable it:

*   [__`TrafficManager.set_hybrid_physics_mode(True)`__](https://carla.readthedocs.io/en/latest/python_api/#carla.TrafficManager.set_hybrid_physics_mode) — This method enables hybrid mode for the TM object calling it.
*   __Running `generate_traffic.py` with the flag `--hybrid`__ — This example script creates a TM and spawns vehicles in autopilot. It then sets these vehicles to hybrid mode when the `--hybrid` flag is passed as a script argument.

To modify the behaviour of hybrid mode, use the following two parameters:

*   __Radius__ *(default = 50 meters)* — The radius is relative to vehicles tagged with `hero`. All vehicles inside this radius will have physics enabled; vehicles outside of the radius will have physics disabled. The size of the radius is modifed using [`traffic_manager.set_hybrid_physics_radius(r)`](python_api.md#carla.TrafficManager.set_hybrid_physics_radius).
*   __Hero vehicle__ — A vehicle tagged with `role_name='hero'` that acts as the center of the radius.
	*   __If there is no hero vehicle,__ all the vehicles will disable physics.
	*   __If there is more than one hero vehicle,__ the radius will be considered for them all, creating different areas of influence with physics enabled.  

The clip below shows how physics are enabled and disabled when hybrid mode is active. The __hero vehicle__ is tagged with a __red square__. Vehicles with __physics disabled__ are tagged with a __blue square__. When inside the hero vehicles radius of influence, __physics are enabled and the tag becomes green__.

![Welcome to CARLA](img/tm_hybrid.gif)


---
## Running multiple Traffic Managers

### Definitions

When working with different clients containing different TMs, understanding the inner implementation of the TM in the client-server architecture becomes especially relevant. There is one key to ruling these scenarios: __the port__.  

A client creates a TM by communicating with the server and passing the intended port to be used for that purpose. The port can either be stated or not, using the default of `8000`.  


* __TM-Server__ — The port is free. This type of TM is in charge of its own logic, managed in `TrafficManagerLocal.cpp`. The following code creates two TM-Servers. Each one connects to a different port, not previously used. 

```py 
tm01 = client01.get_trafficmanager() # tm01 --> tm01 (p=8000)
```
```py
tm02 = client02.get_trafficmanager(5000) # tm02(p=5000) --> tm02 (p=5000)
```

* __TM-Client__ — The port is occupied by another TM. These instances are not in charge of their own logic. Instead, they ask for changes in the parameters of the __TM-Server__ they are connected to in  `TrafficManagerRemote.cpp`. The following code creates two TM-Clients, that connect with the TM-Servers previously created. 

```py 
tm03 = client03.get_trafficmanager() # tm03 --> tm01 (p=8000). 
```
```py
tm04 = client04.get_trafficmanager(5000) # tm04(p=5000) --> tm02 (p=5000)
```

!!! Important
    Note how the default creation of a TM uses always `port=8000`, and so, only the first time a __TM-Server__ is created. The rest will be __TM-Clients__ connecting to it.

The CARLA server keeps a register of all the TM instances internally by storing the port and the client IP (hidden to the user) that link to them. Right now there is no way to check the TM instances that have been created so far. A connection will always be attempted when trying to create an instance and it will either create a new __TM-Server__ or a __TM-Client__.  

!!! Note 
    The class `TrafficManager.cpp` acts as a central hub managing all the different TM instances.  

### Multiclient 

More than one TM instances created with the same port. The first will be a TM-Server. The rest will be TM-Clients connecting to it.  
```py
terminal 1: ./CarlaUE4.sh -carla-rpc-port=4000
terminal 2: python3 generate_traffic.py --port 4000 --tm-port 4050 # TM-Server
terminal 3: python3 generate_traffic.py --port 4000 --tm-port 4050 # TM-Client
```

### MultiTM

Different TM instances with different ports assigned.  
```py
terminal 1: ./CarlaUE4.sh -carla-rpc-port=4000
terminal 2: python3 generate_traffic.py --port 4000 --tm-port 4050 # TM-Server A
terminal 3: python3 generate_traffic.py --port 4000 --tm-port 4550 # TM-Server B
```

### Multisimulation 

Multisimulation is when there are more than one CARLA server running at the same time. The TM declaration is not relevant. As long as the computational power allows for it, the TM can run multiple simulations at a time without any problems.  

```py
terminal 1: ./CarlaUE4.sh -carla-rpc-port=4000 # simulation A 
terminal 2: ./CarlaUE4.sh -carla-rpc-port=5000 # simulation B
terminal 3: python3 generate_traffic.py --port 4000 --tm-port 4050 # TM-Server A connected to simulation A
terminal 4: python3 generate_traffic.py --port 5000 --tm-port 5050 # TM-Server B connected to simulation B
```

The concept of multisimulation is independent from the Traffic Manager itself. The example above runs two CARLA simulations in parallel, A and B. In each of them, a TM-Server is created independently from the other. Simulation A could run a Multiclient TM while simulation B is running a MultiTM, or no TM at all.  

The only possible issue arising from this is a client trying to connect to an already existing TM which is not running on the selected simulation. In case this happens, an error message will appear and the connection will be aborted to prevent interferences between simulations.  


---
## Synchronous mode

If the CARLA server is set to synchronous mode, the TM must also be set to synchronous mode. The script below demonstrates how to achieve this:

```py
...

# Set the simulation to sync mode
init_settings = world.get_settings()
settings = world.get_settings()
settings.synchronous_mode = True
# After that, set the TM to sync mode
my_tm.set_synchronous_mode(True)

...

# Tick the world in the same client
world.apply_settings(init_settings)
world.tick()
...

# Always disable sync mode before the script ends to prevent the server blocking whilst waiting for a tick
settings.synchronous_mode = False
my_tm.set_synchronous_mode(False)
```

The `generate_traffic.py` example script can set the TM to synchronous mode by passing an argument:

```sh
cd PythonAPI/examples
python3 generate_traffic.py -n 50 --sync
```

If more than one TM is set to synchronous mode, synchrony will fail. Follow these guidelines to avoid issues:

- In a __[multiclient](#multiclient)__ situation, only the __TM-Server__ should be set to synchronous mode.
- In a __[multiTM](#multitm)__ situation, only __one TM-Server__ should be set to synchronous mode.
- The __[ScenarioRunner module](https://carla-scenariorunner.readthedocs.io/en/latest/)__ runs a TM automatically. The TM inside ScenarioRunner will be the one set to sync mode. (Should be or will be?)

!!! Warning
    Disable synchronous mode (for both the world and TM) in your script managing ticks before it finishes to prevent the server blocking, waiting forever for a tick.

---

If you have any questions about the TM, then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
