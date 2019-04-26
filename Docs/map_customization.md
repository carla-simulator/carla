<h1>Map customization</h1>

> _This document is for an obsolete feature of carla (CarlaMapGenerator), we highly recommend using Vector Zero's RoadRunner software now for map building._


Creating a new map
------------------

!!! Bug
    Creating a map from scratch with the Carla tools causes a crash with
    UE4.17.2 ([Issue #99](https://github.com/carla-simulator/carla/issues/99)),
    this guide will suggest duplicating an existing level instead of creating
    one from scratch.

<h4>Requirements</h4>

 - Checkout and build Carla from source on [Linux](how_to_build_on_linux.md) or [Windows](how_to_build_on_windows.md)

<h4>Creating</h4>

- Duplicate an existing map
- Remove everything you don't need from the map
    - Keep the folder "Lighting", "AtmosphericFog", "PostProcessVol" and "CarlaMapGenerator" this will keep the climate working as intended and the post process saved.
    - It might be interesting to keep the empty level as a template and duplicate it before starting to populate it.
- In the CarlaMapGenerator, there is a field "seed". You can change the map by altering that seed and clicking "Trigger Road Map Generation". "Save Road Map To Disk" should also be checked.
- You can change the seed until you have a map you are satisfied with.
- After that you can place new PlayerStarts at the places you want the cars to be spawned.
- The AI already works, but the cars won't act randomly. Vehicles will follow the instructions given by the RoadMapGenerator. They will follow the road easily while in straight roads but won't so much when entering Intersections:
![road_instructions_example.png](img/road_instructions_example.png)
  > (This is a debug view of the instructions the road gives to the Vehicle. They will always follow the green arrows, the white points are shared points between one or more routes, by default they order the vehicle to continue straight; Black points are off the road, the vehicle gets no instructions and drives to the left, trying to get back to the road)

- To get a random behavior, you have to place IntersectionEntrances, this will let you redefine the direction the vehicle will take, overwriting the directions given by the road map (until they finish their given order).
(See how it works in our towns).
    - Before version 0.7.1: For every entrance you'll have to create a series of empty actors that will be the waypoints to guide the car through the intersection; Then you'll have to assign the corresponding actors to every Path
    - After version 0.7.1: Every RoutePlanner has an array called routes. Adding an element to this array creates an editable spline in the world with the first point on the RoutePlanner (You might have to select another object before you can see it) This spline defines the possible routes any car will take when entering the intersection (as the Empty actors did before) you might configure this routes as you would edit any Unreal spline. Each route will create an element in the field bellow: "Probabilities" every number in this array defines the chances of any vehicle to take the corresponding route.
- Traffic need to be scripted on the intersections for the map to work properly. See [traffic control assets](traffic_control_assets.md)
- You can now populate the world with landscape buildings and other props.

