# Add a new vehicle

This tutorial details how to add a new vehicle to CARLA. There are two sections, one for 4 wheeled vehicles and one for 2 wheeled vehicles. There is an outline of the basic requirements that must be fulfilled when modeling your vehicle to ensure that it works well in CARLA and instructions on configurations required after the vehicle has been imported into Unreal Engine.

*   [__Add a 4 wheeled vehicle__](#add-a-4-wheeled-vehicle)
	*   [Bind and model the vehicle](#bind-and-model-the-vehicle)
	*   [Import and configure the vehicle](#import-and-configure-the-vehicle)
*   [__Add a 2 wheeled vehicle__](#add-a-2-wheeled-vehicle)

!!! Important
    This tutorial only applies to users that work with a build from source, and have access to the Unreal Engine Editor.

---
## Add a 4 wheeled vehicle

Vehicles added to CARLA need to use a __common base skeleton__ which is found [__here__](https://carla-assets.s3.eu-west-3.amazonaws.com/fbx/VehicleSkeleton.rar). This link will download a folder called `VehicleSkeleton.rar` which contains the base skeleton in two different `.fbx` formats, one in ASCII and the other in binary. The format you use will depend on your 3D modeling software requirements.

__The positions of the skeleton bones can be changed but any other manipulation such as rotation, addition of new bones, or changing the current hierarchy will lead to errors. __

---

### Bind and model the vehicle

This section details the minimum requirements in the modeling stage of your vehicle to make sure it can be used successfully in CARLA. The process involves binding the skeleton correctly to the base and wheels of the vehicle, creating Physical Asset and raycast sensor meshes, and exporting to the correct format.

__1. Import the base skeleton.__

Import the base skeleton into your preferred 3D modeling software. Common editors include Maya and Blender.

__2. Bind the bones.__

Bind the bones to the corresponding portions of the vehicle mesh according to the nomenclature below. Make sure to center the wheels' bones within the mesh.

*   __Front left wheel:__ `Wheel_Front_Left`
*   __Front right wheel:__ `Wheel_Front_Right`
*   __Rear left wheel:__ `Wheel_Rear_Left`
*   __Rear right wheel:__ `Wheel_Rear_Right`
*   __Rest of the mesh:__ `VehicleBase`

!!! Warning
    Do not make any changes to the bone names or the hierarchy nor add any new bones.

__3.  Model your vehicle.__

Vehicles should have between approximately 50,000 - 100,000 tris. We model the vehicles using the size and scale of actual cars.

We recommend that you divide the vehicle into the following materials:

>1. __Bodywork__: The metallic part of the vehicle. This material is changed to Unreal Engine material. Logos and details can be added but, to be visible, they must be painted in a different color by using the alpha channels in the Unreal Engine editor.
- __Glass_Ext__: A layer of glass that allows visibility from the outside to the inside of the vehicle.
- __Glass_Int__: A layer of glass that allows visibility from the inside to the outside of the vehicle.
- __Lights__: Headlights, indicator lights, etc.
- __LightGlass_Ext__: A layer of glass that allows visibility from the outside to the inside of the light.
- __LightGlass_Int__: A layer of glass that allows visibility from the inside to the outside of the light.
- __LicensePlate__: A rectangular plane of 29x12 cm. You can use the CARLA provided `.fbx` for best results, download it [here](https://carla-assets.s3.eu-west-3.amazonaws.com/fbx/LicensePlate.rar). The texture will be assigned automatically in Unreal Engine.
- __Interior__: Any other details that don't fit in the above sections can go into _Interior_.

Materials should be named using the format `M_CarPart_CarName`, e.g., `M_Bodywork_Mustang`.

Textures should be named using the format `T_CarPart_CarName`, e.g., `T_Bodywork_Mustang`. Textures should be sized as 2048x2048.

Unreal Engine automatically creates LODs but you can also create them manually in your 3D editor. Tri counts are as follows:

- __LOD 0__: 100,000 tris
- __LOD 1__: 80,000 tris
- __LOD 2__: 60,000 tris
- __LOD 3__: 30,000 tris


__4. Create the Physical Asset mesh.__

The Physical Asset mesh is an additional mesh that allows Unreal Engine to calculate the vehicle's physics. It should be as simple as possible, with a reduced number of polygons, and should cover the whole vehicle except for the wheels. See the image below for an example.

>>![physical asset mesh](../img/physical_asset_mesh.png)

The Physical Asset mesh should be exported as a separate `.fbx` file. The final file should fulfill the following requirements:

- Have a base mesh. This should be a copy of the Physical Asset mesh. It should have the same name as the original vehicle.
- The Physical Asset mesh must be named using the format `UCX_<vehicle_name>_<number_of_mesh>`, __otherwise it will not be recognized by Unreal Engine.__
- The mesh must not extend beyond the boundaries of the original model.
- The mesh should have the same position as the original model.

>>![base mesh](../img/base_mesh.png)

Export the final mesh as an `.fbx` file with the name `SMC_<vehicle_name>.fbx`.

__5. Create the mesh for the raycast sensor.__

The raycast sensor mesh sets up the vehicle's shape that will be detected by the raycast sensors (RADAR, LiDAR, and Semantic LiDAR). This mesh should have a slightly more defined geometry than the Physical Asset mesh in order to increase the realism of sensor simulation but not as detailed as the car mesh for performance reasons.

Consider the following points when creating the raycast sensor mesh:

- The mesh should cover all aspects of the vehicle, including wheels, side mirrors, and grilles.
- The wheels should be cylinders of no more than 16 loops.
- Various meshes can be joined together if required.
- The mesh(es) must not extend beyond the boundaries of the original model.
- The mesh(es) should have the same position as the original.

>>![collision mesh](../img/collision_mesh.png)

Export the final mesh as an `.fbx` file with the name `SM_sc_<vehicle_name>.fbx`.

__5. Export the vehicle mesh(es).__

Select all the main vehicle mesh(es) and the skeleton base and export as `.fbx`.

---

### Import and configure the vehicle

This section details the process of importing the vehicle into Unreal Engine for use in CARLA. Perform these steps in the Unreal Engine editor.

__1. Create the vehicle folder.__

Create a new folder named `<vehicle_name>` in `Content/Carla/Static/Vehicles/4Wheeled`.

__2. Import the `.fbx`.__

Inside the new vehicle folder, import your main vehicle skeleton `.fbx` by right-clicking in the **_Content Browser_** and selecting **_Import into Game/Carla/Static/Vehicles/4Wheeled/<vehicle_name\>_**.

In the dialogue box that pops up:

- Set **_Import Content Type_** to `Geometry and Skinning Weights`.
- Set **_Normal Import Method_** to `Import Normals`.
- Optionally set **_Material Import Method_** to `Do not create materials`. Uncheck **_Import Textures_** to avoid Unreal Engine creating default materials.

The Skeletal Mesh will appear along with two new files, `<vehicle_name>_PhysicsAssets` and `<vehicle_name>_Skeleton`.

Import the rest of your `.fbx` files separately from the main vehicle skeleton `.fbx` file.

__3. Set the physical asset mesh.__

>1. Open `<vehicle_name>_PhysicsAssets` from the **_Content Browser_**.
- Right-click on the `Vehicle_Base` mesh in the **_Skeleton Tree_** panel and go to **_Copy Collision from StaticMesh_**.
- Search for and select your `SMC_<vehicle_name>` file. You should see the outline of the physical asset mesh appear in the viewport.
- Delete the default capsule shape from the `Vehicle_Base`.
- Select all the wheels:
    - Go to the **_Tools_** panel and change the **_Primitive Type_** to `Sphere`.
    - Go to the **_Details_** panel and change **_Physics Type_** to `Kinematic`.
    - Set **_Linear Damping_** to `0`. This will eliminate any extra friction on the wheels.
- Enable **_Simulation Generates Hit Event_** for all meshes.
- Click **_Re-generate Bodies_**.
- Adjust the wheel sphere to the size of the wheel.
- Save and close the window.

>![Collision mesh](../img/collision_mesh_vehicle.png)

__4. Create the Animation Blueprint.__

>1. In the **_Content Browser_**, right-click inside your vehicle folder and select **_Animation -> Animation Blueprint_**.
- In **_Parent Class_** search for and select `VehicleAnimInstance`.
- In **_Target Skeleton_** search for and select `<vehicle_name>_Skeleton`.
- Press **_OK_** and rename the blueprint as `AnimBP_<vehicle_name>`.

__5. Configure the Animation Blueprint.__

To ease the process of configuring the animation blueprint, we will copy an existing one from a native CARLA vehicle:

>1. Go to `Content/Carla/Static/Vehicle` and choose any CARLA vehicle folder. Open its Animation Blueprint.
- In the **_My Blueprint_** panel, double click on **_AnimGraph_**. You will see the graph come up in the viewport.
- Click and drag to select the **_Mesh Space Ref Pose_**, **_Wheel Handler_**, and **_Component To Local_** components. Right-click and select **_Copy_**.
- Go back to your own vehicle Animation Blueprint and paste the copied contents into the graph area.
- Click and drag from the standing figure in the **_Component To Local_** component to the figure in **_Output Pose_** to join the components together.
- Click **_Compile_** in the top left corner. You should now see a pulsating line flowing through the entire sequence.
- Save and close the window.

>>![add_vehicle_step_04](img/add_vehicle_step_04.jpg)

__6. Prepare the vehicle and wheel blueprints.__

>1. In the **_Content Browser_**, go to `Content/Carla/Blueprints/Vehicles` and create a new folder `<vehicle_name>`.
- Inside the folder, right-click and go to **_Blueprint Class_**. Open the **_All Classes_** section in the pop-up.
- Search for `BaseVehiclePawn` and press **_Select_**.
- Rename the file as `BP_<vehicle_name>`.
- Go to the folder of any of the native CARLA vehicles in `Carla/Blueprints/Vehicles`. From the **_Content Browser_**, copy the four wheel blueprints into the blueprint folder for your own vehicle. Rename the files to replace the old vehicle name with your own vehicle name.

>>![Copy wheel blueprints](../img/copy_wheel_blueprint.png)

__7. Configure the wheel blueprints.__

>1. In your vehicle blueprint folder, open all four of the wheel blueprints.
- In the **_Class Defaults_** panel, set **_Collision Mesh_** to `Wheel_Shape`. __Omitting this step will cause the vehicle wheels to sink into the ground__.
- Adjust the values for wheel shape radius, width, mass, and damping rate according to your vehicle specifications.
- Set **_Tire Config_** to `CommonTireConfig`
- On the front wheels set **_Steer Angle_** according to your preferences (default is `70`). Uncheck **_Affected by Handbrake_**.
- On the rear wheels set **_Steer Angle_** to `0`. Check **_Affected by Handbrake_**.
- When setting the suspension values, you can use the values [here](tuto_D_customize_vehicle_suspension.md) as a guide.
- Compile and save.

>>![wheel shape](../img/wheel_shape.png)

__8. Configure vehicle blueprint.__

>1. From the **_Content Browser_**, open your `BP_<vehicle_name>`.
- In the **_Components_** panel, select **_Mesh (VehicleMesh) (Inherited)_**.
- In the **_Details_** panel, go to **_Skeletal Mesh_** and search for and select the base skeleton file of your vehicle (located in the `Carla/Static/Vehicles/4Wheeled/<vehicle_name>` folder).
- Go to **_Anim Class_** in the **_Details_** panel. Search for and select your `AnimBP_<vehicle_name>` file.
- In the **_Components_** panel, select **_Custom Collision (Inherited)_**.
- Select **_Static Mesh_** in the **_Details_** panel and search for your `SM_sc_<vehicle_name>` raycast sensor mesh.
- In the **_Components_** panel, select **_VehicleMovement (MovementComp) (Inherited)_**.
- In the **_Details_** panel, search for `wheel`. You will find settings for each of the wheels. For each one, click on **_Wheel Class_** and search for the `BP_<vehicle_name>_<wheel_name>` file that corresponds to the correct wheel position.

>>>>![wheel blueprint](../img/wheel_blueprint.png)

If you have any additional meshes for your vehicle (doors, lights, etc.,) separate from the base mesh:

>1. Drag them into the **_Mesh (VehicleMesh) (Inherited)_** hierarchy in the **_Components_** panel.
- Select the extra meshes in the hierarchy and search for `Collision` in the **_Details_** panel.
- Set **_Collision Presets_** to `NoCollision`.
- Select any lights meshes in the hierarchy. Search for `Tag` in the **_Details_** panel and add the tag `emissive`.

Click **_Save_** and **_Compile_**.



__9. Add the vehicle to the Blueprint Library__.

>1. In `Content/Carla/Blueprint/Vehicle`, open the `VehicleFactory` file.
- In the **_Generate Definitions_** tab, double click **_Vehicles_**.
- In the **_Details_** panel, expand the **_Default Value_** section and add a new element to the vehicles array.
- Fill in the **_Make_** and **_Model_** of your vehicle.
- Fill in the **_Class_** value with your `BP_<vehicle_name>` file.
- Optionally, provide a set of recommended colors for the vehicle.
- Compile and save.

>![vehicle factory](../img/vehicle_factory.png)

__10. Test the vehicle__.

Launch CARLA, open a terminal in `PythonAPI/examples` and run the following command:

```sh
python3 manual_control.py --filter <model_name> # The make or model defined in step 9
```

!!! Note
    Even if you used upper case characters in your make and model, they need to be converted to lower case when passed to the filter.

---
## Add a 2 wheeled vehicle

Adding 2 wheeled vehicles is similar to adding a 4 wheeled one but due to the complexity of the animation you'll need to set up aditional bones to guide the driver's animation. [Here](https://carla-assets.s3.eu-west-3.amazonaws.com/fbx/BikeSkeleton.rar) is the link to the reference skeleton for 2 wheeled vehicles.

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
