<!--- 

The images in this document were captured using the following settings in CARLA:

Town: Town 10

Vehicle location: Transform(Location(x=-46.885479, y=20.083447, z=-0.002633), Rotation(pitch=-0.000034, yaw=141.974243, roll=0.000000))

Camera location, small vehicles: Transform(Location(x=-47.696186, y=24.049326, z=1.471929), Rotation(pitch=-10.843717, yaw=-77.215683, roll=0.000139))
Camera location, standard vehicles: Transform(Location(x=-48.672256, y=24.830288, z=1.722733), Rotation(pitch=-13.396630, yaw=-75.692039, roll=0.000119))
Camera location, large vehicles: Transform(Location(x=-49.470921, y=27.835310, z=2.931721), Rotation(pitch=-13.396630, yaw=-75.691978, roll=0.000119))

The weather settings are: 

weather.sun_altitude_angle = 50
weather.sun_azimuth_angle = 260
weather.wetness = 10
weather.precipitation = 10
weather.scattering_intensity = 5
weather.mie_scattering_scale = 0.5
weather.rayleigh_scattering_scale = 0.1

Camera settings:

camera_bp = bp_lib.find('sensor.camera.rgb')
camera_bp.set_attribute('image_size_x', '1920')
camera_bp.set_attribute('image_size_y', '1080')
camera_bp.set_attribute('fstop', '6.0')

Vehicle settings:

control = vehicle.get_control()
control.steer = -0.25
vehicle.apply_control(control)

--->

# CARLA catalogue

The CARLA simulator provides a vast library of 3D assets to populate your autonomous agent's virtual environment. The 3D asset library provides numerous pre-built maps to choose from, a diverse array of vehicle models for traffic simulation along with models of pedestrians and other structures or obstacles that can be dynamically added to your simulation during runtime. This catalogue documents all of the 3D assets available to use in your simulations. CARLA also provides a number of example Python scripts to use as guidance for utilizing the API in various contexts.

* [__Vehicles catalogue__](catalogue_vehicles.md)
* [__Pedestrians catalogue__](catalogue_pedestrians.md)
* [__Props catalogue__](catalogue_props.md)
<!-- * [__Script catalogue__](catalogue_scripts.md) -->

## Maps

There are two maps to chose from. An urban environment with skyscrapers, commercial units, industrial units and residential streets and an off-road, open-cast mine environment.

| Town       | Summary |
| -----------| ------  |
| [__Town10__](map_town10.md) | A downtown urban environment with skyscrapers, residential buildings and an ocean promenade.|
| [__Mine01__](map_mine.md) | An industrial, off-road, open-cast mining environment. |

## Vehicles

CARLA provides a diverse array of vehicles, with high fidelity models of real world cars, trucks and bikes, for replicating traffic in your simulations. Browse and choose the vehicles you like in the [__vehicle catalogue__](catalogue_vehicles.md).

![vehicles_overview](../img/catalogue/vehicles/vehicle_montage.webp)

## Pedestrians

CARLA's asset library includes a variety of pedestrians to simulate foot traffic in the 3D environments surrounding your agent. Browse and choose the pedestrians you want in the [__pedestrian catalogue__](catalogue_pedestrians.md).

![pedestrian_overview](../img/catalogue/pedestrians/pedestrians_overview.webp)

## Props

CARLA's props model the various structures and items you might find on or near roads, such as kiosks, statues, benches, boxes, bins, debris or trash. The props can be placed anywhere in your simulation dynamically during runtime. Browse and choose your props in the [__props catalogue__](catalogue_props.md).

![props_overview](../img/catalogue/props/props_overview.webp)

---

## Inspect the vehicles in the simulator

To examine a vehicle from the catalogue, use the following code, retrieve the blueprint ID from the [__vehicles catalogue__](catalogue_vehicles.md) and paste it into the line `bp_lib.find('blueprint.id.goes_here')`:

```py
client = carla.Client('localhost', 2000)
world = client.get_world()
bp_lib = world.get_blueprint_library()
spectator = world.get_spectator()

# Set up the vehicle transform
vehicle_loc = carla.Location(x=-46.9, y=20.0, z=0.2)
vehicle_rot = carla.Rotation(pitch=0.0, yaw=142.0, roll=0.0)
vehicle_trans = carla.Transform(vehicle_loc,vehicle_rot)

# Paste the blueprint ID here:
vehicle_bp = bp_lib.find('vehicle.lincoln.mkz') 

# Set up the view transform
camera_loc = carla.Location(x=-48.7, y=24.8, z=1.7)
camera_rot = carla.Rotation(pitch=-13.4, yaw=-75.7, roll=0.0)
camera_trans = carla.Transform(camera_loc,camera_rot)

# Spawn the vehicle
vehicle = world.spawn_actor(vehicle_bp, vehicle_trans)

# Move the spectator
spectator.set_transform(camera_trans)

```

Don't forget to destroy the vehicle before you try and spawn another to avoid a collision:

```py
vehicle.destroy()
```

<!-- ## Scripts

CARLA comes provided with a library of example scripts that can be used to explore maps, vehicles or use as guidance for using the API in various contexts. You could also use them as the basis for your own scripts. Choose a script for your application in the [__scripts catalogue__](catalogue_scripts.md). -->