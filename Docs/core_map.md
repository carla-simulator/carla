# Maps and navigation

After discussing about the world and its actors, it is time to put everything into place and understand the map and how do the actors navigate it.  

- [__The map__](#the-map)  
	- [Changing the map](#changing-the-map)  
	- [Landmarks](#landmarks)  
	- [Lanes](#lanes)  
	- [Junctions](#junctions)  
	- [Waypoints](#waypoints)  
	- [Environment Objects](#environment-objects)
- [__Navigation in CARLA__](#navigation-in-carla)  
	- [Navigating through waypoints](#navigating-through-waypoints)  
	- [Generating a map navigation](#generating-a-map-navigation)


---
## The map

A map includes both the 3D model of a town and its road definition. A map's road definition is based on an OpenDRIVE file, a standarized, annotated road definition format. The way the [OpenDRIVE standard 1.4](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) defines roads, lanes, junctions, etc. determines the functionality of the Python API and the reasoning behind decisions made.

The Python API acts as a high level querying system to navigate these roads. It is constantly evolving to provide a wider set of tools.

### Changing the map

__To change the map, the world has to change too__. The simulation will be recreated from scratch. You can either restart with the same map in a new world or you can change both the map and the world:

- `reload_world()` creates a new instance of the world with the same map.
- `load_world()` changes the current map and creates a new world.

```py
world = client.load_world('Town01')
```

Each map has a `name` attribute that matches the name of the currently loaded city, e.g. _Town01_. To get a list of the available maps:

```py
print(client.get_available_maps())
```

### Landmarks

Traffic signs defined in the OpenDRIVE file are translated to CARLA as landmark objects that can be queried from the API. The following methods and classes can be used to manipulate and work with landmark objects:

- __[`carla.Landmark`](https://carla.readthedocs.io/en/latest/python_api/#carla.Landmark)__ objects represent OpenDRIVE signals. The attributes and methods of this class describe the landmark and its area of influence.
	- [`carla.LandmarkOrientation`](https://carla.readthedocs.io/en/latest/python_api/#carla.LandmarkOrientation) states the orientation of the landmark with regard to the road's geometry definition.
	- [`carla.LandmarkType`](https://carla.readthedocs.io/en/latest/python_api/#carla.LandmarkType) contains common landmark types to facilitate translation to OpenDRIVE types.
- __[`carla.Waypoint`](https://carla.readthedocs.io/en/latest/python_api/#carla.Waypoint)__ can get landmarks located a certain distance ahead of it. The landmark type to get can be specified.
- __[`carla.Map`](https://carla.readthedocs.io/en/latest/python_api/#carla.Map)__ retrieves sets of landmarks. It can return all landmarks in the map, or those which have a common ID, type or group.
- __[`carla.World`](https://carla.readthedocs.io/en/latest/python_api/#carla.World)__ acts as intermediary between landmarks and the `carla.TrafficSign` and `carla.TrafficLight` that represent them in the simulation.

```py
my_waypoint.get_landmarks(200.0,True)
``` 

### Waypoints

A [`carla.Waypoint`](python_api.md#carla.Waypoint) is a 3D-directed point in the CARLA world corresponding to an OpenDRIVE lane. Everything related to waypoints happens on the client-side; communication with the server is only needed once to get the [map object](python_api.md#carlamap) containing the waypoint information.

Each waypoint contains a [`carla.Transform`](python_api.md#carla.Transform) which states its location on the map and the orientation of the lane containing it. The variables `road_id`,`section_id`,`lane_id` and `s` correspond to the OpenDRIVE road. The `id` of the waypoint is constructed from a hash combination of these four values.

!!! Note
    Waypoints closer than __2cm within the same road__ share the same `id`.

A waypoint holds information about the __lane__ containing it. This information includes the lane's left and right __lane markings__, a boolean to determine if it's inside a junction, the lane type, width, and lane changing permissions.

```py
# Access lane information from a waypoint
inside_junction = waypoint.is_junction()
width = waypoint.lane_width
right_lm_color = waypoint.right_lane_marking.color
```

### Lanes

The lane types defined by [OpenDRIVE standard 1.4](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) are translated to the API in [`carla.LaneType`](python_api.md#carla.LaneType) as a series of enum values.

The lane markings surrounding a lane are accessed through [`carla.LaneMarking`](python_api.md#carla.LaneMarking). Lane markings are defined by a series of variables:

- __color:__ [`carla.LaneMarkingColor`](python_api.md#carla.LaneMarkingColor) are enum values that define the marking's color.
- __lane_change:__ [`carla.LaneChange`](python_api.md#carla.LaneChange) states if the lane permits turning left, right, both or none.
- __type:__ [`carla.LaneMarkingType`](python_api.md#carla.LaneMarkingType) are enum values  that define the type of marking according to the OpenDRIVE standard.
- __width:__ defines the marking's thickness.

The below example shows to get information about the lane type, lane markings, and lane change permissions at a specific waypoint:

```py
# Get the lane type of the waypoint
lane_type = waypoint.lane_type

# Get the type of lane marking on the left.
left_lanemarking_type = waypoint.left_lane_marking.type()

# Get available lane changes for this waypoint.
lane_change = waypoint.lane_change
```

### Junctions

A [`carla.Junction`](python_api.md#carla.Junction) represents an OpenDRIVE junction. This class encompasses a junction with a bounding box to identify lanes or vehicles within it.

The `carla.Junction` class contains the `get_waypoints` method which returns a pair of waypoints for every lane within the junction. Each pair is located at the start and end points of the junction boundaries.

```py
waypoints_junc = my_junction.get_waypoints()
```

### Environment Objects

Every object on a CARLA map has a set of associated variables which can be found [here][env_obj]. Included in these variables is a [unique ID][env_obj_id] that can be used to [toggle][toggle_env_obj] that object's visibility on the map. You can use the Python API to [fetch][fetch_env_obj] the IDs of each environment object based on their [semantic tag][semantic_tag]:

		# Get the buildings in the world
	    world = client.get_world()
		env_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)

		# Access individual building IDs and save in a set
		building_01 = env_objs[0]
		building_02 = env_objs[1]
		objects_to_toggle = {building_01.id, building_02.id}

		# Toggle buildings off
		world.enable_environment_objects(objects_to_toggle, False)
		# Toggle buildings on
		world.enable_environment_objects(objects_to_toggle, True)

[env_obj]: https://carla.readthedocs.io/en/latest/python_api/#carla.EnvironmentObject
[env_obj_id]: https://carla.readthedocs.io/en/latest/python_api/#carla.EnvironmentObject.id
[toggle_env_obj]: https://carla.readthedocs.io/en/latest/python_api/#carla.World.enable_environment_objects
[fetch_env_obj]: https://carla.readthedocs.io/en/latest/python_api/#carla.World.get_environment_objects
[semantic_tag]: https://carla.readthedocs.io/en/latest/python_api/#carla.CityObjectLabel


---
## Navigation in CARLA

Navigation in CARLA is managed via the Waypoint API, a combination of methods from [`carla.Waypoint`](python_api.md#carla.Waypoint) and [`carla.Map`](python_api.md#carla.Map).

The client must initially communicate with the server to retrieve the map object containing the waypoint information. This is only required once, all subsequent queries are performed on the client side.

### Navigating through waypoints

The Waypoint API exposes methods that allow waypoints to connect to each other and construct a path along a road for vehicles to navigate:

- `next(d)` creates a list of waypoints within an approximate distance, `d`, __in the direction of the lane__. The list contains one waypoint for each possible deviation.
- `previous(d)` creates a list of waypoints waypoint within an approximate distance, `d`, __in the opposite direction of the lane__. The list contains one waypoint for each possible deviation.
- `next_until_lane_end(d)` and `previous_until_lane_start(d)` return a list of waypoints a distance `d` apart. The lists go from the current waypoint to the end and beginning of its lane, respectively.
- `get_right_lane()` and `get_left_lane()` return the equivalent waypoint in an adjacent lane, if one exists. A lane change maneuver can be made by finding the next waypoint to the one on its right/left lane, and moving to it.

```py
# Find next waypoint 2 meters ahead.
waypoint = waypoint.next(2.0)
```

### Generating map navigation

The client needs to make a request to the server to get the `.xodr` map file and parse it to a [`carla.Map`](python_api.md#carla.Map) object. This only needs to be done once.

To get the map object:

```py
map = world.get_map()
```

The map object contains __recommended spawn points__ for the creation of vehicles. You can get a list of these spawn points, each one containing a [`carla.Transform`](python_api.md#carlatransform), using the method below. Bear in mind that the spawn points may be occupied already, resulting in failed creation of vehicles due to collisions.

```py
spawn_points = world.get_map().get_spawn_points()
```

You can get started with waypoints by __[getting](python_api.md#carla.Map.get_waypoint) the closest waypoint__ to a specific location or to a particular `road_id`, `lane_id` and `s` value in the map's OpenDRIVE definition:

```py
# Nearest waypoint in the center of a Driving or Sidewalk lane.
waypoint01 = map.get_waypoint(vehicle.get_location(),project_to_road=True, lane_type=(carla.LaneType.Driving | carla.LaneType.Sidewalk))

#Nearest waypoint but specifying OpenDRIVE parameters. 
waypoint02 = map.get_waypoint_xodr(road_id,lane_id,s)
```

The below example shows how to __generate a collection of waypoints__ to visualize the city lanes. This will create waypoints all over the map, for every road and lane. All of them will approximately 2 meters apart:

```py
waypoint_list = map.generate_waypoints(2.0)
```

To __generate a minimal graph of road topology__, use the example below. This will return a list of pairs (tuples) of waypoints. The first element in each pair connects with the second element and both define the start and end points of each lane in the map. More information on this method is found in the [PythonAPI](python_api.md#carla.Map.get_topology).

```py
waypoint_tuple_list = map.get_topology()
```

The example below __converts a `carla.Transform` to geographical latitude and longitude coordinates,__ in the form of a [`carla.GeoLocation`](python_api.md#carla.GeoLocation):

```py
my_geolocation = map.transform_to_geolocation(vehicle.transform)
```

Use the following example to __save road information__ in OpenDRIVE format to disk:

```py
info_map = map.to_opendrive()
```

---

