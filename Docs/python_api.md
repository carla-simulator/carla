<h1>Python API Reference</h1>

!!! important
    Versions prior to 0.9.0 have a very different API. For the documentation of
    the stable version please switch to the
    [stable branch](https://carla.readthedocs.io/en/stable/).

## `carla.Client`

- `Client(host, port, worker_threads=0)`
- `set_timeout(float_seconds)`
- `get_client_version()`
- `get_server_version()`
- `get_world()`

## `carla.World`

- `id`
- `map_name`
- `get_blueprint_library()`
- `get_spectator()`
- `get_weather()`
- `set_weather(weather_parameters)`
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
- `__len__()`
- `__iter__()`

## `carla.ActorAttribute`

- `id`
- `type`
- `recommended_values`
- `is_modifiable`
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
- `is_alive`
- `get_world()`
- `get_location()`
- `get_transform()`
- `set_location(location)`
- `set_transform(transform)`
- `destroy()`

## `carla.Vehicle(carla.Actor)`

- `control`
- `apply_control(vehicle_control)`
- `set_autopilot(enabled=True)`

## `carla.Sensor(carla.Actor)`

- `is_listening`
- `listen(callback_function)`
- `stop()`

## `carla.Image`

- `frame_number`
- `transform`
- `width`
- `height`
- `type`
- `fov`
- `raw_data`
- `convert(color_converter)`
- `save_to_disk(path, color_converter=None)`
- `__len__()`
- `__iter__()`

## `carla.LidarMeasurement`

- `frame_number`
- `transform`
- `horizontal_angle`
- `channels`
- `raw_data`
- `get_point_count(channel)`
- `save_to_disk(path)`
- `__len__()`
- `__iter__()`

## `carla.VehicleControl`

- `throttle`
- `steer`
- `brake`
- `hand_brake`
- `reverse`
- `__eq__()`
- `__ne__()`

## `carla.WeatherParameters`

- `cloudyness`
- `precipitation`
- `precipitation_deposits`
- `wind_intensity`
- `sun_azimuth_angle`
- `sun_altitude_angle`
- `__eq__()`
- `__ne__()`

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
- `a`

## `carla.ColorConverter`

- `None`
- `Depth`
- `LogarithmicDepth`
- `CityScapesPalette`
