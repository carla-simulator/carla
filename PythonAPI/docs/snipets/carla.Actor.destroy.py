# ===================================================
# Destroy actors that have been spawned
# ===================================================

# This snipet shows how to destroy actors in the scene, and some considerations that should be taken into account
#   when destroying sensors and walkers. 


# Destroying actors simply requires a call to the destroy() method.
#   Let's destroy all the vehicles that are currently in the scene.
all_actors = my_world.get_actors() # Returns a carla.ActorsList
all_vehicles = all_actors.filter('vehicle.*.*')

# The destroy() method will block the script, so it is a good idea to apply these in a batch of commands. 
print('Destroying all vehicles...')
client.apply_batch([carla.command.DestroyActor(x) for x in all_vehicles])


# There are two special cases that need some preparation:

# 1. Sensors: Stop their listen() function before destroying them, so that no process remains 
all_sensors = all_actors.filter('sensor.*.*')

for i in range(0, len(all_sensors):
    all_sensors[i].stop()
client.apply_batch([carla.command.DestroyActor(x) for x in all_sensors])


# 2. Walkers: Remember that walkers usually have a carla.WalkerAIController attached that is in charge of navigation. 
#    In this case, stop the carla.WalkerAIController, and then destroy the actor and the controller. 
all_walker_controllers = all_actors.filter('controller.*.*')
all_walker = all_actors.filter('walker.*.*')

for i in range(0, len(all_walker_controllers):
    all_walker_controllers[i].stop()

client.apply_batch([carla.command.DestroyActor(x) for x in all_walker_controllers])
client.apply_batch([carla.command.DestroyActor(x) for x in all_walkers])

