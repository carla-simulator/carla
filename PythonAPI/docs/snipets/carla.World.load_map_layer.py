# This recipe toggles on several layers in our "_Opt" maps

# Load town one with only minimum layout (roads, sidewalks, traffic lights and traffic signs)
world = client.load_world('Town01_Opt', carla.MapLayer.None)

# Toggle all buildings on
world.load_map_layer(carla.MapLayer.Buildings)

# Toggle all foliage on
world.load_map_layer(carla.MapLayer.Foliage)

# Toggle all parked vehicles on
world.load_map_layer(carla.MapLayer.ParkedVehicles)
