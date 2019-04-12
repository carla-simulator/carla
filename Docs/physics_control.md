<h1>Vehicle Physics Control</h1>

Physics control properties can be tuned for vehicles and its wheels

These properties are controlled through a `VehiclePhysicsControl` object, which also contains a property to control each wheel's physics through a `WheelPhysicsControl` object.

- *torque_curve*: Curve that indicates the torque measured in Nm for a specific RPM of the vehicle's engine
- *max_rpm*: The maximum RPM of the vehicle's engine
- *moi*: The moment of inertia of the vehicle's engine
- *damping_rate_full_throttle*: Damping rate when the throttle is maximum.
- *damping_rate_zero_throttle_clutch_engaged*: Damping rate when the thottle is zero with clutch engaged
- *damping_rate_zero_throttle_clutch_disengaged*: Damping rate when the thottle is zero with clutch disengaged

- *use_gear_autobox*: If true, the vehicle will have automatic transmission
- *gear_switch_time*: Switching time between gears
- *clutch_strength*: The clutch strength of the vehicle. Measured in Kgm^2/s

- *mass*: The mass of the vehicle measured in Kg
- *drag_coefficient*: Drag coefficient of the vehicle's chassis
- *center_of_mass*: The center of mass of the vehicle
- *steering_curve*: Curve that indicates the maximum steering for a specific forward speed
- *wheels*: List of `carla.WheelPhysicsControl` objects.


## `carla.WheelPhysicsControl`

- *tire_friction*: Scalar value that indicates the friction of the wheel.
- *damping_rate*: The damping rate of the wheel.
- *max_steer_angle*: The maximum angle in degrees that the wheel can steer.
- *is_steerable*: If true, the wheel will steer.


##### Example
```py

# Create front and back Wheels Physics Control
front_wheel = carla.WheelPhysicsControl(
    tire_friction =4.5,
    damping_rate = 1.0,
    max_steer_angle = 70.0,
    is_steerable=False
)

back_wheel = carla.WheelPhysicsControl(
    tire_friction=4.5,
    damping_rate=1.0,
    is_steerable=True
)

wheels = [front_wheel, front_wheel, back_wheel, back_wheel]

# Create Vehicle Physics Control
torque_curve = [carla.Vector2D(x=0, y=400),
                carla.Vector2D(x=1315, y=655)]

steering_curve = [carla.Vector2D(x=0, y=1),
                carla.Vector2D(x=100.0, y=1),
                carla.Vector2D(x=300.12, y=1)]

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
    center_of_mass=carla.Vector3D(x=00, y=0, z=-10),
    steering_curve=steering_curve,
    wheels=wheels
)

# Apply Vehicle Physics Control for a given carla.Vehicle
vehicle.apply_physics_control(physics_control)
```