# ===================================================
# Set the attributes of a blueprint to desired values
# ===================================================

# In this snipet, different blueprints corresponding to different types of actor are chosen,
# and their attributes set to specific values. 


# First get the blueprint library from the world object, and find the desired blueprint. 
#   You can retrieve all the blueprints of a specific type.
walkers_bps = world.get_blueprint_library().filter("walker.pedestrian.*")
#   But in this case, we chose a pedestrian.
bp_library = world.get_blueprint_library()
walker_bp = bp_library.filter('walker.pedestrian.0002')


# Set a specific value for an attribute
walker_bp.set_attribute('is_invincible', True)


# The attributes provide with a list of recommended values that can be used for referenced, or to choose at random. 
#   This may be especially relevant to create some diversity on vehicle colors. 
vehicle_bp = world.get_blueprint_library().filter('vehicle.bmw.*')
color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
vehicle_bp.set_attribute('color', color)


# All the actor attributes are set in the same manner. 
#   Here is another example, where the output image of a camera is set.
camera_bp = world.get_blueprint_library().filter('sensor.camera.rgb')
camera_bp.set_attribute('image_size_x', 600)
camera_bp.set_attribute('image_size_y', 600)


# All the blueprints available in the library, their attributes and types are listed in the blueprint library reference: 
# https://carla.readthedocs.io/en/latest/bp_library/