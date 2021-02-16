# This recipe toggles off several layers in our "_Opt" maps

# Load town one with minimum layout (roads, sidewalks, traffic lights and traffic signs)
# as well as buildings and parked vehicles
world = client.load_world('Town01_Opt', carla.MapLayer.Buildings | carla.MapLayer.ParkedVehicles) 

# Toggle all buildings off
world.unload_map_layer(carla.MapLayer.Buildings)

# Toggle all parked vehicles off
world.unload_map_layer(carla.MapLayer.ParkedVehicles)
