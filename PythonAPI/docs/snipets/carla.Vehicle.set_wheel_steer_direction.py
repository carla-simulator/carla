# Sets the appearance of the vehicles front wheels to 40°. Vehicle physics will not be affected.
# The wheel setters only work while the wheel animation override is enabled.

vehicle.set_wheel_animation_override(True)
vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FR_Wheel, 40.0)
vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FL_Wheel, 40.0)

# Hands the wheels back to the physics, or to the automatic roll without physics.
vehicle.set_wheel_animation_override(False)
