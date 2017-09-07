How to add assets
=================

Static content should be added to `Content/Static` to be processed as LFS.
Static content includes

  * Files that once added won't change (often), like the meshes, materials, etc.
  * Big files (~ >1MB).

Adding a vehicle
----------------

Follow
[Art Guide](https://docs.unrealengine.com/latest/INT/Engine/Physics/Vehicles/VehcileContentCreation/index.html)
for creating the Skeletal Mesh and Physics Asset. And
[Vehicles User Guide](https://docs.unrealengine.com/latest/INT/Engine/Physics/Vehicles/VehicleUserGuide/)
for the rest.

  * Import fbx as Skelletal Mesh to its own folder inside `Content/Static/Vehicles`. A Physics asset and a Skeleton should be automatically created and linked the three together.
  * Tune the Physics asset. Delete the automatically created ones and add boxes to the `Vehicle_Base` bone matching the shape, make sure generate hit events is enabled. Add a sphere for each wheel and set their "Physics Type" to "Kinematic".
  * Inside that folder create an "Animation Blueprint", while creating select "VehicleAnimInstance" as parent class and the skeleton of this car model as the target skeleton. Add the animation graph as shown in the links given above (or look for it in other cars' animation, like Mustang).
  * Create folder `Content/Blueprints/Vehicles/<vehicle-model>`
  * Inside that folder create two blueprint classes derived from "VehicleWheel" class. Call them `<vehicle-model>_FrontWheel` and `<vehicle-model>_RearWheel`. Set their "Shape Radius" to exactly match the mesh wheel radius (careful, radius not diameter). Set their "Tire Config" to "CommonTireConfig". On the front wheel uncheck "Affected by Handbrake" and on the rear wheel set "Steer Angle" to zero.
  * Inside the same folder create a blueprint class derived from `BaseVehiclePawn` call it `<vehicle-model>`. Open it for edit and select component "Mesh", setup the "Skeletal Mesh" and the "Anim Class" to the corresponding ones. Then select component "VehicleMovement", under "Vehicle Setup" expand "Wheel Setups", setup each wheel
    - 0 : Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`Wheel_Front_Left`
    - 1 : Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`Wheel_Front_Right`
    - 2 : Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`Wheel_Rear_Left`
    - 3 : Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`Wheel_Rear_Right`
  * Add a box component to it called "Box" (see Mustang blueprint) and set the size to cover vehicle's area as seen from above. Make sure the following is set
    - Hidden in game
    - Simulate physics is disable
    - Generate overlap events is disable
    - Collision presets is set to "NoCollision"
  * Test it, go to CarlaGameMode blueprint and change "Default Pawn Class" to the newly created car blueprint

Map generation
--------------

For the road generation, the following meshes are expected to be found

```
# Enum                            Filepath
RoadTwoLanes_LaneLeft          at "Content/Static/Road/St_Road_TileRoad_RoadL.uasset"
RoadTwoLanes_LaneRight         at "Content/Static/Road/St_Road_TileRoad_RoadR.uasset"
RoadTwoLanes_SidewalkLeft      at "Content/Static/SideWalk/St_Road_TileRoad_SidewalkL.uasset"
RoadTwoLanes_SidewalkRight     at "Content/Static/SideWalk/St_Road_TileRoad_SidewalkR.uasset"
RoadTwoLanes_LaneMarkingSolid  at "Content/Static/RoadLines/St_Road_TileRoad_LaneMarkingSolid.uasset"
RoadTwoLanes_LaneMarkingBroken at "Content/Static/RoadLines/St_Road_TileRoad_LaneMarkingBroken.uasset"

Road90DegTurn_Lane0            at "Content/Static/Road/St_Road_Curve_Road1.uasset"
Road90DegTurn_Lane1            at "Content/Static/Road/St_Road_Curve_Road2.uasset"
Road90DegTurn_Lane2            at "Content/Static/Road/St_Road_Curve_Road3.uasset"
Road90DegTurn_Lane3            at "Content/Static/Road/St_Road_Curve_Road4.uasset"
Road90DegTurn_Lane3            at "Content/Static/Road/St_Road_Curve_Road5.uasset"
Road90DegTurn_Lane3            at "Content/Static/Road/St_Road_Curve_Road6.uasset"
Road90DegTurn_Lane3            at "Content/Static/Road/St_Road_Curve_Road7.uasset"
Road90DegTurn_Lane3            at "Content/Static/Road/St_Road_Curve_Road8.uasset"
Road90DegTurn_Lane3            at "Content/Static/Road/St_Road_Curve_Road9.uasset"
Road90DegTurn_Sidewalk0        at "Content/Static/SideWalk/St_Road_Curve_Sidewalk1.uasset"
Road90DegTurn_Sidewalk1        at "Content/Static/SideWalk/St_Road_Curve_Sidewalk2.uasset"
Road90DegTurn_Sidewalk2        at "Content/Static/SideWalk/St_Road_Curve_Sidewalk3.uasset"
Road90DegTurn_Sidewalk3        at "Content/Static/SideWalk/St_Road_Curve_Sidewalk4.uasset"
Road90DegTurn_LaneMarking      at "Content/Static/Road/St_Road_Curve_LaneMarking.uasset"

RoadTIntersection_Lane0        at "Content/Static/Road/St_Road_TCross_Road1.uasset"
RoadTIntersection_Lane1        at "Content/Static/Road/St_Road_TCross_Road2.uasset"
RoadTIntersection_Lane2        at "Content/Static/Road/St_Road_TCross_Road3.uasset"
RoadTIntersection_Lane3        at "Content/Static/Road/St_Road_TCross_Road4.uasset"
RoadTIntersection_Lane3        at "Content/Static/Road/St_Road_TCross_Road5.uasset"
RoadTIntersection_Lane3        at "Content/Static/Road/St_Road_TCross_Road6.uasset"
RoadTIntersection_Lane3        at "Content/Static/Road/St_Road_TCross_Road7.uasset"
RoadTIntersection_Lane3        at "Content/Static/Road/St_Road_TCross_Road8.uasset"
RoadTIntersection_Lane3        at "Content/Static/Road/St_Road_TCross_Road9.uasset"
RoadTIntersection_Sidewalk0    at "Content/Static/SideWalk/St_Road_TCross_Sidewalk1.uasset"
RoadTIntersection_Sidewalk1    at "Content/Static/SideWalk/St_Road_TCross_Sidewalk2.uasset"
RoadTIntersection_Sidewalk2    at "Content/Static/SideWalk/St_Road_TCross_Sidewalk3.uasset"
RoadTIntersection_Sidewalk3    at "Content/Static/SideWalk/St_Road_TCross_Sidewalk4.uasset"
RoadTIntersection_LaneMarking  at "Content/Static/RoadLines/St_Road_TCross_LaneMarking.uasset"

RoadXIntersection_Lane0        at "Content/Static/Road/St_Road_XCross_Road1.uasset"
RoadXIntersection_Lane1        at "Content/Static/Road/St_Road_XCross_Road2.uasset"
RoadXIntersection_Lane2        at "Content/Static/Road/St_Road_XCross_Road3.uasset"
RoadXIntersection_Lane3        at "Content/Static/Road/St_Road_XCross_Road4.uasset"
RoadXIntersection_Lane3        at "Content/Static/Road/St_Road_XCross_Road5.uasset"
RoadXIntersection_Lane3        at "Content/Static/Road/St_Road_XCross_Road6.uasset"
RoadXIntersection_Lane3        at "Content/Static/Road/St_Road_XCross_Road7.uasset"
RoadXIntersection_Lane3        at "Content/Static/Road/St_Road_XCross_Road8.uasset"
RoadXIntersection_Lane3        at "Content/Static/Road/St_Road_XCross_Road9.uasset"
RoadXIntersection_Sidewalk0    at "Content/Static/SideWalk/St_Road_XCross_Sidewalk1.uasset"
RoadXIntersection_Sidewalk1    at "Content/Static/SideWalk/St_Road_XCross_Sidewalk2.uasset"
RoadXIntersection_Sidewalk2    at "Content/Static/SideWalk/St_Road_XCross_Sidewalk3.uasset"
RoadXIntersection_Sidewalk3    at "Content/Static/SideWalk/St_Road_XCross_Sidewalk4.uasset"
RoadXIntersection_LaneMarking  at "Content/Static/RoadLines/St_Road_XCross_LaneMarking.uasset"
```
