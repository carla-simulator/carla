# Python API reference (0.9.0)

This reference contains all the details about the Python API as it was for __CARLA 0.9.0__.

<div class="build-buttons">
<p>
<a href="../../python_api" target="_blank" class="btn btn-neutral" title="Go back to the latest version of the Python API">
Back to latest</a>
</p>
</div>

!!! important
    Versions prior to 0.9.0 have a very different API. For the documentation of the stable version please switch to the [stable branch](https://carla.readthedocs.io/en/stable/).

## `carla.Client`

- `Client(host, port, worker_threads=0)`
- `set_timeout(milliseconds)`
- `get_client_version()`
- `get_server_version()`
- `ping()`
- `get_world()`

## `carla.World`

- `get_blueprint_library()`
- `get_spectator()`
- `spawn_actor(blueprint, transform, attach_to=None)`
- `try_spawn_actor(blueprint, transform, attach_to=None)`

## `carla.BlueprintLibrary`

- `find(id)`
- `filter(wildcard_pattern)`
- `__getitem__(pos)`
- `__len__()`
- `__iter__()`

## `carla.ActorBlueprint`

- `id`
- `tags`
- `contains_tag(tag)`
- `match_tags(wildcard_pattern)`
- `contains_attribute(key)`
- `get_attribute(key)`
- `set_attribute(key, value)`

## `carla.ActorAttribute`

- `is_modifiable`
- `type`
- `recommended_values`
- `as_bool()`
- `as_int()`
- `as_float()`
- `as_str()`
- `as_color()`
- `__eq__()`
- `__ne__()`
- `__nonzero__()`
- `__bool__()`
- `__int__()`
- `__float__()`
- `__str__()`

## `carla.Actor`

- `id`
- `type_id`
- `get_world()`
- `get_location()`
- `get_transform()`
- `set_location(location)`
- `set_transform(transform)`
- `destroy()`

## `carla.Vehicle(carla.Actor)`

- `apply_control(vehicle_control)`
- `set_autopilot(enabled=True)`

## `carla.Sensor(carla.Actor)`

- `listen(callback_function)`

## `carla.Image`

- `frame_number`
- `width`
- `height`
- `type`
- `fov`
- `raw_data`

## `carla.VehicleControl`

- `throttle`
- `steer`
- `brake`
- `hand_brake`
- `reverse`

## `carla.Location`

- `x`
- `y`
- `z`

## `carla.Rotation`

- `pitch`
- `yaw`
- `roll`

## `carla.Transform`

- `location`
- `rotation`

## `carla.Color`

- `r`
- `g`
- `b`
