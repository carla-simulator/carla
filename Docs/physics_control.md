<h1>Friction Triggers</h1>

**Friction Triggers** are box triggers that can be added on runtime and let users define a different friction of the vehicles' wheels when being inside those type of triggers. For example, this could be useful for making slippery surfaces in certain regions of a map dynamically.

In order to spawn a friction trigger using PythonAPI, users must first get the trigger.friction blueprint definition, and then set the following necessary attributes to that blueprint definition:

- *friction*: The friction of the trigger box when vehicles are inside it.
- *extent_x*: The extent of the bounding box in the X coordinate in centimeters.
- *extent_y*: The extent of the bounding box in the Y coordinate in centimeters.
- *extent_z*: The extent of the bounding box in the Z coordinate in centimeters.

Once done that, define a transform to specify the location and rotation of the friction trigger, and simply call the `spawn_actor(trigger_blueprint, trigger_transform, attach_to=None)` method inside `carla.World`.

**Example**
```py
# Get Blueprint definition
bp = carla_world.get_blueprint_library().find('trigger.friction')

# Fill definition's attributes
friction = 0.0
trigger_size = 500.0
bp.set_attribute('friction', str(friction))
bp.set_attribute('extent_x', str(trigger_size))
bp.set_attribute('extent_y', str(trigger_size))
bp.set_attribute('extent_z', str(trigger_size))

# Define trigger box transform
t = carla.Transform()
t.location = carla.Location(44.4, 0.5, 0.08)
t.rotation = carla.Rotation(pitch=0.0, yaw=0.0, roll=0.0)
carla_world.spawn_actor(bp, t, attach_to=None)
```

<h1>Vehicle Physics Control</h1>

Physics control properties can be tuned for vehicles and its wheels. These changes are applied **only** on runtime, and values set back to default ones when the execution ends.

These properties are controlled through a `carla.VehiclePhysicsControl` object, which also provides the control of each wheel's physics through a `carla.WheelPhysicsControl` object.

- *torque_curve*: Curve that indicates the torque measured in Nm for a specific RPM of the vehicle's engine.
- *max_rpm*: The maximum RPM of the vehicle's engine.
- *moi*: The moment of inertia of the vehicle's engine.
- *damping_rate_full_throttle*: Damping rate when the throttle is maximum.
- *damping_rate_zero_throttle_clutch_engaged*: Damping rate when the thottle is zero with clutch engaged.
- *damping_rate_zero_throttle_clutch_disengaged*: Damping rate when the throttle is zero with clutch disengaged.
- *use_gear_autobox*: If true, the vehicle will have automatic transmission.
- *gear_switch_time*: Switching time between gears.
- *clutch_strength*: The clutch strength of the vehicle. Measured in Kgm^2/s.
- *mass*: The mass of the vehicle measured in Kg.
- *drag_coefficient*: Drag coefficient of the vehicle's chassis.
- *center_of_mass*: The center of mass of the vehicle.
- *steering_curve*: Curve that indicates the maximum steering for a specific forward speed.
- *wheels*: List of `carla.WheelPhysicsControl` objects.


## `carla.WheelPhysicsControl`
The properties that can be changed from a `carla.WheelPhysicsControl` are the following:

- *tire_friction*: Scalar value that indicates the friction of the wheel.
- *damping_rate*: The damping rate of the wheel.
- *max_steer_angle*: The maximum angle in degrees that the wheel can steer.
- *is_steerable*: If true, the wheel will steer.

##### Example
```py
# Create front and back Wheels Physics Control
front_wheel = carla.WheelPhysicsControl(
    tire_friction=4.5,
    damping_rate=1.0,
    max_steer_angle=70.0,
    is_steerable=True
)

back_wheel = carla.WheelPhysicsControl(
    tire_friction=4.5,
    damping_rate=1.0,
    is_steerable=False
)

wheels = [front_wheel, front_wheel, back_wheel, back_wheel]

# Create Vehicle Physics Control
torque_curve = [carla.Vector2D(x=0, y=400),
                carla.Vector2D(x=1300, y=600)]

steering_curve = [carla.Vector2D(x=0, y=1),
                  carla.Vector2D(x=100, y=1),
                  carla.Vector2D(x=300, y=1)]

physics_control = carla.VehiclePhysicsControl(
    torque_curve=torque_curve,
    max_rpm=10000,
    moi=1.0,
    damping_rate_full_throttle=0.0,
    damping_rate_zero_throttle_clutch_engaged=0,
    damping_rate_zero_throttle_clutch_disengaged=0.0,
    use_gear_autobox=True,
    gear_switch_time=0.5,
    clutch_strength=10,
    mass=10000,
    drag_coefficient=0.25,
    center_of_mass=carla.Vector3D(x=0, y=0, z=-10),
    steering_curve=steering_curve,
    wheels=wheels
)

# Apply Vehicle Physics Control for a given carla.Vehicle
vehicle.apply_physics_control(physics_control)
```