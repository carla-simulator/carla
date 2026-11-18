# This recipe turns on deterministic collisions, so that a synchronous
# simulation with crashes in it repeats exactly from one run to the next.
# The setting applies when the map loads, so reload it keeping the settings.

settings = world.get_settings()
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.05
settings.deterministic_collisions = True
world.apply_settings(settings)

# reset_settings=False: the reloaded map keeps the settings just applied.
world = client.reload_world(False)

# ... spawn the actors and tick the world ...

# Back to the default (faster) collision detection.
settings.deterministic_collisions = False
world.apply_settings(settings)
world = client.reload_world(False)
