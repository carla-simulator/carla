# CARLA API Examples

## Quick Start

```python
import carla

# Connect to server
client = carla.Client('localhost', 2000)
client.set_timeout(10.0)

# Get world
world = client.get_world()

# Spawn vehicle
blueprint_library = world.get_blueprint_library()
vehicle_bp = blueprint_library.filter('vehicle.*')[0]
spawn_point = world.get_map().get_spawn_points()[0]
vehicle = world.spawn_actor(vehicle_bp, spawn_point)
```

## Common Patterns

### Getting Actors
```python
# Get all vehicles
vehicles = world.get_actors().filter('vehicle.*')

# Get specific actor by ID
actor = world.get_actor(actor_id)
```

### Controlling Vehicle
```python
vehicle.apply_control(carla.VehicleControl(
    throttle=0.5,
    steer=0.0,
    brake=0.0
))
```

### Sensors
```python
# Attach camera
camera_bp = blueprint_library.find('sensor.camera.rgb')
camera = world.spawn_actor(camera_bp, transform, attach_to=vehicle)
camera.listen(lambda image: image.save_to_disk('output.png'))
```
