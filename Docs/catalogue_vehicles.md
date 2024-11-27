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

# Vehicle catalogue

* __Car__
	* [__Audi__ - TT](#audi-tt)
	* [__Dodge__ - Charger](#dodge-charger)
	* [__Dodge__ - Police Charger](#dodge-police-charger)
	* [__Ford__ - Crown (taxi)](#ford-crown-taxi)
	* [__Lincoln__ - MKZ](#lincoln-mkz)
	* [__Mercedes__ - Coupe](#mercedes-coupe)
	* [__Mini__ - Cooper](#mini-cooper-s)
	* [__Nissan__ - Patrol](#nissan-patrol)
* __Truck__
	* [__CARLA Motors__ - Firetruck](#carla-motors-firetruck)
	* [__CARLA Motors__ - CarlaCola](#carla-motors-carlacola)
* __Van__
	* [__Ford__ - Ambulance](#ford-ambulance)
	* [__Mercedes__ - Sprinter](#mercedes-sprinter)
* __Bus__
	* [__Mitsubishi__ - Fusorosa](#mitsubishi-fusorosa)


---

## Inspect the vehicles in the simulator

To examine a vehicle from the catalog, use the following code, retrieve the blueprint ID from the vehicle's details below and paste it into the line `bp_lib.find('blueprint.id.goes_here')`:

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

---

## Cars
### Audi - TT

![audi_tt](../img/catalogue/vehicles/audi_tt.png)


* __Manufacturer__: Audi
* __Model__: TT
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ue4.audi.tt<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### BMW - Gran Tourer

![bmw_grandtourer](../img/catalogue/vehicles/bmw_grantourer.png)


* __Manufacturer__: BMW
* __Model__: Gran Tourer
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ue4.bmw.grantourer<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Chevrolet - Impala

![chevrolet_impala](../img/catalogue/vehicles/chevrolet_impala.png)


* __Manufacturer__: Chevrolet
* __Model__: Impala
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ue4.chevrolet.impala<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Dodge - Charger

![dodge_charger_2020](../img/catalogue/vehicles/dodge_charger.png)


* __Manufacturer__: Dodge
* __Model__: Charger 2020
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.dodge.charger<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Dodge - Police Charger

![dodge_charger_police](../img/catalogue/vehicles/dodge_charger_police.png)


* __Manufacturer__: Dodge
* __Model__: Police Charger 2020
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.dodgecop.charger<span>

* __Base type__: car

* __Special type__: emergency

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Ford - Crown (taxi)

![ford_crown](../img/catalogue/vehicles/taxi_ford.png)


* __Manufacturer__: Ford
* __Model__: Crown (taxi)
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.taxi.ford<span>

* __Base type__: car

* __Special type__: taxi

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Ford - Mustang

![ford_mustang](../img/catalogue/vehicles/ford_mustang.png)


* __Manufacturer__: Ford
* __Model__: Mustang
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ue4.ford.mustang<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Lincoln - MKZ

![lincoln_mkz](../img/catalogue/vehicles/lincoln_mkz.png)

* __Manufacturer__: Lincoln
* __Model__: MKZ
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.lincoln.mkz<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Mercedes - Coupe

![mercedes_coupe](../img/catalogue/vehicles/mercedes_coupe.png)


* __Manufacturer__: Mercedes
* __Model__: Coupe
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ue4.mercedes.ccc<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Mini - Cooper S

![mini_cooper](../img/catalogue/vehicles/mini_cooper.png)

* __Manufacturer__: Mini
* __Model__: Cooper S 2021
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.mini.cooper<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Nissan - Patrol

![nissan_patrol](../img/catalogue/vehicles/nissan_patrol.png)

* __Manufacturer__: Nissan
* __Model__: Patrol
* __Class__: SUV
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.nissan.patrol<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

---

## Trucks
### CARLA Motors - CarlaCola

![carlamotors_carlacola](../img/catalogue/vehicles/carlacola.png)


* __Manufacturer__: CARLA Motors
* __Model__: CarlaCola
* __Class__: Truck
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.carlacola.actors<span>

* __Base type__: truck

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### CARLA Motors - Firetruck

![carlamotors_firetruck](../img/catalogue/vehicles/firetruck.png)


* __Manufacturer__: CARLA Motors
* __Model__: Firetruck
* __Class__: Truck
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.firetruck.actors<span>

* __Base type__: truck

* __Special type__: emergency

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

---

## Vans
### Ford - Ambulance

![ford_ambulance](../img/catalogue/vehicles/ambulance_ford.png)

* __Manufacturer__: Ford
* __Model__: Ambulance
* __Class__: Van
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ambulance.ford<span>

* __Base type__: van

* __Special type__: emergency

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Mercedes - Sprinter

![mercedes_sprinter](../img/catalogue/vehicles/mercedes_sprinter.png)


* __Manufacturer__: Mercedes
* __Model__: Sprinter
* __Class__: Van
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.sprinter.mercedes<span>

* __Base type__: van

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

---

## Buses
### Mitsubishi - Fusorosa

![mitsubishi_fusorosa](../img/catalogue/vehicles/mitsubishi_fusorosa.png)


* __Manufacturer__: Mitsubishi
* __Model__: Fusorosa
* __Class__: Bus
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">ehicle.fuso.mitsubishi<span>

* __Base type__: bus

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

---