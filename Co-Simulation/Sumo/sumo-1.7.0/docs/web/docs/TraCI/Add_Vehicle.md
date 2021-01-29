---
title: TraCI/Add Vehicle
permalink: /TraCI/Add_Vehicle/
---

## Command 0x74: Add Vehicle

|            |                 |          |         |                    |                 |
| :--------: | :-------------: | :------: | :-----: | :----------------: | :-------------: |
|   string   |     string      |  string  | string  |       double       |     double      |
| vehicle ID | vehicle type ID | route ID | lane ID | insertion position | insertion speed |

This command adds a new vehicle to the simulation.

### Parameters

1.  vehicle ID
    - identifier to assign to the newly created vehicle (must be
    unique)
2.  vehicle type ID
- identifier of pre-registered vehicle type to use
3.  route ID
- identifier of pre-registered route the vehicle should follow
4.  lane ID
- identifier of lane the vehicle should start on, or an empty
      string. If an empty string is passed, the first lane of the
      route's first edge is used. Note that the lane ID, if given,
      must be on the first edge of the vehicle's route.
5.  insertion position
- offset in meters from the start of the lane where the vehicle
      should be added
6.  insertion speed
- maximum initial velocity of the vehicle, or -1. If a negative
      insertion speed is passed, the vehicle will depart at maximum
      speed.

### Return value

As usual, successful execution of this command is indicated with a
RTYPE_OK response. If, for any reason, the vehicle could not be added
(most commonly, because the lane is blocked), RTYPE_ERR is returned
instead.