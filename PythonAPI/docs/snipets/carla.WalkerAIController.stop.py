# To destroy the pedestrians, stop them from the navigation, and then destroy the objects (actor and controller).

import carla

client = carla.Client('localhost', 2000)
all_id = []
all_actors = world.get_actors(all_id)

# stop pedestrians (list is [controller, actor, controller, actor ...])
for i in range(0, len(all_id), 2):
    all_actors[i].stop()

# destroy pedestrian (actor and controller)
client.apply_batch([carla.command.DestroyActor(x) for x in all_id])
