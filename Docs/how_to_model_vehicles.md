<h1>How to model vehicles</h1>

## 4-Wheeled Vehicles

<h3>Modelling</h3>

Vehicles must have a minimum of 10.000 and a maximum of 17.000 Tris
approximately. We model the vehicles using the size and scale of actual cars.
The bottom part of the vehicle consists of a plane adjusted to the Bodywork.

The vehicle must be divided in 6 materials:

  1. **BodyWork:**
    The Bodywork includes the chassis, doors, car handle, and front and back
    parts of the vehicle. The BodyWork material is controlled by Unreal Engine.
    You can add logos and some details, but remember, all the details will be
    painted by Unreal using the same color. Use the alpha channel if you want to
    paint details with a different color.

  2. **Wheels:**
    Model the Wheels with hubcaps and add details to the tire with Substance. In
    the UV, add the tires and the hubcaps separately.

  3. **Interior:**
    The Interior includes the seats, the steering wheel, and the bottom of the
    vehicle. You don’t need to add much detail here.

  4. **Details:**
    Lights, logos, exhaust pipes, protections, and grille.

  5. **Glass:**
    Light glasses, windows, etc. This material is controlled by Unreal.

  6. **LicencePlate:**
    Add just a plane, the materials are controlled by Unreal. Depending on the
    car, it is possible to have several materials, but following this
    criterion.

<h3>Nomenclature of Material</h3>

* `M*"CarName"* Bodywork`

* `M*"CarName"* Wheel`

* `M*"CarName"* Interior`

* `M*"CarName"* Details`

* `M*"CarName"* Glass`

* `M\_"CarName"\_LicencePlate`

<h3>Textures</h3>

The size of the textures is 2048x2048.

* `T* "CarName"* PartOfMaterial* d (\_BaseColor*)`

* `T* "CarName"* PartOfMaterial* n (\_Normal*)`

* `T* "CarName"* PartOfMaterial* orm (\_OcclusionRoughnessMetallic*)`

<h3>RIG</h3>

The easiest way is to copy the "General4wheeledSkeleton" present in our project,
either by exporting it and copying it to your model or by creating your skeleton
using the same bone names and orientation.

The model and every bone must be oriented towards positive X axis with the Z
axis facing upwards.

_Bone Setup:_

Vhehicle_Base: The origin point of the mesh, place it in the point (0,0,0) of the scene.

* Wheel_Front_Left: Set the joint's position in the middle of the Wheel.

* Wheel_Front_Right: Set the joint's position in the middle of the Wheel.

* Wheel_Rear_Left: Set the joint's position in the middle of the Wheel.

* Wheel_Rear_Left: Set the joint's position in the middle of the Wheel.

<h3>LODs</h3>

All vehicle LODs must be made in Maya or other 3D software. Because Unreal does
not generate LODs automatically, you can adjust the number of Tris to make a
smooth transitions between levels.

* _Level 0_ - Original

* _Level 1_ - Deleted 2.000/2.500 Tris (_Do not delete the interior and steering wheel_)

* _Level 2_ - Deleted 2.000/2.500 Tris (_Do not delete the interior_)

* _Level 3_ - Deleted 2.000/2.500 Tris (_Delete the interior_)

* _Level 4_ - Simple shape of a vehicle.
