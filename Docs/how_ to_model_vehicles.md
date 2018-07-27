
# How to model vehicles

## 4-Wheels Vehicles


### **Modeling**

The vehicles of Carla must have Min 10.000 - Max 17.000 Tris aprox. we base on real cars and real scales to modeling. The part of bottom of car, modeling a plane and adjust it to the Bodywork. 

The car must been divide in 6 materials:

**1.BodyWork:**

The Bodywork include the chassis, doors, car handle, front and back. All to refer the chassis. The material of BodyWork will be controlled by Unreal. Can put the logos and a bit details, but remember, all that details will be painted by Unreal with the same color. To do channel alpha if you want paint details with different color.

**2.Wheels:**

Model the Wheels with the hubcaps and put details the car tire with substance.
The UV, put the tires separately and the hubcaps to.

**3.Interior:**

the seats and steering wheel. You don’t need much detail. In this section put the part of bottom the car.

**4.Details:**

Lights, logos, exhaust pipes, protections, grille.

**5.Glass:**

Light glasses, windows etc..Material controlled by Unreal
 
**6.LicencePlate:**

Put plane and the material controlled by Unreal



	              Depending on the car is possible put more material or less but following this criterion.
                  
                  
### **Nomenclature of Material**

+ M_”NameCar”_ Bodywork 

+ M_ ”NameCar” _ Wheel

+ M_”NameCar”_ Interior

+ M_”NameCar”_ Details

+ M_”NameCar”_ Glass

+ M_”NameCar”_LicencePlate

### **Textures**

The size of texture 2048 x 2048

+ T_ ”NameCar”_ PartOfMaterial_ d (_BaseColor_)

+ T_ ”NameCar”_ PartOfMaterial_ n (_Normal_)

+ T_ ”NameCar”_ PartOfMaterial_ orm (_OcclusionRoughnessMetallic_)


### **RIG**

If you want a simpler way you might copy our "General4wheeledSkeleton" from our project, either by exporting it and copying it into your model or by creating your skelleton using the same bone names and orientation.

As with the 4 wheeled vehicles, orient the model towards positive "x" and every bone axis towards positive x and with the z axis facing upwards.

_Bone Setup:_ 

Vheicle_Base: The origin point of the mesh. Place it in the point 0,0,0 of the scene.

+ Wheel_Front_Left: Set the position joint in the middle of Wheel.

+ Wheel_Front_Right:Set the position joint in the middle of Wheel.

+ Wheel_Rear_Left:Set the position joint in the middle of Wheel.

+ Wheel_Rear_Left:Set the position joint in the middle of Wheel.

### **LODs**

All LODs of vehicles must be made in maya or other software 3D. Because Unreal does not generate Lods automatically. You can put more or less tris in order to the transition between them is not evident.

_Level - 0_ - Original

_Level - 1_ - Deleted 2.000/2.500 Tries (_Do not delete the interior and steering wheell_ )

_Level - 2_ - Deleted 2.000/2.500 Tries (_Do not delete the interior_)

_Level - 3_ - Deleted 2.000/2.500 Tries  (_Delete the interior_)

_Level - 4_ - That has a simple form of car. 

