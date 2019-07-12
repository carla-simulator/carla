### Overview
- [**carla**](#carla) <sub>_Module_</sub>  
    - [**ActorAttributeType**](#carla.ActorAttributeType) <sub>_Class_</sub>  
        - [**Bool**](#carla.ActorAttributeType.Bool) <sub>_Instance variable_</sub>
        - [**Int**](#carla.ActorAttributeType.Int) <sub>_Instance variable_</sub>
        - [**Float**](#carla.ActorAttributeType.Float) <sub>_Instance variable_</sub>
        - [**String**](#carla.ActorAttributeType.String) <sub>_Instance variable_</sub>
        - [**RGBColor**](#carla.ActorAttributeType.RGBColor) <sub>_Instance variable_</sub>
    - [**ActorList**](#carla.ActorList) <sub>_Class_</sub>  
        - [**find**(**self**, **actor_id**)](#carla.ActorList.find) <sub>_Method_</sub>
        - [**filter**(**self**, **wildcard_pattern**)](#carla.ActorList.filter) <sub>_Method_</sub>
        - [**\__getitem__**(**self**, **pos**)](#carla.ActorList.__getitem__) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.ActorList.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.ActorList.__iter__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.ActorList.__str__) <sub>_Method_</sub>
    - [**AttachmentType**](#carla.AttachmentType) <sub>_Class_</sub>  
        - [**Rigid**](#carla.AttachmentType.Rigid) <sub>_Instance variable_</sub>
        - [**SpringArm**](#carla.AttachmentType.SpringArm) <sub>_Instance variable_</sub>
    - [**BlueprintLibrary**](#carla.BlueprintLibrary) <sub>_Class_</sub>  
        - [**find**(**self**)](#carla.BlueprintLibrary.find) <sub>_Method_</sub>
        - [**filter**(**self**, **wildcard_pattern**)](#carla.BlueprintLibrary.filter) <sub>_Method_</sub>
        - [**\__getitem__**(**self**, **pos**)](#carla.BlueprintLibrary.__getitem__) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.BlueprintLibrary.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.BlueprintLibrary.__iter__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.BlueprintLibrary.__str__) <sub>_Method_</sub>
    - [**ColorConverter**](#carla.ColorConverter) <sub>_Class_</sub>  
        - [**Raw**](#carla.ColorConverter.Raw) <sub>_Instance variable_</sub>
        - [**Depth**](#carla.ColorConverter.Depth) <sub>_Instance variable_</sub>
        - [**LogarithmicDepth**](#carla.ColorConverter.LogarithmicDepth) <sub>_Instance variable_</sub>
        - [**CityScapesPalette**](#carla.ColorConverter.CityScapesPalette) <sub>_Instance variable_</sub>
    - [**DebugHelper**](#carla.DebugHelper) <sub>_Class_</sub>  
        - [**draw_point**(**self**, **location**, **size**=0.1f, **color**=(255,0,0), **life_time**=-1.0f, **persistent_lines**=True)](#carla.DebugHelper.draw_point) <sub>_Method_</sub>
        - [**draw_line**(**self**, **begin**, **end**, **thickness**=0.1f, **color**=(255,0,0), **life_time**=-1.0f, **persistent_lines**=True)](#carla.DebugHelper.draw_line) <sub>_Method_</sub>
        - [**draw_arrow**(**self**, **begin**, **end**, **thickness**=0.1f, **arrow_size**=0.1f, **color**=(255,0,0), **life_time**=-1.0f, **persistent_lines**=True)](#carla.DebugHelper.draw_arrow) <sub>_Method_</sub>
        - [**draw_box**(**self**, **box**, **rotation**, **thickness**=0.1f, **color**=(255,0,0), **life_time**=-1.0f, **persistent_lines**=True)](#carla.DebugHelper.draw_box) <sub>_Method_</sub>
        - [**draw_string**(**self**, **location**, **text**, **draw_shadow**=False, **color**=(255,0,0), **life_time**=-1.0f, **persistent_lines**=True)](#carla.DebugHelper.draw_string) <sub>_Method_</sub>
    - [**LaneChange**](#carla.LaneChange) <sub>_Class_</sub>  
        - [**NONE**](#carla.LaneChange.NONE) <sub>_Instance variable_</sub>
        - [**Right**](#carla.LaneChange.Right) <sub>_Instance variable_</sub>
        - [**Left**](#carla.LaneChange.Left) <sub>_Instance variable_</sub>
        - [**Both**](#carla.LaneChange.Both) <sub>_Instance variable_</sub>
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
    - [**SensorData**](#carla.SensorData) <sub>_Class_</sub>  
        - [**frame**](#carla.SensorData.frame) <sub>_Instance variable_</sub>
        - [**timestamp**](#carla.SensorData.timestamp) <sub>_Instance variable_</sub>
        - [**transform**](#carla.SensorData.transform) <sub>_Instance variable_</sub>
    - [**TrafficLightState**](#carla.TrafficLightState) <sub>_Class_</sub>  
        - [**Red**](#carla.TrafficLightState.Red) <sub>_Instance variable_</sub>
        - [**Yellow**](#carla.TrafficLightState.Yellow) <sub>_Instance variable_</sub>
        - [**Green**](#carla.TrafficLightState.Green) <sub>_Instance variable_</sub>
        - [**Off**](#carla.TrafficLightState.Off) <sub>_Instance variable_</sub>
        - [**Unknown**](#carla.TrafficLightState.Unknown) <sub>_Instance variable_</sub>
    - [**Actor**](#carla.Actor) <sub>_Class_</sub>  
        - [**id**](#carla.Actor.id) <sub>_Instance variable_</sub>
        - [**type_id**](#carla.Actor.type_id) <sub>_Instance variable_</sub>
        - [**parent**](#carla.Actor.parent) <sub>_Instance variable_</sub>
        - [**semantic_tags**](#carla.Actor.semantic_tags) <sub>_Instance variable_</sub>
        - [**is_alive**](#carla.Actor.is_alive) <sub>_Instance variable_</sub>
        - [**attributes**](#carla.Actor.attributes) <sub>_Instance variable_</sub>
        - [**destroy**(**self**)](#carla.Actor.destroy) <sub>_Method_</sub>
        - [**get_world**(**self**)](#carla.Actor.get_world) <sub>_Method_</sub>
        - [**get_location**(**self**)](#carla.Actor.get_location) <sub>_Method_</sub>
        - [**get_velocity**(**self**)](#carla.Actor.get_velocity) <sub>_Method_</sub>
        - [**get_angular_velocity**(**self**)](#carla.Actor.get_angular_velocity) <sub>_Method_</sub>
        - [**get_acceleration**(**self**)](#carla.Actor.get_acceleration) <sub>_Method_</sub>
        - [**set_location**(**self**, **location**)](#carla.Actor.set_location) <sub>_Method_</sub>
        - [**set_transform**(**self**, **transform**)](#carla.Actor.set_transform) <sub>_Method_</sub>
        - [**set_velocity**(**self**, **velocity**)](#carla.Actor.set_velocity) <sub>_Method_</sub>
        - [**set_angular_velocity**(**self**, **angular_velocity**)](#carla.Actor.set_angular_velocity) <sub>_Method_</sub>
        - [**add_impulse**(**self**, **impulse**)](#carla.Actor.add_impulse) <sub>_Method_</sub>
        - [**set_simulate_physics**(**self**, **enabled**=True)](#carla.Actor.set_simulate_physics) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Actor.__str__) <sub>_Method_</sub>
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
        - [**\__eq__**(**self**, **other**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.ActorAttribute.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.ActorAttribute.__ne__) <sub>_Method_</sub>
        - [**\__nonzero__**(**self**)](#carla.ActorAttribute.__nonzero__) <sub>_Method_</sub>
        - [**\__bool__**(**self**)](#carla.ActorAttribute.__bool__) <sub>_Method_</sub>
        - [**\__int__**(**self**)](#carla.ActorAttribute.__int__) <sub>_Method_</sub>
        - [**\__float__**(**self**)](#carla.ActorAttribute.__float__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.ActorAttribute.__str__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.ActorAttribute.__str__) <sub>_Method_</sub>
    - [**ActorBlueprint**](#carla.ActorBlueprint) <sub>_Class_</sub>  
        - [**id**](#carla.ActorBlueprint.id) <sub>_Instance variable_</sub>
        - [**tags**](#carla.ActorBlueprint.tags) <sub>_Instance variable_</sub>
        - [**has_tag**(**self**, **tag**)](#carla.ActorBlueprint.has_tag) <sub>_Method_</sub>
        - [**match_tags**(**self**, **wildcard_pattern**)](#carla.ActorBlueprint.match_tags) <sub>_Method_</sub>
        - [**has_attribute**(**self**, **id**)](#carla.ActorBlueprint.has_attribute) <sub>_Method_</sub>
        - [**get_attribute**(**self**, **id**)](#carla.ActorBlueprint.get_attribute) <sub>_Method_</sub>
        - [**set_attribute**(**self**, **id**, **value**)](#carla.ActorBlueprint.set_attribute) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.ActorBlueprint.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.ActorBlueprint.__iter__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.ActorBlueprint.__str__) <sub>_Method_</sub>
    - [**ActorSnapshot**](#carla.ActorSnapshot) <sub>_Class_</sub>  
        - [**id**](#carla.ActorSnapshot.id) <sub>_Instance variable_</sub>
        - [**get_transform**(**self**)](#carla.ActorSnapshot.get_transform) <sub>_Method_</sub>
        - [**get_velocity**(**self**)](#carla.ActorSnapshot.get_velocity) <sub>_Method_</sub>
        - [**get_angular_velocity**(**self**)](#carla.ActorSnapshot.get_angular_velocity) <sub>_Method_</sub>
        - [**get_acceleration**(**self**)](#carla.ActorSnapshot.get_acceleration) <sub>_Method_</sub>
        - [**\__self__**(**self**)](#carla.ActorSnapshot.__self__) <sub>_Method_</sub>
    - [**BoundingBox**](#carla.BoundingBox) <sub>_Class_</sub>  
        - [**location**](#carla.BoundingBox.location) <sub>_Instance variable_</sub>
        - [**extent**](#carla.BoundingBox.extent) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **location**, **extent**)](#carla.BoundingBox.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.BoundingBox.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.BoundingBox.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.BoundingBox.__str__) <sub>_Method_</sub>
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
        - [**show_recorder_file_info**(**self**, **filename**, **show_all**)](#carla.Client.show_recorder_file_info) <sub>_Method_</sub>
        - [**show_recorder_collisions**(**self**, **filename**, **category1**, **category2**)](#carla.Client.show_recorder_collisions) <sub>_Method_</sub>
        - [**show_recorder_actors_blocked**(**self**, **filename**, **min_time**, **min_distance**)](#carla.Client.show_recorder_actors_blocked) <sub>_Method_</sub>
        - [**replay_file**(**self**, **name**, **start**, **duration**, **follow_id**)](#carla.Client.replay_file) <sub>_Method_</sub>
        - [**set_replayer_time_factor**(**self**, **time_factor**)](#carla.Client.set_replayer_time_factor) <sub>_Method_</sub>
        - [**apply_batch**(**self**, **commands**)](#carla.Client.apply_batch) <sub>_Method_</sub>
        - [**apply_batch_sync**(**self**, **commands**)](#carla.Client.apply_batch_sync) <sub>_Method_</sub>
    - [**CollisionEvent**](#carla.CollisionEvent) <sub>_Class_</sub>  
        - [**actor**](#carla.CollisionEvent.actor) <sub>_Instance variable_</sub>
        - [**other_actor**](#carla.CollisionEvent.other_actor) <sub>_Instance variable_</sub>
        - [**normal_impulse**](#carla.CollisionEvent.normal_impulse) <sub>_Instance variable_</sub>
    - [**Color**](#carla.Color) <sub>_Class_</sub>  
        - [**r**](#carla.Color.r) <sub>_Instance variable_</sub>
        - [**g**](#carla.Color.g) <sub>_Instance variable_</sub>
        - [**b**](#carla.Color.b) <sub>_Instance variable_</sub>
        - [**a**](#carla.Color.a) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **r**=0, **g**=0, **b**=0, **a**=255)](#carla.Color.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Color.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Color.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Color.__str__) <sub>_Method_</sub>
    - [**GearPhysicsControl**](#carla.GearPhysicsControl) <sub>_Class_</sub>  
        - [**ratio**](#carla.GearPhysicsControl.ratio) <sub>_Instance variable_</sub>
        - [**down_ratio**](#carla.GearPhysicsControl.down_ratio) <sub>_Instance variable_</sub>
        - [**up_ratio**](#carla.GearPhysicsControl.up_ratio) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **ratio**=1.0, **down_ratio**=0.5, **up_ratio**=0.65)](#carla.GearPhysicsControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.GearPhysicsControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.GearPhysicsControl.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.GearPhysicsControl.__str__) <sub>_Method_</sub>
    - [**GeoLocation**](#carla.GeoLocation) <sub>_Class_</sub>  
        - [**latitude**](#carla.GeoLocation.latitude) <sub>_Instance variable_</sub>
        - [**longitude**](#carla.GeoLocation.longitude) <sub>_Instance variable_</sub>
        - [**altitude**](#carla.GeoLocation.altitude) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **latitude**=0.0, **longitude**=0.0, **altitude**=0.0)](#carla.GeoLocation.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.GeoLocation.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.GeoLocation.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.GeoLocation.__str__) <sub>_Method_</sub>
    - [**Map**](#carla.Map) <sub>_Class_</sub>  
        - [**name**](#carla.Map.name) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **name**, **xodr_content**)](#carla.Map.__init__) <sub>_Method_</sub>
        - [**get_spawn_points**(**self**)](#carla.Map.get_spawn_points) <sub>_Method_</sub>
        - [**get_waypoint**(**self**, **location**, **project_to_road**=True, **lane_type**=carla.LaneType.Driving)](#carla.Map.get_waypoint) <sub>_Method_</sub>
        - [**get_topology**(**self**)](#carla.Map.get_topology) <sub>_Method_</sub>
        - [**generate_waypoints**(**self**, **distance**)](#carla.Map.generate_waypoints) <sub>_Method_</sub>
        - [**transform_to_geolocation**(**self**, **location**)](#carla.Map.transform_to_geolocation) <sub>_Method_</sub>
        - [**to_opendrive**(**self**)](#carla.Map.to_opendrive) <sub>_Method_</sub>
        - [**save_to_disk**(**self**, **path**)](#carla.Map.save_to_disk) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Map.__str__) <sub>_Method_</sub>
    - [**Rotation**](#carla.Rotation) <sub>_Class_</sub>  
        - [**pitch**](#carla.Rotation.pitch) <sub>_Instance variable_</sub>
        - [**yaw**](#carla.Rotation.yaw) <sub>_Instance variable_</sub>
        - [**roll**](#carla.Rotation.roll) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **pitch**=0.0, **yaw**=0.0, **roll**=0.0)](#carla.Rotation.__init__) <sub>_Method_</sub>
        - [**get_forward_vector**(**self**)](#carla.Rotation.get_forward_vector) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Rotation.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Rotation.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Rotation.__str__) <sub>_Method_</sub>
    - [**Timestamp**](#carla.Timestamp) <sub>_Class_</sub>  
        - [**frame**](#carla.Timestamp.frame) <sub>_Instance variable_</sub>
        - [**elapsed_seconds**](#carla.Timestamp.elapsed_seconds) <sub>_Instance variable_</sub>
        - [**delta_seconds**](#carla.Timestamp.delta_seconds) <sub>_Instance variable_</sub>
        - [**platform_timestamp**](#carla.Timestamp.platform_timestamp) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **frame**, **elapsed_seconds**, **delta_seconds**, **platform_timestamp**)](#carla.Timestamp.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Timestamp.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Timestamp.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Timestamp.__str__) <sub>_Method_</sub>
    - [**TrafficSign**](#carla.TrafficSign) <sub>_Class_</sub>  
        - [**trigger_volume**](#carla.TrafficSign.trigger_volume) <sub>_Instance variable_</sub>
    - [**Transform**](#carla.Transform) <sub>_Class_</sub>  
        - [**location**](#carla.Transform.location) <sub>_Instance variable_</sub>
        - [**rotation**](#carla.Transform.rotation) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **location**, **rotation**)](#carla.Transform.__init__) <sub>_Method_</sub>
        - [**transform**(**self**, **in_point**)](#carla.Transform.transform) <sub>_Method_</sub>
        - [**get_forward_vector**(**self**)](#carla.Transform.get_forward_vector) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Transform.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Transform.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Transform.__str__) <sub>_Method_</sub>
    - [**Vector2D**](#carla.Vector2D) <sub>_Class_</sub>  
        - [**x**](#carla.Vector2D.x) <sub>_Instance variable_</sub>
        - [**y**](#carla.Vector2D.y) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **x**=0.0, **y**=0.0)](#carla.Vector2D.__init__) <sub>_Method_</sub>
        - [**\__add__**(**self**, **other**)](#carla.Vector2D.__add__) <sub>_Method_</sub>
        - [**\__mul__**(**self**, **other**)](#carla.Vector2D.__mul__) <sub>_Method_</sub>
        - [**\__sub__**(**self**, **other**)](#carla.Vector2D.__sub__) <sub>_Method_</sub>
        - [**\__truediv__**(**self**, **other**)](#carla.Vector2D.__truediv__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Vector2D.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Vector2D.__ne__) <sub>_Method_</sub>
        - [**\__self__**(**self**)](#carla.Vector2D.__self__) <sub>_Method_</sub>
    - [**Vector3D**](#carla.Vector3D) <sub>_Class_</sub>  
        - [**x**](#carla.Vector3D.x) <sub>_Instance variable_</sub>
        - [**y**](#carla.Vector3D.y) <sub>_Instance variable_</sub>
        - [**z**](#carla.Vector3D.z) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **x**=0.0, **y**=0.0, **z**=0.0)](#carla.Vector3D.__init__) <sub>_Method_</sub>
        - [**\__add__**(**self**, **other**)](#carla.Vector3D.__add__) <sub>_Method_</sub>
        - [**\__mul__**(**self**, **other**)](#carla.Vector3D.__mul__) <sub>_Method_</sub>
        - [**\__sub__**(**self**, **other**)](#carla.Vector3D.__sub__) <sub>_Method_</sub>
        - [**\__truediv__**(**self**, **other**)](#carla.Vector3D.__truediv__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Vector3D.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Vector3D.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Vector3D.__str__) <sub>_Method_</sub>
    - [**VehicleControl**](#carla.VehicleControl) <sub>_Class_</sub>  
        - [**throttle**](#carla.VehicleControl.throttle) <sub>_Instance variable_</sub>
        - [**steer**](#carla.VehicleControl.steer) <sub>_Instance variable_</sub>
        - [**brake**](#carla.VehicleControl.brake) <sub>_Instance variable_</sub>
        - [**hand_brake**](#carla.VehicleControl.hand_brake) <sub>_Instance variable_</sub>
        - [**reverse**](#carla.VehicleControl.reverse) <sub>_Instance variable_</sub>
        - [**manual_gear_shift**](#carla.VehicleControl.manual_gear_shift) <sub>_Instance variable_</sub>
        - [**gear**](#carla.VehicleControl.gear) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **throttle**=0.0, **steer**=0.0, **brake**=0.0, **hand_brake**=True, **reverse**=True, **manual_gear_shift**=True, **gear**=0)](#carla.VehicleControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.VehicleControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.VehicleControl.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.VehicleControl.__str__) <sub>_Method_</sub>
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
        - [**final_ratio**](#carla.VehiclePhysicsControl.final_ratio) <sub>_Instance variable_</sub>
        - [**forward_gears**](#carla.VehiclePhysicsControl.forward_gears) <sub>_Instance variable_</sub>
        - [**mass**](#carla.VehiclePhysicsControl.mass) <sub>_Instance variable_</sub>
        - [**drag_coefficient**](#carla.VehiclePhysicsControl.drag_coefficient) <sub>_Instance variable_</sub>
        - [**center_of_mass**](#carla.VehiclePhysicsControl.center_of_mass) <sub>_Instance variable_</sub>
        - [**steering_curve**](#carla.VehiclePhysicsControl.steering_curve) <sub>_Instance variable_</sub>
        - [**wheels**](#carla.VehiclePhysicsControl.wheels) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **torque_curve**=[[0.0, 500.0], [5000.0, 500.0]], **max_rpm**=5000.0, **moi**=1.0, **damping_rate_full_throttle**=0.15, **damping_rate_zero_throttle_clutch_engaged**=2.0, **damping_rate_zero_throttle_clutch_disengaged**=0.35, **use_gear_autobox**=True, **gear_switch_time**=0.5, **clutch_strength**=10.0, **mass**=1000.0, **drag_coefficient**=0.3, **center_of_mass**=[0.0, 0.0, 0.0], **steering_curve**=[0.0, 0.0, 0.0], **wheels**=list())](#carla.VehiclePhysicsControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.VehiclePhysicsControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.VehiclePhysicsControl.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.VehiclePhysicsControl.__str__) <sub>_Method_</sub>
    - [**WalkerAIController**](#carla.WalkerAIController) <sub>_Class_</sub>  
        - [**start**(**self**)](#carla.WalkerAIController.start) <sub>_Method_</sub>
        - [**stop**(**self**)](#carla.WalkerAIController.stop) <sub>_Method_</sub>
        - [**go_to_location**(**self**, **destination**)](#carla.WalkerAIController.go_to_location) <sub>_Method_</sub>
        - [**set_max_speed**(**self**, **speed**=1.4)](#carla.WalkerAIController.set_max_speed) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.WalkerAIController.__str__) <sub>_Method_</sub>
    - [**WalkerBoneControl**](#carla.WalkerBoneControl) <sub>_Class_</sub>  
        - [**bone_transforms**](#carla.WalkerBoneControl.bone_transforms) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **list(name,transform)**)](#carla.WalkerBoneControl.__init__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.WalkerBoneControl.__str__) <sub>_Method_</sub>
    - [**WalkerControl**](#carla.WalkerControl) <sub>_Class_</sub>  
        - [**direction**](#carla.WalkerControl.direction) <sub>_Instance variable_</sub>
        - [**speed**](#carla.WalkerControl.speed) <sub>_Instance variable_</sub>
        - [**jump**](#carla.WalkerControl.jump) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **direction**=[1.0, 0.0, 0.0], **speed**=0.0, **jump**=False)](#carla.WalkerControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.WalkerControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.WalkerControl.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.WalkerControl.__str__) <sub>_Method_</sub>
    - [**Waypoint**](#carla.Waypoint) <sub>_Class_</sub>  
        - [**id**](#carla.Waypoint.id) <sub>_Instance variable_</sub>
        - [**transform**](#carla.Waypoint.transform) <sub>_Instance variable_</sub>
        - [**is_intersection**](#carla.Waypoint.is_intersection) <sub>_Instance variable_</sub>
        - [**is_junction**](#carla.Waypoint.is_junction) <sub>_Instance variable_</sub>
        - [**lane_width**](#carla.Waypoint.lane_width) <sub>_Instance variable_</sub>
        - [**road_id**](#carla.Waypoint.road_id) <sub>_Instance variable_</sub>
        - [**section_id**](#carla.Waypoint.section_id) <sub>_Instance variable_</sub>
        - [**lane_id**](#carla.Waypoint.lane_id) <sub>_Instance variable_</sub>
        - [**s**](#carla.Waypoint.s) <sub>_Instance variable_</sub>
        - [**lane_change**](#carla.Waypoint.lane_change) <sub>_Instance variable_</sub>
        - [**lane_type**](#carla.Waypoint.lane_type) <sub>_Instance variable_</sub>
        - [**right_lane_marking**](#carla.Waypoint.right_lane_marking) <sub>_Instance variable_</sub>
        - [**left_lane_marking**](#carla.Waypoint.left_lane_marking) <sub>_Instance variable_</sub>
        - [**next**(**self**, **distance**)](#carla.Waypoint.next) <sub>_Method_</sub>
        - [**get_right_lane**(**self**)](#carla.Waypoint.get_right_lane) <sub>_Method_</sub>
        - [**get_left_lane**(**self**)](#carla.Waypoint.get_left_lane) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Waypoint.__str__) <sub>_Method_</sub>
    - [**WeatherParameters**](#carla.WeatherParameters) <sub>_Class_</sub>  
        - [**cloudiness**](#carla.WeatherParameters.cloudiness) <sub>_Instance variable_</sub>
        - [**precipitation**](#carla.WeatherParameters.precipitation) <sub>_Instance variable_</sub>
        - [**precipitation_deposits**](#carla.WeatherParameters.precipitation_deposits) <sub>_Instance variable_</sub>
        - [**wind_intensity**](#carla.WeatherParameters.wind_intensity) <sub>_Instance variable_</sub>
        - [**sun_azimuth_angle**](#carla.WeatherParameters.sun_azimuth_angle) <sub>_Instance variable_</sub>
        - [**sun_altitude_angle**](#carla.WeatherParameters.sun_altitude_angle) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **cloudiness**=0.0, **precipitation**=0.0, **precipitation_deposits**=0.0, **wind_intensity**=0.0, **sun_azimuth_angle**=0.0, **sun_altitude_angle**=0.0)](#carla.WeatherParameters.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.WeatherParameters.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.WeatherParameters.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.WeatherParameters.__str__) <sub>_Method_</sub>
    - [**WheelPhysicsControl**](#carla.WheelPhysicsControl) <sub>_Class_</sub>  
        - [**tire_friction**](#carla.WheelPhysicsControl.tire_friction) <sub>_Instance variable_</sub>
        - [**damping_rate**](#carla.WheelPhysicsControl.damping_rate) <sub>_Instance variable_</sub>
        - [**max_steer_angle**](#carla.WheelPhysicsControl.max_steer_angle) <sub>_Instance variable_</sub>
        - [**radius**](#carla.WheelPhysicsControl.radius) <sub>_Instance variable_</sub>
        - [**max_brake_torque**](#carla.WheelPhysicsControl.max_brake_torque) <sub>_Instance variable_</sub>
        - [**max_handbrake_torque**](#carla.WheelPhysicsControl.max_handbrake_torque) <sub>_Instance variable_</sub>
        - [**position**](#carla.WheelPhysicsControl.position) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **tire_friction**=2.0, **damping_rate**=0.25, **max_steer_angle**=70.0, **radius**=30.0, **position**=(0.0,0.0,0.0))](#carla.WheelPhysicsControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.WheelPhysicsControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.WheelPhysicsControl.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.WheelPhysicsControl.__str__) <sub>_Method_</sub>
    - [**World**](#carla.World) <sub>_Class_</sub>  
        - [**id**](#carla.World.id) <sub>_Instance variable_</sub>
        - [**debug**](#carla.World.debug) <sub>_Instance variable_</sub>
        - [**get_blueprint_library**(**self**)](#carla.World.get_blueprint_library) <sub>_Method_</sub>
        - [**get_map**(**self**)](#carla.World.get_map) <sub>_Method_</sub>
        - [**get_spectator**(**self**)](#carla.World.get_spectator) <sub>_Method_</sub>
        - [**get_settings**(**self**)](#carla.World.get_settings) <sub>_Method_</sub>
        - [**apply_settings**(**self**, **world_settings**)](#carla.World.apply_settings) <sub>_Method_</sub>
        - [**get_weather**(**self**)](#carla.World.get_weather) <sub>_Method_</sub>
        - [**set_weather**(**self**, **weather**)](#carla.World.set_weather) <sub>_Method_</sub>
        - [**get_snapshot**(**self**)](#carla.World.get_snapshot) <sub>_Method_</sub>
        - [**get_actor**(**self**, **actor_id**)](#carla.World.get_actor) <sub>_Method_</sub>
        - [**get_actors**(**self**)](#carla.World.get_actors) <sub>_Method_</sub>
        - [**spawn_actor**(**self**, **blueprint**, **transform**, **attach_to**=None, **attachment**=Rigid)](#carla.World.spawn_actor) <sub>_Method_</sub>
        - [**try_spawn_actor**(**self**, **blueprint**, **transform**, **attach_to**=None, **attachment**=Rigid)](#carla.World.try_spawn_actor) <sub>_Method_</sub>
        - [**wait_for_tick**(**self**, **seconds**=10.0)](#carla.World.wait_for_tick) <sub>_Method_</sub>
        - [**on_tick**(**self**, **callback**)](#carla.World.on_tick) <sub>_Method_</sub>
        - [**remove_on_tick**(**self**, **callback_id**)](#carla.World.remove_on_tick) <sub>_Method_</sub>
        - [**tick**(**self**)](#carla.World.tick) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.World.__str__) <sub>_Method_</sub>
    - [**WorldSettings**](#carla.WorldSettings) <sub>_Class_</sub>  
        - [**synchronous_mode**](#carla.WorldSettings.synchronous_mode) <sub>_Instance variable_</sub>
        - [**no_rendering_mode**](#carla.WorldSettings.no_rendering_mode) <sub>_Instance variable_</sub>
        - [**fixed_delta_seconds**](#carla.WorldSettings.fixed_delta_seconds) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **synchronous_mode**=False, **no_rendering_mode**=False, **fixed_delta_seconds**=0.0)](#carla.WorldSettings.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.WorldSettings.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.WorldSettings.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.WorldSettings.__str__) <sub>_Method_</sub>
    - [**WorldSnapshot**](#carla.WorldSnapshot) <sub>_Class_</sub>  
        - [**id**](#carla.WorldSnapshot.id) <sub>_Instance variable_</sub>
        - [**frame**](#carla.WorldSnapshot.frame) <sub>_Instance variable_</sub>
        - [**timestamp**](#carla.WorldSnapshot.timestamp) <sub>_Instance variable_</sub>
        - [**has_actor**(**self**, **actor_id**)](#carla.WorldSnapshot.has_actor) <sub>_Method_</sub>
        - [**find**(**self**, **actor_id**)](#carla.WorldSnapshot.find) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.WorldSnapshot.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.WorldSnapshot.__iter__) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.WorldSnapshot.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.WorldSnapshot.__ne__) <sub>_Method_</sub>
        - [**\__self__**(**self**)](#carla.WorldSnapshot.__self__) <sub>_Method_</sub>
    - [**GnssEvent**](#carla.GnssEvent) <sub>_Class_</sub>  
        - [**latitude**](#carla.GnssEvent.latitude) <sub>_Instance variable_</sub>
        - [**longitude**](#carla.GnssEvent.longitude) <sub>_Instance variable_</sub>
        - [**altitude**](#carla.GnssEvent.altitude) <sub>_Instance variable_</sub>
        - [**\__str__**(**self**)](#carla.GnssEvent.__str__) <sub>_Method_</sub>
    - [**Image**](#carla.Image) <sub>_Class_</sub>  
        - [**width**](#carla.Image.width) <sub>_Instance variable_</sub>
        - [**height**](#carla.Image.height) <sub>_Instance variable_</sub>
        - [**fov**](#carla.Image.fov) <sub>_Instance variable_</sub>
        - [**raw_data**](#carla.Image.raw_data) <sub>_Instance variable_</sub>
        - [**convert**(**self**, **color_converter**)](#carla.Image.convert) <sub>_Method_</sub>
        - [**save_to_disk**(**self**, **path**, **color_converter**=Raw)](#carla.Image.save_to_disk) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.Image.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.Image.__iter__) <sub>_Method_</sub>
        - [**\__getitem__**(**self**, **pos**)](#carla.Image.__getitem__) <sub>_Method_</sub>
        - [**\__setitem__**(**self**, **pos**, **color**)](#carla.Image.__setitem__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Image.__str__) <sub>_Method_</sub>
    - [**LaneInvasionEvent**](#carla.LaneInvasionEvent) <sub>_Class_</sub>  
        - [**actor**](#carla.LaneInvasionEvent.actor) <sub>_Instance variable_</sub>
        - [**crossed_lane_markings**](#carla.LaneInvasionEvent.crossed_lane_markings) <sub>_Instance variable_</sub>
        - [**\__str__**(**self**)](#carla.LaneInvasionEvent.__str__) <sub>_Method_</sub>
    - [**LidarMeasurement**](#carla.LidarMeasurement) <sub>_Class_</sub>  
        - [**horizontal_angle**](#carla.LidarMeasurement.horizontal_angle) <sub>_Instance variable_</sub>
        - [**channels**](#carla.LidarMeasurement.channels) <sub>_Instance variable_</sub>
        - [**raw_data**](#carla.LidarMeasurement.raw_data) <sub>_Instance variable_</sub>
        - [**get_point_count**(**self**, **channel**)](#carla.LidarMeasurement.get_point_count) <sub>_Method_</sub>
        - [**save_to_disk**(**self**, **path**)](#carla.LidarMeasurement.save_to_disk) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.LidarMeasurement.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.LidarMeasurement.__iter__) <sub>_Method_</sub>
        - [**\__getitem__**(**self**, **pos**)](#carla.LidarMeasurement.__getitem__) <sub>_Method_</sub>
        - [**\__setitem__**(**self**, **pos**, **location**)](#carla.LidarMeasurement.__setitem__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.LidarMeasurement.__str__) <sub>_Method_</sub>
    - [**Location**](#carla.Location) <sub>_Class_</sub>  
        - [**x**](#carla.Location.x) <sub>_Instance variable_</sub>
        - [**y**](#carla.Location.y) <sub>_Instance variable_</sub>
        - [**z**](#carla.Location.z) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **x**=0.0, **y**=0.0, **z**=0.0)](#carla.Location.__init__) <sub>_Method_</sub>
        - [**distance**(**self**, **location**)](#carla.Location.distance) <sub>_Method_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Location.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Location.__ne__) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Location.__str__) <sub>_Method_</sub>
    - [**ObstacleDetectionEvent**](#carla.ObstacleDetectionEvent) <sub>_Class_</sub>  
        - [**actor**](#carla.ObstacleDetectionEvent.actor) <sub>_Instance variable_</sub>
        - [**other_actor**](#carla.ObstacleDetectionEvent.other_actor) <sub>_Instance variable_</sub>
        - [**distance**](#carla.ObstacleDetectionEvent.distance) <sub>_Instance variable_</sub>
        - [**\__str__**(**self**)](#carla.ObstacleDetectionEvent.__str__) <sub>_Method_</sub>
    - [**Sensor**](#carla.Sensor) <sub>_Class_</sub>  
        - [**is_listening**](#carla.Sensor.is_listening) <sub>_Instance variable_</sub>
        - [**listen**(**self**, **callback**)](#carla.Sensor.listen) <sub>_Method_</sub>
        - [**stop**(**self**)](#carla.Sensor.stop) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Sensor.__str__) <sub>_Method_</sub>
    - [**TrafficLight**](#carla.TrafficLight) <sub>_Class_</sub>  
        - [**state**](#carla.TrafficLight.state) <sub>_Instance variable_</sub>
        - [**set_state**(**self**, **state**)](#carla.TrafficLight.set_state) <sub>_Method_</sub>
        - [**get_state**(**self**)](#carla.TrafficLight.get_state) <sub>_Method_</sub>
        - [**set_green_time**(**self**, **green_time**)](#carla.TrafficLight.set_green_time) <sub>_Method_</sub>
        - [**get_green_time**(**self**)](#carla.TrafficLight.get_green_time) <sub>_Method_</sub>
        - [**set_yellow_time**(**self**, **yellow_time**)](#carla.TrafficLight.set_yellow_time) <sub>_Method_</sub>
        - [**get_yellow_time**(**self**)](#carla.TrafficLight.get_yellow_time) <sub>_Method_</sub>
        - [**set_red_time**(**self**, **red_time**)](#carla.TrafficLight.set_red_time) <sub>_Method_</sub>
        - [**get_red_time**(**self**)](#carla.TrafficLight.get_red_time) <sub>_Method_</sub>
        - [**get_elapsed_time**(**self**)](#carla.TrafficLight.get_elapsed_time) <sub>_Method_</sub>
        - [**freeze**(**self**, **freeze**)](#carla.TrafficLight.freeze) <sub>_Method_</sub>
        - [**is_frozen**(**self**)](#carla.TrafficLight.is_frozen) <sub>_Method_</sub>
        - [**get_pole_index**(**self**)](#carla.TrafficLight.get_pole_index) <sub>_Method_</sub>
        - [**get_group_traffic_lights**(**self**)](#carla.TrafficLight.get_group_traffic_lights) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.TrafficLight.__str__) <sub>_Method_</sub>
    - [**Vehicle**](#carla.Vehicle) <sub>_Class_</sub>  
        - [**bounding_box**](#carla.Vehicle.bounding_box) <sub>_Instance variable_</sub>
        - [**apply_control**(**self**, **control**)](#carla.Vehicle.apply_control) <sub>_Method_</sub>
        - [**get_control**(**self**)](#carla.Vehicle.get_control) <sub>_Method_</sub>
        - [**apply_physics_control**(**self**, **physics_control**)](#carla.Vehicle.apply_physics_control) <sub>_Method_</sub>
        - [**get_physics_control**(**self**)](#carla.Vehicle.get_physics_control) <sub>_Method_</sub>
        - [**set_autopilot**(**self**, **enabled**=True)](#carla.Vehicle.set_autopilot) <sub>_Method_</sub>
        - [**get_speed_limit**(**self**)](#carla.Vehicle.get_speed_limit) <sub>_Method_</sub>
        - [**get_traffic_light_state**(**self**)](#carla.Vehicle.get_traffic_light_state) <sub>_Method_</sub>
        - [**is_at_traffic_light**(**self**)](#carla.Vehicle.is_at_traffic_light) <sub>_Method_</sub>
        - [**get_traffic_light**(**self**)](#carla.Vehicle.get_traffic_light) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Vehicle.__str__) <sub>_Method_</sub>
    - [**Walker**](#carla.Walker) <sub>_Class_</sub>  
        - [**bounding_box**](#carla.Walker.bounding_box) <sub>_Instance variable_</sub>
        - [**apply_control**(**self**, **control**)](#carla.Walker.apply_control) <sub>_Method_</sub>
        - [**apply_control**(**self**, **control**)](#carla.Walker.apply_control) <sub>_Method_</sub>
        - [**get_control**(**self**)](#carla.Walker.get_control) <sub>_Method_</sub>
        - [**\__str__**(**self**)](#carla.Walker.__str__) <sub>_Method_</sub>
- [**command**](#command) <sub>_Module_</sub>  
    - [**ApplyAngularVelocity**](#command.ApplyAngularVelocity) <sub>_Class_</sub>  
        - [**actor_id**](#command.ApplyAngularVelocity.actor_id) <sub>_Instance variable_</sub>
        - [**angular_velocity**](#command.ApplyAngularVelocity.angular_velocity) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **angular_velocity**)](#command.ApplyAngularVelocity.__init__) <sub>_Method_</sub>
    - [**ApplyImpulse**](#command.ApplyImpulse) <sub>_Class_</sub>  
        - [**actor_id**](#command.ApplyImpulse.actor_id) <sub>_Instance variable_</sub>
        - [**impulse**](#command.ApplyImpulse.impulse) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **impulse**)](#command.ApplyImpulse.__init__) <sub>_Method_</sub>
    - [**ApplyTransform**](#command.ApplyTransform) <sub>_Class_</sub>  
        - [**actor_id**](#command.ApplyTransform.actor_id) <sub>_Instance variable_</sub>
        - [**transform**](#command.ApplyTransform.transform) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **transform**)](#command.ApplyTransform.__init__) <sub>_Method_</sub>
    - [**ApplyVehicleControl**](#command.ApplyVehicleControl) <sub>_Class_</sub>  
        - [**actor_id**](#command.ApplyVehicleControl.actor_id) <sub>_Instance variable_</sub>
        - [**control**](#command.ApplyVehicleControl.control) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **control**)](#command.ApplyVehicleControl.__init__) <sub>_Method_</sub>
    - [**ApplyVelocity**](#command.ApplyVelocity) <sub>_Class_</sub>  
        - [**actor_id**](#command.ApplyVelocity.actor_id) <sub>_Instance variable_</sub>
        - [**velocity**](#command.ApplyVelocity.velocity) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **velocity**)](#command.ApplyVelocity.__init__) <sub>_Method_</sub>
    - [**ApplyWalkerControl**](#command.ApplyWalkerControl) <sub>_Class_</sub>  
        - [**actor_id**](#command.ApplyWalkerControl.actor_id) <sub>_Instance variable_</sub>
        - [**control**](#command.ApplyWalkerControl.control) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **control**)](#command.ApplyWalkerControl.__init__) <sub>_Method_</sub>
    - [**ApplyWalkerState**](#command.ApplyWalkerState) <sub>_Class_</sub>  
        - [**actor_id**](#command.ApplyWalkerState.actor_id) <sub>_Instance variable_</sub>
        - [**transform**](#command.ApplyWalkerState.transform) <sub>_Instance variable_</sub>
        - [**speed**](#command.ApplyWalkerState.speed) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **transform**, **speed**)](#command.ApplyWalkerState.__init__) <sub>_Method_</sub>
    - [**DestroyActor**](#command.DestroyActor) <sub>_Class_</sub>  
        - [**actor_id**](#command.DestroyActor.actor_id) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**)](#command.DestroyActor.__init__) <sub>_Method_</sub>
    - [**Response**](#command.Response) <sub>_Class_</sub>  
        - [**actor_id**](#command.Response.actor_id) <sub>_Instance variable_</sub>
        - [**error**](#command.Response.error) <sub>_Instance variable_</sub>
        - [**has_error**(**self**)](#command.Response.has_error) <sub>_Method_</sub>
    - [**SetAutopilot**](#command.SetAutopilot) <sub>_Class_</sub>  
        - [**actor_id**](#command.SetAutopilot.actor_id) <sub>_Instance variable_</sub>
        - [**enabled**](#command.SetAutopilot.enabled) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **enabled**)](#command.SetAutopilot.__init__) <sub>_Method_</sub>
    - [**SetSimulatePhysics**](#command.SetSimulatePhysics) <sub>_Class_</sub>  
        - [**actor_id**](#command.SetSimulatePhysics.actor_id) <sub>_Instance variable_</sub>
        - [**enabled**](#command.SetSimulatePhysics.enabled) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**, **actor**, **enabled**)](#command.SetSimulatePhysics.__init__) <sub>_Method_</sub>
    - [**SpawnActor**](#command.SpawnActor) <sub>_Class_</sub>  
        - [**transform**](#command.SpawnActor.transform) <sub>_Instance variable_</sub>
        - [**parent_id**](#command.SpawnActor.parent_id) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#command.SpawnActor.__init__) <sub>_Method_</sub>
        - [**\__init__**(**self**, **blueprint**, **transform**)](#command.SpawnActor.__init__) <sub>_Method_</sub>
        - [**\__init__**(**self**, **blueprint**, **transform**, **parent**)](#command.SpawnActor.__init__) <sub>_Method_</sub>
        - [**then**(**self**, **command**)](#command.SpawnActor.then) <sub>_Method_</sub>

## carla.ActorAttributeType<a name="carla.ActorAttributeType"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.ActorAttributeType.Bool"></a>**<font color="#f8805a">Bool</font>**  
- <a name="carla.ActorAttributeType.Int"></a>**<font color="#f8805a">Int</font>**  
- <a name="carla.ActorAttributeType.Float"></a>**<font color="#f8805a">Float</font>**  
- <a name="carla.ActorAttributeType.String"></a>**<font color="#f8805a">String</font>**  
- <a name="carla.ActorAttributeType.RGBColor"></a>**<font color="#f8805a">RGBColor</font>**  

---

## carla.ActorList<a name="carla.ActorList"></a> <sub><sup>_class_</sup></sub>
Class that provides access to actors.  

<h3>Methods</h3>
- <a name="carla.ActorList.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
    - **Parameters:**
        - `actor_id` (_int_)  
- <a name="carla.ActorList.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
- <a name="carla.ActorList.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
- <a name="carla.ActorList.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.AttachmentType<a name="carla.AttachmentType"></a> <sub><sup>_class_</sup></sub>
Class that defines the attachment options. See  [`world.spawn_actor`](#[carla.World.spawn_actor](#carla.World.spawn_actor)).  

<h3>Instance Variables</h3>
- <a name="carla.AttachmentType.Rigid"></a>**<font color="#f8805a">Rigid</font>**  
- <a name="carla.AttachmentType.SpringArm"></a>**<font color="#f8805a">SpringArm</font>**  

---

## carla.BlueprintLibrary<a name="carla.BlueprintLibrary"></a> <sub><sup>_class_</sup></sub>
Class that provides access to blueprints.  

<h3>Methods</h3>
- <a name="carla.BlueprintLibrary.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Filters a list of ActorBlueprint with id or tags matching wildcard_pattern. The pattern is matched against each blueprint's id and tags.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Return:** _[carla.BlueprintLibrary](#carla.BlueprintLibrary)_  
    - **Note:** <font color="#8E8E8E">_wildcard_pattern follows Unix shell-style wildcards (fnmatch).
_</font>  
- <a name="carla.BlueprintLibrary.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Return:** _[carla.ActorBlueprint](#carla.ActorBlueprint)_  
- <a name="carla.BlueprintLibrary.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ColorConverter<a name="carla.ColorConverter"></a> <sub><sup>_class_</sup></sub>
Class that defines the color converter options.  

<h3>Instance Variables</h3>
- <a name="carla.ColorConverter.Raw"></a>**<font color="#f8805a">Raw</font>**  
- <a name="carla.ColorConverter.Depth"></a>**<font color="#f8805a">Depth</font>**  
- <a name="carla.ColorConverter.LogarithmicDepth"></a>**<font color="#f8805a">LogarithmicDepth</font>**  
- <a name="carla.ColorConverter.CityScapesPalette"></a>**<font color="#f8805a">CityScapesPalette</font>**  

---

## carla.DebugHelper<a name="carla.DebugHelper"></a> <sub><sup>_class_</sup></sub>
Class that provides drawing debug shapes.  

<h3>Methods</h3>
- <a name="carla.DebugHelper.draw_point"></a>**<font color="#7fb800">draw_point</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**size**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `size` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time=0` instead_.  
- <a name="carla.DebugHelper.draw_line"></a>**<font color="#7fb800">draw_line</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**begin**</font>, <font color="#00a6ed">**end**</font>, <font color="#00a6ed">**thickness**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
    - **Parameters:**
        - `begin` (_[carla.Location](#carla.Location)_)  
        - `end` (_[carla.Location](#carla.Location)_)  
        - `thickness` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time=0` instead_.  
- <a name="carla.DebugHelper.draw_arrow"></a>**<font color="#7fb800">draw_arrow</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**begin**</font>, <font color="#00a6ed">**end**</font>, <font color="#00a6ed">**thickness**=0.1f</font>, <font color="#00a6ed">**arrow_size**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
    - **Parameters:**
        - `begin` (_[carla.Location](#carla.Location)_)  
        - `end` (_[carla.Location](#carla.Location)_)  
        - `thickness` (_float_)  
        - `arrow_size` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time=0` instead_.  
- <a name="carla.DebugHelper.draw_box"></a>**<font color="#7fb800">draw_box</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**box**</font>, <font color="#00a6ed">**rotation**</font>, <font color="#00a6ed">**thickness**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
    - **Parameters:**
        - `box` (_[carla.BoundingBox](#carla.BoundingBox)_)  
        - `rotation` (_[carla.Rotation](#carla.Rotation)_)  
        - `thickness` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time=0` instead_.  
- <a name="carla.DebugHelper.draw_string"></a>**<font color="#7fb800">draw_string</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**text**</font>, <font color="#00a6ed">**draw_shadow**=False</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `text` (_str_)  
        - `draw_shadow` (_bool_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, set a high `life_time` instead_.  

---

## carla.LaneChange<a name="carla.LaneChange"></a> <sub><sup>_class_</sup></sub>
Class that defines the lane change options.  

<h3>Instance Variables</h3>
- <a name="carla.LaneChange.NONE"></a>**<font color="#f8805a">NONE</font>**  
Traffic rules do not allow turning right or left, only going straight.  
- <a name="carla.LaneChange.Right"></a>**<font color="#f8805a">Right</font>**  
Traffic rules allow turning right.  
- <a name="carla.LaneChange.Left"></a>**<font color="#f8805a">Left</font>**  
Traffic rules allow turning left.  
- <a name="carla.LaneChange.Both"></a>**<font color="#f8805a">Both</font>**  
Traffic rules allow turning right or left.  

---

## carla.LaneMarking<a name="carla.LaneMarking"></a> <sub><sup>_class_</sup></sub>
Struct that defines a lane marking.  

<h3>Instance Variables</h3>
- <a name="carla.LaneMarking.type"></a>**<font color="#f8805a">type</font>** (_[carla.LaneMarkingType](#carla.LaneMarkingType)_)  
Lane marking type.  
- <a name="carla.LaneMarking.color"></a>**<font color="#f8805a">color</font>** (_[carla.Color](#carla.Color)_)  
Actual color of the marking.  
- <a name="carla.LaneMarking.lane_change"></a>**<font color="#f8805a">lane_change</font>** (_[carla.LaneChange](#carla.LaneChange)_)  
Lane change availability.  
- <a name="carla.LaneMarking.width"></a>**<font color="#f8805a">width</font>** (_float_)  
Horizontal lane marking thickness.  

---

## carla.LaneMarkingColor<a name="carla.LaneMarkingColor"></a> <sub><sup>_class_</sup></sub>
Class that defines the lane marking colors.  

<h3>Instance Variables</h3>
- <a name="carla.LaneMarkingColor.Standard"></a>**<font color="#f8805a">Standard</font>**  
White by default.  
- <a name="carla.LaneMarkingColor.Blue"></a>**<font color="#f8805a">Blue</font>**  
- <a name="carla.LaneMarkingColor.Green"></a>**<font color="#f8805a">Green</font>**  
- <a name="carla.LaneMarkingColor.Red"></a>**<font color="#f8805a">Red</font>**  
- <a name="carla.LaneMarkingColor.White"></a>**<font color="#f8805a">White</font>**  
- <a name="carla.LaneMarkingColor.Yellow"></a>**<font color="#f8805a">Yellow</font>**  
- <a name="carla.LaneMarkingColor.Other"></a>**<font color="#f8805a">Other</font>**  

---

## carla.LaneMarkingType<a name="carla.LaneMarkingType"></a> <sub><sup>_class_</sup></sub>
Class that defines the lane marking types that OpenDRIVE accepts.  

<h3>Instance Variables</h3>
- <a name="carla.LaneMarkingType.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneMarkingType.Other"></a>**<font color="#f8805a">Other</font>**  
- <a name="carla.LaneMarkingType.Broken"></a>**<font color="#f8805a">Broken</font>**  
- <a name="carla.LaneMarkingType.Solid"></a>**<font color="#f8805a">Solid</font>**  
- <a name="carla.LaneMarkingType.SolidSolid"></a>**<font color="#f8805a">SolidSolid</font>**  
For double solid line.  
- <a name="carla.LaneMarkingType.SolidBroken"></a>**<font color="#f8805a">SolidBroken</font>**  
From inside to outside except for center lane which is from left to right.  
- <a name="carla.LaneMarkingType.BrokenSolid"></a>**<font color="#f8805a">BrokenSolid</font>**  
From inside to outside except for center lane which is from left to right.  
- <a name="carla.LaneMarkingType.BrokenBroken"></a>**<font color="#f8805a">BrokenBroken</font>**  
From inside to outside except for center lane which is from left to right.  
- <a name="carla.LaneMarkingType.BottsDots"></a>**<font color="#f8805a">BottsDots</font>**  
- <a name="carla.LaneMarkingType.Grass"></a>**<font color="#f8805a">Grass</font>**  
Grass edge.  
- <a name="carla.LaneMarkingType.Curb"></a>**<font color="#f8805a">Curb</font>**  

---

## carla.LaneType<a name="carla.LaneType"></a> <sub><sup>_class_</sup></sub>
All the possible lane types that OpenDRIVE accepts.  

<h3>Instance Variables</h3>
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

## carla.SensorData<a name="carla.SensorData"></a> <sub><sup>_class_</sup></sub>
Base class for all the objects containing data generated by a sensor.  

<h3>Instance Variables</h3>
- <a name="carla.SensorData.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
Frame count when the data was generated.  
- <a name="carla.SensorData.timestamp"></a>**<font color="#f8805a">timestamp</font>** (_float_)  
Simulation-time when the data was generated.  
- <a name="carla.SensorData.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Sensor's transform when the data was generated.  

---

## carla.TrafficLightState<a name="carla.TrafficLightState"></a> <sub><sup>_class_</sup></sub>
All possible states for traffic lights.  

<h3>Instance Variables</h3>
- <a name="carla.TrafficLightState.Red"></a>**<font color="#f8805a">Red</font>**  
- <a name="carla.TrafficLightState.Yellow"></a>**<font color="#f8805a">Yellow</font>**  
- <a name="carla.TrafficLightState.Green"></a>**<font color="#f8805a">Green</font>**  
- <a name="carla.TrafficLightState.Off"></a>**<font color="#f8805a">Off</font>**  
- <a name="carla.TrafficLightState.Unknown"></a>**<font color="#f8805a">Unknown</font>**  

---

## carla.Actor<a name="carla.Actor"></a> <sub><sup>_class_</sup></sub>
Base class for all actors.
Actor is anything that plays a role in the simulation and can be moved around, examples of actors are vehicles, pedestrians, and sensors.  

<h3>Instance Variables</h3>
- <a name="carla.Actor.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Unique id identifying this actor. Note ids are unique during a given episode.  
- <a name="carla.Actor.type_id"></a>**<font color="#f8805a">type_id</font>** (_str_)  
Id of the blueprint that created this actor, e.g. "vehicle.ford.mustang".  
- <a name="carla.Actor.parent"></a>**<font color="#f8805a">parent</font>** (_[carla.Actor](#carla.Actor)_)  
Parent actor of this instance, None if this instance is not attached to another actor.  
- <a name="carla.Actor.semantic_tags"></a>**<font color="#f8805a">semantic_tags</font>** (_list(int)_)  
List of semantic tags of all components of this actor, see semantic segmentation sensor for the list of available tags. E.g., a traffic light actor could contain "pole" and "traffic light" tags.  
- <a name="carla.Actor.is_alive"></a>**<font color="#f8805a">is_alive</font>** (_bool_)  
Return whether this object was destroyed using this actor handle.  
- <a name="carla.Actor.attributes"></a>**<font color="#f8805a">attributes</font>** (_dict_)  
Dictionary of attributes of the blueprint that created this actor.  

<h3>Methods</h3>
- <a name="carla.Actor.destroy"></a>**<font color="#7fb800">destroy</font>**(<font color="#00a6ed">**self**</font>)  
Tell the simulator to destroy this Actor, and return whether the actor was successfully destroyed. It has no effect if the Actor was already successfully destroyed.  
    - **Return:** _bool_  
    - **Warning:** <font color="#ED2F2F">_This function blocks until the destruction operation is completed by the simulator.
_</font>  
- <a name="carla.Actor.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
Return the world this actor belongs to.  
    - **Return:** _[carla.World](#carla.World)_  
- <a name="carla.Actor.get_location"></a>**<font color="#7fb800">get_location</font>**(<font color="#00a6ed">**self**</font>)  
Return the current location of the actor.  
    - **Return:** _[carla.Location](#carla.Location)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the location received in the last tick.
_</font>  
- <a name="carla.Actor.get_velocity"></a>**<font color="#7fb800">get_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Return the current 3D velocity of the actor.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the velocity received in the last tick.
_</font>  
- <a name="carla.Actor.get_angular_velocity"></a>**<font color="#7fb800">get_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Return the current 3D angular velocity of the actor.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the angular velocity received in the last tick.
_</font>  
- <a name="carla.Actor.get_acceleration"></a>**<font color="#7fb800">get_acceleration</font>**(<font color="#00a6ed">**self**</font>)  
Return the current 3D acceleration of the actor.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the acceleration received in the last tick.
_</font>  
- <a name="carla.Actor.set_location"></a>**<font color="#7fb800">set_location</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Teleport the actor to a given location.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
- <a name="carla.Actor.set_transform"></a>**<font color="#7fb800">set_transform</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**transform**</font>)  
Teleport the actor to a given transform.  
    - **Parameters:**
        - `transform` (_[carla.Transform](#carla.Transform)_)  
- <a name="carla.Actor.set_velocity"></a>**<font color="#7fb800">set_velocity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**velocity**</font>)  
Set the actor's velocity.  
    - **Parameters:**
        - `velocity` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Actor.set_angular_velocity"></a>**<font color="#7fb800">set_angular_velocity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**angular_velocity**</font>)  
Set the actor's angular velocity.  
    - **Parameters:**
        - `angular_velocity` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Actor.add_impulse"></a>**<font color="#7fb800">add_impulse</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**impulse**</font>)  
Add impulse to the actor.  
    - **Parameters:**
        - `impulse` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Actor.set_simulate_physics"></a>**<font color="#7fb800">set_simulate_physics</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=True</font>)  
Enable or disable physics simulation on this actor.  
    - **Parameters:**
        - `enabled` (_bool_)  
- <a name="carla.Actor.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _str_  

---

## carla.ActorAttribute<a name="carla.ActorAttribute"></a> <sub><sup>_class_</sup></sub>
Class that defines an attribute of a [carla.ActorBlueprint](#carla.ActorBlueprint).  

<h3>Instance Variables</h3>
- <a name="carla.ActorAttribute.id"></a>**<font color="#f8805a">id</font>** (_str_)  
- <a name="carla.ActorAttribute.type"></a>**<font color="#f8805a">type</font>** (_[carla.ActorAttributeType](#carla.ActorAttributeType)_)  
- <a name="carla.ActorAttribute.recommended_values"></a>**<font color="#f8805a">recommended_values</font>** (_list(str)_)  
- <a name="carla.ActorAttribute.is_modifiable"></a>**<font color="#f8805a">is_modifiable</font>** (_bool_)  

<h3>Methods</h3>
- <a name="carla.ActorAttribute.as_bool"></a>**<font color="#7fb800">as_bool</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_int"></a>**<font color="#7fb800">as_int</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_float"></a>**<font color="#7fb800">as_float</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_str"></a>**<font color="#7fb800">as_str</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_color"></a>**<font color="#7fb800">as_color</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_bool_)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_int_)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_float_)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_str_)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Color](#carla.Color)_)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.ActorAttribute](#carla.ActorAttribute)_)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_bool_)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_int_)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_float_)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_str_)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Color](#carla.Color)_)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.ActorAttribute](#carla.ActorAttribute)_)  
- <a name="carla.ActorAttribute.__nonzero__"></a>**<font color="#7fb800">\__nonzero__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__bool__"></a>**<font color="#7fb800">\__bool__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__int__"></a>**<font color="#7fb800">\__int__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__float__"></a>**<font color="#7fb800">\__float__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorBlueprint<a name="carla.ActorBlueprint"></a> <sub><sup>_class_</sup></sub>
Class that contains all the necessary information for spawning an Actor.  

<h3>Instance Variables</h3>
- <a name="carla.ActorBlueprint.id"></a>**<font color="#f8805a">id</font>** (_str_)  
- <a name="carla.ActorBlueprint.tags"></a>**<font color="#f8805a">tags</font>** (_list(str)_)  

<h3>Methods</h3>
- <a name="carla.ActorBlueprint.has_tag"></a>**<font color="#7fb800">has_tag</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**tag**</font>)  
    - **Parameters:**
        - `tag` (_str_)  
    - **Return:** _bool_  
- <a name="carla.ActorBlueprint.match_tags"></a>**<font color="#7fb800">match_tags</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Test if any of the flags or id matches wildcard_pattern.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Return:** _bool_  
    - **Note:** <font color="#8E8E8E">_Wildcard_pattern follows Unix shell-style wildcards.
_</font>  
- <a name="carla.ActorBlueprint.has_attribute"></a>**<font color="#7fb800">has_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
    - **Parameters:**
        - `id` (_str_)  
    - **Return:** _bool_  
- <a name="carla.ActorBlueprint.get_attribute"></a>**<font color="#7fb800">get_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
    - **Parameters:**
        - `id` (_str_)  
    - **Return:** _[carla.ActorAttribute](#carla.ActorAttribute)_  
- <a name="carla.ActorBlueprint.set_attribute"></a>**<font color="#7fb800">set_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>, <font color="#00a6ed">**value**</font>)  
    - **Parameters:**
        - `id` (_str_)  
        - `value` (_str_)  
- <a name="carla.ActorBlueprint.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorSnapshot<a name="carla.ActorSnapshot"></a> <sub><sup>_class_</sup></sub>
Class that provides access to the data of a [carla.Actor](#carla.Actor) in a [carla.WorldSnapshot](#carla.WorldSnapshot).  

<h3>Instance Variables</h3>
- <a name="carla.ActorSnapshot.id"></a>**<font color="#f8805a">id</font>** (_int_)  

<h3>Methods</h3>
- <a name="carla.ActorSnapshot.get_transform"></a>**<font color="#7fb800">get_transform</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _[carla.Transform](#carla.Transform)_  
- <a name="carla.ActorSnapshot.get_velocity"></a>**<font color="#7fb800">get_velocity</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.ActorSnapshot.get_angular_velocity"></a>**<font color="#7fb800">get_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.ActorSnapshot.get_acceleration"></a>**<font color="#7fb800">get_acceleration</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.ActorSnapshot.__self__"></a>**<font color="#7fb800">\__self__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.BoundingBox<a name="carla.BoundingBox"></a> <sub><sup>_class_</sup></sub>
Bounding box helper class.  

<h3>Instance Variables</h3>
- <a name="carla.BoundingBox.location"></a>**<font color="#f8805a">location</font>** (_[carla.Location](#carla.Location)_)  
The center of the bounding box relative to its parent actor.  
- <a name="carla.BoundingBox.extent"></a>**<font color="#f8805a">extent</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
It contains the vector from the center of the bounding box to one of the vertex of the box.  
So, if you want to know the _X bounding box size_, you can just do `extent.x * 2`.  

<h3>Methods</h3>
- <a name="carla.BoundingBox.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**extent**</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `extent` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.BoundingBox.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.BoundingBox](#carla.BoundingBox)_)  
- <a name="carla.BoundingBox.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.BoundingBox](#carla.BoundingBox)_)  
- <a name="carla.BoundingBox.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Client<a name="carla.Client"></a> <sub><sup>_class_</sup></sub>
Client used to connect to a Carla server.  

<h3>Methods</h3>
- <a name="carla.Client.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**host**</font>, <font color="#00a6ed">**port**</font>, <font color="#00a6ed">**worker_threads**=0</font>)  
Client constructor.  
    - **Parameters:**
        - `host` (_str_) – IP address where a CARLA Simulator instance is running.  
        - `port` (_int_) – TCP port where the CARLA Simulator instance is running.  
        - `worker_threads` (_int_) – Number of working threads used for background updates. If 0, use all available concurrency.  
- <a name="carla.Client.set_timeout"></a>**<font color="#7fb800">set_timeout</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**</font>)  
Set the timeout in seconds allowed to block when doing networking calls.  
    - **Parameters:**
        - `seconds` (_float_) – New timeout value in seconds.  
- <a name="carla.Client.get_client_version"></a>**<font color="#7fb800">get_client_version</font>**(<font color="#00a6ed">**self**</font>)  
Get the client version as a string.  
    - **Return:** _str_  
- <a name="carla.Client.get_server_version"></a>**<font color="#7fb800">get_server_version</font>**(<font color="#00a6ed">**self**</font>)  
Get the server version as a string.  
    - **Return:** _str_  
- <a name="carla.Client.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
Get the world currently active in the simulation.  
    - **Return:** _[carla.World](#carla.World)_  
- <a name="carla.Client.get_available_maps"></a>**<font color="#7fb800">get_available_maps</font>**(<font color="#00a6ed">**self**</font>)  
Get a list of strings of the maps available on server. The result can be something like:  
  '/Game/Carla/Maps/Town01'  
  '/Game/Carla/Maps/Town02'  
  '/Game/Carla/Maps/Town03'  
  '/Game/Carla/Maps/Town04'  
  '/Game/Carla/Maps/Town05'  
  '/Game/Carla/Maps/Town06'  
  '/Game/Carla/Maps/Town07'.  
    - **Return:** _list(str)_  
- <a name="carla.Client.reload_world"></a>**<font color="#7fb800">reload_world</font>**(<font color="#00a6ed">**self**</font>)  
Reload the current world, note that a new world is created with default settings using the same map. All actors present in the world will be destroyed.  
    - **Raises:** RuntimeError  
- <a name="carla.Client.load_world"></a>**<font color="#7fb800">load_world</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**map_name**</font>)  
Load a new world with default settings using `map_name` map. All actors present in the current world will be destroyed.  
    - **Parameters:**
        - `map_name` (_str_) – Name of the map to load, accepts both full paths and map names, e.g. '/Game/Carla/Maps/Town01' or 'Town01'.  
- <a name="carla.Client.start_recorder"></a>**<font color="#7fb800">start_recorder</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>)  
If we use a simple name like 'recording.log' then it will be saved at server folder 'CarlaUE4/Saved/recording.log'. If we use some folder in the name, then it will be considered to be an absolute path, like '/home/carla/recording.log'.  
    - **Parameters:**
        - `filename` (_str_) – Name of the file to write the recorded data.  
- <a name="carla.Client.stop_recorder"></a>**<font color="#7fb800">stop_recorder</font>**(<font color="#00a6ed">**self**</font>)  
Stops the recording in progress.  
- <a name="carla.Client.show_recorder_file_info"></a>**<font color="#7fb800">show_recorder_file_info</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**show_all**</font>)  
Will show info about the recorded file (frames, times, events, state, positions...) We have the option to show all the details per frame, that includes all the traffic light states, position of all actors, and animations data.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `show_all` (_bool_) – Show all detailed info, or just a summary.  
- <a name="carla.Client.show_recorder_collisions"></a>**<font color="#7fb800">show_recorder_collisions</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**category1**</font>, <font color="#00a6ed">**category2**</font>)  
This will show which collisions were recorded in the file. We can use a filter for the collisions we want, using two categories. The categories can be:  
  'h' = Hero  
  'v' = Vehicle  
  'w' = Walker  
  't' = Traffic light  
  'o' = Other  
  'a' = Any  
So, if you want to see only collisions about a vehicle and a walker, we would use for category1 'v' and category2 'w'. Or if you want all the collisions (filter off) you can use 'a' as both categories.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `category1` (_single char_) – Character specifying the category of the first actor.  
        - `category2` (_single char_) – Character specifying the category of the second actor.  
- <a name="carla.Client.show_recorder_actors_blocked"></a>**<font color="#7fb800">show_recorder_actors_blocked</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**min_time**</font>, <font color="#00a6ed">**min_distance**</font>)  
Shows which actors seem blocked by some reason. The idea is to calculate which actors are not moving as much as 'min_distance' for a period of 'min_time'. By default min_time = 60 seconds (1 min) and min_distance = 100 centimeters (1 m).  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `min_time` (_float_) – How many seconds has to be stoped an actor to be considered as blocked.  
        - `min_distance` (_float_) – How many centimeters needs to move the actor in order to be considered as moving, and not blocked.  
- <a name="carla.Client.replay_file"></a>**<font color="#7fb800">replay_file</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>, <font color="#00a6ed">**start**</font>, <font color="#00a6ed">**duration**</font>, <font color="#00a6ed">**follow_id**</font>)  
Playback a file.  
    - **Parameters:**
        - `name` (_str_) – Name of the file.  
        - `start` (_float_) – Time in seconds where to start the playback. If it is negative, then it starts from the end.  
        - `duration` (_float_) – Id of the actor to follow. If this is 0 then camera is disabled.  
        - `follow_id` (_int_)  
- <a name="carla.Client.set_replayer_time_factor"></a>**<font color="#7fb800">set_replayer_time_factor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**time_factor**</font>)  
Apply a different playback speed to current playback. Can be used several times while a playback is in curse.  
    - **Parameters:**
        - `time_factor` (_float_) – A value of 1.0 means normal time factor. A value < 1.0 means slow motion (for example 0.5 is half speed) A value > 1.0 means fast motion (for example 2.0 is double speed).  
- <a name="carla.Client.apply_batch"></a>**<font color="#7fb800">apply_batch</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**commands**</font>)  
This function executes the whole list of commands on a single simulation step. For example, to set autopilot on some actors, we could use:   [sample_code](https://github.com/carla-simulator/carla/blob/10c5f6a482a21abfd00220c68c7f12b4110b7f63/PythonAPI/examples/spawn_npc.py#L126)   We don't have control about the response of each command. If we need that, we can use apply_batch_sync().  
    - **Parameters:**
        - `commands` (_list_) – A list of commands to execute in batch. Each command has a different number of parameters. Currently, we can use these [commands](#command.ApplyAngularVelocity):  
  SpawnActor  
  DestroyActor  
  ApplyVehicleControl  
  ApplyWalkerControl  
  ApplyTransform  
  ApplyVelocity  
  AplyAngularVelocity  
  ApplyImpulse  
  SetSimulatePhysics  
  SetAutopilot.  
- <a name="carla.Client.apply_batch_sync"></a>**<font color="#7fb800">apply_batch_sync</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**commands**</font>)  
This function executes the whole list of commands on a single simulation step, blocks until the commands are executed, and returns a list of [`command.Response`](#command.Response) that can be used to determine whether a single command succeeded or not. [sample_code](https://github.com/carla-simulator/carla/blob/10c5f6a482a21abfd00220c68c7f12b4110b7f63/PythonAPI/examples/spawn_npc.py#L112-L116).  
    - **Parameters:**
        - `commands` (_list_) – A list of commands to execute in batch. For a list of commands available see function above apply_batch().  
    - **Return:** _list_  

---

## carla.CollisionEvent<a name="carla.CollisionEvent"></a><sub><sup>([carla.SensorData](#carla.SensorData))</sup></sub> <sub><sup>_class_</sup></sub>
Class that defines a registered collision.  

<h3>Instance Variables</h3>
- <a name="carla.CollisionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get "self" actor. Actor that measured the collision.  
- <a name="carla.CollisionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get the actor to which we collided.  
- <a name="carla.CollisionEvent.normal_impulse"></a>**<font color="#f8805a">normal_impulse</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Normal impulse result of the collision.  

---

## carla.Color<a name="carla.Color"></a> <sub><sup>_class_</sup></sub>
Class that defines a 32-bit BGRA color.  

<h3>Instance Variables</h3>
- <a name="carla.Color.r"></a>**<font color="#f8805a">r</font>**  
- <a name="carla.Color.g"></a>**<font color="#f8805a">g</font>**  
- <a name="carla.Color.b"></a>**<font color="#f8805a">b</font>**  
- <a name="carla.Color.a"></a>**<font color="#f8805a">a</font>**  

<h3>Methods</h3>
- <a name="carla.Color.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**r**=0</font>, <font color="#00a6ed">**g**=0</font>, <font color="#00a6ed">**b**=0</font>, <font color="#00a6ed">**a**=255</font>)  
Client constructor.  
    - **Parameters:**
        - `r` (_int_)  
        - `g` (_int_)  
        - `b` (_int_)  
        - `a` (_int_)  
- <a name="carla.Color.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
- <a name="carla.Color.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
- <a name="carla.Color.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.GearPhysicsControl<a name="carla.GearPhysicsControl"></a> <sub><sup>_class_</sup></sub>
Class that provides access to vehicle transmission details.  

<h3>Instance Variables</h3>
- <a name="carla.GearPhysicsControl.ratio"></a>**<font color="#f8805a">ratio</font>** (_float_)  
- <a name="carla.GearPhysicsControl.down_ratio"></a>**<font color="#f8805a">down_ratio</font>** (_float_)  
- <a name="carla.GearPhysicsControl.up_ratio"></a>**<font color="#f8805a">up_ratio</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.GearPhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**ratio**=1.0</font>, <font color="#00a6ed">**down_ratio**=0.5</font>, <font color="#00a6ed">**up_ratio**=0.65</font>)  
    - **Parameters:**
        - `ratio` (_float_)  
        - `down_ratio` (_float_)  
        - `up_ratio` (_float_)  
- <a name="carla.GearPhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.GearPhysicsControl](#carla.GearPhysicsControl)_)  
- <a name="carla.GearPhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.GearPhysicsControl](#carla.GearPhysicsControl)_)  
- <a name="carla.GearPhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.GeoLocation<a name="carla.GeoLocation"></a> <sub><sup>_class_</sup></sub>
Class that contains geolocation simulated data.  

<h3>Instance Variables</h3>
- <a name="carla.GeoLocation.latitude"></a>**<font color="#f8805a">latitude</font>** (_float_)  
- <a name="carla.GeoLocation.longitude"></a>**<font color="#f8805a">longitude</font>** (_float_)  
- <a name="carla.GeoLocation.altitude"></a>**<font color="#f8805a">altitude</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.GeoLocation.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**latitude**=0.0</font>, <font color="#00a6ed">**longitude**=0.0</font>, <font color="#00a6ed">**altitude**=0.0</font>)  
    - **Parameters:**
        - `latitude` (_float_)  
        - `longitude` (_float_)  
        - `altitude` (_float_)  
- <a name="carla.GeoLocation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Geolocation](#carla.Geolocation)_)  
- <a name="carla.GeoLocation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Geolocation](#carla.Geolocation)_)  
- <a name="carla.GeoLocation.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Map<a name="carla.Map"></a> <sub><sup>_class_</sup></sub>
Map description that provides a Waypoint query system, that extracts the information from the OpenDRIVE file.  

<h3>Instance Variables</h3>
- <a name="carla.Map.name"></a>**<font color="#f8805a">name</font>**  
Map name. Comes from the Unreal's UMap name if loaded from a Carla server.  

<h3>Methods</h3>
- <a name="carla.Map.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>, <font color="#00a6ed">**xodr_content**</font>)  
Constructor for this class useful if you want to use a `XODR` (OpenDRIVE) file without importing it from any Carla server running.  
    - **Parameters:**
        - `name` (_str_) – Name of the current map.  
        - `xodr_content` (_str_) – XODR content as string.  
    - **Return:** _list([carla.Transform](#carla.Transform))_  
- <a name="carla.Map.get_spawn_points"></a>**<font color="#7fb800">get_spawn_points</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of transformations corresponding to the recommended spawn points over the map.  
    - **Return:** _list([carla.Transform](#carla.Transform))_  
- <a name="carla.Map.get_waypoint"></a>**<font color="#7fb800">get_waypoint</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**project_to_road**=True</font>, <font color="#00a6ed">**lane_type**=[carla.LaneType.Driving](#carla.LaneType.Driving)</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) – Location where you want to get the [carla.Waypoint](#carla.Waypoint).  
        - `project_to_road` (_bool_) – If **True**, the waypoint will be at the center of the nearest lane.  
If **False**, the waypoint will be at the given location. Also, in this second case, the result may be `None` if the waypoint is not found.  
        - `lane_type` (_[carla.LaneType](#carla.LaneType)_) – This parameter is used to limit the search on a certain lane type. This can be used like a flag: `LaneType.Driving & LaneType.Shoulder`.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Map.get_topology"></a>**<font color="#7fb800">get_topology</font>**(<font color="#00a6ed">**self**</font>)  
It provides a minimal graph of the topology of the current OpenDRIVE file. It is constituted by a list of pairs of waypoints, where the first waypoint is the origin and the second one is the destination. It can be loaded into [NetworkX](https://networkx.github.io/). A valid output could be: `[ (w0, w1), (w0, w2), (w1, w3), (w2, w3), (w0, w4) ]`.  
    - **Return:** _list(tuple([carla.Waypoint](#carla.Waypoint), [carla.Waypoint](#carla.Waypoint)))_  
- <a name="carla.Map.generate_waypoints"></a>**<font color="#7fb800">generate_waypoints</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints positioned on the center of the lanes  all over the map with an approximate distance between them.  
    - **Parameters:**
        - `distance` (_float_) – Approximate distance between the waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Map.transform_to_geolocation"></a>**<font color="#7fb800">transform_to_geolocation</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Converts a given [carla.Location](#carla.Location) `(x, y, z)` to a [carla.GeoLocation](#carla.GeoLocation) `(lat, lon, alt)`.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) – Location to convert.  
    - **Return:** _[carla.GeoLocation](#carla.GeoLocation)_  
- <a name="carla.Map.to_opendrive"></a>**<font color="#7fb800">to_opendrive</font>**(<font color="#00a6ed">**self**</font>)  
Returns the OpenDRIVE of the current map as string.  
    - **Return:** _str_  
- <a name="carla.Map.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Save the OpenDRIVE of the current map to disk.  
    - **Parameters:**
        - `path` – Path where it will be saved.  
- <a name="carla.Map.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Rotation<a name="carla.Rotation"></a> <sub><sup>_class_</sup></sub>
Class that represents a 3D rotation. All rotation angles are stored in degrees.

![UE4_Rotation](https://d26ilriwvtzlb.cloudfront.net/8/83/BRMC_9.jpg)   _Unreal Engine's standard (from [UE4 docs](https://wiki.unrealengine.com/Blueprint_Rotating_Movement_Component))_.  

<h3>Instance Variables</h3>
- <a name="carla.Rotation.pitch"></a>**<font color="#f8805a">pitch</font>** (_float_)  
- <a name="carla.Rotation.yaw"></a>**<font color="#f8805a">yaw</font>** (_float_)  
- <a name="carla.Rotation.roll"></a>**<font color="#f8805a">roll</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.Rotation.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pitch**=0.0</font>, <font color="#00a6ed">**yaw**=0.0</font>, <font color="#00a6ed">**roll**=0.0</font>)  
    - **Parameters:**
        - `pitch` (_float_)  
        - `yaw` (_float_)  
        - `roll` (_float_)  
- <a name="carla.Rotation.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes a forward vector using the current rotation.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Rotation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Rotation](#carla.Rotation)_)  
- <a name="carla.Rotation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Rotation](#carla.Rotation)_)  
- <a name="carla.Rotation.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Timestamp<a name="carla.Timestamp"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.Timestamp.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
The number of frames elapsed since the simulator was launched.  
- <a name="carla.Timestamp.elapsed_seconds"></a>**<font color="#f8805a">elapsed_seconds</font>** (_float_)  
Simulated seconds elapsed since the beginning of the current episode.  
- <a name="carla.Timestamp.delta_seconds"></a>**<font color="#f8805a">delta_seconds</font>** (_float_)  
Simulated seconds elapsed since the previous frame.  
- <a name="carla.Timestamp.platform_timestamp"></a>**<font color="#f8805a">platform_timestamp</font>** (_float_)  
Time-stamp of the frame at which this measurement was taken, in seconds as given by the OS.  

<h3>Methods</h3>
- <a name="carla.Timestamp.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**frame**</font>, <font color="#00a6ed">**elapsed_seconds**</font>, <font color="#00a6ed">**delta_seconds**</font>, <font color="#00a6ed">**platform_timestamp**</font>)  
    - **Parameters:**
        - `frame` (_int_)  
        - `elapsed_seconds` (_float_)  
        - `delta_seconds` (_float_)  
        - `platform_timestamp` (_float_)  
- <a name="carla.Timestamp.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.Timestamp.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.Timestamp.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.TrafficSign<a name="carla.TrafficSign"></a><sub><sup>([carla.Actor](#carla.Actor))</sup></sub> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.TrafficSign.trigger_volume"></a>**<font color="#f8805a">trigger_volume</font>**  

---

## carla.Transform<a name="carla.Transform"></a> <sub><sup>_class_</sup></sub>
Class that defines a transformation without scaling.  

<h3>Instance Variables</h3>
- <a name="carla.Transform.location"></a>**<font color="#f8805a">location</font>** (_[carla.Location](#carla.Location)_)  
- <a name="carla.Transform.rotation"></a>**<font color="#f8805a">rotation</font>** (_[carla.Rotation](#carla.Rotation)_)  

<h3>Methods</h3>
- <a name="carla.Transform.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**rotation**</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `rotation` (_[carla.Rotation](#carla.Rotation)_)  
- <a name="carla.Transform.transform"></a>**<font color="#7fb800">transform</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**in_point**</font>)  
Transform a 3D point using the current transformation.  
    - **Parameters:**
        - `in_point` (_[carla.Location](#carla.Location)_) – Location in the space to which the transformation will be applied.  
- <a name="carla.Transform.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes a forward vector using the rotation of the current transformation.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Transform.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Transform](#carla.Transform)_)  
- <a name="carla.Transform.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Transform](#carla.Transform)_)  
- <a name="carla.Transform.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Vector2D<a name="carla.Vector2D"></a> <sub><sup>_class_</sup></sub>
Vector 2D helper class.  

<h3>Instance Variables</h3>
- <a name="carla.Vector2D.x"></a>**<font color="#f8805a">x</font>** (_float_)  
- <a name="carla.Vector2D.y"></a>**<font color="#f8805a">y</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.Vector2D.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
- <a name="carla.Vector2D.__add__"></a>**<font color="#7fb800">\__add__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__mul__"></a>**<font color="#7fb800">\__mul__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__sub__"></a>**<font color="#7fb800">\__sub__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__truediv__"></a>**<font color="#7fb800">\__truediv__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__self__"></a>**<font color="#7fb800">\__self__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Vector3D<a name="carla.Vector3D"></a> <sub><sup>_class_</sup></sub>
Vector 3D helper class.  

<h3>Instance Variables</h3>
- <a name="carla.Vector3D.x"></a>**<font color="#f8805a">x</font>** (_float_)  
- <a name="carla.Vector3D.y"></a>**<font color="#f8805a">y</font>** (_float_)  
- <a name="carla.Vector3D.z"></a>**<font color="#f8805a">z</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.Vector3D.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>, <font color="#00a6ed">**z**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
        - `z` (_float_)  
- <a name="carla.Vector3D.__add__"></a>**<font color="#7fb800">\__add__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__mul__"></a>**<font color="#7fb800">\__mul__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__sub__"></a>**<font color="#7fb800">\__sub__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__truediv__"></a>**<font color="#7fb800">\__truediv__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.VehicleControl<a name="carla.VehicleControl"></a> <sub><sup>_class_</sup></sub>
VehicleControl is used for controlling the basic movement of a vehicle.  

<h3>Instance Variables</h3>
- <a name="carla.VehicleControl.throttle"></a>**<font color="#f8805a">throttle</font>** (_float_)  
A scalar value to control the vehicle throttle [0.0, 1.0].  
- <a name="carla.VehicleControl.steer"></a>**<font color="#f8805a">steer</font>** (_float_)  
A scalar value to control the vehicle steering [-1.0, 1.0].  
- <a name="carla.VehicleControl.brake"></a>**<font color="#f8805a">brake</font>** (_float_)  
A scalar value to control the vehicle brake [0.0, 1.0].  
- <a name="carla.VehicleControl.hand_brake"></a>**<font color="#f8805a">hand_brake</font>** (_bool_)  
If true, hand brake will be used.  
- <a name="carla.VehicleControl.reverse"></a>**<font color="#f8805a">reverse</font>** (_bool_)  
If true, the vehicle will move reverse.  
- <a name="carla.VehicleControl.manual_gear_shift"></a>**<font color="#f8805a">manual_gear_shift</font>** (_bool_)  
If true, the vehicle will be controlled by changing gears manually.  
- <a name="carla.VehicleControl.gear"></a>**<font color="#f8805a">gear</font>** (_int_)  
Controls the gear value of the vehicle.  

<h3>Methods</h3>
- <a name="carla.VehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**throttle**=0.0</font>, <font color="#00a6ed">**steer**=0.0</font>, <font color="#00a6ed">**brake**=0.0</font>, <font color="#00a6ed">**hand_brake**=True</font>, <font color="#00a6ed">**reverse**=True</font>, <font color="#00a6ed">**manual_gear_shift**=True</font>, <font color="#00a6ed">**gear**=0</font>)  
VehicleControl constructor.  
    - **Parameters:**
        - `throttle` (_float_)  
        - `steer` (_float_)  
        - `brake` (_float_)  
        - `hand_brake` (_bool_)  
        - `reverse` (_bool_)  
        - `manual_gear_shift` (_bool_)  
        - `gear` (_int_)  
- <a name="carla.VehicleControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehicleControl](#carla.VehicleControl)_)  
- <a name="carla.VehicleControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehicleControl](#carla.VehicleControl)_)  
- <a name="carla.VehicleControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.VehiclePhysicsControl<a name="carla.VehiclePhysicsControl"></a> <sub><sup>_class_</sup></sub>
VehiclePhysicsControl is used for controlling the physics parameters of a vehicle.  

<h3>Instance Variables</h3>
- <a name="carla.VehiclePhysicsControl.torque_curve"></a>**<font color="#f8805a">torque_curve</font>** (_list([carla.Vector2D](#carla.Vector2D))_)  
Curve that indicates the torque measured in Nm for a specific RPM of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.max_rpm"></a>**<font color="#f8805a">max_rpm</font>** (_float_)  
The maximum RPM of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.moi"></a>**<font color="#f8805a">moi</font>** (_float_)  
The moment of inertia of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.damping_rate_full_throttle"></a>**<font color="#f8805a">damping_rate_full_throttle</font>** (_float_)  
Damping rate when the throttle is maximum.  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_engaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_engaged</font>** (_float_)  
Damping rate when the throttle is zero with clutch engaged.  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_disengaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_disengaged</font>** (_float_)  
Damping rate when the throttle is zero with clutch disengaged.  
- <a name="carla.VehiclePhysicsControl.use_gear_autobox"></a>**<font color="#f8805a">use_gear_autobox</font>** (_bool_)  
If true, the vehicle will have an automatic transmission.  
- <a name="carla.VehiclePhysicsControl.gear_switch_time"></a>**<font color="#f8805a">gear_switch_time</font>** (_float_)  
Switching time between gears.  
- <a name="carla.VehiclePhysicsControl.clutch_strength"></a>**<font color="#f8805a">clutch_strength</font>** (_float_)  
The clutch strength of the vehicle. Measured in Kgm^2/s.  
- <a name="carla.VehiclePhysicsControl.final_ratio"></a>**<font color="#f8805a">final_ratio</font>** (_float_)  
The fixed ratio from transmission to wheels.  
- <a name="carla.VehiclePhysicsControl.forward_gears"></a>**<font color="#f8805a">forward_gears</font>** (_list([carla.GearPhysicsControl](#carla.GearPhysicsControl))_)  
List of GearPhysicsControl objects.  
- <a name="carla.VehiclePhysicsControl.mass"></a>**<font color="#f8805a">mass</font>** (_float_)  
The mass of the vehicle measured in Kg.  
- <a name="carla.VehiclePhysicsControl.drag_coefficient"></a>**<font color="#f8805a">drag_coefficient</font>** (_float_)  
Drag coefficient of the vehicle's chassis.  
- <a name="carla.VehiclePhysicsControl.center_of_mass"></a>**<font color="#f8805a">center_of_mass</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
The center of mass of the vehicle.  
- <a name="carla.VehiclePhysicsControl.steering_curve"></a>**<font color="#f8805a">steering_curve</font>** (_list([carla.Vector2D](#carla.Vector2D))_)  
Curve that indicates the maximum steering for a specific forward speed.  
- <a name="carla.VehiclePhysicsControl.wheels"></a>**<font color="#f8805a">wheels</font>** (_list([carla.WheelPhysicsControl](#carla.WheelPhysicsControl))_)  
List of [carla.WheelPhysicsControl](#carla.WheelPhysicsControl) objects. This list should have 4 elements, where index 0 corresponds to the front left wheel, index 1 corresponds to the front right wheel, index 2 corresponds to the back left wheel and index 3 corresponds to the back right wheel. For 2 wheeled vehicles, set the same values for both front and back wheels.  

<h3>Methods</h3>
- <a name="carla.VehiclePhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**torque_curve**=[[0.0, 500.0], [5000.0, 500.0]]</font>, <font color="#00a6ed">**max_rpm**=5000.0</font>, <font color="#00a6ed">**moi**=1.0</font>, <font color="#00a6ed">**damping_rate_full_throttle**=0.15</font>, <font color="#00a6ed">**damping_rate_zero_throttle_clutch_engaged**=2.0</font>, <font color="#00a6ed">**damping_rate_zero_throttle_clutch_disengaged**=0.35</font>, <font color="#00a6ed">**use_gear_autobox**=True</font>, <font color="#00a6ed">**gear_switch_time**=0.5</font>, <font color="#00a6ed">**clutch_strength**=10.0</font>, <font color="#00a6ed">**mass**=1000.0</font>, <font color="#00a6ed">**drag_coefficient**=0.3</font>, <font color="#00a6ed">**center_of_mass**=[0.0, 0.0, 0.0]</font>, <font color="#00a6ed">**steering_curve**=[0.0, 0.0, 0.0]</font>, <font color="#00a6ed">**wheels**=list()</font>)  
VehiclePhysicsControl constructor.  
- <a name="carla.VehiclePhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
- <a name="carla.VehiclePhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
- <a name="carla.VehiclePhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerAIController<a name="carla.WalkerAIController"></a><sub><sup>([carla.Actor](#carla.Actor))</sup></sub> <sub><sup>_class_</sup></sub>
Class used for controlling the automation of a pedestrian.  

<h3>Methods</h3>
- <a name="carla.WalkerAIController.start"></a>**<font color="#7fb800">start</font>**(<font color="#00a6ed">**self**</font>)  
Initializes walker controller.  
- <a name="carla.WalkerAIController.stop"></a>**<font color="#7fb800">stop</font>**(<font color="#00a6ed">**self**</font>)  
Stops walker controller.  
- <a name="carla.WalkerAIController.go_to_location"></a>**<font color="#7fb800">go_to_location</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**destination**</font>)  
Sets the destination that the pedestrian will reach.  
    - **Parameters:**
        - `destination` (_[carla.Location](#carla.Location)_)  
- <a name="carla.WalkerAIController.set_max_speed"></a>**<font color="#7fb800">set_max_speed</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**speed**=1.4</font>)  
Sets the speed of the pedestrian.  
    - **Parameters:**
        - `speed` (_float_) – Speed is in m/s.  
- <a name="carla.WalkerAIController.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerBoneControl<a name="carla.WalkerBoneControl"></a> <sub><sup>_class_</sup></sub>
Class used for controlling the skeleton of a walker. See [walker bone control](walker_bone_control.md).  

<h3>Instance Variables</h3>
- <a name="carla.WalkerBoneControl.bone_transforms"></a>**<font color="#f8805a">bone_transforms</font>** (_list([name,transform])_)  

<h3>Methods</h3>
- <a name="carla.WalkerBoneControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**list(name,transform)**</font>)  
    - **Parameters:**
        - `list(name,transform)` (_tuple_)  
- <a name="carla.WalkerBoneControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerControl<a name="carla.WalkerControl"></a> <sub><sup>_class_</sup></sub>
WalkerControl is used for controlling the basic movement of a walker.  

<h3>Instance Variables</h3>
- <a name="carla.WalkerControl.direction"></a>**<font color="#f8805a">direction</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Vector that controls the direction of the walker.  
- <a name="carla.WalkerControl.speed"></a>**<font color="#f8805a">speed</font>** (_float_)  
A scalar value to control the walker speed.  
- <a name="carla.WalkerControl.jump"></a>**<font color="#f8805a">jump</font>** (_bool_)  
If true, the walker will perform a jump.  

<h3>Methods</h3>
- <a name="carla.WalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**direction**=[1.0, 0.0, 0.0]</font>, <font color="#00a6ed">**speed**=0.0</font>, <font color="#00a6ed">**jump**=False</font>)  
VehicleControl constructor.  
- <a name="carla.WalkerControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WalkerControl](#carla.WalkerControl)_)  
- <a name="carla.WalkerControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WalkerControl](#carla.WalkerControl)_)  
- <a name="carla.WalkerControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Waypoint<a name="carla.Waypoint"></a> <sub><sup>_class_</sup></sub>
3D directed point that stores information about the road definition that OpenDRIVE provides.  

<h3>Instance Variables</h3>
- <a name="carla.Waypoint.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Waypoint id, it's generated using a hash combination of its `road_id`, `section_id`,  `lane_id` and `s` values, all them come from the OpenDRIVE. The `s` precision is set  to 2 centimeters, so 2 waypoints at a distance `s` less than 2 centimeters in the same  road, section and lane, will have the same `id`.  
- <a name="carla.Waypoint.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Transform indicating it's position and orientation according to the current lane information.  
- <a name="carla.Waypoint.is_intersection"></a>**<font color="#f8805a">is_intersection</font>** (_bool_)  
_Deprecated, use is_junction instead_.  
- <a name="carla.Waypoint.is_junction"></a>**<font color="#f8805a">is_junction</font>** (_bool_)  
True if the current Waypoint is on a junction.  
- <a name="carla.Waypoint.lane_width"></a>**<font color="#f8805a">lane_width</font>** (_float_)  
Horizontal size of the road at current `s`.  
- <a name="carla.Waypoint.road_id"></a>**<font color="#f8805a">road_id</font>** (_int_)  
OpenDRIVE road's id.  
- <a name="carla.Waypoint.section_id"></a>**<font color="#f8805a">section_id</font>** (_int_)  
OpenDRIVE section's id, based on the order that they are originally defined.  
- <a name="carla.Waypoint.lane_id"></a>**<font color="#f8805a">lane_id</font>** (_int_)  
OpenDRIVE lane's id, this value can be positive or negative which represents the direction of the  current lane with respect to the road. For more information refer to OpenDRIVE [documentation](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf#page=20).  
- <a name="carla.Waypoint.s"></a>**<font color="#f8805a">s</font>** (_float_)  
OpenDRIVE `s` value of the current position.  
- <a name="carla.Waypoint.lane_change"></a>**<font color="#f8805a">lane_change</font>** (_[carla.LaneChange](#carla.LaneChange)_)  
Lane change definition of the current Waypoint's location, based on the traffic rules defined in the OpenDRIVE file. Basically, it tells you if a lane change can be done and in which direction.  
- <a name="carla.Waypoint.lane_type"></a>**<font color="#f8805a">lane_type</font>** (_[carla.LaneType](#carla.LaneType)_)  
The lane type of the current Waypoint, based on OpenDRIVE types.  
- <a name="carla.Waypoint.right_lane_marking"></a>**<font color="#f8805a">right_lane_marking</font>** (_[carla.LaneMarking](#carla.LaneMarking)_)  
The right lane marking information based on the direction of the Waypoint.  
- <a name="carla.Waypoint.left_lane_marking"></a>**<font color="#f8805a">left_lane_marking</font>** (_[carla.LaneMarking](#carla.LaneMarking)_)  
The left lane marking information based on the direction of the Waypoint.  

<h3>Methods</h3>
- <a name="carla.Waypoint.next"></a>**<font color="#7fb800">next</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of Waypoints at a certain approximate distance from the current Waypoint, taking into account the shape of the road and its possible deviations, without performing any lane change.  
The list may be empty if the road ends before the specified distance, for instance, a lane ending with the only option of incorporating to another road.  
    - **Parameters:**
        - `distance` (_float_) – The approximate distance where to get the next Waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.get_right_lane"></a>**<font color="#7fb800">get_right_lane</font>**(<font color="#00a6ed">**self**</font>)  
Generates a Waypoint at the center of the right lane based on the direction of the current Waypoint, regardless if the lane change is allowed in this location.  
Can return `None` if the lane does not exist.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Waypoint.get_left_lane"></a>**<font color="#7fb800">get_left_lane</font>**(<font color="#00a6ed">**self**</font>)  
Generates a Waypoint at the center of the left lane based on the direction of the current Waypoint, regardless if the lane change is allowed in this location.  
Can return `None` if the lane does not exist.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Waypoint.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WeatherParameters<a name="carla.WeatherParameters"></a> <sub><sup>_class_</sup></sub>
WeatherParameters class is used for requesting and changing the lighting and weather conditions inside the world.  

<h3>Instance Variables</h3>
- <a name="carla.WeatherParameters.cloudiness"></a>**<font color="#f8805a">cloudiness</font>** (_float_)  
Weather cloudiness. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.precipitation"></a>**<font color="#f8805a">precipitation</font>** (_float_)  
Precipitation amount for controlling rain intensity. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.precipitation_deposits"></a>**<font color="#f8805a">precipitation_deposits</font>** (_float_)  
Precipitation deposits for controlling the area of puddles on roads. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.wind_intensity"></a>**<font color="#f8805a">wind_intensity</font>** (_float_)  
Wind intensity. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.sun_azimuth_angle"></a>**<font color="#f8805a">sun_azimuth_angle</font>** (_float_)  
The azimuth angle of the sun in degrees. Values range from 0 to 360.  
- <a name="carla.WeatherParameters.sun_altitude_angle"></a>**<font color="#f8805a">sun_altitude_angle</font>** (_float_)  
Altitude angle of the sun in degrees. Values range from -90 to 90.  

<h3>Methods</h3>
- <a name="carla.WeatherParameters.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**cloudiness**=0.0</font>, <font color="#00a6ed">**precipitation**=0.0</font>, <font color="#00a6ed">**precipitation_deposits**=0.0</font>, <font color="#00a6ed">**wind_intensity**=0.0</font>, <font color="#00a6ed">**sun_azimuth_angle**=0.0</font>, <font color="#00a6ed">**sun_altitude_angle**=0.0</font>)  
WeatherParameters constructor.  
    - **Parameters:**
        - `cloudiness` (_float_)  
        - `precipitation` (_float_)  
        - `precipitation_deposits` (_float_)  
        - `wind_intensity` (_float_)  
        - `sun_azimuth_angle` (_float_)  
        - `sun_altitude_angle` (_float_)  
- <a name="carla.WeatherParameters.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
Returns True if `self` and `other` are equal.  
    - **Return:** _bool_  
- <a name="carla.WeatherParameters.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
Returns True if `self` and `other` are not equal.  
    - **Return:** _bool_  
- <a name="carla.WeatherParameters.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WheelPhysicsControl<a name="carla.WheelPhysicsControl"></a> <sub><sup>_class_</sup></sub>
WheelPhysicsControl is used for controlling the physics parameters of a vehicle's wheel.  

<h3>Instance Variables</h3>
- <a name="carla.WheelPhysicsControl.tire_friction"></a>**<font color="#f8805a">tire_friction</font>** (_float_)  
A scalar value that indicates the friction of the wheel.  
- <a name="carla.WheelPhysicsControl.damping_rate"></a>**<font color="#f8805a">damping_rate</font>** (_float_)  
The damping rate of the wheel.  
- <a name="carla.WheelPhysicsControl.max_steer_angle"></a>**<font color="#f8805a">max_steer_angle</font>** (_float_)  
The maximum angle in degrees that the wheel can steer.  
- <a name="carla.WheelPhysicsControl.radius"></a>**<font color="#f8805a">radius</font>** (_float_)  
The radius of the wheel in centimeters.  
- <a name="carla.WheelPhysicsControl.max_brake_torque"></a>**<font color="#f8805a">max_brake_torque</font>** (_float_)  
The maximum brake torque in Nm.  
- <a name="carla.WheelPhysicsControl.max_handbrake_torque"></a>**<font color="#f8805a">max_handbrake_torque</font>** (_float_)  
The maximum handbrake torque in Nm.  
- <a name="carla.WheelPhysicsControl.position"></a>**<font color="#f8805a">position</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
World position of the wheel. Note that it is a read-only parameter.  

<h3>Methods</h3>
- <a name="carla.WheelPhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**tire_friction**=2.0</font>, <font color="#00a6ed">**damping_rate**=0.25</font>, <font color="#00a6ed">**max_steer_angle**=70.0</font>, <font color="#00a6ed">**radius**=30.0</font>, <font color="#00a6ed">**position**=(0.0,0.0,0.0)</font>)  
WheelPhysicsControl constructor.  
- <a name="carla.WheelPhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WheelPhysicsControl](#carla.WheelPhysicsControl)_)  
- <a name="carla.WheelPhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WheelPhysicsControl](#carla.WheelPhysicsControl)_)  
- <a name="carla.WheelPhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.World<a name="carla.World"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.World.id"></a>**<font color="#f8805a">id</font>** (_int_)  
The id of the episode associated with this world.  
- <a name="carla.World.debug"></a>**<font color="#f8805a">debug</font>** (_[carla.DebugHelper](#carla.DebugHelper)_)  

<h3>Methods</h3>
- <a name="carla.World.get_blueprint_library"></a>**<font color="#7fb800">get_blueprint_library</font>**(<font color="#00a6ed">**self**</font>)  
Return the list of blueprints available in this world. These blueprints can be used to spawn actors into the world.  
    - **Return:** _[carla.BlueprintLibrary](#carla.BlueprintLibrary)_  
- <a name="carla.World.get_map"></a>**<font color="#7fb800">get_map</font>**(<font color="#00a6ed">**self**</font>)  
Return the map that describes this world.  
    - **Return:** _[carla.Map](#carla.Map)_  
- <a name="carla.World.get_spectator"></a>**<font color="#7fb800">get_spectator</font>**(<font color="#00a6ed">**self**</font>)  
Return the spectator actor. The spectator controls the view in the simulator window.  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.get_settings"></a>**<font color="#7fb800">get_settings</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _[carla.WorldSettings](#carla.WorldSettings)_  
- <a name="carla.World.apply_settings"></a>**<font color="#7fb800">apply_settings</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**world_settings**</font>)  
Returns the id of the frame when the settings took effect.  
    - **Parameters:**
        - `world_settings` (_[carla.WorldSettings](#carla.WorldSettings)_)  
    - **Return:** _int_  
- <a name="carla.World.get_weather"></a>**<font color="#7fb800">get_weather</font>**(<font color="#00a6ed">**self**</font>)  
Retrieve the weather parameters currently active in the world.  
    - **Return:** _[carla.WeatherParameters](#carla.WeatherParameters)_  
- <a name="carla.World.set_weather"></a>**<font color="#7fb800">set_weather</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**weather**</font>)  
Change the weather in the simulation.  
    - **Parameters:**
        - `weather` (_[carla.WeatherParameters](#carla.WeatherParameters)_)  
- <a name="carla.World.get_snapshot"></a>**<font color="#7fb800">get_snapshot</font>**(<font color="#00a6ed">**self**</font>)  
Return a snapshot of the world at this moment.  
    - **Return:** _[carla.WorldSnapshot](#carla.WorldSnapshot)_  
- <a name="carla.World.get_actor"></a>**<font color="#7fb800">get_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Find actor by id, return None if not found.  
    - **Parameters:**
        - `actor_id` (_int_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.get_actors"></a>**<font color="#7fb800">get_actors</font>**(<font color="#00a6ed">**self**</font>)  
By default it returns a list with every actor present in the world. _A list of ids can be used as a parameter_.  
    - **Return:** _[carla.ActorList](#carla.ActorList)_  
- <a name="carla.World.spawn_actor"></a>**<font color="#7fb800">spawn_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**attach_to**=None</font>, <font color="#00a6ed">**attachment**=Rigid</font>)  
Spawn an actor into the world based on the blueprint provided at transform. If a parent is provided, the actor is attached to parent.  
    - **Parameters:**
        - `blueprint` (_[carla.BlueprintLibrary](#carla.BlueprintLibrary)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_) – If attached to parent, transform acts like a relative_transform to the parent actor.  
        - `attach_to` (_[carla.Actor](#carla.Actor)_)  
        - `attachment` (_[carla.AttachmentType](#carla.AttachmentType)_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.try_spawn_actor"></a>**<font color="#7fb800">try_spawn_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**attach_to**=None</font>, <font color="#00a6ed">**attachment**=Rigid</font>)  
Same as SpawnActor but return none on failure instead of throwing an exception.  
    - **Parameters:**
        - `blueprint` (_[carla.BlueprintLibrary](#carla.BlueprintLibrary)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_) – If attached to parent, transform acts like a relative_transform to the parent actor.  
        - `attach_to` (_[carla.Actor](#carla.Actor)_)  
        - `attachment` (_[carla.AttachmentType](#carla.AttachmentType)_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.wait_for_tick"></a>**<font color="#7fb800">wait_for_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**=10.0</font>)  
Block calling thread until a world tick is received.  
    - **Parameters:**
        - `seconds` (_float_)  
    - **Return:** _[carla.WorldSnapshot](#carla.WorldSnapshot)_  
- <a name="carla.World.on_tick"></a>**<font color="#7fb800">on_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback**</font>)  
Returns the ID of the callback so it can be removed with `remove_on_tick`.  
    - **Parameters:**
        - `callback` (_[carla.WorldSnapshot](#carla.WorldSnapshot)_)  
    - **Return:** _int_  
- <a name="carla.World.remove_on_tick"></a>**<font color="#7fb800">remove_on_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback_id**</font>)  
Removes on tick callbacks.  
- <a name="carla.World.tick"></a>**<font color="#7fb800">tick</font>**(<font color="#00a6ed">**self**</font>)  
Synchronizes with the simulator and returns the id of the newly started frame (only has effect on synchronous mode).  
    - **Return:** _int_  
- <a name="carla.World.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WorldSettings<a name="carla.WorldSettings"></a> <sub><sup>_class_</sup></sub>
More information in our [section](../configuring_the_simulation/).  

<h3>Instance Variables</h3>
- <a name="carla.WorldSettings.synchronous_mode"></a>**<font color="#f8805a">synchronous_mode</font>** (_bool_)  
- <a name="carla.WorldSettings.no_rendering_mode"></a>**<font color="#f8805a">no_rendering_mode</font>** (_bool_)  
- <a name="carla.WorldSettings.fixed_delta_seconds"></a>**<font color="#f8805a">fixed_delta_seconds</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.WorldSettings.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**synchronous_mode**=False</font>, <font color="#00a6ed">**no_rendering_mode**=False</font>, <font color="#00a6ed">**fixed_delta_seconds**=0.0</font>)  
    - **Parameters:**
        - `synchronous_mode` (_bool_)  
        - `no_rendering_mode` (_bool_)  
        - `fixed_delta_seconds` (_float_)  
- <a name="carla.WorldSettings.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.WorldSettings.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.WorldSettings.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WorldSnapshot<a name="carla.WorldSnapshot"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.WorldSnapshot.id"></a>**<font color="#f8805a">id</font>** (_int_)  
- <a name="carla.WorldSnapshot.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
- <a name="carla.WorldSnapshot.timestamp"></a>**<font color="#f8805a">timestamp</font>** (_[carla.Timestamp](#carla.Timestamp)_)  

<h3>Methods</h3>
- <a name="carla.WorldSnapshot.has_actor"></a>**<font color="#7fb800">has_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Check if an actor is present in this snapshot.  
    - **Parameters:**
        - `actor_id` (_int_)  
- <a name="carla.WorldSnapshot.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Find an ActorSnapshot by id, return None if the actor is not found.  
    - **Parameters:**
        - `actor_id` (_int_)  
- <a name="carla.WorldSnapshot.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
Return number of ActorSnapshots present in this WorldSnapshot.  
- <a name="carla.WorldSnapshot.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WorldSnapshot.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WorldSnapshot](#carla.WorldSnapshot)_)  
- <a name="carla.WorldSnapshot.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WorldSnapshot](#carla.WorldSnapshot)_)  
- <a name="carla.WorldSnapshot.__self__"></a>**<font color="#7fb800">\__self__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.GnssEvent<a name="carla.GnssEvent"></a><sub><sup>([carla.SensorData](#carla.SensorData))</sup></sub> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.GnssEvent.latitude"></a>**<font color="#f8805a">latitude</font>** (_float_)  
- <a name="carla.GnssEvent.longitude"></a>**<font color="#f8805a">longitude</font>** (_float_)  
- <a name="carla.GnssEvent.altitude"></a>**<font color="#f8805a">altitude</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.GnssEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Image<a name="carla.Image"></a><sub><sup>([carla.SensorData](#carla.SensorData))</sup></sub> <sub><sup>_class_</sup></sub>
Class that defines an image of 32-bit BGRA colors.  

<h3>Instance Variables</h3>
- <a name="carla.Image.width"></a>**<font color="#f8805a">width</font>** (_int_)  
Image width in pixels.  
- <a name="carla.Image.height"></a>**<font color="#f8805a">height</font>** (_int_)  
Image height in pixels.  
- <a name="carla.Image.fov"></a>**<font color="#f8805a">fov</font>** (_float_)  
Horizontal field of view of the image in degrees.  
- <a name="carla.Image.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  

<h3>Methods</h3>
- <a name="carla.Image.convert"></a>**<font color="#7fb800">convert</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**color_converter**</font>)  
    - **Parameters:**
        - `color_converter` (_[carla.ColorConverter](#carla.ColorConverter)_)  
- <a name="carla.Image.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>, <font color="#00a6ed">**color_converter**=Raw</font>)  
    - **Parameters:**
        - `path` (_str_)  
        - `color_converter` (_[carla.ColorConverter](#carla.ColorConverter)_)  
- <a name="carla.Image.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
- <a name="carla.Image.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>, <font color="#00a6ed">**color**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
        - `color` (_[carla.Color](#carla.Color)_)  
- <a name="carla.Image.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.LaneInvasionEvent<a name="carla.LaneInvasionEvent"></a><sub><sup>([carla.SensorData](#carla.SensorData))</sup></sub> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.LaneInvasionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get "self" actor. Actor that invaded another lane.  
- <a name="carla.LaneInvasionEvent.crossed_lane_markings"></a>**<font color="#f8805a">crossed_lane_markings</font>** (_list([carla.LaneMarking](#carla.LaneMarking))_)  
List of lane markings that have been crossed.  

<h3>Methods</h3>
- <a name="carla.LaneInvasionEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.LidarMeasurement<a name="carla.LidarMeasurement"></a><sub><sup>([carla.SensorData](#carla.SensorData))</sup></sub> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.LidarMeasurement.horizontal_angle"></a>**<font color="#f8805a">horizontal_angle</font>** (_float_)  
- <a name="carla.LidarMeasurement.channels"></a>**<font color="#f8805a">channels</font>** (_int_)  
- <a name="carla.LidarMeasurement.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  

<h3>Methods</h3>
- <a name="carla.LidarMeasurement.get_point_count"></a>**<font color="#7fb800">get_point_count</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**channel**</font>)  
    - **Parameters:**
        - `channel` (_int_)  
- <a name="carla.LidarMeasurement.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
    - **Parameters:**
        - `path` (_str_)  
- <a name="carla.LidarMeasurement.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
- <a name="carla.LidarMeasurement.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>, <font color="#00a6ed">**location**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
        - `location` (_[carla.Location](#carla.Location)_)  
- <a name="carla.LidarMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Location<a name="carla.Location"></a><sub><sup>([carla.Vector3D](#carla.Vector3D))</sup></sub> <sub><sup>_class_</sup></sub>
Represents a location in the world (in meters).  

<h3>Instance Variables</h3>
- <a name="carla.Location.x"></a>**<font color="#f8805a">x</font>** (_float_)  
- <a name="carla.Location.y"></a>**<font color="#f8805a">y</font>** (_float_)  
- <a name="carla.Location.z"></a>**<font color="#f8805a">z</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.Location.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>, <font color="#00a6ed">**z**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
        - `z` (_float_)  
- <a name="carla.Location.distance"></a>**<font color="#7fb800">distance</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Computes the Euclidean distance in meters from this location to another one.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) – The Location from where to compute the distance.  
    - **Return:** _float_  
- <a name="carla.Location.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Location](#carla.Location)_)  
- <a name="carla.Location.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Location](#carla.Location)_)  
- <a name="carla.Location.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ObstacleDetectionEvent<a name="carla.ObstacleDetectionEvent"></a><sub><sup>([carla.SensorData](#carla.SensorData))</sup></sub> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.ObstacleDetectionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get "self" actor. Actor that measured the collision.  
- <a name="carla.ObstacleDetectionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get the actor to which we collided.  
- <a name="carla.ObstacleDetectionEvent.distance"></a>**<font color="#f8805a">distance</font>** (_float_)  
Get obstacle distance.  

<h3>Methods</h3>
- <a name="carla.ObstacleDetectionEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Sensor<a name="carla.Sensor"></a><sub><sup>([carla.Actor](#carla.Actor))</sup></sub> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.Sensor.is_listening"></a>**<font color="#f8805a">is_listening</font>** (_boolean_)  
Is true if the sensor is listening for data.  

<h3>Methods</h3>
- <a name="carla.Sensor.listen"></a>**<font color="#7fb800">listen</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback**</font>)  
    - **Parameters:**
        - `callback` (_function_) – Register a callback to be executed each time a new measurement is received. The callback must accept a single argument containing the sensor data; the type of this object varies depending on the type of sensor, but they all derive from [carla.SensorData](#carla.SensorData).  
- <a name="carla.Sensor.stop"></a>**<font color="#7fb800">stop</font>**(<font color="#00a6ed">**self**</font>)  
Stops listening for data.  
- <a name="carla.Sensor.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.TrafficLight<a name="carla.TrafficLight"></a><sub><sup>([carla.TrafficSign](#carla.TrafficSign))</sup></sub> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="carla.TrafficLight.state"></a>**<font color="#f8805a">state</font>** (_[carla.TrafficLightState](#carla.TrafficLightState)_)  

<h3>Methods</h3>
- <a name="carla.TrafficLight.set_state"></a>**<font color="#7fb800">set_state</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**state**</font>)  
    - **Parameters:**
        - `state` (_[carla.TrafficLightState](#carla.TrafficLightState)_)  
- <a name="carla.TrafficLight.get_state"></a>**<font color="#7fb800">get_state</font>**(<font color="#00a6ed">**self**</font>)  
Return the current state of the traffic light.  
    - **Return:** _[carla.TrafficLightState](#carla.TrafficLightState)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.set_green_time"></a>**<font color="#7fb800">set_green_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**green_time**</font>)  
    - **Parameters:**
        - `green_time` (_float_)  
- <a name="carla.TrafficLight.get_green_time"></a>**<font color="#7fb800">get_green_time</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.set_yellow_time"></a>**<font color="#7fb800">set_yellow_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**yellow_time**</font>)  
    - **Parameters:**
        - `yellow_time` (_float_)  
- <a name="carla.TrafficLight.get_yellow_time"></a>**<font color="#7fb800">get_yellow_time</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.set_red_time"></a>**<font color="#7fb800">set_red_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**red_time**</font>)  
    - **Parameters:**
        - `red_time` (_float_)  
- <a name="carla.TrafficLight.get_red_time"></a>**<font color="#7fb800">get_red_time</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.get_elapsed_time"></a>**<font color="#7fb800">get_elapsed_time</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.freeze"></a>**<font color="#7fb800">freeze</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**freeze**</font>)  
    - **Parameters:**
        - `freeze` (_bool_)  
- <a name="carla.TrafficLight.is_frozen"></a>**<font color="#7fb800">is_frozen</font>**(<font color="#00a6ed">**self**</font>)  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.get_pole_index"></a>**<font color="#7fb800">get_pole_index</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _int_  
- <a name="carla.TrafficLight.get_group_traffic_lights"></a>**<font color="#7fb800">get_group_traffic_lights</font>**(<font color="#00a6ed">**self**</font>)  
Return all traffic lights in the group this one belongs to.  
    - **Note:** <font color="#8E8E8E">_This function calls the simulator.
_</font>  
- <a name="carla.TrafficLight.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Vehicle<a name="carla.Vehicle"></a><sub><sup>([carla.Actor](#carla.Actor))</sup></sub> <sub><sup>_class_</sup></sub>
A vehicle actor.  

<h3>Instance Variables</h3>
- <a name="carla.Vehicle.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
The bounding box of the vehicle.  

<h3>Methods</h3>
- <a name="carla.Vehicle.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
Apply control to this vehicle. The control will take effect on next tick.  
    - **Parameters:**
        - `control` (_[carla.VehicleControl](#carla.VehicleControl)_)  
- <a name="carla.Vehicle.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  
Return the control last applied to this vehicle.  
    - **Return:** _[carla.VehicleControl](#carla.VehicleControl)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.apply_physics_control"></a>**<font color="#7fb800">apply_physics_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**physics_control**</font>)  
Apply physics control to this vehicle. The control will take effect on the next tick.  
    - **Parameters:**
        - `physics_control` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
- <a name="carla.Vehicle.get_physics_control"></a>**<font color="#7fb800">get_physics_control</font>**(<font color="#00a6ed">**self**</font>)  
Return the physics control last applied to this vehicle.  
    - **Return:** _[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_  
    - **Warning:** <font color="#ED2F2F">_This function does call the simulator to retrieve the value._</font>  
- <a name="carla.Vehicle.set_autopilot"></a>**<font color="#7fb800">set_autopilot</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=True</font>)  
Switch on/off this vehicle's server-side autopilot.  
    - **Parameters:**
        - `enabled` (_bool_)  
- <a name="carla.Vehicle.get_speed_limit"></a>**<font color="#7fb800">get_speed_limit</font>**(<font color="#00a6ed">**self**</font>)  
Return the speed limit currently affecting this vehicle. Note that the speed limit is only updated when passing by a speed limit signal, right after spawning a vehicle it might not reflect the actual speed limit of the current road.  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.get_traffic_light_state"></a>**<font color="#7fb800">get_traffic_light_state</font>**(<font color="#00a6ed">**self**</font>)  
Return the state of the traffic light currently affecting this vehicle. If no traffic light is currently affecting the vehicle, return Green.  
    - **Return:** _[carla.TrafficLightState](#carla.TrafficLightState)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.is_at_traffic_light"></a>**<font color="#7fb800">is_at_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  
Return whether a traffic light is affecting this vehicle.  
    - **Return:** _bool_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.get_traffic_light"></a>**<font color="#7fb800">get_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  
Retrieve the traffic light actor currently affecting this vehicle.  
    - **Return:** _[carla.TrafficLight](#carla.TrafficLight)_  
- <a name="carla.Vehicle.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _str_  

---

## carla.Walker<a name="carla.Walker"></a><sub><sup>([carla.Actor](#carla.Actor))</sup></sub> <sub><sup>_class_</sup></sub>
A walking actor, pedestrian.  

<h3>Instance Variables</h3>
- <a name="carla.Walker.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
The bounding box of the walker.  

<h3>Methods</h3>
- <a name="carla.Walker.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
Apply control to this walker. The control will take effect on the next tick.  
    - **Parameters:**
        - `control` (_[carla.WalkerControl](#carla.WalkerControl)_)  
- <a name="carla.Walker.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `control` (_[carla.WalkerBoneControl](#carla.WalkerBoneControl)_)  
- <a name="carla.Walker.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  
Return the control last applied to this walker.  
    - **Return:** _[carla.WalkerControl](#carla.WalkerControl)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Walker.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _str_  

---

## command.ApplyAngularVelocity<a name="command.ApplyAngularVelocity"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.ApplyAngularVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.ApplyAngularVelocity.angular_velocity"></a>**<font color="#f8805a">angular_velocity</font>** (_[carla.Vector3D](#carla.Vector3D)_)  

<h3>Methods</h3>
- <a name="command.ApplyAngularVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**angular_velocity**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `angular_velocity` (_[carla.Vector3D](#carla.Vector3D)_)  

---

## command.ApplyImpulse<a name="command.ApplyImpulse"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.ApplyImpulse.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.ApplyImpulse.impulse"></a>**<font color="#f8805a">impulse</font>** (_[carla.Vector3D](#carla.Vector3D)_)  

<h3>Methods</h3>
- <a name="command.ApplyImpulse.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**impulse**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `impulse` (_[carla.Vector3D](#carla.Vector3D)_)  

---

## command.ApplyTransform<a name="command.ApplyTransform"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.ApplyTransform.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.ApplyTransform.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  

<h3>Methods</h3>
- <a name="command.ApplyTransform.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**transform**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  

---

## command.ApplyVehicleControl<a name="command.ApplyVehicleControl"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.ApplyVehicleControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.ApplyVehicleControl.control"></a>**<font color="#f8805a">control</font>** (_[carla.VehicleControl](#carla.VehicleControl)_)  

<h3>Methods</h3>
- <a name="command.ApplyVehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `control` (_[carla.VehicleControl](#carla.VehicleControl)_)  

---

## command.ApplyVelocity<a name="command.ApplyVelocity"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.ApplyVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.ApplyVelocity.velocity"></a>**<font color="#f8805a">velocity</font>** (_[carla.Vector3D](#carla.Vector3D)_)  

<h3>Methods</h3>
- <a name="command.ApplyVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**velocity**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `velocity` (_[carla.Vector3D](#carla.Vector3D)_)  

---

## command.ApplyWalkerControl<a name="command.ApplyWalkerControl"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.ApplyWalkerControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.ApplyWalkerControl.control"></a>**<font color="#f8805a">control</font>** (_[carla.VehicleControl](#carla.VehicleControl)_)  

<h3>Methods</h3>
- <a name="command.ApplyWalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `control` (_[carla.WalkerControl](#carla.WalkerControl)_)  

---

## command.ApplyWalkerState<a name="command.ApplyWalkerState"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.ApplyWalkerState.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.ApplyWalkerState.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
- <a name="command.ApplyWalkerState.speed"></a>**<font color="#f8805a">speed</font>** (_float_)  

<h3>Methods</h3>
- <a name="command.ApplyWalkerState.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**speed**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  
        - `speed` (_float_)  

---

## command.DestroyActor<a name="command.DestroyActor"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.DestroyActor.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  

<h3>Methods</h3>
- <a name="command.DestroyActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  

---

## command.Response<a name="command.Response"></a> <sub><sup>_class_</sup></sub>
Execution result of a command, contains either an error string or and actor ID, depending on whether or not the command succeeded. See `client.apply_batch_sync()`.  

<h3>Instance Variables</h3>
- <a name="command.Response.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.Response.error"></a>**<font color="#f8805a">error</font>** (_str_)  

<h3>Methods</h3>
- <a name="command.Response.has_error"></a>**<font color="#7fb800">has_error</font>**(<font color="#00a6ed">**self**</font>)  

---

## command.SetAutopilot<a name="command.SetAutopilot"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.SetAutopilot.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.SetAutopilot.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  

<h3>Methods</h3>
- <a name="command.SetAutopilot.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `enabled` (_bool_)  

---

## command.SetSimulatePhysics<a name="command.SetSimulatePhysics"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.SetSimulatePhysics.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
- <a name="command.SetSimulatePhysics.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  

<h3>Methods</h3>
- <a name="command.SetSimulatePhysics.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `enabled` (_bool_)  

---

## command.SpawnActor<a name="command.SpawnActor"></a> <sub><sup>_class_</sup></sub>

<h3>Instance Variables</h3>
- <a name="command.SpawnActor.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
- <a name="command.SpawnActor.parent_id"></a>**<font color="#f8805a">parent_id</font>** (_int_)  

<h3>Methods</h3>
- <a name="command.SpawnActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="command.SpawnActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>)  
    - **Parameters:**
        - `blueprint` (_[carla.ActorBlueprint](#carla.ActorBlueprint)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  
- <a name="command.SpawnActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**parent**</font>)  
    - **Parameters:**
        - `blueprint` (_[carla.ActorBlueprint](#carla.ActorBlueprint)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  
        - `parent` (_[carla.Actor](#carla.Actor) or int_)  
- <a name="command.SpawnActor.then"></a>**<font color="#7fb800">then</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**command**</font>)  
    - **Parameters:**
        - `command` (_[carla.Command](#carla.Command)_) – CommandType.  

---