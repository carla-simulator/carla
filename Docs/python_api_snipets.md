[comment]: <> (=========================)
[comment]: <> (PYTHON API SCRIPT SNIPETS)
[comment]: <> (=========================)
<div id="snipets-container" class="Container" onmouseover='this.style["overflowX"]="scroll";' onmouseout='this.style["overflowX"]="visible";'></div>
<script>
function CopyToClipboard(containerid) {
if (document.selection) {
var range = document.body.createTextRange();
range.moveToElementText(document.getElementById(containerid));
range.select().createTextRange();
document.execCommand("copy");
} 
else if (window.getSelection) {
var range = document.createRange();
range.selectNode(document.getElementById(containerid));
window.getSelection().addRange(range);
document.execCommand("copy");
}
}
</script>
<script>
function CloseSnipet() {
document.getElementById("snipets-container").innerHTML = null;
}
</script>
  
<div id ="carla.World.unload_map_layer-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.unload_map_layer
</p>
<div id="carla.World.unload_map_layer-code" class="SnipetContent">

```py
  
# This recipe toggles off several layers in our "_Opt" maps

# Load town one with minimum layout (roads, sidewalks, traffic lights and traffic signs)
# as well as buildings and parked vehicles
world = client.load_world('Town01_Opt', carla.MapLayer.Buildings | carla.MapLayer.ParkedVehicles) 

# Toggle all buildings off
world.unload_map_layer(carla.MapLayer.Buildings)

# Toggle all parked vehicles off
world.unload_map_layer(carla.MapLayer.ParkedVehicles)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.unload_map_layer-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.DebugHelper.draw_string-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.DebugHelper.draw_string
</p>
<div id="carla.DebugHelper.draw_string-code" class="SnipetContent">

```py
  

# This recipe is a modification of lane_explorer.py example.
# It draws the path of an actor through the world, printing information at each waypoint.

# ...
current_w = map.get_waypoint(vehicle.get_location())
while True:

    next_w = map.get_waypoint(vehicle.get_location(), lane_type=carla.LaneType.Driving | carla.LaneType.Shoulder | carla.LaneType.Sidewalk )
    # Check if the vehicle is moving
    if next_w.id != current_w.id:
        vector = vehicle.get_velocity()
        # Check if the vehicle is on a sidewalk
        if current_w.lane_type == carla.LaneType.Sidewalk:
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else red, 60)
        else:
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else green, 60)
        debug.draw_string(current_w.transform.location, str('%15.0f km/h' % (3.6 * math.sqrt(vector.x**2 + vector.y**2 + vector.z**2))), False, orange, 60)
        draw_transform(debug, current_w.transform, white, 60)

    # Update the current waypoint and sleep for some time
    current_w = next_w
    time.sleep(args.tick_time)
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.DebugHelper.draw_string-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.ActorBlueprint.set_attribute-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.ActorBlueprint.set_attribute
</p>
<div id="carla.ActorBlueprint.set_attribute-code" class="SnipetContent">

```py
  

# This recipe changes attributes of different type of blueprint actors.

# ...
walker_bp = world.get_blueprint_library().filter('walker.pedestrian.0002')
walker_bp.set_attribute('is_invincible', True)

# ...
# Changes attribute randomly by the recommended value
vehicle_bp = wolrd.get_blueprint_library().filter('vehicle.bmw.*')
color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
vehicle_bp.set_attribute('color', color)

# ...

camera_bp = world.get_blueprint_library().filter('sensor.camera.rgb')
camera_bp.set_attribute('image_size_x', 600)
camera_bp.set_attribute('image_size_y', 600)
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.ActorBlueprint.set_attribute-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.World.get_spectator-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.get_spectator
</p>
<div id="carla.World.get_spectator-code" class="SnipetContent">

```py
  

# This recipe spawns an actor and the spectator camera at the actor's location.

# ...
world = client.get_world()
spectator = world.get_spectator()

vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
transform = random.choice(world.get_map().get_spawn_points())
vehicle = world.try_spawn_actor(vehicle_bp, transform)

# Wait for world to get the vehicle actor
world.tick()

world_snapshot = world.wait_for_tick()
actor_snapshot = world_snapshot.find(vehicle.id)

# Set spectator at given transform (vehicle transform)
spectator.set_transform(actor_snapshot.get_transform())
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.get_spectator-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Sensor.listen-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Sensor.listen
</p>
<div id="carla.Sensor.listen-code" class="SnipetContent">

```py
  

# This recipe applies a color conversion to the image taken by a camera sensor,
# so it is converted to a semantic segmentation image.

# ...
camera_bp = world.get_blueprint_library().filter('sensor.camera.semantic_segmentation')
# ...
cc = carla.ColorConverter.CityScapesPalette
camera.listen(lambda image: image.save_to_disk('output/%06d.png' % image.frame, cc))
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Sensor.listen-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.World.load_map_layer-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.load_map_layer
</p>
<div id="carla.World.load_map_layer-code" class="SnipetContent">

```py
  
# This recipe toggles on several layers in our "_Opt" maps

# Load town one with only minimum layout (roads, sidewalks, traffic lights and traffic signs)
world = client.load_world('Town01_Opt', carla.MapLayer.None)

# Toggle all buildings on
world.load_map_layer(carla.MapLayer.Buildings)

# Toggle all foliage on
world.load_map_layer(carla.MapLayer.Foliage)

# Toggle all parked vehicles on
world.load_map_layer(carla.MapLayer.ParkedVehicles)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.load_map_layer-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Map.get_waypoint-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Map.get_waypoint
</p>
<div id="carla.Map.get_waypoint-code" class="SnipetContent">

```py
  

# This recipe shows the current traffic rules affecting the vehicle. 
# Shows the current lane type and if a lane change can be done in the actual lane or the surrounding ones.

# ...
waypoint = world.get_map().get_waypoint(vehicle.get_location(),project_to_road=True, lane_type=(carla.LaneType.Driving | carla.LaneType.Shoulder | carla.LaneType.Sidewalk))
print("Current lane type: " + str(waypoint.lane_type))
# Check current lane change allowed
print("Current Lane change:  " + str(waypoint.lane_change))
# Left and Right lane markings
print("L lane marking type: " + str(waypoint.left_lane_marking.type))
print("L lane marking change: " + str(waypoint.left_lane_marking.lane_change))
print("R lane marking type: " + str(waypoint.right_lane_marking.type))
print("R lane marking change: " + str(waypoint.right_lane_marking.lane_change))
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Map.get_waypoint-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  

<img src="/img/snipets_images/carla.Map.get_waypoint.jpg">
  
</div>
  
<div id ="carla.TrafficLight.set_state-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.TrafficLight.set_state
</p>
<div id="carla.TrafficLight.set_state-code" class="SnipetContent">

```py
  

# This recipe changes from red to green the traffic light that affects the vehicle. 
# This is done by detecting if the vehicle actor is at a traffic light.

# ...
world = client.get_world()
spectator = world.get_spectator()

vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
transform = random.choice(world.get_map().get_spawn_points())
vehicle = world.try_spawn_actor(vehicle_bp, transform)

# Wait for world to get the vehicle actor
world.tick()

world_snapshot = world.wait_for_tick()
actor_snapshot = world_snapshot.find(vehicle.id)

# Set spectator at given transform (vehicle transform)
spectator.set_transform(actor_snapshot.get_transform())
# ...# ...
if vehicle_actor.is_at_traffic_light():
    traffic_light = vehicle_actor.get_traffic_light()
    if traffic_light.get_state() == carla.TrafficLightState.Red:
       # world.hud.notification("Traffic light changed! Good to go!")
        traffic_light.set_state(carla.TrafficLightState.Green)
# ...

  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.TrafficLight.set_state-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  

<img src="/img/snipets_images/carla.TrafficLight.set_state.gif">
  
</div>
  
<div id ="carla.Vehicle.set_wheel_steer_direction-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Vehicle.set_wheel_steer_direction
</p>
<div id="carla.Vehicle.set_wheel_steer_direction-code" class="SnipetContent">

```py
  
# Sets the appearance of the vehicles front wheels to 40°. Vehicle physics will not be affected.

vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FR_Wheel, 40.0)
vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FL_Wheel, 40.0)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Vehicle.set_wheel_steer_direction-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.DebugHelper.draw_box-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.DebugHelper.draw_box
</p>
<div id="carla.DebugHelper.draw_box-code" class="SnipetContent">

```py
  

# This recipe shows how to draw traffic light actor bounding boxes from a world snapshot.

# ....
debug = world.debug
world_snapshot = world.get_snapshot()

for actor_snapshot in world_snapshot:
    actual_actor = world.get_actor(actor_snapshot.id)
    if actual_actor.type_id == 'traffic.traffic_light':
        debug.draw_box(carla.BoundingBox(actor_snapshot.get_transform().location,carla.Vector3D(0.5,0.5,2)),actor_snapshot.get_transform().rotation, 0.05, carla.Color(255,0,0,0),0)
# ...

  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.DebugHelper.draw_box-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  

<img src="/img/snipets_images/carla.DebugHelper.draw_box.jpg">
  
</div>
  
<div id ="carla.WalkerAIController.stop-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.WalkerAIController.stop
</p>
<div id="carla.WalkerAIController.stop-code" class="SnipetContent">

```py
  

#To destroy the pedestrians, stop them from the navigation, and then destroy the objects (actor and controller).

# stop pedestrians (list is [controller, actor, controller, actor ...])
for i in range(0, len(all_id), 2):
    all_actors[i].stop()

# destroy pedestrian (actor and controller)
client.apply_batch([carla.command.DestroyActor(x) for x in all_id])
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.WalkerAIController.stop-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Client.apply_batch_sync-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Client.apply_batch_sync
</p>
<div id="carla.Client.apply_batch_sync-code" class="SnipetContent">

```py
  
# 0. Choose a blueprint fo the walkers
world = client.get_world()
blueprintsWalkers = world.get_blueprint_library().filter("walker.pedestrian.*")
walker_bp = random.choice(blueprintsWalkers)

# 1. Take all the random locations to spawn
spawn_points = []
for i in range(50):
    spawn_point = carla.Transform()
    spawn_point.location = world.get_random_location_from_navigation()
    if (spawn_point.location != None):
        spawn_points.append(spawn_point)

# 2. Build the batch of commands to spawn the pedestrians
batch = []
for spawn_point in spawn_points:
    walker_bp = random.choice(blueprintsWalkers)
    batch.append(carla.command.SpawnActor(walker_bp, spawn_point))

# 2.1 apply the batch
results = client.apply_batch_sync(batch, True)
for i in range(len(results)):
    if results[i].error:
        logging.error(results[i].error)
    else:
        walkers_list.append({"id": results[i].actor_id})

# 3. Spawn walker AI controllers for each walker
batch = []
walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
for i in range(len(walkers_list)):
    batch.append(carla.command.SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))

# 3.1 apply the batch
results = client.apply_batch_sync(batch, True)
for i in range(len(results)):
    if results[i].error:
        logging.error(results[i].error)
    else:
        walkers_list[i]["con"] = results[i].actor_id

# 4. Put altogether the walker and controller ids
for i in range(len(walkers_list)):
    all_id.append(walkers_list[i]["con"])
    all_id.append(walkers_list[i]["id"])
all_actors = world.get_actors(all_id)

# wait for a tick to ensure client receives the last transform of the walkers we have just created
world.wait_for_tick()

# 5. initialize each controller and set target to walk to (list is [controller, actor, controller, actor ...])
for i in range(0, len(all_actors), 2):
    # start walker
    all_actors[i].start()
    # set walk to random point
    all_actors[i].go_to_location(world.get_random_location_from_navigation())
    # random max speed
    all_actors[i].set_max_speed(1 + random.random())    # max speed between 1 and 2 (default is 1.4 m/s)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Client.apply_batch_sync-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.World.enable_environment_objects-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.enable_environment_objects
</p>
<div id="carla.World.enable_environment_objects-code" class="SnipetContent">

```py
  
# This recipe turn visibility off and on for two specifc buildings on the map

# Get the buildings in the world
world = client.get_world()
env_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)

# Access individual building IDs and save in a set
building_01 = env_objs[0]
building_02 = env_objs[1]
objects_to_toggle = {building_01.id, building_02.id}

# Toggle buildings off
world.enable_environment_objects(objects_to_toggle, False)
# Toggle buildings on
world.enable_environment_objects(objects_to_toggle, True)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.enable_environment_objects-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Client.__init__-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Client.__init__
</p>
<div id="carla.Client.__init__-code" class="SnipetContent">

```py
  

# This recipe shows in every script provided in PythonAPI/Examples 
# and it is used to parse the client creation arguments when running the script. 

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-s', '--speed',
        metavar='FACTOR',
        default=1.0,
        type=float,
        help='rate at which the weather changes (default: 1.0)')
    args = argparser.parse_args()

    speed_factor = args.speed
    update_freq = 0.1 / speed_factor

    client = carla.Client(args.host, args.port)

  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Client.__init__-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.World.spawn_actor-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.spawn_actor
</p>
<div id="carla.World.spawn_actor-code" class="SnipetContent">

```py
  

# This recipe attaches different camera / sensors to a vehicle with different attachments.

# ...
camera = world.spawn_actor(rgb_camera_bp, transform, attach_to=vehicle, attachment_type=Attachment.Rigid)
# Default attachment:  Attachment.Rigid
gnss_sensor = world.spawn_actor(sensor_gnss_bp, transform, attach_to=vehicle)
collision_sensor = world.spawn_actor(sensor_collision_bp, transform, attach_to=vehicle)
lane_invasion_sensor = world.spawn_actor(sensor_lane_invasion_bp, transform, attach_to=vehicle)
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.spawn_actor-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  

</div>