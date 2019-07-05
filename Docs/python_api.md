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
- `get_available_maps()`
- `reload_world()`
- `load_world(map_name)`
- `start_recorder(string filename)`
- `replay_file(string filename, float start, float duration, int camera_follow_id)`
- `show_recorder_file_info(string filename)`
- `show_recorder_collisions(string filename, char category1, char category2)`
- `show_recorder_actors_blocked(string filename, float min_time, float min_distance)`
- `set_replayer_speed(float time_factor)`
- `apply_batch(commands, do_tick=False)`
- `apply_batch_sync(commands, do_tick=False)` -> list(carla.command.Response)

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
- `get_snapshot() -> carla.WorldSnapshot`
- `get_actor(actor_id) -> carla.Actor`
- `get_actors(actor_ids=None) -> carla.ActorList`
- `spawn_actor(blueprint, transform, attach_to=None)`
- `try_spawn_actor(blueprint, transform, attach_to=None, attachment_type=carla.AttachmentType.Rigid)`
- `wait_for_tick(seconds=1.0) -> carla.WorldSnapshot`
- `on_tick(callback)`
- `tick()`

## `carla.WorldSettings`

- `synchronous_mode`
- `no_rendering_mode`
- `__eq__(other)`
- `__ne__(other)`

## `carla.WorldSnapshot`

- `id`
- `frame`
- `timestamp`
- `frame_count` _deprecated, use timestamp instead_
- `elapsed_seconds` _deprecated, use timestamp instead_
- `delta_seconds` _deprecated, use timestamp instead_
- `platform_timestamp` _deprecated, use timestamp instead_
- `has_actor(actor_id) -> bool`
- `find(actor_id) -> carla.ActorSnapshot`
- `__len()__`
- `__iter()__`
- `__eq(other)__`
- `__ne(other)__`

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
- `get_angular_velocity()`
- `get_acceleration()`
- `set_location(location)`
- `set_transform(transform)`
- `set_velocity(vector)`
- `set_angular_velocity(vector)`
- `add_impulse(vector)`
- `set_simulate_physics(enabled=True)`
- `destroy()`
- `__str__()`

## `carla.ActorSnapshot`

- `id`
- `get_location()`
- `get_transform()`
- `get_velocity()`
- `get_angular_velocity()`
- `get_acceleration()`
- `__str__()`

## `carla.Vehicle(carla.Actor)`

- `bounding_box`
- `apply_control(vehicle_control)`
- `get_control()`
- `get_physics_control()`
- `apply_physics_control(vehicle_physics_control)`
- `set_autopilot(enabled=True)`
- `get_speed_limit()`
- `get_traffic_light_state()`
- `is_at_traffic_light()`
- `get_traffic_light()`

## `carla.TrafficSign(carla.Actor)`

- `trigger_volume -> carla.BoundingBox`

## `carla.TrafficLight(carla.TrafficSign)`

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

- `frame`
- `frame_number` _deprecated, use `frame` instead_
- `timestamp`
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
- `gear`
- `manual_gear_shift`
- `__eq__(other)`
- `__ne__(other)`


## `carla.WheelPhysicsControl`

- `tire_friction`
- `damping_rate`
- `max_steer_angle`
- `radius`
- `max_brake_torque`
- `max_handbrake_torque`
- `position`
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

## `carla.WalkerControl`

- `direction`
- `speed`
- `jump`
- `__eq__(other)`
- `__ne__(other)`

## `carla.WalkerBoneControl`

- `bone_transforms`


## `carla.Map`

- `__init__(name, xodr_content)`
- `name`
- `get_spawn_points()`
- `get_waypoint(location, project_to_road=True, lane_type=carla.LaneType.Driving)`
- `get_topology()`
- `generate_waypoints(distance)`
- `transform_to_geolocation(location)`
- `to_opendrive()`
- `save_to_disk(path=self.name)`

## `carla.LaneType`

- `NONE`
- `Driving`
- `Stop`
- `Shoulder`
- `Biking`
- `Sidewalk`
- `Border`
- `Restricted`
- `Parking`
- `Bidirectional`
- `Median`
- `Special1`
- `Special2`
- `Special3`
- `RoadWorks`
- `Tram`
- `Rail`
- `Entry`
- `Exit`
- `OffRamp`
- `OnRamp`
- `Any`

## `carla.LaneChange`

- `NONE`
- `Right`
- `Left`
- `Both`

## `carla.LaneMarkingColor`

- `Standard = White`
- `Blue`
- `Green`
- `Red`
- `White`
- `Yellow`
- `Other`

## `carla.LaneMarkingType`

- `NONE`
- `Other`
- `Broken`
- `Solid`
- `SolidSolid`
- `SolidBroken`
- `BrokenSolid`
- `BrokenBroken`
- `BottsDots`
- `Grass`
- `Curb`

## `carla.LaneMarking`

- `type` -> carla.LaneMarking
- `color` -> carla.RoadMarkColor
- `lane_change` -> carla.LaneChange
- `width`

## `carla.Waypoint`

- `id`
- `transform`
- `is_intersection` _deprecated, use `is_junction` instead_
- `is_junction`
- `lane_width`
- `road_id`
- `section_id`
- `lane_id`
- `junction_id`
- `s`
- `lane_change` -> carla.LaneChange
- `lane_type` -> carla.LaneType
- `right_lane_marking` -> carla.LaneMarking
- `left_lane_marking` -> carla.LaneMarking
- `next(distance)` -> list(carla.Waypoint)
- `get_right_lane()` -> carla.Waypoint
- `get_left_lane()` -> carla.Waypoint

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

- `carla.WeatherParameters.Default`
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

## `carla.Vector2D`

- `x`
- `y`
- `__add__(other)`
- `__sub__(other)`
- `__eq__(other)`
- `__ne__(other)`

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

## `carla.GeoLocation`

- `latitude`
- `longitude`
- `altitude`
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

- `frame`
- `frame_count` _deprecated, use `frame` instead_
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

## `carla.AttachmentType`

- `Rigid`
- `SpringArm`

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

`carla.command.FutureActor` (not yet spawned actor handler)

## `carla.command.Response`

- `actor_id`
- `error` -> str|empty
- `has_error()`

## `carla.command.SpawnActor`

- `__init__(blueprint, transform, parent=None)`
- `then(command)`

## `carla.command.DestroyActor`

- `__init__(actor)`
- `actor_id`

## `carla.command.ApplyVehicleControl`

- `__init__(actor, control)`
- `actor_id`
- `control`

## `carla.command.ApplyWalkerControl`

- `__init__(actor, control)`
- `actor_id`
- `control`

## `carla.command.ApplyTransform`

- `__init__(actor, transform)`
- `actor_id`
- `transform`

## `carla.command.ApplyVelocity`

- `__init__(actor, velocity)`
- `actor_id`
- `velocity`

## `carla.command.ApplyAngularVelocity`

- `__init__(actor, angular_velocity)`
- `actor_id`
- `angular_velocity`


## `carla.command.ApplyImpulse`

- `__init__(actor, impulse)`
- `actor_id`
- `impulse`

## `carla.command.SetSimulatePhysics`

- `__init__(actor, bool)`
- `actor_id`
- `enabled`

## `carla.command.SetAutopilot`

- `__init__(actor, bool)`
- `actor_id`
- `enabled`
