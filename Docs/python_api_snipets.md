[comment]: <> (=========================)
[comment]: <> (PYTHON API SCRIPT SNIPETS)
[comment]: <> (=========================)
<div id="snipets-container" onmouseover='this.style["overflowX"]="scroll";'  onmouseout='this.style["overflowX"]="visible";' style="position: fixed; margin-left: 0px; overflow-y: auto; padding-left: 5px; border-left: 1px solid #767677; height: 95%; top: 70px; left: 1100px;"></div>
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
  
<div id ="carla.World.get_spectator-snipet" style="display: none;">
<p class="SnipetFont">
Snipet for carla.World.get_spectator
</p>
<div id="carla.World.get_spectator-code" style="width: 400px; margin-left: 10px">

```py
  
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
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.get_spectator-code')">Copy snipet</button>
</div>
  
<div id ="carla.Actor.add_force-snipet" style="display: none;">
<p class="SnipetFont">
Snipet for carla.Actor.add_force
</p>
<div id="carla.Actor.add_force-code" style="width: 400px; margin-left: 10px">

```py
  
client = carla.Client(127.0.0.1, 2000)
client.set_timeout(10.0)

# Spawn an actor
MY FOOOOOOOOOOOOOORCE   

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Actor.add_force-code')">Copy snipet</button>
</div>
  
<div id ="carla.Actor.add_angular_impulse-snipet" style="display: none;">
<p class="SnipetFont">
Snipet for carla.Actor.add_angular_impulse
</p>
<div id="carla.Actor.add_angular_impulse-code" style="width: 400px; margin-left: 10px">

```py
  
# Check angular velocity
actor.get_angular_velocity()

# Apply angular impulse
actor.add_angular_impulse(10) # degrees * s

# Wait for world update
world.wait_for_tick()

# Check new angular velocity
actor.get_angular_velocity()  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Actor.add_angular_impulse-code')">Copy snipet</button>
</div>
  

</div>