# ===================================================
# Apply a batch of commands and check if they were successful
# ===================================================

# In this snipet we will spawn n vehicles at random in a batch, and use chained commands to set them to autopilot. 
#   By using a batch, all of them will be spawned and set in the same frame. 

# Get all the vehicle blueprints. We will choose at random. 
world = client.get_world()
vehicles_bps = world.get_blueprint_library().filter("walker.vehicle.*")
vehicle_bp = random.choice(vehicle_bp)
vehicles_list = []

# Set the amount of vehicles to be spawned. 
n = 50 

# Get n recommended spawning points, one per vehicle that will be spawned. 
spawn_points = []
for i in range(n):
    spawn_point = carla.Transform()
    spawn_point.location = random.choice(world.get_spawn_points())
    if (spawn_point.location != None):
        spawn_points.append(spawn_point)

# Prepare the commands that will be used. 
#   All the commands available are listed at last in the Python API reference. 
SpawnActor = carla.command.SpawnActor
SetAutopilot = carla.command.SetAutopilot
FutureActor = carla.command.FutureActor # Special command that references a to-be-spawned actor. Used when chaining commands. 

batch = []
for i in range(n):
    vehicle_bp = random.choice(vehicles_bps)
    # For the sake of realism, try to change some blueprint attributes at random
    if vehicle_bp.has_attribute('color'):
        color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
        vehicle_bp.set_attribute('color', color)
    # You can change the role_name of these vehicles so they can be easily identified
    vehicle_bp.set_attribute('role_name','batch_spawn')

    # Append the commands to the batch.
    #   You can create chained commands by using the .then() method.
    #   These commands will be executed one after the other. 
    batch.append(SpawnActor(vehicle_bp, spawn_points[i])
        .then(SetAutopilot(FutureActor, True)))


# Execute the batch and, at the same time, store the responses 
for response in client.apply_batch_sync(batch, synchronous_master):
    #Show the error in failed spawnings, and store the id of those that were successful
    if response.error:
        logging.error(response.error)
    else:
        vehicles_list.append(response.actor_id)

