# Add friction triggers

A __friction trigger__ is a box spawned at runtime that changes the wheel friction of any vehicle inside it, and restores it on the way out. It is how a patch of a map is made slippery without touching the road material — ice on a bridge, an oil spill at a junction.

- [__Attributes__](#attributes)
- [__Example__](#example)
- [__Removing a trigger__](#removing-a-trigger)

---
## Attributes

The blueprint is `static.trigger.friction`:

| Attribute | Type | Default | Meaning |
| --- | --- | --- | --- |
| `friction` | float | 0.0 | Friction __scale__ applied to every wheel while the vehicle is inside. It multiplies the wheel's own friction, whose default is `3.5`, so `0.0` is frictionless ice and values above `3.5` grip harder than the road. |
| `extent_x` | float | | Half-size of the box along X, in __centimeters__. |
| `extent_y` | float | | Half-size along Y, in centimeters. |
| `extent_z` | float | | Half-size along Z, in centimeters. |

The extents are half-sizes, as Unreal box extents are: `extent_x = 700` is a box 14 m wide.

The original friction is saved when a vehicle enters and restored when it leaves, so a trigger never leaves a vehicle permanently altered.

---
## Example

A 14 × 14 × 14 m frictionless box, 100 m along the X axis, drawn in red so it can be seen:

```py
import carla


def main():
    client = carla.Client('127.0.0.1', 2000)
    client.set_timeout(10.0)
    world = client.get_world()

    friction_bp = world.get_blueprint_library().find('static.trigger.friction')

    # Half-extents, in centimeters: a 14 x 14 x 14 m box
    extent = carla.Location(700.0, 700.0, 700.0)

    friction_bp.set_attribute('friction', str(0.0))
    friction_bp.set_attribute('extent_x', str(extent.x))
    friction_bp.set_attribute('extent_y', str(extent.y))
    friction_bp.set_attribute('extent_z', str(extent.z))

    # The transform is in meters, like every other actor transform
    transform = carla.Transform()
    transform.location = carla.Location(100.0, 0.0, 0.0)
    friction_trigger = world.spawn_actor(friction_bp, transform)

    # Optional: draw the trigger. carla.BoundingBox takes meters, so the
    # centimeter extents are scaled down by 100.
    world.debug.draw_box(box=carla.BoundingBox(transform.location, extent * 1e-2),
                         rotation=transform.rotation, life_time=100, thickness=0.5,
                         color=carla.Color(r=255, g=0, b=0))

    return friction_trigger


if __name__ == '__main__':
    main()
```

---
## Removing a trigger

A friction trigger is an actor, so it is destroyed like any other. Any vehicle still inside keeps the trigger's friction until it leaves the volume, so move the vehicles out first if that matters:

```py
friction_trigger.destroy()
```
