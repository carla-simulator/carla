# Check angular velocity
actor.get_angular_velocity()

# Apply angular impulse
actor.add_angular_impulse(10) # degrees * s

# Wait for world update
world.wait_for_tick()

# Check new angular velocity
actor.get_angular_velocity()