# How to add assets

---
## Adding a vehicle

Follow [Art Guide][artlink] for creating the Skeletal Mesh and Physics Asset. And
[Vehicles User Guide][userguide] for the rest.

[artlink]: https://docs.unrealengine.com/en-US/Engine/Physics/Vehicles/VehicleContentCreation/index.html
[userguide]: https://docs.unrealengine.com/latest/INT/Engine/Physics/Vehicles/VehicleUserGuide/

!!! important
    If you want a simpler way you might copy our "General4wheeledSkeleton" from our project,
    either by exporting it and copying it into your model or by creating your skelleton using
    the same bone names and orientation.<br>
    Bind it to your vehicle model and choose it when importing your vehicle into the editor.
    This way you won't need to configure the animation, you might just use
    "General4wheeledAnimation" (step 3)<br>
    You also won't need to configure the bone names for your wheels
    (Step 7. Be carefull, you'll still need to asign the wheel blueprints).

  1. Import fbx as Skelletal Mesh to its own folder inside `Content/Carla/Static/Vehicles`.
  A Physics asset and a Skeleton should be automatically created and linked the three together.

  2. Tune the Physics asset. Delete the automatically created ones and add boxes to the
  `Vehicle_Base` bone matching the shape, make sure generate hit events is enabled.
  Add a sphere for each wheel and set their "Physics Type" to "Kinematic".

  3. Inside that folder create an "Animation Blueprint", while creating select "VehicleAnimInstance"
  as parent class and the skeleton of this car model as the target skeleton.
  Add the animation graph as shown in the links given above
  (or look for it in other cars' animation, like Mustang).

  4. Create folder `Content/Carla/Blueprints/Vehicles/<vehicle-model>`

  5. Inside that folder create two blueprint classes derived from "VehicleWheel" class.
  Call them `<vehicle-model>_FrontWheel` and `<vehicle-model>_RearWheel`. Set their "Shape Radius"
  to exactly match the mesh wheel radius (careful, radius not diameter). Set their "Tire Config" to
  "CommonTireConfig". On the front wheel uncheck "Affected by Handbrake" and on the rear wheel
  set "Steer Angle" to zero.

  6. Inside the same folder create a blueprint class derived from `BaseVehiclePawn` 
  call it `<vehicle-model>`. Open it for edit and select component "Mesh", setup the "Skeletal Mesh"
  and the "Anim Class" to the corresponding ones. Then select the VehicleBounds component and set
  the size to cover vehicle's volume as close as possible.

  7. Select component "VehicleMovement", under "Vehicle Setup" expand "Wheel Setups", setup each wheel
    - 0 : Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`Wheel_Front_Left`
    - 1 : Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`Wheel_Front_Right`
    - 2 : Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`Wheel_Rear_Left`
    - 3 : Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`Wheel_Rear_Right`

  8. Test it, go to CarlaGameMode blueprint and change "Default Pawn Class" to the newly
  created car blueprint.

---
## Adding a 2 wheeled vehicle

Adding 2 wheeled vehicles is similar to adding a 4 wheeled one but due to the complexity of the
animation you'll need to set up aditional bones to guide the driver's animation:

As with the 4 wheeled vehicles, orient the model towards positive "x" and every bone axis towards
positive x and with the z axis facing upwards.

```yaml
Bone Setup:
  - Bike_Rig:                   # The origin point of the mesh. Place it in the point 0 of the scenecomment
    - BikeBody:                 # The model's body centre.
      - Pedals:                 # If the vehicle is a bike bind the pedalier to this bone, will rotate with the bike acceleration.
        - RightPedal:           # Sets the driver's feet position and rotates with the pedalier if the vehicle is a bike.
        - LeftPedal:            # ^
      - RearWheel:              # Rear Wheel of the vehicle
      - Handler:                # Rotates with the frontal wheel of the vehicle bind the vehicle handler to it.
        - HandlerMidBone:       # Positioned over the front wheel bone to orient the handler with the wheel
        - HandlerRight:         # Sets the position of the driver's hand, no need to bind it to anything.
        - HandlerLeft:          # ^
      - Frontwheel:             # Frontal wheel of the vehicle.
      - RightHelperRotator:     # This four additional bones are here for an obsolete system of making the bike stable by using aditional invisible wheels
        - RightHelprWheel:      # ^
      - LeftHelperRotator:      # ^
        - LeftHelperWheel:      # ^
      - Seat:                   # Sets the position of the drivers hip bone. No need to bind it to anything but place it carefully.
```

  1. Import fbx as Skelletal Mesh to its own folder inside `Content/Carla/Static/Vehicles/2Wheeled`.
  When importing select "General2WheeledVehicleSkeleton" as skelleton A Physics asset should be
  automatically created and linked.

  2. Tune the Physics asset. Delete the automatically created ones and add boxes to the `BikeBody`
  bone trying to match the shape as possible, make sure generate hit events is enabled. 
  Add a sphere for each wheel and set their "Physics Type" to "Kinematic".

  3. Create folder `Content/Blueprints/Vehicles/<vehicle-model>`

  4. Inside that folder create two blueprint classes derived from "VehicleWheel" class. Call them
  `<vehicle-model>_FrontWheel` and `<vehicle-model>_RearWheel`. Set their "Shape Radius" to exactly
  match the mesh wheel radius (careful, radius not diameter).
  Set their "Tire Config" to "CommonTireConfig". On the front wheel uncheck "Affected by Handbrake"
  and on the rear wheel set "Steer Angle" to zero.

  5. Inside the same folder create a blueprint class derived from `Base2WheeledVehicle`
  call it `<vehicle-model>`. Open it for edit and select component "Mesh", setup the "Skeletal Mesh"
  and the "Anim Class" to the corresponding ones. Then select the VehicleBounds component and set
  the size to cover vehicle's area as seen from above.

  6. Select component "VehicleMovement", under "Vehicle Setup" expand "Wheel Setups", setup each wheel
    - 0 : Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`FrontWheel`
    - 1 : Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`FrontWheel`
    - 2 : Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`RearWheel`
    - 3 : Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`RearWheel`
    (You'll notice that we are basically placing two wheels in each bone.
    The vehicle class unreal provides does not support vehicles with wheel numbers different
    from 4 so we had to make it believe the vehicle has 4 wheels)

  7. Select the variable "is bike" and tick it if your model is a bike. This will activate the
  pedalier rotation. Leave unmarked if you are setting up a motorbike.

  8. Find the variable back Rotation and set it as it fit better select the component SkeletalMesh
  (The driver) and move it along x axis until its in the seat position.

  9. Test it, go to CarlaGameMode blueprint and change "Default Pawn Class" to the newly
  created bike blueprint.

---
## Map generation

For the road generation, the following meshes are expected to be found

```
# Enum                            Filepath
RoadTwoLanes_LaneLeft          at "Content/Carla/Static/Road/St_Road_TileRoad_RoadL.uasset"
RoadTwoLanes_LaneRight         at "Content/Carla/Static/Road/St_Road_TileRoad_RoadR.uasset"
RoadTwoLanes_SidewalkLeft      at "Content/Carla/Static/SideWalk/St_Road_TileRoad_SidewalkL.uasset"
RoadTwoLanes_SidewalkRight     at "Content/Carla/Static/SideWalk/St_Road_TileRoad_SidewalkR.uasset"
RoadTwoLanes_LaneMarkingSolid  at "Content/Carla/Static/RoadLines/St_Road_TileRoad_LaneMarkingSolid.uasset"
RoadTwoLanes_LaneMarkingBroken at "Content/Carla/Static/RoadLines/St_Road_TileRoad_LaneMarkingBroken.uasset"

Road90DegTurn_Lane0            at "Content/Carla/Static/Road/St_Road_Curve_Road1.uasset"
Road90DegTurn_Lane1            at "Content/Carla/Static/Road/St_Road_Curve_Road2.uasset"
Road90DegTurn_Lane2            at "Content/Carla/Static/Road/St_Road_Curve_Road3.uasset"
Road90DegTurn_Lane3            at "Content/Carla/Static/Road/St_Road_Curve_Road4.uasset"
Road90DegTurn_Lane3            at "Content/Carla/Static/Road/St_Road_Curve_Road5.uasset"
Road90DegTurn_Lane3            at "Content/Carla/Static/Road/St_Road_Curve_Road6.uasset"
Road90DegTurn_Lane3            at "Content/Carla/Static/Road/St_Road_Curve_Road7.uasset"
Road90DegTurn_Lane3            at "Content/Carla/Static/Road/St_Road_Curve_Road8.uasset"
Road90DegTurn_Lane3            at "Content/Carla/Static/Road/St_Road_Curve_Road9.uasset"
Road90DegTurn_Sidewalk0        at "Content/Carla/Static/SideWalk/St_Road_Curve_Sidewalk1.uasset"
Road90DegTurn_Sidewalk1        at "Content/Carla/Static/SideWalk/St_Road_Curve_Sidewalk2.uasset"
Road90DegTurn_Sidewalk2        at "Content/Carla/Static/SideWalk/St_Road_Curve_Sidewalk3.uasset"
Road90DegTurn_Sidewalk3        at "Content/Carla/Static/SideWalk/St_Road_Curve_Sidewalk4.uasset"
Road90DegTurn_LaneMarking      at "Content/Carla/Static/Road/St_Road_Curve_LaneMarking.uasset"

RoadTIntersection_Lane0        at "Content/Carla/Static/Road/St_Road_TCross_Road1.uasset"
RoadTIntersection_Lane1        at "Content/Carla/Static/Road/St_Road_TCross_Road2.uasset"
RoadTIntersection_Lane2        at "Content/Carla/Static/Road/St_Road_TCross_Road3.uasset"
RoadTIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_TCross_Road4.uasset"
RoadTIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_TCross_Road5.uasset"
RoadTIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_TCross_Road6.uasset"
RoadTIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_TCross_Road7.uasset"
RoadTIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_TCross_Road8.uasset"
RoadTIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_TCross_Road9.uasset"
RoadTIntersection_Sidewalk0    at "Content/Carla/Static/SideWalk/St_Road_TCross_Sidewalk1.uasset"
RoadTIntersection_Sidewalk1    at "Content/Carla/Static/SideWalk/St_Road_TCross_Sidewalk2.uasset"
RoadTIntersection_Sidewalk2    at "Content/Carla/Static/SideWalk/St_Road_TCross_Sidewalk3.uasset"
RoadTIntersection_Sidewalk3    at "Content/Carla/Static/SideWalk/St_Road_TCross_Sidewalk4.uasset"
RoadTIntersection_LaneMarking  at "Content/Carla/Static/RoadLines/St_Road_TCross_LaneMarking.uasset"

RoadXIntersection_Lane0        at "Content/Carla/Static/Road/St_Road_XCross_Road1.uasset"
RoadXIntersection_Lane1        at "Content/Carla/Static/Road/St_Road_XCross_Road2.uasset"
RoadXIntersection_Lane2        at "Content/Carla/Static/Road/St_Road_XCross_Road3.uasset"
RoadXIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_XCross_Road4.uasset"
RoadXIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_XCross_Road5.uasset"
RoadXIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_XCross_Road6.uasset"
RoadXIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_XCross_Road7.uasset"
RoadXIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_XCross_Road8.uasset"
RoadXIntersection_Lane3        at "Content/Carla/Static/Road/St_Road_XCross_Road9.uasset"
RoadXIntersection_Sidewalk0    at "Content/Carla/Static/SideWalk/St_Road_XCross_Sidewalk1.uasset"
RoadXIntersection_Sidewalk1    at "Content/Carla/Static/SideWalk/St_Road_XCross_Sidewalk2.uasset"
RoadXIntersection_Sidewalk2    at "Content/Carla/Static/SideWalk/St_Road_XCross_Sidewalk3.uasset"
RoadXIntersection_Sidewalk3    at "Content/Carla/Static/SideWalk/St_Road_XCross_Sidewalk4.uasset"
RoadXIntersection_LaneMarking  at "Content/Carla/Static/RoadLines/St_Road_XCross_LaneMarking.uasset"
```
