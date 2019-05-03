### Overview
- [**carla**](#carla) <sub>_Module_</sub>  
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
    - [**Walker**](#carla.Walker) <sub>_Class_</sub>  
        - [**bounding_box**](#carla.Walker.bounding_box) <sub>_Instance variable_</sub>
        - [**apply_control**(**self**)](#carla.Walker.apply_control) <sub>_Method_</sub>
        - [**get_control**(**self**)](#carla.Walker.get_control) <sub>_Method_</sub>
    - [**TrafficSign**](#carla.TrafficSign) <sub>_Class_</sub>  
        - [**trigger_volume**](#carla.TrafficSign.trigger_volume) <sub>_Instance variable_</sub>
    - [**TrafficLightState**](#carla.TrafficLightState) <sub>_Class_</sub>  
        - [**Red**](#carla.TrafficLightState.Red) <sub>_Instance variable_</sub>
        - [**Yellow**](#carla.TrafficLightState.Yellow) <sub>_Instance variable_</sub>
        - [**Green**](#carla.TrafficLightState.Green) <sub>_Instance variable_</sub>
        - [**Off**](#carla.TrafficLightState.Off) <sub>_Instance variable_</sub>
        - [**Unknown**](#carla.TrafficLightState.Unknown) <sub>_Instance variable_</sub>
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
    - [**WalkerControl**](#carla.WalkerControl) <sub>_Class_</sub>  
        - [**direction**](#carla.WalkerControl.direction) <sub>_Instance variable_</sub>
        - [**speed**](#carla.WalkerControl.speed) <sub>_Instance variable_</sub>
        - [**jump**](#carla.WalkerControl.jump) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#carla.WalkerControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.WalkerControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.WalkerControl.__ne__) <sub>_Method_</sub>
    - [**WheelPhysicsControl**](#carla.WheelPhysicsControl) <sub>_Class_</sub>  
        - [**tire_friction**](#carla.WheelPhysicsControl.tire_friction) <sub>_Instance variable_</sub>
        - [**damping_rate**](#carla.WheelPhysicsControl.damping_rate) <sub>_Instance variable_</sub>
        - [**steer_angle**](#carla.WheelPhysicsControl.steer_angle) <sub>_Instance variable_</sub>
        - [**disable_steering**](#carla.WheelPhysicsControl.disable_steering) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#carla.WheelPhysicsControl.__init__) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.WheelPhysicsControl.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.WheelPhysicsControl.__ne__) <sub>_Method_</sub>
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
    - [**LaneChange**](#carla.LaneChange) <sub>_Class_</sub>  
        - [**NONE**](#carla.LaneChange.NONE) <sub>_Instance variable_</sub>
        - [**Right**](#carla.LaneChange.Right) <sub>_Instance variable_</sub>
        - [**Left**](#carla.LaneChange.Left) <sub>_Instance variable_</sub>
        - [**Both**](#carla.LaneChange.Both) <sub>_Instance variable_</sub>
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
    - [**Map**](#carla.Map) <sub>_Class_</sub>  
        - [**name**](#carla.Map.name) <sub>_Instance variable_</sub>
        - [**get_spawn_points**(**self**)](#carla.Map.get_spawn_points) <sub>_Method_</sub>
        - [**get_waypoint**(**self**)](#carla.Map.get_waypoint) <sub>_Method_</sub>
        - [**get_topology**(**self**)](#carla.Map.get_topology) <sub>_Method_</sub>
        - [**generate_waypoints**(**self**)](#carla.Map.generate_waypoints) <sub>_Method_</sub>
        - [**transform_to_geolocation**(**self**)](#carla.Map.transform_to_geolocation) <sub>_Method_</sub>
        - [**to_opendrive**(**self**)](#carla.Map.to_opendrive) <sub>_Method_</sub>
        - [**save_to_disk**(**self**, **path**)](#carla.Map.save_to_disk) <sub>_Method_</sub>
    - [**LaneMarking**](#carla.LaneMarking) <sub>_Class_</sub>  
        - [**type**](#carla.LaneMarking.type) <sub>_Instance variable_</sub>
        - [**color**](#carla.LaneMarking.color) <sub>_Instance variable_</sub>
        - [**lane_change**](#carla.LaneMarking.lane_change) <sub>_Instance variable_</sub>
        - [**width**](#carla.LaneMarking.width) <sub>_Instance variable_</sub>
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
    - [**ActorAttributeType**](#carla.ActorAttributeType) <sub>_Class_</sub>  
        - [**Bool**](#carla.ActorAttributeType.Bool) <sub>_Instance variable_</sub>
        - [**Int**](#carla.ActorAttributeType.Int) <sub>_Instance variable_</sub>
        - [**Float**](#carla.ActorAttributeType.Float) <sub>_Instance variable_</sub>
        - [**String**](#carla.ActorAttributeType.String) <sub>_Instance variable_</sub>
        - [**RGBColor**](#carla.ActorAttributeType.RGBColor) <sub>_Instance variable_</sub>
    - [**Color**](#carla.Color) <sub>_Class_</sub>  
        - [**r**](#carla.Color.r) <sub>_Instance variable_</sub>
        - [**g**](#carla.Color.g) <sub>_Instance variable_</sub>
        - [**b**](#carla.Color.b) <sub>_Instance variable_</sub>
        - [**a**](#carla.Color.a) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**, **other**)](#carla.Color.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**, **other**)](#carla.Color.__ne__) <sub>_Method_</sub>
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
    - [**BlueprintLibrary**](#carla.BlueprintLibrary) <sub>_Class_</sub>  
        - [**find**(**self**)](#carla.BlueprintLibrary.find) <sub>_Method_</sub>
        - [**filter**(**self**)](#carla.BlueprintLibrary.filter) <sub>_Method_</sub>
        - [**\__getitem__**(**self**)](#carla.BlueprintLibrary.__getitem__) <sub>_Method_</sub>
        - [**\__len__**(**self**)](#carla.BlueprintLibrary.__len__) <sub>_Method_</sub>
        - [**\__iter__**(**self**)](#carla.BlueprintLibrary.__iter__) <sub>_Method_</sub>
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
    - [**Location**](#carla.Location) <sub>_Class_</sub>  
        - [**x**](#carla.Location.x) <sub>_Instance variable_</sub>
        - [**y**](#carla.Location.y) <sub>_Instance variable_</sub>
        - [**z**](#carla.Location.z) <sub>_Instance variable_</sub>
        - [**distance**(**self**)](#carla.Location.distance) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.Location.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Location.__ne__) <sub>_Method_</sub>
    - [**Rotation**](#carla.Rotation) <sub>_Class_</sub>  
        - [**pitch**](#carla.Rotation.pitch) <sub>_Instance variable_</sub>
        - [**yaw**](#carla.Rotation.yaw) <sub>_Instance variable_</sub>
        - [**roll**](#carla.Rotation.roll) <sub>_Instance variable_</sub>
        - [**get_forward_vector**(**self**)](#carla.Rotation.get_forward_vector) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.Rotation.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Rotation.__ne__) <sub>_Method_</sub>
    - [**Transform**](#carla.Transform) <sub>_Class_</sub>  
        - [**location**](#carla.Transform.location) <sub>_Instance variable_</sub>
        - [**rotation**](#carla.Transform.rotation) <sub>_Instance variable_</sub>
        - [**transform**(**self**)](#carla.Transform.transform) <sub>_Method_</sub>
        - [**get_forward_vector**(**self**)](#carla.Transform.get_forward_vector) <sub>_Method_</sub>
        - [**\__eq__**(**self**)](#carla.Transform.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.Transform.__ne__) <sub>_Method_</sub>
    - [**BoundingBox**](#carla.BoundingBox) <sub>_Class_</sub>  
        - [**location**](#carla.BoundingBox.location) <sub>_Instance variable_</sub>
        - [**extent**](#carla.BoundingBox.extent) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.BoundingBox.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.BoundingBox.__ne__) <sub>_Method_</sub>
    - [**GeoLocation**](#carla.GeoLocation) <sub>_Class_</sub>  
        - [**latitude**](#carla.GeoLocation.latitude) <sub>_Instance variable_</sub>
        - [**longitude**](#carla.GeoLocation.longitude) <sub>_Instance variable_</sub>
        - [**altitude**](#carla.GeoLocation.altitude) <sub>_Instance variable_</sub>
        - [**\__eq__**(**self**)](#carla.GeoLocation.__eq__) <sub>_Method_</sub>
        - [**\__ne__**(**self**)](#carla.GeoLocation.__ne__) <sub>_Method_</sub>
- [**commands**](#commands) <sub>_Module_</sub>  
    - [**Response**](#commands.Response) <sub>_Class_</sub>  
        - [**actor_id**](#commands.Response.actor_id) <sub>_Instance variable_</sub>
        - [**error**](#commands.Response.error) <sub>_Instance variable_</sub>
        - [**has_error**(**self**)](#commands.Response.has_error) <sub>_Method_</sub>
    - [**SpawnActor**](#commands.SpawnActor) <sub>_Class_</sub>  
        - [**transform**](#commands.SpawnActor.transform) <sub>_Instance variable_</sub>
        - [**parent_id**](#commands.SpawnActor.parent_id) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.SpawnActor.__init__) <sub>_Method_</sub>
        - [**then**(**self**)](#commands.SpawnActor.then) <sub>_Method_</sub>
    - [**DestroyActor**](#commands.DestroyActor) <sub>_Class_</sub>  
        - [**actor_id**](#commands.DestroyActor.actor_id) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.DestroyActor.__init__) <sub>_Method_</sub>
    - [**ApplyVehicleControl**](#commands.ApplyVehicleControl) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyVehicleControl.actor_id) <sub>_Instance variable_</sub>
        - [**control**](#commands.ApplyVehicleControl.control) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyVehicleControl.__init__) <sub>_Method_</sub>
    - [**ApplyWalkerControl**](#commands.ApplyWalkerControl) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyWalkerControl.actor_id) <sub>_Instance variable_</sub>
        - [**control**](#commands.ApplyWalkerControl.control) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyWalkerControl.__init__) <sub>_Method_</sub>
    - [**ApplyTransform**](#commands.ApplyTransform) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyTransform.actor_id) <sub>_Instance variable_</sub>
        - [**transform**](#commands.ApplyTransform.transform) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyTransform.__init__) <sub>_Method_</sub>
    - [**ApplyVelocity**](#commands.ApplyVelocity) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyVelocity.actor_id) <sub>_Instance variable_</sub>
        - [**velocity**](#commands.ApplyVelocity.velocity) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyVelocity.__init__) <sub>_Method_</sub>
    - [**ApplyAngularVelocity**](#commands.ApplyAngularVelocity) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyAngularVelocity.actor_id) <sub>_Instance variable_</sub>
        - [**angular_velocity**](#commands.ApplyAngularVelocity.angular_velocity) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyAngularVelocity.__init__) <sub>_Method_</sub>
    - [**ApplyImpulse**](#commands.ApplyImpulse) <sub>_Class_</sub>  
        - [**actor_id**](#commands.ApplyImpulse.actor_id) <sub>_Instance variable_</sub>
        - [**impulse**](#commands.ApplyImpulse.impulse) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.ApplyImpulse.__init__) <sub>_Method_</sub>
    - [**SetSimulatePhysics**](#commands.SetSimulatePhysics) <sub>_Class_</sub>  
        - [**actor_id**](#commands.SetSimulatePhysics.actor_id) <sub>_Instance variable_</sub>
        - [**enabled**](#commands.SetSimulatePhysics.enabled) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.SetSimulatePhysics.__init__) <sub>_Method_</sub>
    - [**SetAutopilot**](#commands.SetAutopilot) <sub>_Class_</sub>  
        - [**actor_id**](#commands.SetAutopilot.actor_id) <sub>_Instance variable_</sub>
        - [**enabled**](#commands.SetAutopilot.enabled) <sub>_Instance variable_</sub>
        - [**\__init__**(**self**)](#commands.SetAutopilot.__init__) <sub>_Method_</sub>

## <a name="carla.Actor"></a>Actor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Actor.id"></a>**id**  
- <a name="carla.Actor.type_id"></a>**type_id**  
- <a name="carla.Actor.parent"></a>**parent**  
- <a name="carla.Actor.semantic_tags"></a>**semantic_tags**  
- <a name="carla.Actor.is_alive"></a>**is_alive**  
- <a name="carla.Actor.attributes"></a>**attributes**  

### Methods
- <a name="carla.Actor.get_world"></a>**<font color="#64BA2E">get_world</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.get_location"></a>**<font color="#64BA2E">get_location</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.get_transform"></a>**<font color="#64BA2E">get_transform</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.get_velocity"></a>**<font color="#64BA2E">get_velocity</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.get_angular_velocity"></a>**<font color="#64BA2E">get_angular_velocity</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.get_acceleration"></a>**<font color="#64BA2E">get_acceleration</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.set_location"></a>**<font color="#64BA2E">set_location</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.set_transform"></a>**<font color="#64BA2E">set_transform</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.set_velocity"></a>**<font color="#64BA2E">set_velocity</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.set_angular_velocity"></a>**<font color="#64BA2E">set_angular_velocity</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.add_impulse"></a>**<font color="#64BA2E">add_impulse</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.set_simulate_physics"></a>**<font color="#64BA2E">set_simulate_physics</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Actor.destroy"></a>**<font color="#64BA2E">destroy</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Vehicle"></a>Vehicle <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Vehicle.bounding_box"></a>**bounding_box**  

### Methods
- <a name="carla.Vehicle.apply_control"></a>**<font color="#64BA2E">apply_control</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.get_control"></a>**<font color="#64BA2E">get_control</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.apply_physics_control"></a>**<font color="#64BA2E">apply_physics_control</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.get_physics_control"></a>**<font color="#64BA2E">get_physics_control</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.set_autopilot"></a>**<font color="#64BA2E">set_autopilot</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.get_speed_limit"></a>**<font color="#64BA2E">get_speed_limit</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.get_traffic_light_state"></a>**<font color="#64BA2E">get_traffic_light_state</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.is_at_traffic_light"></a>**<font color="#64BA2E">is_at_traffic_light</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vehicle.get_traffic_light"></a>**<font color="#64BA2E">get_traffic_light</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Walker"></a>Walker <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Walker.bounding_box"></a>**bounding_box**  

### Methods
- <a name="carla.Walker.apply_control"></a>**<font color="#64BA2E">apply_control</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Walker.get_control"></a>**<font color="#64BA2E">get_control</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.TrafficSign"></a>TrafficSign <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.TrafficSign.trigger_volume"></a>**trigger_volume**  

---

## <a name="carla.TrafficLightState"></a>TrafficLightState <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.TrafficLightState.Red"></a>**Red**  
- <a name="carla.TrafficLightState.Yellow"></a>**Yellow**  
- <a name="carla.TrafficLightState.Green"></a>**Green**  
- <a name="carla.TrafficLightState.Off"></a>**Off**  
- <a name="carla.TrafficLightState.Unknown"></a>**Unknown**  

---

## <a name="carla.TrafficLight"></a>TrafficLight <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.TrafficLight.state"></a>**state**  

### Methods
- <a name="carla.TrafficLight.set_state"></a>**<font color="#64BA2E">set_state</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.get_state"></a>**<font color="#64BA2E">get_state</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.set_green_time"></a>**<font color="#64BA2E">set_green_time</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.get_green_time"></a>**<font color="#64BA2E">get_green_time</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.set_yellow_time"></a>**<font color="#64BA2E">set_yellow_time</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.get_yellow_time"></a>**<font color="#64BA2E">get_yellow_time</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.set_red_time"></a>**<font color="#64BA2E">set_red_time</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.get_red_time"></a>**<font color="#64BA2E">get_red_time</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.get_elapsed_time"></a>**<font color="#64BA2E">get_elapsed_time</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.freeze"></a>**<font color="#64BA2E">freeze</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.is_frozen"></a>**<font color="#64BA2E">is_frozen</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.get_pole_index"></a>**<font color="#64BA2E">get_pole_index</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.TrafficLight.get_group_traffic_lights"></a>**<font color="#64BA2E">get_group_traffic_lights</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.VehicleControl"></a>VehicleControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.VehicleControl.throttle"></a>**throttle**  
- <a name="carla.VehicleControl.steer"></a>**steer**  
- <a name="carla.VehicleControl.brake"></a>**brake**  
- <a name="carla.VehicleControl.hand_brake"></a>**hand_brake**  
- <a name="carla.VehicleControl.reverse"></a>**reverse**  
- <a name="carla.VehicleControl.manual_gear_shift"></a>**manual_gear_shift**  
- <a name="carla.VehicleControl.gear"></a>**gear**  

### Methods
- <a name="carla.VehicleControl.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.VehicleControl.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.VehicleControl.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.WalkerControl"></a>WalkerControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.WalkerControl.direction"></a>**direction**  
- <a name="carla.WalkerControl.speed"></a>**speed**  
- <a name="carla.WalkerControl.jump"></a>**jump**  

### Methods
- <a name="carla.WalkerControl.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.WalkerControl.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.WalkerControl.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.WheelPhysicsControl"></a>WheelPhysicsControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.WheelPhysicsControl.tire_friction"></a>**tire_friction**  
- <a name="carla.WheelPhysicsControl.damping_rate"></a>**damping_rate**  
- <a name="carla.WheelPhysicsControl.steer_angle"></a>**steer_angle**  
- <a name="carla.WheelPhysicsControl.disable_steering"></a>**disable_steering**  

### Methods
- <a name="carla.WheelPhysicsControl.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.WheelPhysicsControl.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.WheelPhysicsControl.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.VehiclePhysicsControl"></a>VehiclePhysicsControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.VehiclePhysicsControl.torque_curve"></a>**torque_curve**  
- <a name="carla.VehiclePhysicsControl.max_rpm"></a>**max_rpm**  
- <a name="carla.VehiclePhysicsControl.moi"></a>**moi**  
- <a name="carla.VehiclePhysicsControl.damping_rate_full_throttle"></a>**damping_rate_full_throttle**  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_engaged"></a>**damping_rate_zero_throttle_clutch_engaged**  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_disengaged"></a>**damping_rate_zero_throttle_clutch_disengaged**  
- <a name="carla.VehiclePhysicsControl.use_gear_autobox"></a>**use_gear_autobox**  
- <a name="carla.VehiclePhysicsControl.gear_switch_time"></a>**gear_switch_time**  
- <a name="carla.VehiclePhysicsControl.clutch_strength"></a>**clutch_strength**  
- <a name="carla.VehiclePhysicsControl.mass"></a>**mass**  
- <a name="carla.VehiclePhysicsControl.drag_coefficient"></a>**drag_coefficient**  
- <a name="carla.VehiclePhysicsControl.center_of_mass"></a>**center_of_mass**  
- <a name="carla.VehiclePhysicsControl.steering_curve"></a>**steering_curve**  
- <a name="carla.VehiclePhysicsControl.wheels"></a>**wheels**  

### Methods
- <a name="carla.VehiclePhysicsControl.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.VehiclePhysicsControl.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.VehiclePhysicsControl.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Client"></a>Client <sub><sup>_Class_</sup></sub>

### Methods
- <a name="carla.Client.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**host**</font>, <font color="#2980B9">**port**</font>, <font color="#2980B9">**worker_threads**=0</font>)  
Client constructor.  
    - **Parameters:**
        - `host` (_str_) – IP where Carla is running.  
        - `port` (_int_) – Port where Carla is running.  
        - `worker_threads` (_int_) – Number of working threads.  
- <a name="carla.Client.set_timeout"></a>**<font color="#64BA2E">set_timeout</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**seconds**</font>)  
Sets the server timeout in seconds.  
    - **Parameters:**
        - `seconds` (_float_) – New timeout value in seconds.  
- <a name="carla.Client.get_client_version"></a>**<font color="#64BA2E">get_client_version</font>**(<font color="#2980B9">**self**</font>)  
Get the client version as a string.  
- <a name="carla.Client.get_server_version"></a>**<font color="#64BA2E">get_server_version</font>**(<font color="#2980B9">**self**</font>)  
Get the server version as a string.  
- <a name="carla.Client.get_world"></a>**<font color="#64BA2E">get_world</font>**(<font color="#2980B9">**self**</font>)  
Get the server version as a string.  
- <a name="carla.Client.get_available_maps"></a>**<font color="#64BA2E">get_available_maps</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.reload_world"></a>**<font color="#64BA2E">reload_world</font>**(<font color="#2980B9">**self**</font>)  
    - **Raises:** RuntimeError  
- <a name="carla.Client.load_world"></a>**<font color="#64BA2E">load_world</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**map_name**</font>)  
    - **Parameters:**
        - `map_name` (_str_) – Name of the map to load.  
- <a name="carla.Client.start_recorder"></a>**<font color="#64BA2E">start_recorder</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**filename**</font>)  
If we use a simple name like 'recording.log' then it will be saved at server folder 'CarlaUE4/Saved/recording.log'. If we use some folder in the name, then it will be considered to be an absolute path, like '/home/carla/recording.log'.  
    - **Parameters:**
        - `filename` (_str_) – Name of the file to create.  
- <a name="carla.Client.stop_recorder"></a>**<font color="#64BA2E">stop_recorder</font>**(<font color="#2980B9">**self**</font>)  
Stops the recording in curse.  
- <a name="carla.Client.show_recorder_file_info"></a>**<font color="#64BA2E">show_recorder_file_info</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**filename**</font>, <font color="#2980B9">**show_all**=False</font>)  
Will show info about the recorded file. We have the option to show all the details per frame, that includes all the traffic light states, position of all actors, and animations data.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `show_all` (_bool_) – Show all detailed info, or just a summary.  
- <a name="carla.Client.show_recorder_collisions"></a>**<font color="#64BA2E">show_recorder_collisions</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**filename**</font>, <font color="#2980B9">**category1**='a'</font>, <font color="#2980B9">**category2**='a'</font>)  
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
- <a name="carla.Client.show_recorder_actors_blocked"></a>**<font color="#64BA2E">show_recorder_actors_blocked</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**filename**</font>, <font color="#2980B9">**min_time**=60.0</font>, <font color="#2980B9">**min_distance**=100.0</font>)  
Shows which actors seems blocked by some reason. The idea is to calculate which actors are not moving as much as 'min_distance' for a period of 'min_time'. By default min_time = 60 seconds (1 min) and min_distance = 100 centimeters (1 m).  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `min_time` (_float_) – How many seconds has to be stoped an actor to be considered as blocked.  
        - `min_distance` (_float_) – How many centimeters needs to displace an actor in order to not be considered as blocked.  
- <a name="carla.Client.replay_file"></a>**<font color="#64BA2E">replay_file</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**filename**</font>, <font color="#2980B9">**start**=0.0</font>, <font color="#2980B9">**duration**=0.0</font>, <font color="#2980B9">**camera**=0</font>)  
Playback a file.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to play.  
        - `start` (_float_) – Time in seconds where to start the playback. If it is negative, then it starts from the end.  
        - `duration` (_float_) – Time of playback, after that time the playback stops and all the actors are left driving in autopilot. A value of 0 means playback until the end.  
        - `camera` (_int_) – Id of the actor to follow. If this is 0 then camera is disabled.  
- <a name="carla.Client.set_replayer_time_factor"></a>**<font color="#64BA2E">set_replayer_time_factor</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**time_factor**</font>)  
Apply a different playback speed to current playback. Can be used several times while a playback is in curse.  
    - **Parameters:**
        - `time_factor` (_float_) – These values means:  
  \- A value of 1.0 means normal time factor.  
  \- A value < 1.0 means slow motion (for example 0.5 is half speed)  
  \- A value > 1.0 means fast motion (for example 2.0 is double speed).  
- <a name="carla.Client.apply_batch"></a>**<font color="#64BA2E">apply_batch</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.apply_batch_sync"></a>**<font color="#64BA2E">apply_batch_sync</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.LaneType"></a>LaneType <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneType.NONE"></a>**NONE**  
- <a name="carla.LaneType.Driving"></a>**Driving**  
- <a name="carla.LaneType.Stop"></a>**Stop**  
- <a name="carla.LaneType.Shoulder"></a>**Shoulder**  
- <a name="carla.LaneType.Biking"></a>**Biking**  
- <a name="carla.LaneType.Sidewalk"></a>**Sidewalk**  
- <a name="carla.LaneType.Border"></a>**Border**  
- <a name="carla.LaneType.Restricted"></a>**Restricted**  
- <a name="carla.LaneType.Parking"></a>**Parking**  
- <a name="carla.LaneType.Bidirectional"></a>**Bidirectional**  
- <a name="carla.LaneType.Median"></a>**Median**  
- <a name="carla.LaneType.Special1"></a>**Special1**  
- <a name="carla.LaneType.Special2"></a>**Special2**  
- <a name="carla.LaneType.Special3"></a>**Special3**  
- <a name="carla.LaneType.RoadWorks"></a>**RoadWorks**  
- <a name="carla.LaneType.Tram"></a>**Tram**  
- <a name="carla.LaneType.Rail"></a>**Rail**  
- <a name="carla.LaneType.Entry"></a>**Entry**  
- <a name="carla.LaneType.Exit"></a>**Exit**  
- <a name="carla.LaneType.OffRamp"></a>**OffRamp**  
- <a name="carla.LaneType.OnRamp"></a>**OnRamp**  
- <a name="carla.LaneType.Any"></a>**Any**  

---

## <a name="carla.LaneChange"></a>LaneChange <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneChange.NONE"></a>**NONE**  
- <a name="carla.LaneChange.Right"></a>**Right**  
- <a name="carla.LaneChange.Left"></a>**Left**  
- <a name="carla.LaneChange.Both"></a>**Both**  

---

## <a name="carla.LaneMarkingColor"></a>LaneMarkingColor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneMarkingColor.Standard"></a>**Standard**  
- <a name="carla.LaneMarkingColor.Blue"></a>**Blue**  
- <a name="carla.LaneMarkingColor.Green"></a>**Green**  
- <a name="carla.LaneMarkingColor.Red"></a>**Red**  
- <a name="carla.LaneMarkingColor.White"></a>**White**  
- <a name="carla.LaneMarkingColor.Yellow"></a>**Yellow**  
- <a name="carla.LaneMarkingColor.Other"></a>**Other**  

---

## <a name="carla.LaneMarkingType"></a>LaneMarkingType <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneMarkingType.NONE"></a>**NONE**  
- <a name="carla.LaneMarkingType.Other"></a>**Other**  
- <a name="carla.LaneMarkingType.Broken"></a>**Broken**  
- <a name="carla.LaneMarkingType.Solid"></a>**Solid**  
- <a name="carla.LaneMarkingType.SolidSolid"></a>**SolidSolid**  
- <a name="carla.LaneMarkingType.SolidBroken"></a>**SolidBroken**  
- <a name="carla.LaneMarkingType.BrokenSolid"></a>**BrokenSolid**  
- <a name="carla.LaneMarkingType.BrokenBroken"></a>**BrokenBroken**  
- <a name="carla.LaneMarkingType.BottsDots"></a>**BottsDots**  
- <a name="carla.LaneMarkingType.Grass"></a>**Grass**  
- <a name="carla.LaneMarkingType.Curb"></a>**Curb**  

---

## <a name="carla.Map"></a>Map <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Map.name"></a>**name**  
Map name.  

### Methods
- <a name="carla.Map.get_spawn_points"></a>**<font color="#64BA2E">get_spawn_points</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.get_waypoint"></a>**<font color="#64BA2E">get_waypoint</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.get_topology"></a>**<font color="#64BA2E">get_topology</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.generate_waypoints"></a>**<font color="#64BA2E">generate_waypoints</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.transform_to_geolocation"></a>**<font color="#64BA2E">transform_to_geolocation</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.to_opendrive"></a>**<font color="#64BA2E">to_opendrive</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.save_to_disk"></a>**<font color="#64BA2E">save_to_disk</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**path**</font>)  
Save the OpenDrive of the current map to disk.  
    - **Parameters:**
        - `path` – Path where will be saved.  

---

## <a name="carla.LaneMarking"></a>LaneMarking <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.LaneMarking.type"></a>**type**  
- <a name="carla.LaneMarking.color"></a>**color**  
- <a name="carla.LaneMarking.lane_change"></a>**lane_change**  
- <a name="carla.LaneMarking.width"></a>**width**  

---

## <a name="carla.Waypoint"></a>Waypoint <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Waypoint.id"></a>**id**  
- <a name="carla.Waypoint.transform"></a>**transform**  
- <a name="carla.Waypoint.is_intersection"></a>**is_intersection**  
- <a name="carla.Waypoint.lane_width"></a>**lane_width**  
- <a name="carla.Waypoint.road_id"></a>**road_id**  
- <a name="carla.Waypoint.section_id"></a>**section_id**  
- <a name="carla.Waypoint.lane_id"></a>**lane_id**  
- <a name="carla.Waypoint.s"></a>**s**  
- <a name="carla.Waypoint.lane_change"></a>**lane_change**  
- <a name="carla.Waypoint.lane_type"></a>**lane_type**  
- <a name="carla.Waypoint.right_lane_marking"></a>**right_lane_marking**  
- <a name="carla.Waypoint.left_lane_marking"></a>**left_lane_marking**  

### Methods
- <a name="carla.Waypoint.next"></a>**<font color="#64BA2E">next</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Waypoint.get_right_lane"></a>**<font color="#64BA2E">get_right_lane</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Waypoint.get_left_lane"></a>**<font color="#64BA2E">get_left_lane</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.ActorAttributeType"></a>ActorAttributeType <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ActorAttributeType.Bool"></a>**Bool**  
- <a name="carla.ActorAttributeType.Int"></a>**Int**  
- <a name="carla.ActorAttributeType.Float"></a>**Float**  
- <a name="carla.ActorAttributeType.String"></a>**String**  
- <a name="carla.ActorAttributeType.RGBColor"></a>**RGBColor**  

---

## <a name="carla.Color"></a>Color <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Color.r"></a>**r**  
- <a name="carla.Color.g"></a>**g**  
- <a name="carla.Color.b"></a>**b**  
- <a name="carla.Color.a"></a>**a**  

### Methods
- <a name="carla.Color.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**other**</font>)  
    - **Parameters:**
        - `other`  
- <a name="carla.Color.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**other**</font>)  
    - **Parameters:**
        - `other`  

---

## <a name="carla.ActorAttribute"></a>ActorAttribute <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ActorAttribute.id"></a>**id**  
- <a name="carla.ActorAttribute.type"></a>**type**  
- <a name="carla.ActorAttribute.recommended_values"></a>**recommended_values**  
- <a name="carla.ActorAttribute.is_modifiable"></a>**is_modifiable**  

### Methods
- <a name="carla.ActorAttribute.as_bool"></a>**<font color="#64BA2E">as_bool</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.as_int"></a>**<font color="#64BA2E">as_int</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.as_float"></a>**<font color="#64BA2E">as_float</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.as_str"></a>**<font color="#64BA2E">as_str</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.as_color"></a>**<font color="#64BA2E">as_color</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__nonzero__"></a>**<font color="#64BA2E">\__nonzero__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__bool__"></a>**<font color="#64BA2E">\__bool__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__int__"></a>**<font color="#64BA2E">\__int__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__float__"></a>**<font color="#64BA2E">\__float__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorAttribute.__str__"></a>**<font color="#64BA2E">\__str__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.ActorBlueprint"></a>ActorBlueprint <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.ActorBlueprint.id"></a>**id**  
- <a name="carla.ActorBlueprint.tags"></a>**tags**  

### Methods
- <a name="carla.ActorBlueprint.has_tag"></a>**<font color="#64BA2E">has_tag</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorBlueprint.match_tags"></a>**<font color="#64BA2E">match_tags</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorBlueprint.has_attribute"></a>**<font color="#64BA2E">has_attribute</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorBlueprint.get_attribute"></a>**<font color="#64BA2E">get_attribute</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorBlueprint.set_attribute"></a>**<font color="#64BA2E">set_attribute</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorBlueprint.__len__"></a>**<font color="#64BA2E">\__len__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.ActorBlueprint.__iter__"></a>**<font color="#64BA2E">\__iter__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.BlueprintLibrary"></a>BlueprintLibrary <sub><sup>_Class_</sup></sub>

### Methods
- <a name="carla.BlueprintLibrary.find"></a>**<font color="#64BA2E">find</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.BlueprintLibrary.filter"></a>**<font color="#64BA2E">filter</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.BlueprintLibrary.__getitem__"></a>**<font color="#64BA2E">\__getitem__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.BlueprintLibrary.__len__"></a>**<font color="#64BA2E">\__len__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.BlueprintLibrary.__iter__"></a>**<font color="#64BA2E">\__iter__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Vector2D"></a>Vector2D <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Vector2D.x"></a>**x**  
- <a name="carla.Vector2D.y"></a>**y**  

### Methods
- <a name="carla.Vector2D.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vector2D.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Vector3D"></a>Vector3D <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Vector3D.x"></a>**x**  
- <a name="carla.Vector3D.y"></a>**y**  
- <a name="carla.Vector3D.z"></a>**z**  

### Methods
- <a name="carla.Vector3D.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Vector3D.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Location"></a>Location <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Location.x"></a>**x**  
- <a name="carla.Location.y"></a>**y**  
- <a name="carla.Location.z"></a>**z**  

### Methods
- <a name="carla.Location.distance"></a>**<font color="#64BA2E">distance</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Location.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Location.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Rotation"></a>Rotation <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Rotation.pitch"></a>**pitch**  
- <a name="carla.Rotation.yaw"></a>**yaw**  
- <a name="carla.Rotation.roll"></a>**roll**  

### Methods
- <a name="carla.Rotation.get_forward_vector"></a>**<font color="#64BA2E">get_forward_vector</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Rotation.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Rotation.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.Transform"></a>Transform <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Transform.location"></a>**location**  
- <a name="carla.Transform.rotation"></a>**rotation**  

### Methods
- <a name="carla.Transform.transform"></a>**<font color="#64BA2E">transform</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Transform.get_forward_vector"></a>**<font color="#64BA2E">get_forward_vector</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Transform.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Transform.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.BoundingBox"></a>BoundingBox <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.BoundingBox.location"></a>**location**  
- <a name="carla.BoundingBox.extent"></a>**extent**  

### Methods
- <a name="carla.BoundingBox.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.BoundingBox.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="carla.GeoLocation"></a>GeoLocation <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.GeoLocation.latitude"></a>**latitude**  
- <a name="carla.GeoLocation.longitude"></a>**longitude**  
- <a name="carla.GeoLocation.altitude"></a>**altitude**  

### Methods
- <a name="carla.GeoLocation.__eq__"></a>**<font color="#64BA2E">\__eq__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.GeoLocation.__ne__"></a>**<font color="#64BA2E">\__ne__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.Response"></a>Response <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.Response.actor_id"></a>**actor_id**  
- <a name="commands.Response.error"></a>**error**  

### Methods
- <a name="commands.Response.has_error"></a>**<font color="#64BA2E">has_error</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.SpawnActor"></a>SpawnActor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.SpawnActor.transform"></a>**transform**  
- <a name="commands.SpawnActor.parent_id"></a>**parent_id**  

### Methods
- <a name="commands.SpawnActor.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  
- <a name="commands.SpawnActor.then"></a>**<font color="#64BA2E">then</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.DestroyActor"></a>DestroyActor <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.DestroyActor.actor_id"></a>**actor_id**  

### Methods
- <a name="commands.DestroyActor.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.ApplyVehicleControl"></a>ApplyVehicleControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyVehicleControl.actor_id"></a>**actor_id**  
- <a name="commands.ApplyVehicleControl.control"></a>**control**  

### Methods
- <a name="commands.ApplyVehicleControl.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.ApplyWalkerControl"></a>ApplyWalkerControl <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyWalkerControl.actor_id"></a>**actor_id**  
- <a name="commands.ApplyWalkerControl.control"></a>**control**  

### Methods
- <a name="commands.ApplyWalkerControl.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.ApplyTransform"></a>ApplyTransform <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyTransform.actor_id"></a>**actor_id**  
- <a name="commands.ApplyTransform.transform"></a>**transform**  

### Methods
- <a name="commands.ApplyTransform.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.ApplyVelocity"></a>ApplyVelocity <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyVelocity.actor_id"></a>**actor_id**  
- <a name="commands.ApplyVelocity.velocity"></a>**velocity**  

### Methods
- <a name="commands.ApplyVelocity.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.ApplyAngularVelocity"></a>ApplyAngularVelocity <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyAngularVelocity.actor_id"></a>**actor_id**  
- <a name="commands.ApplyAngularVelocity.angular_velocity"></a>**angular_velocity**  

### Methods
- <a name="commands.ApplyAngularVelocity.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.ApplyImpulse"></a>ApplyImpulse <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.ApplyImpulse.actor_id"></a>**actor_id**  
- <a name="commands.ApplyImpulse.impulse"></a>**impulse**  

### Methods
- <a name="commands.ApplyImpulse.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.SetSimulatePhysics"></a>SetSimulatePhysics <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.SetSimulatePhysics.actor_id"></a>**actor_id**  
- <a name="commands.SetSimulatePhysics.enabled"></a>**enabled**  

### Methods
- <a name="commands.SetSimulatePhysics.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---

## <a name="commands.SetAutopilot"></a>SetAutopilot <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="commands.SetAutopilot.actor_id"></a>**actor_id**  
- <a name="commands.SetAutopilot.enabled"></a>**enabled**  

### Methods
- <a name="commands.SetAutopilot.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>)  

---