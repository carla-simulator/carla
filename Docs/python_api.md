### Overview
- [**carla**](#carla) <sub>_Module_</sub>  
    - [**ActorAttributeType**](#carla.ActorAttributeType) <sub>_Class_</sub>  
        - [**Bool**](#carla.ActorAttributeType.Bool) <sub>_Instance variable_</sub>
        - [**Int**](#carla.ActorAttributeType.Int) <sub>_Instance variable_</sub>
        - [**Float**](#carla.ActorAttributeType.Float) <sub>_Instance variable_</sub>
        - [**String**](#carla.ActorAttributeType.String) <sub>_Instance variable_</sub>
        - [**RGBColor**](#carla.ActorAttributeType.RGBColor) <sub>_Instance variable_</sub>
    - [**ActorList**](#carla.ActorList) <sub>_Class_</sub>  
        - [**find**(**self**)](#carla.ActorList.find) <sub>_Method_</sub>
        - [**filter**(**self**)](#carla.ActorList.filter) <sub>_Method_</sub>
        - [**\__getitem__**(**self**)](#carla.ActorList.__getitem__) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.ActorList.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.ActorList.__iter__) <sub>_Method_</sub>
    - [**BlueprintLibrary**](#carla.BlueprintLibrary) <sub>_Class_</sub>  
        - [**find**(**self**)](#carla.BlueprintLibrary.find) <sub>_Method_</sub>
        - [**filter**(**self**)](#carla.BlueprintLibrary.filter) <sub>_Method_</sub>
        - [**\__getitem__**(**self**)](#carla.BlueprintLibrary.__getitem__) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.BlueprintLibrary.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.BlueprintLibrary.__iter__) <sub>_Method_</sub>
    - [**CollisionEvent**](#carla.CollisionEvent) <sub>_Class_</sub>  
        - [**actor**](#carla.CollisionEvent.actor) <sub>_Instance variable_</sub>
        - [**other_actor**](#carla.CollisionEvent.other_actor) <sub>_Instance variable_</sub>
        - [**normal_impulse**](#carla.CollisionEvent.normal_impulse) <sub>_Instance variable_</sub>
    - [**ColorConverter**](#carla.ColorConverter) <sub>_Class_</sub>  
        - [**Raw**](#carla.ColorConverter.Raw) <sub>_Instance variable_</sub>
        - [**Depth**](#carla.ColorConverter.Depth) <sub>_Instance variable_</sub>
        - [**LogarithmicDepth**](#carla.ColorConverter.LogarithmicDepth) <sub>_Instance variable_</sub>
        - [**CityScapesPalette**](#carla.ColorConverter.CityScapesPalette) <sub>_Instance variable_</sub>
    - [**DebugHelper**](#carla.DebugHelper) <sub>_Class_</sub>  
        - [**draw_point**(**self**)](#carla.DebugHelper.draw_point) <sub>_Method_</sub>
        - [**draw_line**(**self**)](#carla.DebugHelper.draw_line) <sub>_Method_</sub>
        - [**draw_arrow**(**self**)](#carla.DebugHelper.draw_arrow) <sub>_Method_</sub>
        - [**draw_box**(**self**)](#carla.DebugHelper.draw_box) <sub>_Method_</sub>
        - [**draw_string**(**self**)](#carla.DebugHelper.draw_string) <sub>_Method_</sub>
    - [**GnssEvent**](#carla.GnssEvent) <sub>_Class_</sub>  
        - [**latitude**](#carla.GnssEvent.latitude) <sub>_Instance variable_</sub>
        - [**longitude**](#carla.GnssEvent.longitude) <sub>_Instance variable_</sub>
        - [**altitude**](#carla.GnssEvent.altitude) <sub>_Instance variable_</sub>
    - [**LaneChange**](#carla.LaneChange) <sub>_Class_</sub>  
        - [**NONE**](#carla.LaneChange.NONE) <sub>_Instance variable_</sub>
        - [**Right**](#carla.LaneChange.Right) <sub>_Instance variable_</sub>
        - [**Left**](#carla.LaneChange.Left) <sub>_Instance variable_</sub>
        - [**Both**](#carla.LaneChange.Both) <sub>_Instance variable_</sub>
    - [**LaneInvasionEvent**](#carla.LaneInvasionEvent) <sub>_Class_</sub>  
        - [**actor**](#carla.LaneInvasionEvent.actor) <sub>_Instance variable_</sub>
        - [**crossed_lane_markings**](#carla.LaneInvasionEvent.crossed_lane_markings) <sub>_Instance variable_</sub>
    - [**LaneMarking**](#carla.LaneMarking) <sub>_Class_</sub>  
        - [**type**](#carla.LaneMarking.type) <sub>_Instance variable_</sub>
        - [**color**](#carla.LaneMarking.color) <sub>_Instance variable_</sub>
        - [**lane_change**](#carla.LaneMarking.lane_change) <sub>_Instance variable_</sub>
        - [**width**](#carla.LaneMarking.width) <sub>_Instance variable_</sub>
    - [**LaneMarkingColor**](#carla.LaneMarkingColor) <sub>_Class_</sub>  
        - [**Standard**](#carla.LaneMarkingColor.Standard) <sub>_Instance variable_</sub>
        - [**Blue**](#carla.LaneMarkingColor.Blue) <sub>_Instance variable_</sub>
        - [**Green**](#carla.LaneMarkingColor.Green) <sub>_Instance variable_</sub>
        - [**Red**](#carla.LaneMarkingColor.Red) <sub>_Instance variable_</sub>
        - [**White**](#carla.LaneMarkingColor.White) <sub>_Instance variable_</sub>
        - [**Yellow**](#carla.LaneMarkingColor.Yellow) <sub>_Instance variable_</sub>
        - [**Other**](#carla.LaneMarkingColor.Other) <sub>_Instance variable_</sub>
    - [**LaneMarkingType**](#carla.LaneMarkingType) <sub>_Class_</sub>  
        - [**NONE**](#carla.LaneMarkingType.NONE) <sub>_Instance variable_</sub>
        - [**Other**](#carla.LaneMarkingType.Other) <sub>_Instance variable_</sub>
        - [**Broken**](#carla.LaneMarkingType.Broken) <sub>_Instance variable_</sub>
        - [**Solid**](#carla.LaneMarkingType.Solid) <sub>_Instance variable_</sub>
        - [**SolidSolid**](#carla.LaneMarkingType.SolidSolid) <sub>_Instance variable_</sub>
        - [**SolidBroken**](#carla.LaneMarkingType.SolidBroken) <sub>_Instance variable_</sub>
        - [**BrokenSolid**](#carla.LaneMarkingType.BrokenSolid) <sub>_Instance variable_</sub>
        - [**BrokenBroken**](#carla.LaneMarkingType.BrokenBroken) <sub>_Instance variable_</sub>
        - [**BottsDots**](#carla.LaneMarkingType.BottsDots) <sub>_Instance variable_</sub>
        - [**Grass**](#carla.LaneMarkingType.Grass) <sub>_Instance variable_</sub>
        - [**Curb**](#carla.LaneMarkingType.Curb) <sub>_Instance variable_</sub>
    - [**LaneType**](#carla.LaneType) <sub>_Class_</sub>  
        - [**NONE**](#carla.LaneType.NONE) <sub>_Instance variable_</sub>
        - [**Driving**](#carla.LaneType.Driving) <sub>_Instance variable_</sub>
        - [**Stop**](#carla.LaneType.Stop) <sub>_Instance variable_</sub>
        - [**Shoulder**](#carla.LaneType.Shoulder) <sub>_Instance variable_</sub>
        - [**Biking**](#carla.LaneType.Biking) <sub>_Instance variable_</sub>
        - [**Sidewalk**](#carla.LaneType.Sidewalk) <sub>_Instance variable_</sub>
        - [**Border**](#carla.LaneType.Border) <sub>_Instance variable_</sub>
        - [**Restricted**](#carla.LaneType.Restricted) <sub>_Instance variable_</sub>
        - [**Parking**](#carla.LaneType.Parking) <sub>_Instance variable_</sub>
        - [**Bidirectional**](#carla.LaneType.Bidirectional) <sub>_Instance variable_</sub>
        - [**Median**](#carla.LaneType.Median) <sub>_Instance variable_</sub>
        - [**Special1**](#carla.LaneType.Special1) <sub>_Instance variable_</sub>
        - [**Special2**](#carla.LaneType.Special2) <sub>_Instance variable_</sub>
        - [**Special3**](#carla.LaneType.Special3) <sub>_Instance variable_</sub>
        - [**RoadWorks**](#carla.LaneType.RoadWorks) <sub>_Instance variable_</sub>
        - [**Tram**](#carla.LaneType.Tram) <sub>_Instance variable_</sub>
        - [**Rail**](#carla.LaneType.Rail) <sub>_Instance variable_</sub>
        - [**Entry**](#carla.LaneType.Entry) <sub>_Instance variable_</sub>
        - [**Exit**](#carla.LaneType.Exit) <sub>_Instance variable_</sub>
        - [**OffRamp**](#carla.LaneType.OffRamp) <sub>_Instance variable_</sub>
        - [**OnRamp**](#carla.LaneType.OnRamp) <sub>_Instance variable_</sub>
        - [**Any**](#carla.LaneType.Any) <sub>_Instance variable_</sub>
    - [**ObstacleDetectionEvent**](#carla.ObstacleDetectionEvent) <sub>_Class_</sub>  
        - [**actor**](#carla.ObstacleDetectionEvent.actor) <sub>_Instance variable_</sub>
        - [**other_actor**](#carla.ObstacleDetectionEvent.other_actor) <sub>_Instance variable_</sub>
        - [**distance**](#carla.ObstacleDetectionEvent.distance) <sub>_Instance variable_</sub>
    - [**SensorData**](#carla.SensorData) <sub>_Class_</sub>  
        - [**frame_number**](#carla.SensorData.frame_number) <sub>_Instance variable_</sub>
        - [**timestamp**](#carla.SensorData.timestamp) <sub>_Instance variable_</sub>
        - [**transform**](#carla.SensorData.transform) <sub>_Instance variable_</sub>
    - [**TrafficLightState**](#carla.TrafficLightState) <sub>_Class_</sub>  
        - [**Red**](#carla.TrafficLightState.Red) <sub>_Instance variable_</sub>
        - [**Yellow**](#carla.TrafficLightState.Yellow) <sub>_Instance variable_</sub>
        - [**Green**](#carla.TrafficLightState.Green) <sub>_Instance variable_</sub>
        - [**Off**](#carla.TrafficLightState.Off) <sub>_Instance variable_</sub>
        - [**Unknown**](#carla.TrafficLightState.Unknown) <sub>_Instance variable_</sub>
    - [**TrafficSign**](#carla.TrafficSign) <sub>_Class_</sub>  
        - [**trigger_volume**](#carla.TrafficSign.trigger_volume) <sub>_Instance variable_</sub>
    - [**Actor**](#carla.Actor) <sub>_Class_</sub>  
        - [**id**](#carla.Actor.id) <sub>_Instance variable_</sub>
        - [**type_id**](#carla.Actor.type_id) <sub>_Instance variable_</sub>
        - [**parent**](#carla.Actor.parent) <sub>_Instance variable_</sub>
        - [**semantic_tags**](#carla.Actor.semantic_tags) <sub>_Instance variable_</sub>
        - [**is_alive**](#carla.Actor.is_alive) <sub>_Instance variable_</sub>
        - [**attributes**](#carla.Actor.attributes) <sub>_Instance variable_</sub>
        - [**get_world**(**self**)](#carla.Actor.get_world) <sub>_Method_</sub>
        - [**get_location**(**self**)](#carla.Actor.get_location) <sub>_Method_</sub>
        - [**get_transform**(**self**)](#carla.Actor.get_transform) <sub>_Method_</sub>
        - [**get_velocity**(**self**)](#carla.Actor.get_velocity) <sub>_Method_</sub>
        - [**get_angular_velocity**(**self**)](#carla.Actor.get_angular_velocity) <sub>_Method_</sub>
        - [**get_acceleration**(**self**)](#carla.Actor.get_acceleration) <sub>_Method_</sub>
        - [**set_location**(**self**)](#carla.Actor.set_location) <sub>_Method_</sub>
        - [**set_transform**(**self**)](#carla.Actor.set_transform) <sub>_Method_</sub>
        - [**set_velocity**(**self**)](#carla.Actor.set_velocity) <sub>_Method_</sub>
        - [**set_angular_velocity**(**self**)](#carla.Actor.set_angular_velocity) <sub>_Method_</sub>
        - [**add_impulse**(**self**)](#carla.Actor.add_impulse) <sub>_Method_</sub>
        - [**set_simulate_physics**(**self**)](#carla.Actor.set_simulate_physics) <sub>_Method_</sub>
        - [**destroy**(**self**)](#carla.Actor.destroy) <sub>_Method_</sub>
    - [**ActorAttribute**](#carla.ActorAttribute) <sub>_Class_</sub>  
        - [**id**](#carla.ActorAttribute.id) <sub>_Instance variable_</sub>
        - [**type**](#carla.ActorAttribute.type) <sub>_Instance variable_</sub>
        - [**recommended_values**](#carla.ActorAttribute.recommended_values) <sub>_Instance variable_</sub>
        - [**is_modifiable**](#carla.ActorAttribute.is_modifiable) <sub>_Instance variable_</sub>
        - [**as_bool**(**self**)](#carla.ActorAttribute.as_bool) <sub>_Method_</sub>
        - [**as_int**(**self**)](#carla.ActorAttribute.as_int) <sub>_Method_</sub>
        - [**as_float**(**self**)](#carla.ActorAttribute.as_float) <sub>_Method_</sub>
        - [**as_str**(**self**)](#carla.ActorAttribute.as_str) <sub>_Method_</sub>
        - [**as_color**(**self**)](#carla.ActorAttribute.as_color) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__nonzero__**(**self**)](#carla.ActorAttribute.__nonzero__) <sub>_Method_</sub>
        - [**\__bool__**(**self**)](#carla.ActorAttribute.__bool__) <sub>_Method_</sub>
        - [**\__int__**(**self**)](#carla.ActorAttribute.__int__) <sub>_Method_</sub>
        - [**\__float__**(**self**)](#carla.ActorAttribute.__float__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.ActorAttribute.__str__) <sub>_Method_</sub>
    - [**ActorBlueprint**](#carla.ActorBlueprint) <sub>_Class_</sub>  
        - [**id**](#carla.ActorBlueprint.id) <sub>_Instance variable_</sub>
        - [**tags**](#carla.ActorBlueprint.tags) <sub>_Instance variable_</sub>
        - [**has_tag**(**self**)](#carla.ActorBlueprint.has_tag) <sub>_Method_</sub>
        - [**match_tags**(**self**)](#carla.ActorBlueprint.match_tags) <sub>_Method_</sub>
        - [**has_attribute**(**self**)](#carla.ActorBlueprint.has_attribute) <sub>_Method_</sub>
        - [**get_attribute**(**self**)](#carla.ActorBlueprint.get_attribute) <sub>_Method_</sub>
        - [**set_attribute**(**self**)](#carla.ActorBlueprint.set_attribute) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.ActorBlueprint.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.ActorBlueprint.__iter__) <sub>_Method_</sub>
    - [**BoundingBox**](#carla.BoundingBox) <sub>_Class_</sub>  
        - [**location**](#carla.BoundingBox.location) <sub>_Instance variable_</sub>
        - [**extent**](#carla.BoundingBox.extent) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.BoundingBox.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.BoundingBox.__ne__) <sub>_Method_</sub>
    - [**Client**](#carla.Client) <sub>_Class_</sub>  
        - [**\__init__**(**self**, **host**, **port**, **worker_threads**=0)](#carla.Client.__init__) <sub>_Method_</sub>
        - [**set_timeout**(**self**, **seconds**)](#carla.Client.set_timeout) <sub>_Method_</sub>
        - [**get_client_version**(**self**)](#carla.Client.get_client_version) <sub>_Method_</sub>
        - [**get_server_version**(**self**)](#carla.Client.get_server_version) <sub>_Method_</sub>
        - [**get_world**(**self**)](#carla.Client.get_world) <sub>_Method_</sub>
        - [**get_available_maps**(**self**)](#carla.Client.get_available_maps) <sub>_Method_</sub>
        - [**reload_world**(**self**)](#carla.Client.reload_world) <sub>_Method_</sub>
        - [**load_world**(**self**, **map_name**)](#carla.Client.load_world) <sub>_Method_</sub>
        - [**start_recorder**(**self**, **filename**)](#carla.Client.start_recorder) <sub>_Method_</sub>
        - [**stop_recorder**(**self**)](#carla.Client.stop_recorder) <sub>_Method_</sub>
        - [**show_recorder_file_info**(**self**, **filename**, **show_all**=False)](#carla.Client.show_recorder_file_info) <sub>_Method_</sub>
        - [**show_recorder_collisions**(**self**, **filename**, **category1**='a', **category2**='a')](#carla.Client.show_recorder_collisions) <sub>_Method_</sub>
        - [**show_recorder_actors_blocked**(**self**, **filename**, **min_time**=60.0, **min_distance**=100.0)](#carla.Client.show_recorder_actors_blocked) <sub>_Method_</sub>
        - [**replay_file**(**self**, **filename**, **start**=0.0, **duration**=0.0, **camera**=0)](#carla.Client.replay_file) <sub>_Method_</sub>
        - [**set_replayer_time_factor**(**self**, **time_factor**)](#carla.Client.set_replayer_time_factor) <sub>_Method_</sub>
        - [**apply_batch**(**self**)](#carla.Client.apply_batch) <sub>_Method_</sub>
        - [**apply_batch_sync**(**self**)](#carla.Client.apply_batch_sync) <sub>_Method_</sub>
    - [**Color**](#carla.Color) <sub>_Class_</sub>  
        - [**r**](#carla.Color.r) <sub>_Instance variable_</sub>
        - [**g**](#carla.Color.g) <sub>_Instance variable_</sub>
        - [**b**](#carla.Color.b) <sub>_Instance variable_</sub>
        - [**a**](#carla.Color.a) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Color.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Color.__ne__) <sub>_Method_</sub>
    - [**GeoLocation**](#carla.GeoLocation) <sub>_Class_</sub>  
        - [**latitude**](#carla.GeoLocation.latitude) <sub>_Instance variable_</sub>
        - [**longitude**](#carla.GeoLocation.longitude) <sub>_Instance variable_</sub>
        - [**altitude**](#carla.GeoLocation.altitude) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.GeoLocation.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.GeoLocation.__ne__) <sub>_Method_</sub>
    - [**Image**](#carla.Image) <sub>_Class_</sub>  
        - [**width**](#carla.Image.width) <sub>_Instance variable_</sub>
        - [**height**](#carla.Image.height) <sub>_Instance variable_</sub>
        - [**fov**](#carla.Image.fov) <sub>_Instance variable_</sub>
        - [**raw_data**](#carla.Image.raw_data) <sub>_Instance variable_</sub>
        - [**convert**(**self**)](#carla.Image.convert) <sub>_Method_</sub>
        - [**save_to_disk**(**self**)](#carla.Image.save_to_disk) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.Image.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.Image.__iter__) <sub>_Method_</sub>
        - [**\__getitem__**(**self**)](#carla.Image.__getitem__) <sub>_Method_</sub>
        - [**\__setitem__**(**self**)](#carla.Image.__setitem__) <sub>_Method_</sub>
    - [**LidarMeasurement**](#carla.LidarMeasurement) <sub>_Class_</sub>  
        - [**horizontal_angle**](#carla.LidarMeasurement.horizontal_angle) <sub>_Instance variable_</sub>
        - [**channels**](#carla.LidarMeasurement.channels) <sub>_Instance variable_</sub>
        - [**raw_data**](#carla.LidarMeasurement.raw_data) <sub>_Instance variable_</sub>
        - [**get_point_count**(**self**)](#carla.LidarMeasurement.get_point_count) <sub>_Method_</sub>
        - [**save_to_disk**(**self**)](#carla.LidarMeasurement.save_to_disk) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.LidarMeasurement.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.LidarMeasurement.__iter__) <sub>_Method_</sub>
        - [**\__getitem__**(**self**)](#carla.LidarMeasurement.__getitem__) <sub>_Method_</sub>
        - [**\__setitem__**(**self**)](#carla.LidarMeasurement.__setitem__) <sub>_Method_</sub>
    - [**Location**](#carla.Location) <sub>_Class_</sub>  
        - [**x**](#carla.Location.x) <sub>_Instance variable_</sub>
        - [**y**](#carla.Location.y) <sub>_Instance variable_</sub>
        - [**z**](#carla.Location.z) <sub>_Instance variable_</sub>
        - [**distance**(**self**)](#carla.Location.distance) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.Location.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Location.__ne__) <sub>_Method_</sub>
    - [**Map**](#carla.Map) <sub>_Class_</sub>  
        - [**name**](#carla.Map.name) <sub>_Instance variable_</sub>
        - [**get_spawn_points**(**self**)](#carla.Map.get_spawn_points) <sub>_Method_</sub>
        - [**get_waypoint**(**self**)](#carla.Map.get_waypoint) <sub>_Method_</sub>
        - [**get_topology**(**self**)](#carla.Map.get_topology) <sub>_Method_</sub>
        - [**generate_waypoints**(**self**)](#carla.Map.generate_waypoints) <sub>_Method_</sub>
        - [**transform_to_geolocation**(**self**)](#carla.Map.transform_to_geolocation) <sub>_Method_</sub>
        - [**to_opendrive**(**self**)](#carla.Map.to_opendrive) <sub>_Method_</sub>
        - [**save_to_disk**(**self**, **path**)](#carla.Map.save_to_disk) <sub>_Method_</sub>
    - [**Rotation**](#carla.Rotation) <sub>_Class_</sub>  
        - [**pitch**](#carla.Rotation.pitch) <sub>_Instance variable_</sub>
        - [**yaw**](#carla.Rotation.yaw) <sub>_Instance variable_</sub>
        - [**roll**](#carla.Rotation.roll) <sub>_Instance variable_</sub>
        - [**get_forward_vector**(**self**)](#carla.Rotation.get_forward_vector) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.Rotation.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Rotation.__ne__) <sub>_Method_</sub>
    - [**Sensor**](#carla.Sensor) <sub>_Class_</sub>  
        - [**is_listening**](#carla.Sensor.is_listening) <sub>_Instance variable_</sub>
        - [**listen**(**self**)](#carla.Sensor.listen) <sub>_Method_</sub>
        - [**stop**(**self**)](#carla.Sensor.stop) <sub>_Method_</sub>
    - [**Timestamp**](#carla.Timestamp) <sub>_Class_</sub>  
        - [**frame_count**](#carla.Timestamp.frame_count) <sub>_Instance variable_</sub>
        - [**elapsed_seconds**](#carla.Timestamp.elapsed_seconds) <sub>_Instance variable_</sub>
        - [**delta_seconds**](#carla.Timestamp.delta_seconds) <sub>_Instance variable_</sub>
        - [**platform_timestamp**](#carla.Timestamp.platform_timestamp) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.Timestamp.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Timestamp.__ne__) <sub>_Method_</sub>
    - [**TrafficLight**](#carla.TrafficLight) <sub>_Class_</sub>  
        - [**state**](#carla.TrafficLight.state) <sub>_Instance variable_</sub>
        - [**set_state**(**self**)](#carla.TrafficLight.set_state) <sub>_Method_</sub>
        - [**get_state**(**self**)](#carla.TrafficLight.get_state) <sub>_Method_</sub>
        - [**set_green_time**(**self**)](#carla.TrafficLight.set_green_time) <sub>_Method_</sub>
        - [**get_green_time**(**self**)](#carla.TrafficLight.get_green_time) <sub>_Method_</sub>
        - [**set_yellow_time**(**self**)](#carla.TrafficLight.set_yellow_time) <sub>_Method_</sub>
        - [**get_yellow_time**(**self**)](#carla.TrafficLight.get_yellow_time) <sub>_Method_</sub>
        - [**set_red_time**(**self**)](#carla.TrafficLight.set_red_time) <sub>_Method_</sub>
        - [**get_red_time**(**self**)](#carla.TrafficLight.get_red_time) <sub>_Method_</sub>
        - [**get_elapsed_time**(**self**)](#carla.TrafficLight.get_elapsed_time) <sub>_Method_</sub>
        - [**freeze**(**self**)](#carla.TrafficLight.freeze) <sub>_Method_</sub>
        - [**is_frozen**(**self**)](#carla.TrafficLight.is_frozen) <sub>_Method_</sub>
        - [**get_pole_index**(**self**)](#carla.TrafficLight.get_pole_index) <sub>_Method_</sub>
        - [**get_group_traffic_lights**(**self**)](#carla.TrafficLight.get_group_traffic_lights) <sub>_Method_</sub>
    - [**Transform**](#carla.Transform) <sub>_Class_</sub>  
        - [**location**](#carla.Transform.location) <sub>_Instance variable_</sub>
        - [**rotation**](#carla.Transform.rotation) <sub>_Instance variable_</sub>
        - [**transform**(**self**)](#carla.Transform.transform) <sub>_Method_</sub>
        - [**get_forward_vector**(**self**)](#carla.Transform.get_forward_vector) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.Transform.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Transform.__ne__) <sub>_Method_</sub>
    - [**Vector2D**](#carla.Vector2D) <sub>_Class_</sub>  
        - [**x**](#carla.Vector2D.x) <sub>_Instance variable_</sub>
        - [**y**](#carla.Vector2D.y) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.Vector2D.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Vector2D.__ne__) <sub>_Method_</sub>
    - [**Vector3D**](#carla.Vector3D) <sub>_Class_</sub>  
        - [**x**](#carla.Vector3D.x) <sub>_Instance variable_</sub>
        - [**y**](#carla.Vector3D.y) <sub>_Instance variable_</sub>
        - [**z**](#carla.Vector3D.z) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.Vector3D.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Vector3D.__ne__) <sub>_Method_</sub>
    - [**Vehicle**](#carla.Vehicle) <sub>_Class_</sub>  
        - [**bounding_box**](#carla.Vehicle.bounding_box) <sub>_Instance variable_</sub>
        - [**apply_control**(**self**)](#carla.Vehicle.apply_control) <sub>_Method_</sub>
        - [**get_control**(**self**)](#carla.Vehicle.get_control) <sub>_Method_</sub>
        - [**apply_physics_control**(**self**)](#carla.Vehicle.apply_physics_control) <sub>_Method_</sub>
        - [**get_physics_control**(**self**)](#carla.Vehicle.get_physics_control) <sub>_Method_</sub>
        - [**set_autopilot**(**self**)](#carla.Vehicle.set_autopilot) <sub>_Method_</sub>
        - [**get_speed_limit**(**self**)](#carla.Vehicle.get_speed_limit) <sub>_Method_</sub>
        - [**get_traffic_light_state**(**self**)](#carla.Vehicle.get_traffic_light_state) <sub>_Method_</sub>
        - [**is_at_traffic_light**(**self**)](#carla.Vehicle.is_at_traffic_light) <sub>_Method_</sub>
        - [**get_traffic_light**(**self**)](#carla.Vehicle.get_traffic_light) <sub>_Method_</sub>
    - [**VehicleControl**](#carla.VehicleControl) <sub>_Class_</sub>  
        - [**throttle**](#carla.VehicleControl.throttle) <sub>_Instance variable_</sub>
        - [**steer**](#carla.VehicleControl.steer) <sub>_Instance variable_</sub>
        - [**brake**](#carla.VehicleControl.brake) <sub>_Instance variable_</sub>
        - [**hand_brake**](#carla.VehicleControl.hand_brake) <sub>_Instance variable_</sub>
        - [**reverse**](#carla.VehicleControl.reverse) <sub>_Instance variable_</sub>
        - [**manual_gear_shift**](#carla.VehicleControl.manual_gear_shift) <sub>_Instance variable_</sub>
        - [**gear**](#carla.VehicleControl.gear) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#carla.VehicleControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.VehicleControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.VehicleControl.__ne__) <sub>_Method_</sub>
    - [**VehiclePhysicsControl**](#carla.VehiclePhysicsControl) <sub>_Class_</sub>  
        - [**torque_curve**](#carla.VehiclePhysicsControl.torque_curve) <sub>_Instance variable_</sub>
        - [**max_rpm**](#carla.VehiclePhysicsControl.max_rpm) <sub>_Instance variable_</sub>
        - [**moi**](#carla.VehiclePhysicsControl.moi) <sub>_Instance variable_</sub>
        - [**damping_rate_full_throttle**](#carla.VehiclePhysicsControl.damping_rate_full_throttle) <sub>_Instance variable_</sub>
        - [**damping_rate_zero_throttle_clutch_engaged**](#carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_engaged) <sub>_Instance variable_</sub>
        - [**damping_rate_zero_throttle_clutch_disengaged**](#carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_disengaged) <sub>_Instance variable_</sub>
        - [**use_gear_autobox**](#carla.VehiclePhysicsControl.use_gear_autobox) <sub>_Instance variable_</sub>
        - [**gear_switch_time**](#carla.VehiclePhysicsControl.gear_switch_time) <sub>_Instance variable_</sub>
        - [**clutch_strength**](#carla.VehiclePhysicsControl.clutch_strength) <sub>_Instance variable_</sub>
        - [**mass**](#carla.VehiclePhysicsControl.mass) <sub>_Instance variable_</sub>
        - [**drag_coefficient**](#carla.VehiclePhysicsControl.drag_coefficient) <sub>_Instance variable_</sub>
        - [**center_of_mass**](#carla.VehiclePhysicsControl.center_of_mass) <sub>_Instance variable_</sub>
        - [**steering_curve**](#carla.VehiclePhysicsControl.steering_curve) <sub>_Instance variable_</sub>
        - [**wheels**](#carla.VehiclePhysicsControl.wheels) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#carla.VehiclePhysicsControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.VehiclePhysicsControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.VehiclePhysicsControl.__ne__) <sub>_Method_</sub>
    - [**Walker**](#carla.Walker) <sub>_Class_</sub>  
        - [**bounding_box**](#carla.Walker.bounding_box) <sub>_Instance variable_</sub>
        - [**apply_control**(**self**)](#carla.Walker.apply_control) <sub>_Method_</sub>
        - [**get_control**(**self**)](#carla.Walker.get_control) <sub>_Method_</sub>
    - [**WalkerControl**](#carla.WalkerControl) <sub>_Class_</sub>  
        - [**direction**](#carla.WalkerControl.direction) <sub>_Instance variable_</sub>
        - [**speed**](#carla.WalkerControl.speed) <sub>_Instance variable_</sub>
        - [**jump**](#carla.WalkerControl.jump) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#carla.WalkerControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.WalkerControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.WalkerControl.__ne__) <sub>_Method_</sub>
    - [**Waypoint**](#carla.Waypoint) <sub>_Class_</sub>  
        - [**id**](#carla.Waypoint.id) <sub>_Instance variable_</sub>
        - [**transform**](#carla.Waypoint.transform) <sub>_Instance variable_</sub>
        - [**is_intersection**](#carla.Waypoint.is_intersection) <sub>_Instance variable_</sub>
        - [**lane_width**](#carla.Waypoint.lane_width) <sub>_Instance variable_</sub>
        - [**road_id**](#carla.Waypoint.road_id) <sub>_Instance variable_</sub>
        - [**section_id**](#carla.Waypoint.section_id) <sub>_Instance variable_</sub>
        - [**lane_id**](#carla.Waypoint.lane_id) <sub>_Instance variable_</sub>
        - [**s**](#carla.Waypoint.s) <sub>_Instance variable_</sub>
        - [**lane_change**](#carla.Waypoint.lane_change) <sub>_Instance variable_</sub>
        - [**lane_type**](#carla.Waypoint.lane_type) <sub>_Instance variable_</sub>
        - [**right_lane_marking**](#carla.Waypoint.right_lane_marking) <sub>_Instance variable_</sub>
        - [**left_lane_marking**](#carla.Waypoint.left_lane_marking) <sub>_Instance variable_</sub>
        - [**next**(**self**)](#carla.Waypoint.next) <sub>_Method_</sub>
        - [**get_right_lane**(**self**)](#carla.Waypoint.get_right_lane) <sub>_Method_</sub>
        - [**get_left_lane**(**self**)](#carla.Waypoint.get_left_lane) <sub>_Method_</sub>
    - [**WeatherParameters**](#carla.WeatherParameters) <sub>_Class_</sub>  
        - [**cloudyness**](#carla.WeatherParameters.cloudyness) <sub>_Instance variable_</sub>
        - [**precipitation**](#carla.WeatherParameters.precipitation) <sub>_Instance variable_</sub>
        - [**precipitation_deposits**](#carla.WeatherParameters.precipitation_deposits) <sub>_Instance variable_</sub>
        - [**wind_intensity**](#carla.WeatherParameters.wind_intensity) <sub>_Instance variable_</sub>
        - [**sun_azimuth_angle**](#carla.WeatherParameters.sun_azimuth_angle) <sub>_Instance variable_</sub>
        - [**sun_altitude_angle**](#carla.WeatherParameters.sun_altitude_angle) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#carla.WeatherParameters.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.WeatherParameters.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.WeatherParameters.__ne__) <sub>_Method_</sub>
    - [**WheelPhysicsControl**](#carla.WheelPhysicsControl) <sub>_Class_</sub>  
        - [**tire_friction**](#carla.WheelPhysicsControl.tire_friction) <sub>_Instance variable_</sub>
        - [**damping_rate**](#carla.WheelPhysicsControl.damping_rate) <sub>_Instance variable_</sub>
        - [**steer_angle**](#carla.WheelPhysicsControl.steer_angle) <sub>_Instance variable_</sub>
        - [**disable_steering**](#carla.WheelPhysicsControl.disable_steering) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#carla.WheelPhysicsControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.WheelPhysicsControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.WheelPhysicsControl.__ne__) <sub>_Method_</sub>
    - [**World**](#carla.World) <sub>_Class_</sub>  
        - [**id**](#carla.World.id) <sub>_Instance variable_</sub>
        - [**debug**](#carla.World.debug) <sub>_Instance variable_</sub>
        - [**get_blueprint_library**(**self**)](#carla.World.get_blueprint_library) <sub>_Method_</sub>
        - [**get_map**(**self**)](#carla.World.get_map) <sub>_Method_</sub>
        - [**get_spectator**(**self**)](#carla.World.get_spectator) <sub>_Method_</sub>
        - [**get_settings**(**self**)](#carla.World.get_settings) <sub>_Method_</sub>
        - [**apply_settings**(**self**)](#carla.World.apply_settings) <sub>_Method_</sub>
        - [**get_weather**(**self**)](#carla.World.get_weather) <sub>_Method_</sub>
        - [**set_weather**(**self**)](#carla.World.set_weather) <sub>_Method_</sub>
        - [**get_actors**(**self**)](#carla.World.get_actors) <sub>_Method_</sub>
        - [**get_actors**(**self**)](#carla.World.get_actors) <sub>_Method_</sub>
        - [**spawn_actor**(**self**)](#carla.World.spawn_actor) <sub>_Method_</sub>
        - [**try_spawn_actor**(**self**)](#carla.World.try_spawn_actor) <sub>_Method_</sub>
        - [**wait_for_tick**(**self**)](#carla.World.wait_for_tick) <sub>_Method_</sub>
        - [**on_tick**(**self**)](#carla.World.on_tick) <sub>_Method_</sub>
        - [**tick**(**self**)](#carla.World.tick) <sub>_Method_</sub>
    - [**WorldSettings**](#carla.WorldSettings) <sub>_Class_</sub>  
        - [**synchronous_mode**](#carla.WorldSettings.synchronous_mode) <sub>_Instance variable_</sub>
        - [**no_rendering_mode**](#carla.WorldSettings.no_rendering_mode) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.WorldSettings.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.WorldSettings.__ne__) <sub>_Method_</sub>
- [**commands**](#commands) <sub>_Module_</sub>  
    - [**ApplyAngularVelocity**](#commands.ApplyAngularVelocity) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyAngularVelocity.actor_id) <sub>_Instance variable_</sub>
        - [**angular_velocity**](#commands.ApplyAngularVelocity.angular_velocity) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyAngularVelocity.__init__) <sub>_Method_</sub>
    - [**ApplyImpulse**](#commands.ApplyImpulse) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyImpulse.actor_id) <sub>_Instance variable_</sub>
        - [**impulse**](#commands.ApplyImpulse.impulse) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyImpulse.__init__) <sub>_Method_</sub>
    - [**ApplyTransform**](#commands.ApplyTransform) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyTransform.actor_id) <sub>_Instance variable_</sub>
        - [**transform**](#commands.ApplyTransform.transform) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyTransform.__init__) <sub>_Method_</sub>
    - [**ApplyVehicleControl**](#commands.ApplyVehicleControl) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyVehicleControl.actor_id) <sub>_Instance variable_</sub>
        - [**control**](#commands.ApplyVehicleControl.control) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyVehicleControl.__init__) <sub>_Method_</sub>
    - [**ApplyVelocity**](#commands.ApplyVelocity) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyVelocity.actor_id) <sub>_Instance variable_</sub>
        - [**velocity**](#commands.ApplyVelocity.velocity) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyVelocity.__init__) <sub>_Method_</sub>
    - [**ApplyWalkerControl**](#commands.ApplyWalkerControl) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyWalkerControl.actor_id) <sub>_Instance variable_</sub>
        - [**control**](#commands.ApplyWalkerControl.control) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyWalkerControl.__init__) <sub>_Method_</sub>
    - [**DestroyActor**](#commands.DestroyActor) <sub>_Class_</sub>  
        - [**actor_id**](#commands.DestroyActor.actor_id) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.DestroyActor.__init__) <sub>_Method_</sub>
    - [**Response**](#commands.Response) <sub>_Class_</sub>  
        - [**actor_id**](#commands.Response.actor_id) <sub>_Instance variable_</sub>
        - [**error**](#commands.Response.error) <sub>_Instance variable_</sub>
        - [**has_error**(**self**)](#commands.Response.has_error) <sub>_Method_</sub>
    - [**SetAutopilot**](#commands.SetAutopilot) <sub>_Class_</sub>  
        - [**actor_id**](#commands.SetAutopilot.actor_id) <sub>_Instance variable_</sub>
        - [**enabled**](#commands.SetAutopilot.enabled) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.SetAutopilot.__init__) <sub>_Method_</sub>
    - [**SetSimulatePhysics**](#commands.SetSimulatePhysics) <sub>_Class_</sub>  
        - [**actor_id**](#commands.SetSimulatePhysics.actor_id) <sub>_Instance variable_</sub>
        - [**enabled**](#commands.SetSimulatePhysics.enabled) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.SetSimulatePhysics.__init__) <sub>_Method_</sub>
    - [**SpawnActor**](#commands.SpawnActor) <sub>_Class_</sub>  
        - [**transform**](#commands.SpawnActor.transform) <sub>_Instance variable_</sub>
        - [**parent_id**](#commands.SpawnActor.parent_id) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.SpawnActor.__init__) <sub>_Method_</sub>
        - [**then**(**self**)](#commands.SpawnActor.then) <sub>_Method_</sub>

## <a name="carla.ActorAttributeType"></a>ActorAttributeType <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ActorAttributeType.Bool"></a>**<font color="#f8805a">Bool</font>**  
- <a name="carla.ActorAttributeType.Int"></a>**<font color="#f8805a">Int</font>**  
- <a name="carla.ActorAttributeType.Float"></a>**<font color="#f8805a">Float</font>**  
- <a name="carla.ActorAttributeType.String"></a>**<font color="#f8805a">String</font>**  
- <a name="carla.ActorAttributeType.RGBColor"></a>**<font color="#f8805a">RGBColor</font>**  

---

## <a name="carla.ActorList"></a>ActorList <sub><sup>_Class_</sup></sub>

### Methods
- <a name="carla.ActorList.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.BlueprintLibrary"></a>BlueprintLibrary <sub><sup>_Class_</sup></sub>

### Methods
- <a name="carla.BlueprintLibrary.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.CollisionEvent"></a>CollisionEvent <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.CollisionEvent.actor"></a>**<font color="#f8805a">actor</font>**  
- <a name="carla.CollisionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>**  
- <a name="carla.CollisionEvent.normal_impulse"></a>**<font color="#f8805a">normal_impulse</font>**  

---

## <a name="carla.ColorConverter"></a>ColorConverter <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ColorConverter.Raw"></a>**<font color="#f8805a">Raw</font>**  
- <a name="carla.ColorConverter.Depth"></a>**<font color="#f8805a">Depth</font>**  
- <a name="carla.ColorConverter.LogarithmicDepth"></a>**<font color="#f8805a">LogarithmicDepth</font>**  
- <a name="carla.ColorConverter.CityScapesPalette"></a>**<font color="#f8805a">CityScapesPalette</font>**  

---

## <a name="carla.DebugHelper"></a>DebugHelper <sub><sup>_Class_</sup></sub>

### Methods
- <a name="carla.DebugHelper.draw_point"></a>**<font color="#7fb800">draw_point</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.DebugHelper.draw_line"></a>**<font color="#7fb800">draw_line</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.DebugHelper.draw_arrow"></a>**<font color="#7fb800">draw_arrow</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.DebugHelper.draw_box"></a>**<font color="#7fb800">draw_box</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.DebugHelper.draw_string"></a>**<font color="#7fb800">draw_string</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.GnssEvent"></a>GnssEvent <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.GnssEvent.latitude"></a>**<font color="#f8805a">latitude</font>**  
- <a name="carla.GnssEvent.longitude"></a>**<font color="#f8805a">longitude</font>**  
- <a name="carla.GnssEvent.altitude"></a>**<font color="#f8805a">altitude</font>**  

---

## <a name="carla.LaneChange"></a>LaneChange <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneChange.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneChange.Right"></a>**<font color="#f8805a">Right</font>**  
- <a name="carla.LaneChange.Left"></a>**<font color="#f8805a">Left</font>**  
- <a name="carla.LaneChange.Both"></a>**<font color="#f8805a">Both</font>**  

---

## <a name="carla.LaneInvasionEvent"></a>LaneInvasionEvent <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneInvasionEvent.actor"></a>**<font color="#f8805a">actor</font>**  
- <a name="carla.LaneInvasionEvent.crossed_lane_markings"></a>**<font color="#f8805a">crossed_lane_markings</font>**  

---

## <a name="carla.LaneMarking"></a>LaneMarking <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneMarking.type"></a>**<font color="#f8805a">type</font>**  
- <a name="carla.LaneMarking.color"></a>**<font color="#f8805a">color</font>**  
- <a name="carla.LaneMarking.lane_change"></a>**<font color="#f8805a">lane_change</font>**  
- <a name="carla.LaneMarking.width"></a>**<font color="#f8805a">width</font>**  

---

## <a name="carla.LaneMarkingColor"></a>LaneMarkingColor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneMarkingColor.Standard"></a>**<font color="#f8805a">Standard</font>**  
- <a name="carla.LaneMarkingColor.Blue"></a>**<font color="#f8805a">Blue</font>**  
- <a name="carla.LaneMarkingColor.Green"></a>**<font color="#f8805a">Green</font>**  
- <a name="carla.LaneMarkingColor.Red"></a>**<font color="#f8805a">Red</font>**  
- <a name="carla.LaneMarkingColor.White"></a>**<font color="#f8805a">White</font>**  
- <a name="carla.LaneMarkingColor.Yellow"></a>**<font color="#f8805a">Yellow</font>**  
- <a name="carla.LaneMarkingColor.Other"></a>**<font color="#f8805a">Other</font>**  

---

## <a name="carla.LaneMarkingType"></a>LaneMarkingType <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneMarkingType.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneMarkingType.Other"></a>**<font color="#f8805a">Other</font>**  
- <a name="carla.LaneMarkingType.Broken"></a>**<font color="#f8805a">Broken</font>**  
- <a name="carla.LaneMarkingType.Solid"></a>**<font color="#f8805a">Solid</font>**  
- <a name="carla.LaneMarkingType.SolidSolid"></a>**<font color="#f8805a">SolidSolid</font>**  
- <a name="carla.LaneMarkingType.SolidBroken"></a>**<font color="#f8805a">SolidBroken</font>**  
- <a name="carla.LaneMarkingType.BrokenSolid"></a>**<font color="#f8805a">BrokenSolid</font>**  
- <a name="carla.LaneMarkingType.BrokenBroken"></a>**<font color="#f8805a">BrokenBroken</font>**  
- <a name="carla.LaneMarkingType.BottsDots"></a>**<font color="#f8805a">BottsDots</font>**  
- <a name="carla.LaneMarkingType.Grass"></a>**<font color="#f8805a">Grass</font>**  
- <a name="carla.LaneMarkingType.Curb"></a>**<font color="#f8805a">Curb</font>**  

---

## <a name="carla.LaneType"></a>LaneType <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneType.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneType.Driving"></a>**<font color="#f8805a">Driving</font>**  
- <a name="carla.LaneType.Stop"></a>**<font color="#f8805a">Stop</font>**  
- <a name="carla.LaneType.Shoulder"></a>**<font color="#f8805a">Shoulder</font>**  
- <a name="carla.LaneType.Biking"></a>**<font color="#f8805a">Biking</font>**  
- <a name="carla.LaneType.Sidewalk"></a>**<font color="#f8805a">Sidewalk</font>**  
- <a name="carla.LaneType.Border"></a>**<font color="#f8805a">Border</font>**  
- <a name="carla.LaneType.Restricted"></a>**<font color="#f8805a">Restricted</font>**  
- <a name="carla.LaneType.Parking"></a>**<font color="#f8805a">Parking</font>**  
- <a name="carla.LaneType.Bidirectional"></a>**<font color="#f8805a">Bidirectional</font>**  
- <a name="carla.LaneType.Median"></a>**<font color="#f8805a">Median</font>**  
- <a name="carla.LaneType.Special1"></a>**<font color="#f8805a">Special1</font>**  
- <a name="carla.LaneType.Special2"></a>**<font color="#f8805a">Special2</font>**  
- <a name="carla.LaneType.Special3"></a>**<font color="#f8805a">Special3</font>**  
- <a name="carla.LaneType.RoadWorks"></a>**<font color="#f8805a">RoadWorks</font>**  
- <a name="carla.LaneType.Tram"></a>**<font color="#f8805a">Tram</font>**  
- <a name="carla.LaneType.Rail"></a>**<font color="#f8805a">Rail</font>**  
- <a name="carla.LaneType.Entry"></a>**<font color="#f8805a">Entry</font>**  
- <a name="carla.LaneType.Exit"></a>**<font color="#f8805a">Exit</font>**  
- <a name="carla.LaneType.OffRamp"></a>**<font color="#f8805a">OffRamp</font>**  
- <a name="carla.LaneType.OnRamp"></a>**<font color="#f8805a">OnRamp</font>**  
- <a name="carla.LaneType.Any"></a>**<font color="#f8805a">Any</font>**  

---

## <a name="carla.ObstacleDetectionEvent"></a>ObstacleDetectionEvent <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ObstacleDetectionEvent.actor"></a>**<font color="#f8805a">actor</font>**  
- <a name="carla.ObstacleDetectionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>**  
- <a name="carla.ObstacleDetectionEvent.distance"></a>**<font color="#f8805a">distance</font>**  

---

## <a name="carla.SensorData"></a>SensorData <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.SensorData.frame_number"></a>**<font color="#f8805a">frame_number</font>**  
- <a name="carla.SensorData.timestamp"></a>**<font color="#f8805a">timestamp</font>**  
- <a name="carla.SensorData.transform"></a>**<font color="#f8805a">transform</font>**  

---

## <a name="carla.TrafficLightState"></a>TrafficLightState <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.TrafficLightState.Red"></a>**<font color="#f8805a">Red</font>**  
- <a name="carla.TrafficLightState.Yellow"></a>**<font color="#f8805a">Yellow</font>**  
- <a name="carla.TrafficLightState.Green"></a>**<font color="#f8805a">Green</font>**  
- <a name="carla.TrafficLightState.Off"></a>**<font color="#f8805a">Off</font>**  
- <a name="carla.TrafficLightState.Unknown"></a>**<font color="#f8805a">Unknown</font>**  

---

## <a name="carla.TrafficSign"></a>TrafficSign <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.TrafficSign.trigger_volume"></a>**<font color="#f8805a">trigger_volume</font>**  

---

## <a name="carla.Actor"></a>Actor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Actor.id"></a>**<font color="#f8805a">id</font>**  
- <a name="carla.Actor.type_id"></a>**<font color="#f8805a">type_id</font>**  
- <a name="carla.Actor.parent"></a>**<font color="#f8805a">parent</font>**  
- <a name="carla.Actor.semantic_tags"></a>**<font color="#f8805a">semantic_tags</font>**  
- <a name="carla.Actor.is_alive"></a>**<font color="#f8805a">is_alive</font>**  
- <a name="carla.Actor.attributes"></a>**<font color="#f8805a">attributes</font>**  

### Methods
- <a name="carla.Actor.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.get_location"></a>**<font color="#7fb800">get_location</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.get_transform"></a>**<font color="#7fb800">get_transform</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.get_velocity"></a>**<font color="#7fb800">get_velocity</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.get_angular_velocity"></a>**<font color="#7fb800">get_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.get_acceleration"></a>**<font color="#7fb800">get_acceleration</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.set_location"></a>**<font color="#7fb800">set_location</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.set_transform"></a>**<font color="#7fb800">set_transform</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.set_velocity"></a>**<font color="#7fb800">set_velocity</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.set_angular_velocity"></a>**<font color="#7fb800">set_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.add_impulse"></a>**<font color="#7fb800">add_impulse</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.set_simulate_physics"></a>**<font color="#7fb800">set_simulate_physics</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Actor.destroy"></a>**<font color="#7fb800">destroy</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.ActorAttribute"></a>ActorAttribute <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ActorAttribute.id"></a>**<font color="#f8805a">id</font>**  
- <a name="carla.ActorAttribute.type"></a>**<font color="#f8805a">type</font>**  
- <a name="carla.ActorAttribute.recommended_values"></a>**<font color="#f8805a">recommended_values</font>**  
- <a name="carla.ActorAttribute.is_modifiable"></a>**<font color="#f8805a">is_modifiable</font>**  

### Methods
- <a name="carla.ActorAttribute.as_bool"></a>**<font color="#7fb800">as_bool</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_int"></a>**<font color="#7fb800">as_int</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_float"></a>**<font color="#7fb800">as_float</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_str"></a>**<font color="#7fb800">as_str</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_color"></a>**<font color="#7fb800">as_color</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__nonzero__"></a>**<font color="#7fb800">\__nonzero__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__bool__"></a>**<font color="#7fb800">\__bool__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__int__"></a>**<font color="#7fb800">\__int__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__float__"></a>**<font color="#7fb800">\__float__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.ActorBlueprint"></a>ActorBlueprint <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ActorBlueprint.id"></a>**<font color="#f8805a">id</font>**  
- <a name="carla.ActorBlueprint.tags"></a>**<font color="#f8805a">tags</font>**  

### Methods
- <a name="carla.ActorBlueprint.has_tag"></a>**<font color="#7fb800">has_tag</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.match_tags"></a>**<font color="#7fb800">match_tags</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.has_attribute"></a>**<font color="#7fb800">has_attribute</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.get_attribute"></a>**<font color="#7fb800">get_attribute</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.set_attribute"></a>**<font color="#7fb800">set_attribute</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.BoundingBox"></a>BoundingBox <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.BoundingBox.location"></a>**<font color="#f8805a">location</font>**  
- <a name="carla.BoundingBox.extent"></a>**<font color="#f8805a">extent</font>**  

### Methods
- <a name="carla.BoundingBox.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BoundingBox.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Client"></a>Client <sub><sup>_Class_</sup></sub>

### Methods
- <a name="carla.Client.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**host**</font>, <font color="#00a6ed">**port**</font>, <font color="#00a6ed">**worker_threads**=0</font>)  
Client constructor.  
    - **Parameters:**
        - `host` (_str_) – IP where Carla is running.  
        - `port` (_int_) – Port where Carla is running.  
        - `worker_threads` (_int_) – Number of working threads.  
- <a name="carla.Client.set_timeout"></a>**<font color="#7fb800">set_timeout</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**</font>)  
Sets the server timeout in seconds.  
    - **Parameters:**
        - `seconds` (_float_) – New timeout value in seconds.  
- <a name="carla.Client.get_client_version"></a>**<font color="#7fb800">get_client_version</font>**(<font color="#00a6ed">**self**</font>)  
Get the client version as a string.  
- <a name="carla.Client.get_server_version"></a>**<font color="#7fb800">get_server_version</font>**(<font color="#00a6ed">**self**</font>)  
Get the server version as a string.  
- <a name="carla.Client.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
Get the server version as a string.  
- <a name="carla.Client.get_available_maps"></a>**<font color="#7fb800">get_available_maps</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Client.reload_world"></a>**<font color="#7fb800">reload_world</font>**(<font color="#00a6ed">**self**</font>)  
    - **Raises:** RuntimeError  
- <a name="carla.Client.load_world"></a>**<font color="#7fb800">load_world</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**map_name**</font>)  
    - **Parameters:**
        - `map_name` (_str_) – Name of the map to load.  
- <a name="carla.Client.start_recorder"></a>**<font color="#7fb800">start_recorder</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>)  
If we use a simple name like 'recording.log' then it will be saved at server folder 'CarlaUE4/Saved/recording.log'. If we use some folder in the name, then it will be considered to be an absolute path, like '/home/carla/recording.log'.  
    - **Parameters:**
        - `filename` (_str_) – Name of the file to create.  
- <a name="carla.Client.stop_recorder"></a>**<font color="#7fb800">stop_recorder</font>**(<font color="#00a6ed">**self**</font>)  
Stops the recording in curse.  
- <a name="carla.Client.show_recorder_file_info"></a>**<font color="#7fb800">show_recorder_file_info</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**show_all**=False</font>)  
Will show info about the recorded file. We have the option to show all the details per frame, that includes all the traffic light states, position of all actors, and animations data.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `show_all` (_bool_) – Show all detailed info, or just a summary.  
- <a name="carla.Client.show_recorder_collisions"></a>**<font color="#7fb800">show_recorder_collisions</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**category1**='a'</font>, <font color="#00a6ed">**category2**='a'</font>)  
This will show which collisions were recorded in the file. We can use a filter for the collisions we want, using two categories. The categories can be:  
  \- `h` = Hero  
  \- `v` = Vehicle  
  \- `w` = Walker  
  \- `t` = Traffic light  
  \- `o` = Other  
  \- `a` = Any  
So, if you want to see only collisions about a vehicle and a walker, we would use for category1 'v' and category2 'w'. Or if you want all the collisions (filter off) you can use 'a' as categories.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `category1` (_single char_) – Character specifying the category of the first actor.  
        - `category2` (_single char_) – Character specifying the category of the second actor.  
- <a name="carla.Client.show_recorder_actors_blocked"></a>**<font color="#7fb800">show_recorder_actors_blocked</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**min_time**=60.0</font>, <font color="#00a6ed">**min_distance**=100.0</font>)  
Shows which actors seems blocked by some reason. The idea is to calculate which actors are not moving as much as 'min_distance' for a period of 'min_time'. By default min_time = 60 seconds (1 min) and min_distance = 100 centimeters (1 m).  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `min_time` (_float_) – How many seconds has to be stoped an actor to be considered as blocked.  
        - `min_distance` (_float_) – How many centimeters needs to displace an actor in order to not be considered as blocked.  
- <a name="carla.Client.replay_file"></a>**<font color="#7fb800">replay_file</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**start**=0.0</font>, <font color="#00a6ed">**duration**=0.0</font>, <font color="#00a6ed">**camera**=0</font>)  
Playback a file.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to play.  
        - `start` (_float_) – Time in seconds where to start the playback. If it is negative, then it starts from the end.  
        - `duration` (_float_) – Time of playback, after that time the playback stops and all the actors are left driving in autopilot. A value of 0 means playback until the end.  
        - `camera` (_int_) – Id of the actor to follow. If this is 0 then camera is disabled.  
- <a name="carla.Client.set_replayer_time_factor"></a>**<font color="#7fb800">set_replayer_time_factor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**time_factor**</font>)  
Apply a different playback speed to current playback. Can be used several times while a playback is in curse.  
    - **Parameters:**
        - `time_factor` (_float_) – These values means:  
  \- A value of 1.0 means normal time factor.  
  \- A value < 1.0 means slow motion (for example 0.5 is half speed)  
  \- A value > 1.0 means fast motion (for example 2.0 is double speed).  
- <a name="carla.Client.apply_batch"></a>**<font color="#7fb800">apply_batch</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Client.apply_batch_sync"></a>**<font color="#7fb800">apply_batch_sync</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Color"></a>Color <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Color.r"></a>**<font color="#f8805a">r</font>**  
- <a name="carla.Color.g"></a>**<font color="#f8805a">g</font>**  
- <a name="carla.Color.b"></a>**<font color="#f8805a">b</font>**  
- <a name="carla.Color.a"></a>**<font color="#f8805a">a</font>**  

### Methods
- <a name="carla.Color.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other`  
- <a name="carla.Color.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other`  

---

## <a name="carla.GeoLocation"></a>GeoLocation <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.GeoLocation.latitude"></a>**<font color="#f8805a">latitude</font>**  
- <a name="carla.GeoLocation.longitude"></a>**<font color="#f8805a">longitude</font>**  
- <a name="carla.GeoLocation.altitude"></a>**<font color="#f8805a">altitude</font>**  

### Methods
- <a name="carla.GeoLocation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.GeoLocation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Image"></a>Image <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Image.width"></a>**<font color="#f8805a">width</font>**  
- <a name="carla.Image.height"></a>**<font color="#f8805a">height</font>**  
- <a name="carla.Image.fov"></a>**<font color="#f8805a">fov</font>**  
- <a name="carla.Image.raw_data"></a>**<font color="#f8805a">raw_data</font>**  

### Methods
- <a name="carla.Image.convert"></a>**<font color="#7fb800">convert</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.LidarMeasurement"></a>LidarMeasurement <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LidarMeasurement.horizontal_angle"></a>**<font color="#f8805a">horizontal_angle</font>**  
- <a name="carla.LidarMeasurement.channels"></a>**<font color="#f8805a">channels</font>**  
- <a name="carla.LidarMeasurement.raw_data"></a>**<font color="#f8805a">raw_data</font>**  

### Methods
- <a name="carla.LidarMeasurement.get_point_count"></a>**<font color="#7fb800">get_point_count</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Location"></a>Location <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Location.x"></a>**<font color="#f8805a">x</font>**  
- <a name="carla.Location.y"></a>**<font color="#f8805a">y</font>**  
- <a name="carla.Location.z"></a>**<font color="#f8805a">z</font>**  

### Methods
- <a name="carla.Location.distance"></a>**<font color="#7fb800">distance</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Location.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Location.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Map"></a>Map <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Map.name"></a>**<font color="#f8805a">name</font>**  
Map name.  

### Methods
- <a name="carla.Map.get_spawn_points"></a>**<font color="#7fb800">get_spawn_points</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Map.get_waypoint"></a>**<font color="#7fb800">get_waypoint</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Map.get_topology"></a>**<font color="#7fb800">get_topology</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Map.generate_waypoints"></a>**<font color="#7fb800">generate_waypoints</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Map.transform_to_geolocation"></a>**<font color="#7fb800">transform_to_geolocation</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Map.to_opendrive"></a>**<font color="#7fb800">to_opendrive</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Map.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Save the OpenDrive of the current map to disk.  
    - **Parameters:**
        - `path` – Path where will be saved.  

---

## <a name="carla.Rotation"></a>Rotation <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Rotation.pitch"></a>**<font color="#f8805a">pitch</font>**  
- <a name="carla.Rotation.yaw"></a>**<font color="#f8805a">yaw</font>**  
- <a name="carla.Rotation.roll"></a>**<font color="#f8805a">roll</font>**  

### Methods
- <a name="carla.Rotation.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Rotation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Rotation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Sensor"></a>Sensor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Sensor.is_listening"></a>**<font color="#f8805a">is_listening</font>**  

### Methods
- <a name="carla.Sensor.listen"></a>**<font color="#7fb800">listen</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Sensor.stop"></a>**<font color="#7fb800">stop</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Timestamp"></a>Timestamp <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Timestamp.frame_count"></a>**<font color="#f8805a">frame_count</font>**  
- <a name="carla.Timestamp.elapsed_seconds"></a>**<font color="#f8805a">elapsed_seconds</font>**  
- <a name="carla.Timestamp.delta_seconds"></a>**<font color="#f8805a">delta_seconds</font>**  
- <a name="carla.Timestamp.platform_timestamp"></a>**<font color="#f8805a">platform_timestamp</font>**  

### Methods
- <a name="carla.Timestamp.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Timestamp.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.TrafficLight"></a>TrafficLight <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.TrafficLight.state"></a>**<font color="#f8805a">state</font>**  

### Methods
- <a name="carla.TrafficLight.set_state"></a>**<font color="#7fb800">set_state</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.get_state"></a>**<font color="#7fb800">get_state</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.set_green_time"></a>**<font color="#7fb800">set_green_time</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.get_green_time"></a>**<font color="#7fb800">get_green_time</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.set_yellow_time"></a>**<font color="#7fb800">set_yellow_time</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.get_yellow_time"></a>**<font color="#7fb800">get_yellow_time</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.set_red_time"></a>**<font color="#7fb800">set_red_time</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.get_red_time"></a>**<font color="#7fb800">get_red_time</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.get_elapsed_time"></a>**<font color="#7fb800">get_elapsed_time</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.freeze"></a>**<font color="#7fb800">freeze</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.is_frozen"></a>**<font color="#7fb800">is_frozen</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.get_pole_index"></a>**<font color="#7fb800">get_pole_index</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.TrafficLight.get_group_traffic_lights"></a>**<font color="#7fb800">get_group_traffic_lights</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Transform"></a>Transform <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Transform.location"></a>**<font color="#f8805a">location</font>**  
- <a name="carla.Transform.rotation"></a>**<font color="#f8805a">rotation</font>**  

### Methods
- <a name="carla.Transform.transform"></a>**<font color="#7fb800">transform</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Transform.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Transform.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Transform.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Vector2D"></a>Vector2D <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Vector2D.x"></a>**<font color="#f8805a">x</font>**  
- <a name="carla.Vector2D.y"></a>**<font color="#f8805a">y</font>**  

### Methods
- <a name="carla.Vector2D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vector2D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Vector3D"></a>Vector3D <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Vector3D.x"></a>**<font color="#f8805a">x</font>**  
- <a name="carla.Vector3D.y"></a>**<font color="#f8805a">y</font>**  
- <a name="carla.Vector3D.z"></a>**<font color="#f8805a">z</font>**  

### Methods
- <a name="carla.Vector3D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vector3D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Vehicle"></a>Vehicle <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Vehicle.bounding_box"></a>**<font color="#f8805a">bounding_box</font>**  

### Methods
- <a name="carla.Vehicle.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.apply_physics_control"></a>**<font color="#7fb800">apply_physics_control</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.get_physics_control"></a>**<font color="#7fb800">get_physics_control</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.set_autopilot"></a>**<font color="#7fb800">set_autopilot</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.get_speed_limit"></a>**<font color="#7fb800">get_speed_limit</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.get_traffic_light_state"></a>**<font color="#7fb800">get_traffic_light_state</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.is_at_traffic_light"></a>**<font color="#7fb800">is_at_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Vehicle.get_traffic_light"></a>**<font color="#7fb800">get_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.VehicleControl"></a>VehicleControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.VehicleControl.throttle"></a>**<font color="#f8805a">throttle</font>**  
- <a name="carla.VehicleControl.steer"></a>**<font color="#f8805a">steer</font>**  
- <a name="carla.VehicleControl.brake"></a>**<font color="#f8805a">brake</font>**  
- <a name="carla.VehicleControl.hand_brake"></a>**<font color="#f8805a">hand_brake</font>**  
- <a name="carla.VehicleControl.reverse"></a>**<font color="#f8805a">reverse</font>**  
- <a name="carla.VehicleControl.manual_gear_shift"></a>**<font color="#f8805a">manual_gear_shift</font>**  
- <a name="carla.VehicleControl.gear"></a>**<font color="#f8805a">gear</font>**  

### Methods
- <a name="carla.VehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.VehicleControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.VehicleControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.VehiclePhysicsControl"></a>VehiclePhysicsControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.VehiclePhysicsControl.torque_curve"></a>**<font color="#f8805a">torque_curve</font>**  
- <a name="carla.VehiclePhysicsControl.max_rpm"></a>**<font color="#f8805a">max_rpm</font>**  
- <a name="carla.VehiclePhysicsControl.moi"></a>**<font color="#f8805a">moi</font>**  
- <a name="carla.VehiclePhysicsControl.damping_rate_full_throttle"></a>**<font color="#f8805a">damping_rate_full_throttle</font>**  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_engaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_engaged</font>**  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_disengaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_disengaged</font>**  
- <a name="carla.VehiclePhysicsControl.use_gear_autobox"></a>**<font color="#f8805a">use_gear_autobox</font>**  
- <a name="carla.VehiclePhysicsControl.gear_switch_time"></a>**<font color="#f8805a">gear_switch_time</font>**  
- <a name="carla.VehiclePhysicsControl.clutch_strength"></a>**<font color="#f8805a">clutch_strength</font>**  
- <a name="carla.VehiclePhysicsControl.mass"></a>**<font color="#f8805a">mass</font>**  
- <a name="carla.VehiclePhysicsControl.drag_coefficient"></a>**<font color="#f8805a">drag_coefficient</font>**  
- <a name="carla.VehiclePhysicsControl.center_of_mass"></a>**<font color="#f8805a">center_of_mass</font>**  
- <a name="carla.VehiclePhysicsControl.steering_curve"></a>**<font color="#f8805a">steering_curve</font>**  
- <a name="carla.VehiclePhysicsControl.wheels"></a>**<font color="#f8805a">wheels</font>**  

### Methods
- <a name="carla.VehiclePhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.VehiclePhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.VehiclePhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Walker"></a>Walker <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Walker.bounding_box"></a>**<font color="#f8805a">bounding_box</font>**  

### Methods
- <a name="carla.Walker.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Walker.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.WalkerControl"></a>WalkerControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.WalkerControl.direction"></a>**<font color="#f8805a">direction</font>**  
- <a name="carla.WalkerControl.speed"></a>**<font color="#f8805a">speed</font>**  
- <a name="carla.WalkerControl.jump"></a>**<font color="#f8805a">jump</font>**  

### Methods
- <a name="carla.WalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WalkerControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WalkerControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.Waypoint"></a>Waypoint <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Waypoint.id"></a>**<font color="#f8805a">id</font>**  
- <a name="carla.Waypoint.transform"></a>**<font color="#f8805a">transform</font>**  
- <a name="carla.Waypoint.is_intersection"></a>**<font color="#f8805a">is_intersection</font>**  
- <a name="carla.Waypoint.lane_width"></a>**<font color="#f8805a">lane_width</font>**  
- <a name="carla.Waypoint.road_id"></a>**<font color="#f8805a">road_id</font>**  
- <a name="carla.Waypoint.section_id"></a>**<font color="#f8805a">section_id</font>**  
- <a name="carla.Waypoint.lane_id"></a>**<font color="#f8805a">lane_id</font>**  
- <a name="carla.Waypoint.s"></a>**<font color="#f8805a">s</font>**  
- <a name="carla.Waypoint.lane_change"></a>**<font color="#f8805a">lane_change</font>**  
- <a name="carla.Waypoint.lane_type"></a>**<font color="#f8805a">lane_type</font>**  
- <a name="carla.Waypoint.right_lane_marking"></a>**<font color="#f8805a">right_lane_marking</font>**  
- <a name="carla.Waypoint.left_lane_marking"></a>**<font color="#f8805a">left_lane_marking</font>**  

### Methods
- <a name="carla.Waypoint.next"></a>**<font color="#7fb800">next</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Waypoint.get_right_lane"></a>**<font color="#7fb800">get_right_lane</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Waypoint.get_left_lane"></a>**<font color="#7fb800">get_left_lane</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.WeatherParameters"></a>WeatherParameters <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.WeatherParameters.cloudyness"></a>**<font color="#f8805a">cloudyness</font>**  
- <a name="carla.WeatherParameters.precipitation"></a>**<font color="#f8805a">precipitation</font>**  
- <a name="carla.WeatherParameters.precipitation_deposits"></a>**<font color="#f8805a">precipitation_deposits</font>**  
- <a name="carla.WeatherParameters.wind_intensity"></a>**<font color="#f8805a">wind_intensity</font>**  
- <a name="carla.WeatherParameters.sun_azimuth_angle"></a>**<font color="#f8805a">sun_azimuth_angle</font>**  
- <a name="carla.WeatherParameters.sun_altitude_angle"></a>**<font color="#f8805a">sun_altitude_angle</font>**  

### Methods
- <a name="carla.WeatherParameters.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WeatherParameters.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WeatherParameters.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.WheelPhysicsControl"></a>WheelPhysicsControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.WheelPhysicsControl.tire_friction"></a>**<font color="#f8805a">tire_friction</font>**  
- <a name="carla.WheelPhysicsControl.damping_rate"></a>**<font color="#f8805a">damping_rate</font>**  
- <a name="carla.WheelPhysicsControl.steer_angle"></a>**<font color="#f8805a">steer_angle</font>**  
- <a name="carla.WheelPhysicsControl.disable_steering"></a>**<font color="#f8805a">disable_steering</font>**  

### Methods
- <a name="carla.WheelPhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WheelPhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WheelPhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.World"></a>World <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.World.id"></a>**<font color="#f8805a">id</font>**  
- <a name="carla.World.debug"></a>**<font color="#f8805a">debug</font>**  

### Methods
- <a name="carla.World.get_blueprint_library"></a>**<font color="#7fb800">get_blueprint_library</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.get_map"></a>**<font color="#7fb800">get_map</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.get_spectator"></a>**<font color="#7fb800">get_spectator</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.get_settings"></a>**<font color="#7fb800">get_settings</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.apply_settings"></a>**<font color="#7fb800">apply_settings</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.get_weather"></a>**<font color="#7fb800">get_weather</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.set_weather"></a>**<font color="#7fb800">set_weather</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.get_actors"></a>**<font color="#7fb800">get_actors</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.get_actors"></a>**<font color="#7fb800">get_actors</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.spawn_actor"></a>**<font color="#7fb800">spawn_actor</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.try_spawn_actor"></a>**<font color="#7fb800">try_spawn_actor</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.wait_for_tick"></a>**<font color="#7fb800">wait_for_tick</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.on_tick"></a>**<font color="#7fb800">on_tick</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.World.tick"></a>**<font color="#7fb800">tick</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="carla.WorldSettings"></a>WorldSettings <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.WorldSettings.synchronous_mode"></a>**<font color="#f8805a">synchronous_mode</font>**  
- <a name="carla.WorldSettings.no_rendering_mode"></a>**<font color="#f8805a">no_rendering_mode</font>**  

### Methods
- <a name="carla.WorldSettings.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WorldSettings.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.ApplyAngularVelocity"></a>ApplyAngularVelocity <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyAngularVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.ApplyAngularVelocity.angular_velocity"></a>**<font color="#f8805a">angular_velocity</font>**  

### Methods
- <a name="commands.ApplyAngularVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.ApplyImpulse"></a>ApplyImpulse <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyImpulse.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.ApplyImpulse.impulse"></a>**<font color="#f8805a">impulse</font>**  

### Methods
- <a name="commands.ApplyImpulse.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.ApplyTransform"></a>ApplyTransform <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyTransform.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.ApplyTransform.transform"></a>**<font color="#f8805a">transform</font>**  

### Methods
- <a name="commands.ApplyTransform.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.ApplyVehicleControl"></a>ApplyVehicleControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyVehicleControl.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.ApplyVehicleControl.control"></a>**<font color="#f8805a">control</font>**  

### Methods
- <a name="commands.ApplyVehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.ApplyVelocity"></a>ApplyVelocity <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.ApplyVelocity.velocity"></a>**<font color="#f8805a">velocity</font>**  

### Methods
- <a name="commands.ApplyVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.ApplyWalkerControl"></a>ApplyWalkerControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyWalkerControl.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.ApplyWalkerControl.control"></a>**<font color="#f8805a">control</font>**  

### Methods
- <a name="commands.ApplyWalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.DestroyActor"></a>DestroyActor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.DestroyActor.actor_id"></a>**<font color="#f8805a">actor_id</font>**  

### Methods
- <a name="commands.DestroyActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.Response"></a>Response <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.Response.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.Response.error"></a>**<font color="#f8805a">error</font>**  

### Methods
- <a name="commands.Response.has_error"></a>**<font color="#7fb800">has_error</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.SetAutopilot"></a>SetAutopilot <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.SetAutopilot.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.SetAutopilot.enabled"></a>**<font color="#f8805a">enabled</font>**  

### Methods
- <a name="commands.SetAutopilot.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.SetSimulatePhysics"></a>SetSimulatePhysics <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.SetSimulatePhysics.actor_id"></a>**<font color="#f8805a">actor_id</font>**  
- <a name="commands.SetSimulatePhysics.enabled"></a>**<font color="#f8805a">enabled</font>**  

### Methods
- <a name="commands.SetSimulatePhysics.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  

---

## <a name="commands.SpawnActor"></a>SpawnActor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.SpawnActor.transform"></a>**<font color="#f8805a">transform</font>**  
- <a name="commands.SpawnActor.parent_id"></a>**<font color="#f8805a">parent_id</font>**  

### Methods
- <a name="commands.SpawnActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="commands.SpawnActor.then"></a>**<font color="#7fb800">then</font>**(<font color="#00a6ed">**self**</font>)  

---