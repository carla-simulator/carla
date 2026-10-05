
# This recipe changes attributes of different type of blueprint actors.

# ...
walker_bp = world.get_blueprint_library().find('walker.pedestrian.0002')
walker_bp.set_attribute('is_invincible', 'true')

# ...
# Changes attribute randomly by the recommended value
vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
vehicle_bp.set_attribute('color', color)

# ...

camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
camera_bp.set_attribute('image_size_x', '600')
camera_bp.set_attribute('image_size_y', '600')
# ...
