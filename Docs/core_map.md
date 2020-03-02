# 3rd. Maps and navigation

After discussing about the world and its actors, it is time to put everything into place and understand the map and how do the actors navigate it. 

  * [__The map__](#the-map)  
	* Map changing  
	* Lanes  
	* Junctions  
	* Waypoints  
  * [__Map navigation__](#map-navigation)

---
## The map

Understanding the map in CARLA is equivalent to understanding the road. All of the maps have an OpenDRIVE file defining the road layout fully annotated. The way the [OpenDRIVE standard 1.4](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) defines roads, lanes, junctions, etc. is extremely important. It determines the possibilities of the API and the reasoning behind many decisions made.  
The Python API provides a higher level querying system to navigate these roads. It is constantly evolving to provide a wider set of tools.

#### Changing the map

This was briefly mentioned in [1st. World and client](core_world.md), so let's expand a bit on it: __To change the map, the world has to change too__. Everything will be rebooted and created from scratch, besides the Unreal Editor itself.  
Using `reload_world()` creates a new instance of the world with the same map while `load_world()` is used to change the current one:  

```py
world = client.load_world('Town01')
```
The client can also get a list of available maps. Each map has a `name` attribute that matches the name of the currently loaded city, e.g. _Town01._:
```py
print(client.get_available_maps())
```
So far there are seven different maps available. Each of these has a specific structure or unique features that are useful for different purposes, so a brief sum up on these: 

|Town | Summary |
| -- | -- |
|__Town 01__ | As __Town 02__, a basic town layout with all "T junctions". These are the most stable.|
|__Town 02__ | As __Town 01__, a basic town layout with all "T junctions". These are the most stable.|
|__Town 03__ | The most complex town with a roundabout, unevenness, a tunnel. Essentially a medley.|
|__Town 04__ | An infinite loop in a highway.|
|__Town 05__ | Squared-grid town with cross junctions and a bridge.|
|__Town 06__ | Long highways with a lane exit and a [Michigan left](https://en.wikipedia.org/wiki/Michigan_left). |
|__Town 07__ | A rural environment with narrow roads, barely non traffic lights and barns.|  

<br>

Users can also [customize a map](tuto_A_map_customization.md) or even [create a new map](tuto_A_map_creation.md) to be used in CARLA. These are more advanced steps and have been developed in their own tutorials.  


#### Lanes

The different types of lane as defined by [OpenDRIVE standard 1.4](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) are translated to the API in [carla.LaneType](python_api.md#carla.LaneType). The surrounding lane markings for each lane can also be accessed using [carla.LaneMarking](python_api.md#carla.LaneMarkingType).  
A lane marking is defined by: a [carla.LaneMarkingType](python_api.md#carla.LaneMarkingType) and a [carla.LaneMarkingColor](python_api.md#carla.LaneMarkingColor), a __width__ to state thickness and a variable stating lane changing permissions with [carla.LaneChange](python_api.md#carla.LaneChange).  

Both lanes and lane markings are accessed by waypoints to locate a vehicle within the road and aknowledge traffic permissions.  

```py
# Get the lane type where the waypoint is. 
lane_type = waypoint.lane_type
# Get the type of lane marking on the left. 
left_lanemarking_type = waypoint.left_lane_marking.type()
# Get available lane changes for this waypoint.
lane_change = waypoint.lane_change
```

#### Junctions

To ease managing junctions with OpenDRIVE, the [carla.Junction](python_api.md#carla.Junction) class provides for a bounding box to state whereas lanes or vehicles are inside of it.  
There is also a method to get a pair of waypoints per lane determining the starting and ending point inside the junction boundaries for each lane:  
```py
waypoints_junc = my_junction.get_waypoints()
```

#### Waypoints

[carla.Waypoint](python_api.md#carla.Waypoint) objects are 3D-directed points that are prepared to mediate between the world and the openDRIVE definition of the road.  
Each waypoint contains a [carla.Transform](python_api.md#carla.Transform) summarizing a point on the map inside a lane and the orientation of the lane. The variables `road_id`,`section_id`,`lane_id` and `s` that translate this transform to the OpenDRIVE road and are used to create an __identifier__ of the waypoint.  

!!! Note
    Due to granularity, waypoints closer than __2cm within the same road__ will share the same `id`. 

Besides that, each waypoint also contains some information regarding the __lane__ it is contained in and its left and right __lane markings__ and a boolean to determine when it is inside a junction: 

```py
inside_junction = waypoint.is_junction()
width = waypoint.lane_width
# Get right lane marking color
right_lm_color = waypoint.right_lane_marking.color
```

Finally regarding navigation, waypoints have a set of methods to ease the flow inside the road:  
The `next(d)` creates new waypoint at an approximate distance `d` following the direction of the current lane, while `previous(d)` will do so on the opposite direction.  
`next_until_lane_end(d)` and `previous_until_lane_start(d)` will use said distance to find a list of equally distant waypoints contained in the lane. All of these methods follow traffic rules to determine only places where the vehicle can go:
```py
# Disable physics, in this example the vehicle is teleported.
vehicle.set_simulate_physics(False)
while True:
    # Find next waypoint 2 meters ahead.
    waypoint = random.choice(waypoint.next(2.0))
    # Teleport the vehicle.
    vehicle.set_transform(waypoint.transform)
```  

!!! Note
    These methods return a list. If there is more than one possible location (for example at junctions where the lane diverges), the returned list will contain as many waypoints. 

Waypoints can also find their equivalent at the center of an adjacent lane (if said lane exists) using `get_right_lane()` and `get_left_lane()`. This is useful to find the next waypoint on a neighbour lane to then perform a lane change: 


---
## Map Navigation

The instance of the map is provided by the world. Once it is retrieved, it provides acces to different methods that will be useful to create routes and make vehicles roam around the city and reach goal destinations:  
```py
map = world.get_map()
```

* __Get recommended spawn points for vehicles__: assigned by developers with no ensurance of the spot being free: 
```py
spawn_points = world.get_map().get_spawn_points()
```

* __Get a waypoint__: returns the closest waypoint for a specific location in the simulation or the one belonging to a certain `road_id`, `lane_id` and `s` in OpenDRIVE:
```py
# Nearest waypoint on the center of a Driving or Sidewalk lane.
waypoint01 = map.get_waypoint(vehicle.get_location(),project_to_road=True, lane_type=(carla.LaneType.Driving | carla.LaneType.Sidewalk))
#Nearest waypoint but specifying OpenDRIVE parameters. 
waypoint02 = map.get_waypoint_xodr(road_id,lane_id,s)
```

* __Generate a collection of waypoitns__: to visualize the city lanes. Creates waypoints all over the map for every road and lane at an approximated distance between them:
```py
waypoint_list = map.generate_waypoints(2.0)
```

* __Generate road topology__: useful for routing. Returns a list of pairs (tuples) of waypoints. For each pair, the first element connects with the second one and both define the starting and ending point of each lane in the map:
```py
waypoint_tuple_list = map.get_topology()
```

* __Simulation point to world coordinates__: transforms a certain location to world coordinates with latitude and longitude defined with the [carla.Geolocation](python_api.md#carla.Geolocation): 
```py
my_geolocation = map.transform_to_geolocation(vehicle.transform)
```

* __Road information__: converts road information to OpenDRIVE format, and saved to disk:
```py
info_map = map.to_opendrive()
```

---
That is a wrap as regarding maps and navigation around the cities in CARLA.  
The next step should be learning more about sensors, the different types and the data they retrieve. Keep reading to learn more or visit the forum to post any doubts or suggestions that have come to mind during this reading: 
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
<a href="../core_sensors" target="_blank" class="btn btn-neutral" title="4th. Sensors and data">
4th. Sensors and data</a>
</p>
</div>
</div>