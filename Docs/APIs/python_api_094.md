# Python API reference (0.9.4)

This reference contains all the details about the Python API as it was for __CARLA 0.9.4__.

<div class="build-buttons">
<p>
<a href="../../python_api" target="_blank" class="btn btn-neutral" title="Go back to the latest version of the Python API">
Back to latest</a>
</p>
</div>

## `carla.Client`

- `Client(host, port, worker_threads=0)`
- `set_timeout(float_seconds)`
- `get_client_version()`
- `get_server_version()`
- `get_world()`
- `get_available_maps()`
- `reload_world()`
- `load_world(map_name)`
- `start_recorder(string filename)`
- `replay_file(string filename, float start, float duration, int camera_follow_id)`
- `show_recorder_file_info(string filename)`
- `show_recorder_collisions(string filename, char category1, char category2)`
- `show_recorder_actors_blocked(string filename, float min_time, float min_distance)`
- `apply_batch(commands, do_tick=False)`

## `carla.World`

- `id`
- `debug`
- `get_blueprint_library()`
- `get_map()`
- `get_spectator()`
- `get_settings()`
- `apply_settings(world_settings)`
- `get_weather()`
- `set_weather(weather_parameters)`
- `get_actors()`
- `spawn_actor(blueprint, transform, attach_to=None)`
- `try_spawn_actor(blueprint, transform, attach_to=None)`
- `wait_for_tick(seconds=1.0)`
- `on_tick(callback)`
- `tick()`

## `carla.WorldSettings`

- `synchronous_mode`
- `no_rendering_mode`
- `__eq__(other)`
- `__ne__(other)`

## `carla.DebugHelper`

- `draw_point(location, size=0.1, color=carla.Color(), life_time=-1.0, persistent_lines=True)`
- `draw_line(begin, end, thickness=0.1, color=carla.Color(), life_time=-1.0, persistent_lines=True)`
- `draw_arrow(begin, end, thickness=0.1, arrow_size=0.1, color=carla.Color(), life_time=-1.0, persistent_lines=True)`
- `draw_box(box, rotation, thickness=0.1, color=carla.Color(), life_time=-1.0, persistent_lines=True)`
- `draw_string(location, text, draw_shadow=False, color=carla.Color(), life_time=-1.0, persistent_lines=True)`

## `carla.BlueprintLibrary`

- `find(id)`
- `filter(wildcard_pattern)`
- `__getitem__(pos)`
- `__len__()`
- `__iter__()`

## `carla.ActorBlueprint`

- `id`
- `tags`
- `has_tag(tag)`
- `match_tags(wildcard_pattern)`
- `has_attribute(key)`
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
- `__eq__(other)`
- `__ne__(other)`
- `__nonzero__()`
- `__bool__()`
- `__int__()`
- `__float__()`
- `__str__()`

## `carla.ActorList`

- `find(id)`
- `filter(wildcard_pattern)`
- `__getitem__(pos)`
- `__len__()`
- `__iter__()`

## `carla.Actor`

- `id`
- `type_id`
- `parent`
- `semantic_tags`
- `is_alive`
- `attributes`
- `get_world()`
- `get_location()`
- `get_transform()`
- `get_velocity()`
- `get_acceleration()`
- `set_location(location)`
- `set_transform(transform)`
- `set_simulate_physics(enabled=True)`
- `destroy()`

## `carla.Vehicle(carla.Actor)`

- `bounding_box`
- `apply_control(vehicle_control)`
- `get_control()`
- `set_autopilot(enabled=True)`
- `get_physics_control()`
- `apply_physics_control(vehicle_physics_control)`
- `get_speed_limit()`
- `get_traffic_light_state()`
- `is_at_traffic_light()`
- `get_traffic_light()`

## `carla.TrafficLight(carla.Actor)`

- `state`
- `set_state(traffic_light_state)`
- `get_state()`
- `set_green_time(green_time)`
- `get_green_time()`
- `set_yellow_time(yellow_time)`
- `get_yellow_time()`
- `set_red_time(red_time)`
- `get_red_time()`
- `get_elapsed_time()`
- `freeze(True)`
- `is_frozen()`
- `get_pole_index()`
- `get_group_traffic_lights()`

## `carla.Sensor(carla.Actor)`

- `is_listening`
- `listen(callback_function)`
- `stop()`

## `carla.SensorData`

- `frame_number`
- `transform`

## `carla.Image(carla.SensorData)`

- `width`
- `height`
- `fov`
- `raw_data`
- `convert(color_converter)`
- `save_to_disk(path, color_converter=None)`
- `__len__()`
- `__iter__()`
- `__getitem__(pos)`
- `__setitem__(pos, color)`

## `carla.LidarMeasurement(carla.SensorData)`

- `horizontal_angle`
- `channels`
- `raw_data`
- `get_point_count(channel)`
- `save_to_disk(path)`
- `__len__()`
- `__iter__()`
- `__getitem__(pos)`
- `__setitem__(pos, location)`

## `carla.CollisionEvent(carla.SensorData)`

- `actor`
- `other_actor`
- `normal_impulse`

## `carla.LaneInvasionEvent(carla.SensorData)`

- `actor`
- `crossed_lane_markings`

## `carla.GnssEvent(carla.SensorData)`

- `latitude`
- `longitude`
- `altitude`

## `carla.ObstacleDetectionSensorEvent(carla.SensorData)`

- `actor`
- `other_actor`
- `distance`

## `carla.VehicleControl`

- `throttle`
- `steer`
- `brake`
- `hand_brake`
- `reverse`
- `__eq__(other)`
- `__ne__(other)`


## `carla.WheelsPhysicsControl`
- `tire_friction`
- `damping_rate`
- `steer_angle`
- `disable_steering`
- `__eq__(other)`
- `__ne__(other)`

## `carla.VehiclePhysicsControl`

- `torque_curve`
- `max_rpm`
- `moi`
- `damping_rate_full_throttle`
- `damping_rate_zero_throttle_clutch_engaged`
- `damping_rate_zero_throttle_clutch_disengaged`
- `use_gear_autobox`
- `gear_switch_time`
- `clutch_strength`
- `mass`
- `drag_coefficient`
- `center_of_mass`
- `steering_curve`
- `wheels`
- `__eq__(other)`
- `__ne__(other)`

## `carla.Map`

- `name`
- `get_spawn_points()`
- `get_waypoint(location, project_to_road=True)`
- `get_topology()`
- `generate_waypoints(distance)`
- `to_opendrive()`
- `save_to_disk(path=self.name)`

## `carla.Waypoint`

- `transform`
- `is_intersection`
- `lane_width`
- `road_id`
- `lane_id`
- `lane_change`
- `lane_type`
- `next(distance)`
- `get_right_lane()`
- `get_left_lane()`

## `carla.LaneChange`
- `None`
- `Right`
- `Left`
- `Both`

## `carla.WeatherParameters`

- `cloudyness`
- `precipitation`
- `precipitation_deposits`
- `wind_intensity`
- `sun_azimuth_angle`
- `sun_altitude_angle`
- `__eq__(other)`
- `__ne__(other)`

Static presets

- `carla.WeatherParameters.ClearNoon`
- `carla.WeatherParameters.CloudyNoon`
- `carla.WeatherParameters.WetNoon`
- `carla.WeatherParameters.WetCloudyNoon`
- `carla.WeatherParameters.MidRainyNoon`
- `carla.WeatherParameters.HardRainNoon`
- `carla.WeatherParameters.SoftRainNoon`
- `carla.WeatherParameters.ClearSunset`
- `carla.WeatherParameters.CloudySunset`
- `carla.WeatherParameters.WetSunset`
- `carla.WeatherParameters.WetCloudySunset`
- `carla.WeatherParameters.MidRainSunset`
- `carla.WeatherParameters.HardRainSunset`
- `carla.WeatherParameters.SoftRainSunset`

## `carla.Vector3D`

- `x`
- `y`
- `z`
- `__add__(other)`
- `__sub__(other)`
- `__eq__(other)`
- `__ne__(other)`

## `carla.Location`

- `x`
- `y`
- `z`
- `distance(other)`
- `__add__(other)`
- `__sub__(other)`
- `__eq__(other)`
- `__ne__(other)`

## `carla.Rotation`

- `pitch`
- `yaw`
- `roll`
- `get_forward_vector()`
- `__eq__(other)`
- `__ne__(other)`

## `carla.Transform`

- `location`
- `rotation`
- `transform(geom_object)`
- `get_forward_vector()`
- `__eq__(other)`
- `__ne__(other)`

## `carla.BoundingBox`

- `location`
- `extent`
- `__eq__(other)`
- `__ne__(other)`

## `carla.Timestamp`

- `frame_count`
- `elapsed_seconds`
- `delta_seconds`
- `platform_timestamp`
- `__eq__(other)`
- `__ne__(other)`

## `carla.Color`

- `r`
- `g`
- `b`
- `a`
- `__eq__(other)`
- `__ne__(other)`

## `carla.ColorConverter`

- `Raw`
- `Depth`
- `LogarithmicDepth`
- `CityScapesPalette`

## `carla.ActorAttributeType`

- `Bool`
- `Int`
- `Float`
- `RGBColor`

## `carla.TrafficLightState`

- `Red`
- `Yellow`
- `Green`
- `Off`
- `Unknown`

## `carla.LaneMarking`

- `Other`
- `Broken`
- `Solid`

# module `carla.command`

## `carla.command.DestroyActor`

- `actor_id`

## `carla.command.ApplyVehicleControl`

- `actor_id`
- `control`

## `carla.command.ApplyWalkerControl`

- `actor_id`
- `control`

## `carla.command.ApplyTransform`

- `actor_id`
- `transform`

## `carla.command.ApplyVelocity`

- `actor_id`
- `velocity`

## `carla.command.ApplyAngularVelocity`

- `actor_id`
- `angular_velocity`


## `carla.command.ApplyImpulse`

- `actor_id`
- `impulse`

## `carla.command.SetSimulatePhysics`

- `actor_id`
- `enabled`

## `carla.command.SetAutopilot`

- `actor_id`
- `enabled`
