# Add a new vehicle

---
## Add a 4 wheeled vehicle

Follow [Art Guide][artlink] for creating the Skeletal Mesh and Physics Asset. And [Vehicles User Guide][userguide] for the rest.

[artlink]: https://docs.unrealengine.com/en-US/Engine/Physics/Vehicles/VehicleContentCreation/index.html
[userguide]: https://docs.unrealengine.com/latest/INT/Engine/Physics/Vehicles/VehicleUserGuide/

!!! important
    If you want a simpler way you might copy our "General4wheeledSkeleton" from our project,
    either by exporting it and copying it into your model or by creating your skelleton using
    the same bone names and orientation.<br>
    Bind it to your vehicle model and choose it when importing your vehicle into the editor.
    This way you won't need to configure the animation, you might just use
    "General4wheeledAnimation" (step 4)<br>
    You also won't need to configure the bone names for your wheels
    (Step 8. Be carefull, you'll still need to asign the wheel blueprints).

__1.__ Import fbx as Skelletal Mesh to its own folder inside `Content/Carla/Static/Vehicles`. A Physics asset and a Skeleton should be automatically created and linked the three together.  
<br>
__2.__ Delete the automatically created ones and add boxes to the `Vehicle_Base` bone matching the shape, make sure generate hit events is enabled. Add a sphere for each wheel.  
<br>
__3.__ __Tune the physics.__ In `Details/Physics`, set their "Physics Type" to __`Kinematic`__, and enable the __`Simulation generates hit events`__ option.  
<br>
__4.__ Inside that folder create an "Animation Blueprint", while creating select "VehicleAnimInstance" as parent class and the skeleton of this car model as the target skeleton. Add the animation graph as shown in the links given above (or look for it in other cars' animation, like Mustang).  
<br>
__5.__ Create folder `Content/Carla/Blueprints/Vehicles/<vehicle-model>`  
<br>
__6.__ Inside that folder create two blueprint classes derived from "VehicleWheel" class. Call them `<vehicle-model>_FrontWheel` and `<vehicle-model>_RearWheel`. Set their "Shape Radius"
to exactly match the mesh wheel radius (careful, radius not diameter). Set their "Tire Config" to "CommonTireConfig". On the front wheel uncheck "Affected by Handbrake" and on the rear wheel set "Steer Angle" to zero.  
<br>
__7.__ Inside the same folder __crate a child blueprint class__ derived from `BaseVehiclePawn` call it `<vehicle-model>`. Open it for edit and select component "Mesh", setup the "Skeletal Mesh"
and the "Anim Class" to the corresponding ones. Then select the VehicleBounds component and set the size to cover vehicle's volume as close as possible.  
<br>
__8.__ Select component "VehicleMovement", under "Vehicle Setup" expand "Wheel Setups", setup each wheel  
    - 0: Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`Wheel_Front_Left`  
    - 1: Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`Wheel_Front_Right`  
    - 2: Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`Wheel_Rear_Left`  
    - 3: Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`Wheel_Rear_Right`  
<br>
__9.__ Test it, go to CarlaGameMode blueprint and change "Default Pawn Class" to the newly created car blueprint.

---
## Add a 2 wheeled vehicle

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

__1.__ Import fbx as Skelletal Mesh to its own folder inside `Content/Carla/Static/Vehicles/2Wheeled`. When importing select "General2WheeledVehicleSkeleton" as skelleton A Physics asset should be automatically created and linked.  

__2.__ Tune the Physics asset. Delete the automatically created ones and add boxes to the `BikeBody` bone trying to match the shape as possible, make sure generate hit events is enabled. 
  Add a sphere for each wheel and set their "Physics Type" to "Kinematic".  

__3.__ Create folder `Content/Blueprints/Vehicles/<vehicle-model>`  

__4.__ Inside that folder create two blueprint classes derived from "VehicleWheel" class. Call them `<vehicle-model>_FrontWheel` and `<vehicle-model>_RearWheel`. Set their "Shape Radius" to exactly match the mesh wheel radius (careful, radius not diameter). Set their "Tire Config" to "CommonTireConfig". On the front wheel uncheck "Affected by Handbrake" and on the rear wheel set "Steer Angle" to zero.  

__5.__ Inside the same folder create a blueprint class derived from `Base2WheeledVehicle` call it `<vehicle-model>`. Open it for edit and select component "Mesh", setup the "Skeletal Mesh"
  and the "Anim Class" to the corresponding ones. Then select the VehicleBounds component and set the size to cover vehicle's area as seen from above.

__6.__ Select component "VehicleMovement", under "Vehicle Setup" expand "Wheel Setups", setup each wheel.  

*   __0:__ Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`FrontWheel`  
*   __1:__ Wheel Class=`<vehicle-model>_FrontWheel`, Bone Name=`FrontWheel`  
*   __2:__ Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`RearWheel`  
*   __3:__ Wheel Class=`<vehicle-model>_RearWheel`, Bone Name=`RearWheel`  
(You'll notice that we are basically placing two wheels in each bone. The vehicle class unreal provides does not support vehicles with wheel numbers different from 4 so we had to make it believe the vehicle has 4 wheels)

__7.__ Select the variable "is bike" and tick it if your model is a bike. This will activate the
  pedalier rotation. Leave unmarked if you are setting up a motorbike.

__8.__ Find the variable back Rotation and set it as it fit better select the component SkeletalMesh
  (The driver) and move it along x axis until its in the seat position.  

__9.__ Test it, go to CarlaGameMode blueprint and change "Default Pawn Class" to the newly
  created bike blueprint.