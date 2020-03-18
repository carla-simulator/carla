# Core concepts

This page introduces the main features and modules in CARLA. Detailed explanations of the different subjects can be found in their corresponding page.  

In order to learn about the different classes and methods in the API, take a look at the [Python API reference](python_api.md). Besides, the [Code recipes](ref_code_recipes.md) reference contains some common code chunks, specially useful during these first steps. 

  * [__First steps__](#first-steps)  
	* 1st. World and client
	* 2nd. Actors and blueprints
	* 3rd. Maps and navigation
	* 4th. Sensors and data
  * [__Advanced steps__](#advanced-steps)  

!!! Important
    **This documentation refers to CARLA 0.9.X**. <br>
    The API changed significantly from previous versions (0.8.X). There is another documentation regarding those versions that can be found [here](https://carla.readthedocs.io/en/stable/getting_started/). 

---
## First steps

### 1st. World and client

__The client__ is the module the user runs to ask for information or changes in the simulation. A client runs with an IP and a specific port. It communicates with the server via terminal. There can be many clients running at the same time. Advanced multiclient managing requires thorough understanding of CARLA and [synchrony](adv_synchrony_timestep.md).  

__The world__ is an object representing the simulation. It acts as an abstract layer containing the main methods to spawn actors, change the weather, get the current state of the world, etc. There is only one world per simulation. It will be destroyed and substituted for a new one when the map is changed.  

### 2nd. Actors and blueprints
An actor is anything that plays a role in the simulation.  

* Vehicles.
* Walkers.
* Sensors.
* The spectator.
* Traffic signs and traffic lights.

__Blueprints__ are already-made actor layouts necessary to spawn an actor. Basically, models with animations and a set of attributes. Some of these attributes can be customized by the user, others don't. There is a [__Blueprint library__](bp_library.md) containing all the blueprints available as well as information on them.  

### 3rd. Maps and navigation

__The map__ is the object representing the simulated world, the town mostly. There are seven maps available. All of them use OpenDRIVE 1.4 standard to describe the roads.  

__Roads, lanes and junctions__ are managed by the [Python API](python_api.md) to be accessed from the client. These are used along with the __waypoint__ class to provide vehicles with a navigation path.  

__Traffic signs__ and __traffic lights__ have bounding boxes placed on the road. Vehicles become aware of them once inside their bounding box.

### 4th. Sensors and data

__Sensors__ wait for some event to happen, and then gather data from the simulation. They call for a function defining how to manage the data. Depending on which, sensors retrieve different types of __sensor data__. 

A sensor is an actor attached to a parent vehicle. It follows the vehicle around, gathering information of the surroundings. The sensors available are defined by their blueprints in the [Blueprint library](bp_library.md).  

* Cameras (RGB, depth and semantic segmentation).  
* Collision detector.  
* Gnss sensor.  
* IMU sensor.  
* Lidar raycast.  
* Lane invasion detector.  
* Obstacle detector.  
* Radar.  

---
## Advanced steps  

Hereunder are listed some advanced CARLA features. However, it is highly encouraged to first take a closer look to the pages regarding the first steps in order to learn the basics.  

* __Recorder.__ Allows for reenacting previous simulations using snapshots of the world.  
* __Rendering options.__ Graphics quality settings, off-screen rendering and a no-rendering mode. 
* __Simulation time and synchrony.__ Everything regarding the simulation time and server-client communication.  
* __Traffic manager.__ This module is in charge of every vehicle set to autopilot mode. It simulates traffic in the city for the simulation to look like a real urban environment. 

---
That is a wrap on the CARLA basics. The next step takes a closer look to the world and the clients connecting to it.  

Keep reading to learn more. Visit the forum to post any doubts or suggestions that have come to mind during this reading.  

<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="CARLA forum">
CARLA forum</a>
</p>
</div>
<div class="build-buttons">
<p>
<a href="../core_world" target="_blank" class="btn btn-neutral" title="1st. World and client">
1st. World and client</a>
</p>
</div>
</div>