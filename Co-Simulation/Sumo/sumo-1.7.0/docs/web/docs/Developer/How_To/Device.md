---
title: Developer/How To/Device
permalink: /Developer/How_To/Device/
---

This Page describes how to implement a new device.

## What is a device good for?

A device is a container for data and functionality which resides in
individual vehicles. Devices are notified about all vehicle movements
and may interact with the vehicle or with other devices. An important
aspect of devices is, that it is possible to equip only a fraction of
the simulated vehicles. Usually devices support some kind of output. The
following is a list of available devices and their functionality

### MSDevice_Tripinfo

Records start and arrival of a vehicle as well as aggregate measures
about a completed trip such as average speed and waiting time. This
device is also used for expressing aggregate results from other devices
such as emissions ([MSDevice_HBEFA](#msdevice_hbefa)). This
devices is activated using the option **--tripinfo-output** {{DT_FILE}} and is then active for all
vehicles.

### MSDevice_Vehroutes

Records the edges traveled by a vehicle and optionally the times at
which each edge was left. It can also record dynamic route changes. This
device is also used for expressing aggregate results from other devices
such as emissions ([MSDevice_HBEFA](#msdevice_hbefa)). This
devices is activated using the option **--vehroute-output** {{DT_FILE}} and is then active for all
vehicles.

### MSDevice_Routing

Triggers periodic rerouting of equipped vehicles. This devices is
activated using the options **--vehroute-output.\*** and is then active for selected vehicles.
It is possible to equip specific vehicles or a fraction of the vehicle
fleet.

### MSDevice_Person

This device is automatically created if a person rides in a vehicle. It
is used when managing boarding and alighting

### MSDevice_HBEFA

Computes emissions of a vehicle as described in
[Definition_of_Vehicles,_Vehicle_Types,_and_Routes\#Vehicle_Emission_Classes](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_emission_classes).
This devices is activated using the options **--device.hbefa.\*** and is then active for
selected vehicles. It is possible to equip specific vehicles or a
fraction of the vehicle fleet.

### MSDevice_Battery

This device is used for modelling energy use and battery capacity of
electric vehicles.

### MSDevice_SSM

This device logs surrogate safety measures for equipped vehicles, see
[Simulation/Output/SSM_Device](../../Simulation/Output/SSM_Device.md).

### MSDevice_Example

This device serves as an implementation example for custom devices.

## Steps for implementing a new device

### Copy Example Device

The suggested way for creating a new device is to create a new class by
copying the files *src/microsim/devices/MSDevice_Example.{h,cpp}*.

In order to compile the new class it must be added to
**src/microsim/devices/Makefile.am** (on Linux) or added to project
**z_libmicrosim_devices** (on Windows).

In order to be available for use, the device must also be added in
*src/microsim/devices/MSDevice.cpp* to the functions *insertOptions* and
*buildVehicleDevices*.

### Assign Device to Vehicles

Devices are assigned by global options for equipping all or a fraction
of the vehicle fleet. Alternatively, they can be defined by setting `<vehicle>` or
`<vType>`-[generic
parameters](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices).
The new assignment options are generated automatically by calling the
function *MSDevice::insertDefaultAssignmentsOptions*. The test whether a
specific vehicle should be equipped with the device is done by calling
*MSDevice::equippedByDefaultAssignmentOptions*. The usage of these
methods is demonstrated in *MSDevice_Example*.

### Adding your own functionality

Devices work by updating their state periodically (i.e.
*MSDevice_Routing*) or everytime the vehicle moves (i.e.
*MSDevice_Battery*). Many devices perform some kind of output either
periodically (*MSDevice_Routing*) in response to computed events
(*MSDevice_BTreceiver*) or at the time the vehicle leaves the
simulation (*MSDevice_Vehroutes*). Before the vehicle is removed the
method *generateOutput* is called for each device.

### Testing your code

!!! note
    It is important to test new code and suggestions for how to do so can be found in *tests/sumo/extended/device_example/*.

### Show your work

If you have implemented a new device and you think it may be useful to
other users of SUMO, please contribute to our community and tell us
about your work. We would be happy to help you in getting your device
into the main development tree.