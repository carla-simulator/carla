# Generate detailed colliders

This tutorial explains how to create more accurate collision boundaries for vehicles (relative to the original shape of the object). These can be used as physics collider, compatible with collision detection, or as a secondary collider used by raycast-based sensors such a the LIDAR to retrieve more accurate data. New colliders can be integrated into CARLA so that all the community can benefit from these. Find out more about how to contribute to the content repository [here](cont_contribution_guidelines.md).  

There are two approaches to create the new colliders, but they are not completely equivalent.  

*   __Raycast colliders__ — This approach requires some basic 3D modelling skills. A secondary collider is added to the vehicle so that raycast-based sensors such as the LIDAR retrieve more precise data.  
*   __Physics colliders__ — This approach follows the [tutorial](https://bitbucket.org/yankagan/carla-content/wiki/Home) created by the contributor __[Yan Kaganovsky / yankagan](https://github.com/yankagan)__ to create a mesh with no need of manual modelling. This mesh is then used as main collider for the vehicle, for physics and sensor detection (unless a secondary collider is added).  

---

*   [__Raycast colliders__](#raycast-colliders)  
	*   [1-Export the vehicle FBX](#1-export-the-vehicle-fbx)  
	*   [2-Generate a low density mesh](#2-generate-a-low-density-mesh)  
	*   [3-Import the mesh into UE](#3-import-the-mesh-into-ue)  
	*   [4-Add the mesh as collider](#4-add-the-mesh-as-collider)  

---

*   [__Physics colliders__](#physics-colliders)  
	*   [0-Prerequisites](#0-prerequisites)  
	*   [1-Define custom collision for wheels in Unreal Editor](#1-define-custom-collision-for-wheels-in-unreal-editor)  
	*   [2-Export the vehicle as FBX](#2-export-the-vehicle-as-fbx)  
	*   [3 to 4-Import to Blender and create custom boundary](#3-to-4-import-to-blender-and-create-custom-boundary)  
	*   [5-Export from Blender to FBX](#5-export-from-blender-to-fbx)  
	*   [6 to 8-Import collider and define physics](#6-to-8-import-collider-and-define-physics)  

---
## Raycast colliders

### 1-Export the vehicle FBX

First of all, the original mesh of the vehicle is necessary to be used as reference. For the sake of learning, this tutorial exports the mesh of a CARLA vehicle.  
__1.1__ open CARLA in UE and go to `Content/Carla/Static/Vehicles/4Wheeled/<model_of_vehicle>`.  
__1.2__ Press `right-click` on `SM_<model_of_vehicle>` to export the vehicle mesh as FBX.  

### 2-Generate a low density mesh

__2.1__ Open a 3D modelling software and, using the original mesh as reference, model a low density mesh that stays reliable to the original.  

![manual_meshgen](img/tuto_D_colliders_mesh.jpg)

__2.2__ Save the new mesh as FBX. Name de mesh as `sm_sc_<model_of_vehicle>.fbx`. E.g. `sm_sc_audiTT.fbx`.  

!!! Note
    As for the wheels and additional elements such as roofs, mudguards, etc. the new mesh should follow the geometry quite accurately. Placing simple cubes will not do it.  

### 3-Import the mesh into UE

__3.1__ Open CARLA in UE and go to `Content/Carla/Static/Vehicles/4Wheeled/<model_of_vehicle>`.  
__3.2__ Press `right-click` to import the new mesh `SM_sc_<model_of_vehicle>.fbx`.  

### 4-Add the mesh as collider

__4.1__ Go to `Content/Carla/Blueprints/Vehicles/<model_of_vehicle>` and open the blueprint of the vehicle named as `BP_<model_of_vehicle>`.  

__4.2__ Select the `CustomCollision` element and add the `SM_sc_<model_of_vehicle>.fbx` in the `Static mesh` property.  

![manual_customcollision](img/tuto_D_colliders_final.jpg)

__4.3__ Press `Compile` in the toolbar above and save the changes.  

!!! Note
    For vehicles such as motorbikes and bicycles, change the collider mesh of the vehicle itself using the same component, `CustomCollision`.  

---
## Physics colliders

!!! Important
    This tutorial is based on a [contribution](https://bitbucket.org/yankagan/carla-content/wiki/Home) made by __[yankagan](https://github.com/yankagan)__! The contributor also wants to aknowledge __Francisco E__ for the tutorial on [how to import custom collisions in UE](https://www.youtube.com/watch?v=SEH4f0HrCDM).  

[This video](https://www.youtube.com/watch?v=CXK2M2cNQ4Y) shows the results achieved after following this tutorial.  

### 0-Prerequisites

*   __Build CARLA from source__ on [Linux](build_linux.md) or [Windows](build_windows.md).  
*   __Blender 2.80 or newer__ from the [official site](https://www.blender.org/download/) for free (open-source 3D modelling software).  
*   __VHACD Plugin for Blender__ following the using the instructions in [here](https://github.com/andyp123/blender_vhacd). This plugin automatically creates an approximation of a selected object using a collection of convex hulls. [Read more](https://github.com/kmammou/v-hacd).  

!!! Note
    This [series](https://www.youtube.com/watch?v=ppASl6yaguU) and [Udemy course](https://www.udemy.com/course/blender-3d-from-zero-to-hero/?pmtag=MRY1010) may be a good introduction to Blender for newcomers. 


### 1-Define custom collision for wheels in Unreal Editor

__Step 1.__ *(in UE)* — Add collision boundaries for the wheels. The steps are detailed in the following video.  

[![auto_step01](img/tuto_D_colliders_01.jpg)](https://www.youtube.com/watch?v=bECnsTw6ehI)

### 2-Export the vehicle as FBX

__Step 2.__ *(in UE)* — Export the skeletal mesh of a vehicle to an FBX file.  
__2.1__ Go to `Content/Carla/Static/Vehicles/4Wheeled/<model_of_vehicle>`.  
__2.2__ Press `right-click` on `SM_<model_of_vehicle>` to export the vehicle mesh as FBX.  


### 3 to 4-Import to Blender and create custom boundary

__Step 3.__ *(in Blender)* — Import the FBX file into Blender.  
__Step 4.__ *(in Blender)* — Add convex hull meshes to form the new collision boundary (UE requirement for computational efficiency). This is the hardest step. If the entire car is selected, the collision boundary created by VHACD will be imprecise and messy. It will contain sharp edges which will mess-up the drive on the road. It's important that the wheels have smooth boundaries around them. Using convex decomposition on the car's body the mirrors would still not look right. For computer vision, the details of the vehicle are important. For said reason, these step has been divided into two parts. 

__4.1__ Cut out the bottom parts of the wheels, the side mirrors and the top part of the car's body to create the first boundary using the VHACD tool. Cut out the bottom half of the car to create the second boundary (top part of the car) using the VHACD tool.  

[![auto_step03](img/tuto_D_colliders_03.jpg)](https://www.youtube.com/watch?v=oROkK3OCuOA)

__4.2__ Create separate boundaries for side mirrors using the VHACD tool.  

[![auto_step04](img/tuto_D_colliders_04.jpg)](https://www.youtube.com/watch?v=L3upzdC602s)

!!! Warning
    Be very careful about naming the object. Each boundary should have begin with `UCX_`, and the rest of the name has to be __exactly__ the same as the original mesh.  

### 5-Export from Blender to FBX

__Step 5.__ *(in Blender)* — Export the custom collision boundaries into an FBX file.  
__5.1__ Select only the original vehicle and all the newly added objects for collision.  
__5.2__ In the export menu, check `selected objects` and select only "Mesh".  

[![auto_step05](img/tuto_D_colliders_05.jpg)](https://youtu.be/aJPyskYjzWo)

### 6 to 8-Import collider and define physics

__Step 6.__ *(in UE)* — Import the new FBX into CARLA as an Unreal asset file (static mesh).  
__Step 7.__ *(in UE)* — Import the custom collider into the physics asset for the specific vehicle, so that it is used for computations.  
__Step 8.__ *(in UE)* — Create constraints that connect the different joints and define the physics of all parts.  

[![auto_step0608](img/tuto_D_colliders_0608.jpg)](https://www.youtube.com/watch?v=aqFNwAyj2CA)

---

That is a wrap on how to change the default colliders for vehicles in CARLA.  

Open CARLA and mess around for a while. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
