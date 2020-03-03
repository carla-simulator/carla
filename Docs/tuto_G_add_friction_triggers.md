# How to add friction triggers

*Friction Triggers* are box triggers that can be added on runtime and let users define
a different friction of the vehicles' wheels when being inside those type of triggers.
For example, this could be useful for making slippery surfaces in certain regions of
a map dynamically.

In order to spawn a friction trigger using PythonAPI, users must first get the
`static.trigger.friction` blueprint definition, and then set the following necessary
attributes to that blueprint definition:

- *friction*: The friction of the trigger box when vehicles are inside it.
- *extent_x*: The extent of the bounding box in the X coordinate in centimeters.
- *extent_y*: The extent of the bounding box in the Y coordinate in centimeters.
- *extent_z*: The extent of the bounding box in the Z coordinate in centimeters.

Once done that, define a transform to specify the location and rotation for the friction
trigger and spawn it.

##### Example

```py
import carla

def main():
    # Connect to client
    client = carla.Client('127.0.0.1', 2000)
    client.set_timeout(2.0)

    # Get World and Actors
    world = client.get_world()
    actors = world.get_actors()

    # Find Trigger Friction Blueprint
    friction_bp = world.get_blueprint_library().find('static.trigger.friction')

    extent = carla.Location(700.0, 700.0, 700.0)

    friction_bp.set_attribute('friction', str(0.0))
    friction_bp.set_attribute('extent_x', str(extent.x))
    friction_bp.set_attribute('extent_y', str(extent.y))
    friction_bp.set_attribute('extent_z', str(extent.z))

    # Spawn Trigger Friction
    transform = carla.Transform()
    transform.location = carla.Location(100.0, 0.0, 0.0)
    world.spawn_actor(friction_bp, transform)

    # Optional for visualizing trigger
    world.debug.draw_box(box=carla.BoundingBox(transform.location, extent * 1e-2), rotation=transform.rotation, life_time=100, thickness=0.5, color=carla.Color(r=255,g=0,b=0))

if __name__ == '__main__':
    main()
```
