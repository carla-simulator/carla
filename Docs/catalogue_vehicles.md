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

## Generation 2

* __Car__
	* [__Dodge__ - Charger 2020](#dodge-charger-2020)
	* [__Dodge__ - Police Charger 2020](#dodge-police-charger-2020)
	* [__Ford__ - Crown (taxi)](#ford-crown-taxi)
	* [__Lincoln__ - MKZ 2020](#lincoln-mkz-2020)
	* [__Mercedes__ - Coupe 2020](#mercedes-coupe-2020)
	* [__Mini__ - Cooper S 2021](#mini-cooper-s-2021)
	* [__Nissan__ - Patrol 2021](#nissan-patrol-2021)
* __Truck__
	* [__CARLA Motors__ - European HGV (cab-over-engine type)](#carla-motors-european-hgv-cab-over-engine-type)
	* [__CARLA Motors__ - Firetruck](#carla-motors-firetruck)
	* [__Tesla__ - Cybertruck](#tesla-cybertruck)
* __Van__
	* [__Ford__ - Ambulance](#ford-ambulance)
	* [__Mercedes__ - Sprinter](#mercedes-sprinter)
	* [__Volkswagen__ - T2 2021](#volkswagen-t2-2021)
* __Bus__
	* [__Mitsubishi__ - Fusorosa](#mitsubishi-fusorosa)


## Generation 1

* __Car__
	* [__Audi__ - A2](#audi-a2)
	* [__Audi__ - E-Tron](#audi-e-tron)
	* [__Audi__ - TT](#audi-tt)
	* [__BMW__ - Gran Tourer](#bmw-gran-tourer)
	* [__Chevrolet__ - Impala](#chevrolet-impala)
	* [__Citroen__ - C3](#citroen-c3)
	* [__Dodge__ - Police Charger](#dodge-police-charger)
	* [__Ford__ - Mustang](#ford-mustang)
	* [__Jeep__ - Wrangler Rubicon](#jeep-wrangler-rubicon)
	* [__Lincoln__ - MKZ 2017](#lincoln-mkz-2017)
	* [__Mercedes__ - Coupe](#mercedes-coupe)
	* [__Micro__ - Microlino](#micro-microlino)
	* [__Mini__ - Cooper S](#mini-cooper-s)
	* [__Nissan__ - Micra](#nissan-micra)
	* [__Nissan__ - Patrol](#nissan-patrol)
	* [__Seat__ - Leon](#seat-leon)
	* [__Tesla__ - Model 3](#tesla-model-3)
	* [__Toyota__ - Prius](#toyota-prius)
* __Truck__
	* [__CARLA Motors__ - CarlaCola](#carla-motors-carlacola)
* __Van__
	* [__Volkswagen__ - T2](#volkswagen-t2)
* __Motorcycle__
	* [__Harley Davidson__ - Low Rider](#harley-davidson-low-rider)
	* [__Kawasaki__ - Ninja](#kawasaki-ninja)
	* [__Vespa__ - ZX 125](#vespa-zx-125)
	* [__Yamaha__ - YZF](#yamaha-yzf)
* __Bicycle__
	* [__BH__ - Crossbike](#bh-crossbike)
	* [__Diamondback__ - Century](#diamondback-century)
	* [__Gazelle__ - Omafiets](#gazelle-omafiets)


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
vehicle_bp = bp_lib.find('vehicle.lincoln.mkz_2020') 

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
### Audi - A2

![audi_a2](../img/catalogue/vehicles/audi_a2.webp)


* __Manufacturer__: Audi
* __Model__: A2
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.audi.a2<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Audi - E-Tron

![audi_etron](../img/catalogue/vehicles/audi_etron.webp)


* __Manufacturer__: Audi
* __Model__: E-Tron
* __Class__: SUV
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.audi.etron<span>

* __Base type__: car

* __Special type__: electric

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Audi - TT

![audi_tt](../img/catalogue/vehicles/audi_tt.webp)


* __Manufacturer__: Audi
* __Model__: TT
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.audi.tt<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### BMW - Gran Tourer

![bmw_grandtourer](../img/catalogue/vehicles/bmw_grandtourer.webp)


* __Manufacturer__: BMW
* __Model__: Gran Tourer
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.bmw.grandtourer<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Chevrolet - Impala

![chevrolet_impala](../img/catalogue/vehicles/chevrolet_impala.webp)


* __Manufacturer__: Chevrolet
* __Model__: Impala
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.chevrolet.impala<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Citroen - C3

![citroen_c3](../img/catalogue/vehicles/citroen_c3.webp)


* __Manufacturer__: Citroen
* __Model__: C3
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.citroen.c3<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Dodge - Charger 2020

![dodge_charger_2020](../img/catalogue/vehicles/dodge_charger_2020.webp)


* __Manufacturer__: Dodge
* __Model__: Charger 2020
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.dodge.charger_2020<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Dodge - Police Charger

![dodge_charger_police](../img/catalogue/vehicles/dodge_charger_police.webp)


* __Manufacturer__: Dodge
* __Model__: Police Charger
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.dodge.charger_police<span>

* __Base type__: car

* __Special type__: emergency

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Dodge - Police Charger 2020

![dodge_charger_police_2020](../img/catalogue/vehicles/dodge_charger_police_2020.webp)


* __Manufacturer__: Dodge
* __Model__: Police Charger 2020
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.dodge.charger_police_2020<span>

* __Base type__: car

* __Special type__: emergency

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Ford - Crown (taxi)

![ford_crown](../img/catalogue/vehicles/ford_crown.webp)


* __Manufacturer__: Ford
* __Model__: Crown (taxi)
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ford.crown<span>

* __Base type__: car

* __Special type__: taxi

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Ford - Mustang

![ford_mustang](../img/catalogue/vehicles/ford_mustang.webp)


* __Manufacturer__: Ford
* __Model__: Mustang
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ford.mustang<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Jeep - Wrangler Rubicon

![jeep_wrangler_rubicon](../img/catalogue/vehicles/jeep_wrangler_rubicon.webp)


* __Manufacturer__: Jeep
* __Model__: Wrangler Rubicon
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.jeep.wrangler_rubicon<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Lincoln - MKZ 2017

![lincoln_mkz_2017](../img/catalogue/vehicles/lincoln_mkz_2017.webp)


* __Manufacturer__: Lincoln
* __Model__: MKZ 2017
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.lincoln.mkz_2017<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Lincoln - MKZ 2020

![lincoln_mkz_2020](../img/catalogue/vehicles/lincoln_mkz_2020.webp)


* __Manufacturer__: Lincoln
* __Model__: MKZ 2020
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.lincoln.mkz_2020<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Mercedes - Coupe

![mercedes_coupe](../img/catalogue/vehicles/mercedes_coupe.webp)


* __Manufacturer__: Mercedes
* __Model__: Coupe
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.mercedes.coupe<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Mercedes - Coupe 2020

![mercedes_coupe_2020](../img/catalogue/vehicles/mercedes_coupe_2020.webp)


* __Manufacturer__: Mercedes
* __Model__: Coupe 2020
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.mercedes.coupe_2020<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Micro - Microlino

![micro_microlino](../img/catalogue/vehicles/micro_microlino.webp)


* __Manufacturer__: Micro
* __Model__: Microlino
* __Class__: Micro
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.micro.microlino<span>

* __Base type__: car

* __Special type__: electric

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Mini - Cooper S

![mini_cooper_s](../img/catalogue/vehicles/mini_cooper_s.webp)


* __Manufacturer__: Mini
* __Model__: Cooper S
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.mini.cooper_s<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Mini - Cooper S 2021

![mini_cooper_s_2021](../img/catalogue/vehicles/mini_cooper_s_2021.webp)


* __Manufacturer__: Mini
* __Model__: Cooper S 2021
* __Class__: Standard
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.mini.cooper_s_2021<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Nissan - Micra

![nissan_micra](../img/catalogue/vehicles/nissan_micra.webp)


* __Manufacturer__: Nissan
* __Model__: Micra
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.nissan.micra<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Nissan - Patrol

![nissan_patrol](../img/catalogue/vehicles/nissan_patrol.webp)


* __Manufacturer__: Nissan
* __Model__: Patrol
* __Class__: SUV
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.nissan.patrol<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Nissan - Patrol 2021

![nissan_patrol_2021](../img/catalogue/vehicles/nissan_patrol_2021.webp)


* __Manufacturer__: Nissan
* __Model__: Patrol 2021
* __Class__: SUV
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.nissan.patrol_2021<span>

* __Base type__: car

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Seat - Leon

![seat_leon](../img/catalogue/vehicles/seat_leon.webp)


* __Manufacturer__: Seat
* __Model__: Leon
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.seat.leon<span>

* __Base type__: car

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Tesla - Model 3

![tesla_model3](../img/catalogue/vehicles/tesla_model3.webp)


* __Manufacturer__: Tesla
* __Model__: Model 3
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.tesla.model3<span>

* __Base type__: car

* __Special type__: electric

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Toyota - Prius

![toyota_prius](../img/catalogue/vehicles/toyota_prius.webp)


* __Manufacturer__: Toyota
* __Model__: Prius
* __Class__: Compact
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.toyota.prius<span>

* __Base type__: car

* __Special type__: electric

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

---

## Trucks
### CARLA Motors - CarlaCola

![carlamotors_carlacola](../img/catalogue/vehicles/carlamotors_carlacola.webp)


* __Manufacturer__: CARLA Motors
* __Model__: CarlaCola
* __Class__: Truck
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.carlamotors.carlacola<span>

* __Base type__: truck

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### CARLA Motors - European HGV (cab-over-engine type)

![carlamotors_european_hgv](../img/catalogue/vehicles/carlamotors_european_hgv.webp)


* __Manufacturer__: CARLA Motors
* __Model__: European HGV (cab-over-engine type)
* __Class__: Truck
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.carlamotors.european_hgv<span>

* __Base type__: truck

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### CARLA Motors - Firetruck

![carlamotors_firetruck](../img/catalogue/vehicles/carlamotors_firetruck.webp)


* __Manufacturer__: CARLA Motors
* __Model__: Firetruck
* __Class__: Truck
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.carlamotors.firetruck<span>

* __Base type__: truck

* __Special type__: emergency

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Tesla - Cybertruck

![tesla_cybertruck](../img/catalogue/vehicles/tesla_cybertruck.webp)


* __Manufacturer__: Tesla
* __Model__: Cybertruck
* __Class__: Truck
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.tesla.cybertruck<span>

* __Base type__: truck

* __Special type__: electric

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

---

## Vans
### Ford - Ambulance

![ford_ambulance](../img/catalogue/vehicles/ford_ambulance.webp)


* __Manufacturer__: Ford
* __Model__: Ambulance
* __Class__: Van
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.ford.ambulance<span>

* __Base type__: van

* __Special type__: emergency

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Mercedes - Sprinter

![mercedes_sprinter](../img/catalogue/vehicles/mercedes_sprinter.webp)


* __Manufacturer__: Mercedes
* __Model__: Sprinter
* __Class__: Van
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.mercedes.sprinter<span>

* __Base type__: van

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

### Volkswagen - T2

![volkswagen_t2](../img/catalogue/vehicles/volkswagen_t2.webp)


* __Manufacturer__: Volkswagen
* __Model__: T2
* __Class__: Standard
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.volkswagen.t2<span>

* __Base type__: van

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Volkswagen - T2 2021

![volkswagen_t2_2021](../img/catalogue/vehicles/volkswagen_t2_2021.webp)


* __Manufacturer__: Volkswagen
* __Model__: T2 2021
* __Class__: Van
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.volkswagen.t2_2021<span>

* __Base type__: van

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#99c635;">True<span>

---

## Buses
### Mitsubishi - Fusorosa

![mitsubishi_fusorosa](../img/catalogue/vehicles/mitsubishi_fusorosa.webp)


* __Manufacturer__: Mitsubishi
* __Model__: Fusorosa
* __Class__: Bus
* __Generation__: 2
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.mitsubishi.fusorosa<span>

* __Base type__: bus

* __Has lights__: <span style="color:#99c635;">True<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

---

## Motorcycles
### Harley Davidson - Low Rider

![harley-davidson_low_rider](../img/catalogue/vehicles/harley-davidson_low_rider.webp)


* __Manufacturer__: Harley Davidson
* __Model__: Low Rider
* __Class__: Motorcycle
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.harley-davidson.low_rider<span>

* __Base type__: motorcycle

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Kawasaki - Ninja

![kawasaki_ninja](../img/catalogue/vehicles/kawasaki_ninja.webp)


* __Manufacturer__: Kawasaki
* __Model__: Ninja
* __Class__: Motorcycle
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.kawasaki.ninja<span>

* __Base type__: motorcycle

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Vespa - ZX 125

![vespa_zx125](../img/catalogue/vehicles/vespa_zx125.webp)


* __Manufacturer__: Vespa
* __Model__: ZX 125
* __Class__: Motorcycle
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.vespa.zx125<span>

* __Base type__: motorcycle

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Yamaha - YZF

![yamaha_yzf](../img/catalogue/vehicles/yamaha_yzf.webp)


* __Manufacturer__: Yamaha
* __Model__: YZF
* __Class__: Motorcycle
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.yamaha.yzf<span>

* __Base type__: motorcycle

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

---

## Bicycles
### BH - Crossbike

![bh_crossbike](../img/catalogue/vehicles/bh_crossbike.webp)


* __Manufacturer__: BH
* __Model__: Crossbike
* __Class__: Bicycle
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.bh.crossbike<span>

* __Base type__: bicycle

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Diamondback - Century

![diamondback_century](../img/catalogue/vehicles/diamondback_century.webp)


* __Manufacturer__: Diamondback
* __Model__: Century
* __Class__: Bicycle
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.diamondback.century<span>

* __Base type__: bicycle

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

### Gazelle - Omafiets

![gazelle_omafiets](../img/catalogue/vehicles/gazelle_omafiets.webp)


* __Manufacturer__: Gazelle
* __Model__: Omafiets
* __Class__: Bicycle
* __Generation__: 1
* __Blueprint ID__: <span style="color:#00a6ed;">vehicle.gazelle.omafiets<span>

* __Base type__: bicycle

* __Has lights__: <span style="color:#f16c6c;">False<span>

* __Has opening doors__: <span style="color:#f16c6c;">False<span>

---

