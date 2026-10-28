# Add friction triggers

A __friction trigger__ is a box spawned at runtime that changes the wheel friction of any vehicle inside it, and restores it on the way out. It is how a patch of a map is made slippery without touching the road material — ice on a bridge, an oil spill at a junction.

!!! Note
    A vehicle's grip can also be set directly through its physics control, wheel by wheel, with no actor to spawn or destroy: see [`friction_force_multiplier`](tuto_vehicle_physics.md). A trigger is still the better fit when the slippery patch belongs to a place on the map rather than to a particular vehicle.

- [__Attributes__](#attributes)
- [__Example__](#example)
- [__Removing a trigger__](#removing-a-trigger)

---
## Attributes

The blueprint is `static.trigger.friction`:

| Attribute | Type | Default | Meaning |
| --- | --- | --- | --- |
| `friction` | float | 3.5 | Grip of every wheel while the vehicle is inside. It is written straight into the wheel's [`friction_force_multiplier`](python_api.md#carla.WheelPhysicsControl.friction_force_multiplier), so `0.0` is frictionless ice and values above 3.5 grip harder than the road. The default is the value the wheels already carry, so a trigger left at it changes nothing. |
| `extent_x` | float | | Half-size of the box along X, in __centimeters__. |
| `extent_y` | float | | Half-size along Y, in centimeters. |
| `extent_z` | float | | Half-size along Z, in centimeters. |

The extents are half-sizes, as Unreal box extents are: `extent_x = 700` is a box 14 m wide.

A vehicle's friction is saved when it enters the box and put back when it leaves. It does not have to enter: a vehicle spawned inside a trigger adopts its friction while it sets up its wheels.

---
## Example

Spawn a frictionless 14 × 14 × 14 m box, 100 m along the X axis:

```py
friction_bp = world.get_blueprint_library().find('static.trigger.friction')

# Half-extents, in centimeters: a 14 x 14 x 14 m box
extent = carla.Location(700.0, 700.0, 700.0)

friction_bp.set_attribute('friction', str(0.0))
friction_bp.set_attribute('extent_x', str(extent.x))
friction_bp.set_attribute('extent_y', str(extent.y))
friction_bp.set_attribute('extent_z', str(extent.z))

# The transform is in meters, like every other actor transform
transform = carla.Transform(carla.Location(100.0, 0.0, 0.0))
friction_trigger = world.spawn_actor(friction_bp, transform)
```

The trigger volume is invisible. Draw it to see where it is — [`carla.BoundingBox`](python_api.md#carla.BoundingBox) works in meters, so the centimeter extents are scaled down by 100:

```py
world.debug.draw_box(box=carla.BoundingBox(transform.location, extent * 1e-2),
                     rotation=transform.rotation, life_time=100, thickness=0.5,
                     color=carla.Color(r=255, g=0, b=0))
```

---
## Removing a trigger

A friction trigger is an actor, so it is destroyed like any other:

```py
friction_trigger.destroy()
```

!!! Warning
    Drive the vehicles out of the box first. A trigger drops its overlap callbacks as it is destroyed, so a vehicle still inside is never restored: it keeps the trigger's friction for the rest of the simulation. Put the grip back through the vehicle's [physics control](tuto_vehicle_physics.md) if that happens.
