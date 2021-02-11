# This recipe toggles off and on several layers in our "_Opt" maps

# Load layered map for Town 01
world = client.load_world('Town01_Opt')

# Toggle all buildings off
world.unload_map_layer(carla.MapLayer.Buildings)

# Toggle all foliage off
world.unload_map_layer(carla.MapLayer.Foliage)

# Toggle all parked vehicles off
world.unload_map_layer(carla.MapLayer.ParkedVehicles)

# Toggle all buildings on
world.load_map_layer(carla.MapLayer.Buildings)

# Toggle all foliage on
world.load_map_layer(carla.MapLayer.Foliage)

# Toggle all parked vehicles on
world.load_map_layer(carla.MapLayer.ParkedVehicles)