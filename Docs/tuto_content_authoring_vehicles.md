# WVTruck Branch - Changelog

This document provides a comprehensive summary of all the changes and fixes that have been applied to the original DReyeVR codebase in order to achieve a fully functional, out-of-the-box build on both Windows and Ubuntu platforms. These modifications address broken dependency URLs, outdated library versions, build script defects, and missing tooling that previously prevented a clean compilation. Additionally, new features have been introduced to provide greater flexibility during the installation process.

---

## 1. Dependency URL and Version Updates

Several third-party dependency URLs used by the CARLA build system were broken or outdated, causing the build to fail during dependency resolution.

### 1.1 Boost Download URL
The original JFrog Artifactory host has been decommissioned. Updated to the official Boost archives mirror.

| | URL |
|---|---|
| **Before** | `https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/...` |
| **After** | `https://archives.boost.io/release/${BOOST_VERSION}/source/...` |

### 1.2 libpng Download URL

libpng 1.6.37 was relocated to an `older-releases` subdirectory on SourceForge, causing a 404 error. The URL has been corrected. The library version remains unchanged.

| | URL |
|---|---|
| **Before** | `https://sourceforge.net/projects/libpng/files/libpng16/${LIBPNG_VERSION}/...` |
| **After** | `https://sourceforge.net/projects/libpng/files/libpng16/older-releases/${LIBPNG_VERSION}/...` |

### 1.3 Xerces-C Version Upgrade (3.2.3 to 3.3.0)

Xerces-C has been upgraded from 3.2.3 to 3.3.0 across all build scripts and path references on both Linux and Windows.

### 1.4 CMake Compatibility Fixes

Added `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to the Xerces-C and zlib installer scripts to prevent build failures with recent CMake versions.

### 1.5 CARLA Assets Download URL

The original CARLA content assets server is no longer accessible. The download process has been updated to use Backblaze B2 storage:

| Platform | URL |
|---|---|
| Linux/Mac | `https://carla-assets.s3.us-east-005.backblazeb2.com/${CONTENT_ID}.tar.gz` |
| Windows | `https://carla-assets.s3.us-east-005.backblazeb2.com/%CONTENT_ID%.tar.gz` |

---

## 2. PythonAPI Build Fixes

### 2.1 Python Version Compatibility

Python 3.8+ can fail to compile the Boost b2 engine (Boost 1.72.0), which is required for building the CARLA PythonAPI. Python 3.7 has been verified to work correctly.

If `pyenv` is used to manage Python versions, a symlink fix is required for the Python headers:

```bash
ln -s ~/.pyenv/versions/3.7.9/include/python3.7m ~/.pyenv/versions/3.7.9/include/python3.7
```

### 2.2 PythonAPI Segfault Fix (`import carla`)

Once the PythonAPI is built, `import carla` crashes with a segmentation fault on Ubuntu 22.04. The root cause is that clang-14's optimizer enables strict aliasing at `-O2`/`-O3`, which conflicts with type-punning patterns used internally by boost::python. This corrupts exception handling cleanup during module initialization.

The fix adds `-fno-strict-aliasing` to the compiler flags in `setup.py`, disabling the problematic optimization for the Python bindings only.

### 2.3 New `setup.py` for the PythonAPI

Added the `setup.py` file for building the CARLA Python API extension module (`carla.libcarla`). Supports both Linux and Windows builds, linking against all required static libraries (libcarla_client, rpc, boost, Recast, Detour, osm2odr, xerces-c, proj, sqlite3, etc.).

### 2.3 Windows `BuildPythonAPI.bat` Fixes

- Fixed Python detection logic: the fallback from `python` to `py -3` was not working due to incorrect error level handling.
- Added `call` keyword to the Python invocation to prevent premature batch file termination.

### 2.4 Windows `install_zlib.bat` Fix

Fixed a trailing backslash in the source directory path that caused CMake to misinterpret it as an escape sequence.

---

## 3. Windows Build Support

The original CARLA repository contained several `.bat` build and installer scripts with defects that prevented compilation on Windows. Corrected versions have been included and are deployed during `make install`, fixing the full dependency installation and CarlaUE4 compilation process:

| Script | Purpose |
|---|---|
| `BuildCarlaUE4.bat` | Compile the CarlaUE4 Unreal Engine project |
| `BuildOSM2ODR.bat` | Build the OpenStreetMap to OpenDRIVE converter |
| `Package.bat` | Create an optimized shipping package |
| `install_boost.bat` | Download and build Boost |
| `install_chrono.bat` | Download and build Project Chrono |
| `install_gtest.bat` | Download and build Google Test |
| `install_proj.bat` | Download and build the PROJ library |
| `install_recast.bat` | Download and build Recast & Detour |
| `install_rpclib.bat` | Download and build rpclib |

---

## 4. Linux Build Modernization

### 4.1 Compiler Toolchain Update

The hard-coded `clang-8`/`lld-8` requirement has been replaced with the system default `clang`/`lld` packages, as the old versions are no longer available on modern Ubuntu distributions. Tested and verified with **clang-14** (default on Ubuntu 22.04).

### 4.2 Extended Ubuntu Support

The Linux installation guide now covers **Ubuntu 20.04 and newer** (including 22.04 LTS), replacing the previous guide that only addressed Ubuntu 20.04.

---

## 5. VehicleFactory Asset Management

The `VehicleFactory.uasset` is a CARLA blueprint that defines the available vehicle catalog. Users with custom vehicle configurations typically maintain their own version of this asset. New flags have been added to support both workflows.

### 5.1 Install (`make install`)

By default, the installer **preserves** the user's existing `VehicleFactory.uasset`. To install the default CARLA version:

```bash
make install CARLA=../carla USE_DEFAULT_VEHICLE_FACTORY=1
```

### 5.2 Reverse-Install (`make r-install`)

By default, the reverse-install **skips** `VehicleFactory.uasset` to prevent overwriting the repository's version. To include it:

```bash
make r-install CARLA=../carla OVERWRITE_DEFAULT_VEHICLE_FACTORY=1
```

---

## 6. Git LFS Integration

Git Large File Storage (LFS) has been configured for the repository to properly handle Unreal Engine binary assets (`.uasset` and `.umap` files):

```
*.uasset filter=lfs diff=lfs merge=lfs -text
*.umap filter=lfs diff=lfs merge=lfs -text
```

---

## 7. Documentation Updates

- **`Install.md`**: Restructured into a step-by-step guide. Updated repository URLs, added Python compatibility notes, documented the Backblaze B2 asset download process, corrected the Scenario Runner repository name, and documented the `USE_DEFAULT_VEHICLE_FACTORY` option.

- **`Install_Ubuntu.md`**: Rewritten for Ubuntu 20.04+. Replaced `clang-8` instructions with system defaults and added `pyenv`-based Python version management guidance.

- **`DReyeVR.mk.help`**: Documented the `USE_DEFAULT_VEHICLE_FACTORY` and `OVERWRITE_DEFAULT_VEHICLE_FACTORY` options.

<br><br><br>
# Content authoring - vehicles

CARLA provides a comprehensive set of vehicles out of the box in the blueprint library. CARLA allows the user to expand upon this with custom vehicles for maximum extensibility.

3D modelling of detailed vehicles is highly complex and requires a significant degree of skill. We therefore refer the reader to alternative sources of documentation on 3D modelling, since this is beyond the scope of this guide. There are, however, numerous sources of vehicle models in both free and proprietary online repositories. Hence the user has many options to turn to for creating custom vehicles for use in CARLA.

The key factors in preparing a custom vehicle for CARLA lie in rigging the vehicle armature and then importing into the Unreal Engine. After rigging and importing, blueprints need to be set for the car and the wheels. Then apply materials and add the glass parts of the vehicle. We will cover these steps in the following guide.

* __[Modeling](#modeling)__
	* [Naming conventions](#naming-conventions)
* __[Rigging](#rigging-the-vehicle-using-an-armature)__
	* [Rigging troubleshooting](#rigging-troubleshooting)
	* [Import](#import)
	* [Armature](#add-an-armature)
    * [Parenting](#parenting)
    * [Assignment](#assigning-car-parts-to-bones)
	* [Blender add-on](#blender-ue4-vehicle-rigging-add-on)
    * [Export](#export)
* __[Import into Unreal Engine](#importing-into-unreal-engine)__
	* [Physics asset](#setting-the-physics-asset)
	* [Animation](#creating-the-animation)
    * [Blueprint](#creating-the-blueprint)
* __[Materials](#materials)__
	* [Applying materials](#applying-a-material-to-your-vehicle)
		* [Color](#color)
		* [Clear coat](#clear-coat)
		* [Orange peel](#orange-peel)
		* [Flakes](#flakes)
		* [Dust](#dust)
* __[Glass](#glass)__
	* [Glass meshes](#glass-meshes)
	* [Glass material](#glass-material)
	* [Single layer glass](#single-layer-glass)
* __[Wheels](#wheels)__
	* [Wheel blueprint](#wheel-blueprint)
	* [Collision mesh](#collision-mesh)
	* [Tire configuration](#tire-configuration)
	* [Wheel dimensions](#wheel-dimensions)
* __[Emissive meshes](#emissive-meshes)__
	* [UV map](#uv-map)
	* [Importing](#importing)

## Modeling

Vehicles should have between 50,000 and 100,000 faces. We recommend triangulating the model prior to export as best practice. CARLA vehicles are modeled using the size and scale of actual cars as reference. Please ensure you pay careful attention to the units of your 3D application. Some work in centimeters while others work in meters.

### Naming conventions

For ease and consistency we recommend that you divide the vehicle into the following parts and name them accordingly. Details specific to glass and lights will be covered in later sections:

>1. __Bodywork__: The metallic part of the vehicle. This material is changed to Unreal Engine material. Logos and details can be added but, to be visible, they must be painted in a different color by using the alpha channels in the Unreal Engine editor.
- __Glass_Ext__: A layer of glass that allows visibility from the outside to the inside of the vehicle.
- __Glass_Int__: A layer of glass that allows visibility from the inside to the outside of the vehicle.
- __Lights__: Headlights, indicator lights, etc.
- __LightGlass_Ext__: A layer of glass that allows visibility from the outside to the inside of the light.
- __LightGlass_Int__: A layer of glass that allows visibility from the inside to the outside of the light.
- __LicensePlate__: A rectangular plane of 29x12 cm. You can use the CARLA provided `.fbx` for best results, download it [here](https://carla-assets.s3.us-east-005.backblazeb2.com/fbx/LicensePlate.rar). The texture will be assigned automatically in Unreal Engine.
- __Interior__: Any other details that don't fit in the above sections can go into _Interior_.

Materials should be named using the format `M_CarPart_CarName`, e.g, `M_Bodywork_Mustang`.

Textures should be named using the format `T_CarPart_CarName`, e.g, `T_Bodywork_Mustang`. Textures should be sized as 2048x2048.

## Rigging the vehicle using an armature

To look realistic within the simulation, the car needs to have rotating and wheels, the front pair of which can turn with steering inputs. Therefore to prepare a vehicle for CARLA, an armature needs to be rigged to the car to identify the wheels and allow their movement.

### Rigging troubleshooting

The first step to prepare a vehicle for importing into Unreal is to build the rig correctly. After reviewing the rig of the assets you sent us, I confirmed that the rig is incorrect: it has an incorrect scale of 0.1, which is interfering with the rig’s behavior. This is most likely due to an error when creating the rig in Blender and importing it into Unreal Engine.

It is also worth noting that the first bone in the rig you sent is named VehicleBase. It is best to name it exactly as shown in the documentation images: “Vehicle_Base”.

<img src="img/WVTruck/WVTruck_1.png" alt="WVTruck_1" width="650">
<br><br>
Depending on the Blender version or the project configuration, a mismatch can occur between Blender and Unreal Engine scales. Since Unreal Engine uses centimeters as its unit of measurement, the best practice is to configure Blender the same way. You can do this in the Scene settings menu, as shown in the image.
<br><br>

<img src="img/WVTruck/wvtruck_3.png" alt="WVTruck_3" width="450">

<br>
After adjusting Blender’s scene scale, you may need to update the camera’s clipping distance in case your model disappears from the viewport.
You can do this from Blender’s information panel. Press N in the viewport to open this panel, then go to the View tab, and then adjust the Clip Start and Clip End values. You can use values similar to those shown in the picture below.

<img src="img/WVTruck/wvtruck_4.png" alt="WVTruck_4" width="450">

<br>

To make sure everything is correct, you can do this from the same menu mentioned above, in the Item tab, where you can inspect the transform of the selected object. Ideally, no bone in the armature should have any scale transforms—its scale should be 1. In addition, the first bone, “Vehicle_Base”, should be centered in the scene, meaning its location should be 0 on all three axes.

<img src="img/WVTruck/wvtruck_2.png" alt="WVTruck_2" width="800">


### Import

Import or model the vehicle model mesh in your 3D modelling application. In this guide we will use Blender 3D. Ensure that the wheels are separable from the main body. Each wheel must be accessible as a distinct object.

![model_in_blender](img/tuto_content_authoring_vehicles/import_model_blender.png)

It is important to ensure that the vehicle faces in the positive X direction, so the hood and windshield should be facing towards positive X. The car should also be oriented such that the floor to roof direction is in the positive Z direction. The wheels should be just grazing the X-Y plane and the origin should be situated where you would expect the vehicle's center of mass to be in the X-Y plane (not in the Z plane though).

### Add an armature

Now add an armature to the center of the vehicle, ensure the object is properly centered, the root of the armature bone should be set at the origin. Switch to edit mode and rotate the armature 90 around the x axis.

![armature_init](img/tuto_content_authoring_vehicles/vehicle_base_bone.png)

Now select the armature and add 4 more bones. Each of these bones needs to be located such that the root of the bone coincides with the centre of the each wheel. This can be achieved by locating the 3D cursor at the center of each wheel in edit mode. Select one of the wheels in object mode, select a vertex, press A to select all vertices then `Shift+S` and select `Cursor to selected`. This will locate the cursor in the center of the wheel. Then, in object mode, select the armature, switch to edit mode, select a bone and choose `Selection to cursor`. Your bone will now coincide with the wheel. Rotate each bone such that it lines up with the base of the armature.

For each wheel, it is recommended to name the bone according to the wheel it needs to be coupled to, this will help in identification later when you need to assign vertex groups to each bone.

![armature_full](img/tuto_content_authoring_vehicles/all_vehicle_bones.png)

### Parenting

Now select all the parts of the body and all 4 wheels using shift or control in the project outliner,  then control select the armature you have created (this order is important, it won't work if you select these in reverse order). Press `Ctrl+p` and select `With empty groups` to bind the mesh to the armature.

![bind_armature](img/tuto_content_authoring_vehicles/bind_armature.gif)

Now you have parented the mesh to the armature, you now need to assign each wheel to its respective bone. Select a wheel either in the outliner or the editor. Switch to edit mode, and select all the vertices of the wheel (shortcut - `a`).

### Assigning car parts to bones

Select the mesh tab of the properties (the green triangle). Inside the vertex groups tab of the mesh properties panel, you should now see the bones of your armature. Select the bone corresponding to the wheel you are editing and select `Assign`. Once you have rigged the wheels, rig all other parts of the vehicle to the base bone.

![assign_bone](img/tuto_content_authoring_vehicles/assign_vertex_group.gif)

Once you have assigned all of the mesh parts to the armature you can test if it works by selecting the armature and moving to pose mode and moving the relevant bones. The vehicle base bone should move the whole vehicle, while the wheel bones should each move and rotate their respective wheels. Ensure to undo any posing you might do with `Ctrl+Z`.

![test_armature](img/tuto_content_authoring_vehicles/test_pose.gif)

### Blender UE4 vehicle rigging add-on

There is a very useful add on for blender for rigging a vehicle for import into CARLA that helps streamline the above steps. Please see the [__add-on webpage__](https://continuebreak.com/creations/ue4-vehicle-rigging-addon-blender/) for instructions.

### Export

Now we will export our rigged model into FBX format for import into Unreal Engine. Select `Export > FBX (.fbx)` from the File menu. In the `Object Types` section of the `Include` panel, shift select the `Armature` and `Mesh` options.

In the `Transform` panel. Change `Forward` to `X Forward` and change `Up` to `Z Up`. This is important to ensure the vehicle is oriented correctly in the Unreal Engine.

In the `Armature` section uncheck `Add Leaf Bones` and uncheck `Bake Animation`.

![export_fbx](img/tuto_content_authoring_vehicles/export_fbx.gif)

## Importing into unreal engine

Launch the Unreal Editor with the `make launch` command from the CARLA root directory (the one where you have built CARLA from source). Open a content browser, set up an appropriate directory and right click and select `Import to ....`. Choose the FBX file that you previously exported from Blender (or another 3D modelling application). Import with default settings.

### Setting the physics asset

You will now have 3 things in your content browser directory, the mesh, the skeleton and the physics asset. Double click on the physics asset to adjust it.

![regenerate_body](img/tuto_content_authoring_vehicles/physics_asset.png)

First, select the main body, in the `Details` menu on the right, change the `Linear Damping` to 0.0 in the `Physics` section, check `Simulation Generates Hit Events` in the `Collision` section and change the `Primitive Type` from `Capsule` to `Box` in the `Body Creation` section. Then press `Regenerate bodies`. The capsule will now change to a rectangular box. It’s good practice not to place the collision box at ground level; it should cover the same area as the vehicle’s chassis. 

![physics_details](img/tuto_content_authoring_vehicles/physics_details.png)

Now select all the wheels (in the `Skeleton Tree` section on the left).

![regenerate_wheels](img/tuto_content_authoring_vehicles/wheels_asset.png)

Change `Linear Damping` to 0.0, set `Physics Type` to `Kinematic`, set `Collision Response` to `Disabled` and select the `Primitive Type` as `Sphere`. Press `Re-generate Bodies` once more.

![regenerate_wheels](img/tuto_content_authoring_vehicles/wheel_physics_details.png)

### Creating the animation

In the content browser directory where you have your new vehicle asset, right click and choose `Animation > Animation Blueprint`. In the popup that opens, search for `VehicleAnimInstance` in the `Parent Class` section and for the `Target Skeleton` search for the skeleton corresponding to your new vehicle, you should be able to see the name in your content browser. After selecting these two things press OK. This will create a new animation blueprint for your vehicle.

![animation_blueprint](img/tuto_content_authoring_vehicles/create_anim_blueprint.png)

To simplify things, we can copy the animation from another vehicle. In a second content browser, open `Content > Carla > Static > Vehicles > 4Wheeled` and choose any vehicle. Open the animation blueprint of your chosen vehicle and then copy all nodes that are not the `Output pose` node from this into your new animation blueprint. Connect the nodes by dragging a new connection between the final node to the output node. Press compile and the animation blueprint is now set.

![copy_nodes](img/tuto_content_authoring_vehicles/animation_blueprint_setup.gif)

### Creating the blueprint

Navigate with your content browser into `Content > Carla > Blueprints > Vehicles > LincolnMKZ2017` or a similar vehicle. In here you will find a set of blueprints set up for the 4 wheels. Copy these into the directory containing your own vehicle and rename them to ensure you can distinguish them later. You can set up your own custom wheels if you prefer, please refer to the later [__wheels section__](#wheels)

![copy_wheels](img/tuto_content_authoring_vehicles/copy_wheels.png)

Right click in the content browser directory where your new vehicle assets are and chose `Blueprint Class`. Search in the `All Classes` menu for `BaseVehiclePawn` and choose this class. Name the blueprint and open it. Select `Mesh` in the `Components` tab on the left and then drag the vehicle mesh into the Mesh section on the right hand side.

![blueprint_with_mesh](img/tuto_content_authoring_vehicles/blueprint_with_mesh.png)

In `Anim Class` search for the animation corresponding to your new vehicle that you set up in the previous step.

Next, select `Vehicle Movement` in the `Components` menu of the blueprint class and in the right `Details` menu navigate to the `Vehicle Setup` section. Now for each wheel, find the relevant wheel blueprint that you previously copied and renamed for the `Wheel Class` attribute. Do the same for each wheel. Compile and save.

![wheel_setup](img/tuto_content_authoring_vehicles/vehicle_wheel_setup.gif)

You can also add lights to the vehicle to simulate the headlights and interior lighting. To do this, click Add Component and select the type of light you want to use. 

<img src="img/WVTruck/wvtruck_5.png"  width="400">

<br><br>
After that, place the light in the correct position using the Move gizmo, and use the light’s cone/preview shape as a guide to aim and adjust it.


<img src="img/WVTruck/wvtruck_6.png">

<br><br>
Finally, you can tweak the light’s settings—such as its intensity and attenuation distance—from this panel, which appears once the light is selected. Any changes made here will update how the light preview looks in the viewport.

<img src="img/WVTruck/wvtruck_7.png"  width="600">


<br><br>
Now navigate to `Content > Carla > Blueprints > Vehicles > VehicleFactory` and double click this to open the Vehicle Factory.

Select the `Vehicles` node and expand the `Vehicles` item in the `Default value` section on the right hand side.

![vehicle_factory](img/tuto_content_authoring_vehicles/vehicle_factory_page.png)

Press the plus icon to add your new vehicle. Scroll down to the last entry and expand it, it should be empty. Name the make and model of your vehicle and under the class section find your blueprint class that you created in the previous section. Leave the number of wheels as 4 and put the generation as 2. Compile and save. Do a global save for safety and you are now..ready to run your vehicle in a simulation.

Press play in the unreal toolbar to run the simulation. Once it is running, open a terminal and run the `manual_control.py` script with the filter option to specify your new vehicle model:

```sh
python manual_control.py --filter my_vehicle_make
```
![manual_control](img/tuto_content_authoring_vehicles/manual_control.gif)

As it is, the vehicle currently has no textures or colors applied. The next step is to apply materials to give your vehicle a finish like a real road vehicle.

## Materials

Once you have your vehicle imported as a basic asset with the mesh and blueprints laid out, you now want to add materials to your vehicle to facilitate photorealistic rendering in the Unreal Engine, for maximum fidelity in your machine learning training data.

The Unreal Editor boasts a comprehensive materials workflow that facilitates the creation of highly realistic materials. This does, however, add a significant degree of complexity to the process. For this reason, CARLA is provided with a large library of material prototypes for you to use without having to start from scratch.

### Applying a material to your vehicle

CARLA provides a prototype material for replicating the glossy finish of vehicles that can mimic numerous different types of vehicle paint jobs and features. Open Unreal editor and in the content browser, locate the material in `Content > Carla > Static > GenericMaterials > 00_MastersOpt`. The basic material is called `M_CarPaint_Master`. Right click on this material and choose `Create Material Instance` from the context material. Name it and move it into the folder where your new vehicle content is stored.

In the Unreal Editor, move the spectator to a point near the floor and drag the skeletal mesh of the vehicle from the content browser into the scene, the body of your vehicle will now appear there. 

__Important:__ The following steps show how to apply materials to a vehicle using an asset dragged into the scene for easier preview and visualization. However, any changes made to this placed instance will not affect the original asset.
To assign the final materials to the vehicle, open the corresponding components (e.g., Skeletal Mesh, glass meshes, etc.) by double-clicking them in the Content Browser, and then assign the materials in the editor window that opens for each component.

![add_model](img/tuto_content_authoring_vehicles/add_model.gif)

Now, in the details panel on the right hand side, drag your new material instance into the `Element 0` position of the `Materials` section. You will see the bodywork take on a new grey, glossy material property.

![apply_material](img/tuto_content_authoring_vehicles/apply_material.gif)

Double click on the material in the content browser and we can start editing the parameters. There are a numerous parameters here that alter various properties that are important to mimic real world car paint jobs. The most important parameters are the following:

#### __Color__

The color settings govern the overall color of the car. The base color is simply the primary color of the car this will govern the overall color:

![change_base_color](img/tuto_content_authoring_vehicles/change_base_color.gif)

#### __Clear coat__

The clear coat settings govern the appearance of the finish and how it reacts to light. The roughness uses a texture to apply imperfections to the vehicle surface, scattering light more with higher values to create a matte look. Subtle adjustments and low values are recommended for a realistic look. Generally, car paint jobs are smooth and reflective, however, this effect might be used more generously to model specialist matte finishes of custom paint jobs.

![change_roughness](img/tuto_content_authoring_vehicles/roughness.gif)

An important parameter to govern the "shininess" or "glossiness" of your car is the `Clear Coat Intensity`. High values close to 1 will make the coat shiny and glossy.

#### __Orange peel__

Finishes on real cars (particularly on mass produced cars for the general market) tend to have imperfections that appear as slight ripples in the paint. The orange peel effect mimics this and makes cars look more realistic.

![change_orange_peel](img/tuto_content_authoring_vehicles/orange_peel.gif)

#### __Flakes__

Some cars have paint jobs that include flakes of other material, such as metals or ceramics, to give the car a `metallic` or `pearlescant` appearance, adding extra glints and reflections that react in an attractive way to light. The flakes parameters allows CARLA to mimic this. To mimic metallic finishes, it would be

![flakes](img/tuto_content_authoring_vehicles/flakes.gif)

#### __Dust__

Cars often accumulate grease and dust on the body that adds additional texture to the paint, affecting the way it reflects the light. The dust parameters allow you to add patches of disruption to the coat to mimic foreign materials sticking to the paint.

![dust](img/tuto_content_authoring_vehicles/change_dust.gif)

## Glass

Creating realistic glass in CARLA requires some tricks to capture the real refractive and reflective behavior of glass used in motor vehicles. The CARLA garage vehicles have 4 layers of meshes for the glass, with 2 different materials. The layers are separated by a few millimeters and there are separate materials for the interior and exterior facing glass layers to ensure that the glass looks realistic from both inside and outside the vehicle.

There are 2 layers of glass for the appearance of the vehicle from outside and 2 layers for the appearance of glass from the interior of the vehicle. What makes glass look like glass is the reflections coming from both surfaces of the glass that makes a very subtle doubling of the reflection.

### Glass meshes

Here we see the glass parts attached to the main bodywork (not the doors or other moving parts) of the Lincoln.

![main_glass](img/tuto_content_authoring_vehicles/glass.png)

If we separate the constituent mesh parts, we can see that the glass profile is separated into 4 different layers.

![main_glass_expanded](img/tuto_content_authoring_vehicles/glass_expanded.png)

The 4 layers are separated into 2 groups, the exterior layers, with normals facing out of the vehicle and the interior layers, with mesh normals facing into the vehicle interior. The following diagram demonstrates

![glass_layers](img/tuto_content_authoring_vehicles/glass_layers.png)

Once you have created your mesh layers, import them in the content browser into the Unreal Editor in the folder where you have stored your vehicle.

Shift select the 4 glass layers and drag them into the map so you can see them.

![drag_glass](img/tuto_content_authoring_vehicles/drag_glass.gif)

### Glass material

Double click the external layer of the glass, then navigate in a second content browser window to `Content > Carla > Static > Vehicles > GeneralMaterials` and find the `Glass` material. Drag the glass material to the material slot of the mesh item. Repeat this process for each layer of the glass.

The glass will now be transparent, but with reflectivity that reflects nearby objects and light sources. You should also check the interior glass, ensure there is a proper glass effect there.

![glass_reflections](img/tuto_content_authoring_vehicles/glass_reflections.gif)

### Single layer glass

For a quicker way to produce the glass parts of vehicles, the only critical part is the outermost glass layer. You can apply the glass material to this in Unreal Editor and get a result that might be suitable to your needs, however, views from inside the vehicle (i.e. if you instantiate a camera on the dashboard or behind the steering wheel) will seem to have no glass (no refraction or reflection). We recommend the above process to produce maximally realistic glass.

Now you have created the blueprint, added meshes, completed rigging, created materials for the paint finish and the glass, you should have a very realistic looking vehicle.

![finished_lincoln](img/tuto_content_authoring_vehicles/finished_lincoln.png)

## Wheels

If you copied the wheels when you were [creating the blueprint](#creating-the-blueprint), this might suit your purposes if your vehicle is very similar to vehicles that are already in the CARLA library. However, if your vehicle has non-standard wheel dimensions or grip characteristics, you should follow this section to set up your wheel blueprints to best match the physical characteristics of your vehicle's wheels.

For the wheels of CARLA vehicles, we need to set up a blueprint class for each wheel to deal with the mechanics and collision properties. You will set up 4 blueprint classes, we recommend the following prefixes or suffixes to identify the wheels:

- __RRW__ - **R**ear **R**ight **W**heel
- __RLW__ - **R**ear **L**eft **W**heel
- __FRW__ - **F**ront **R**ight **W**heel
- __FLW__ - **F**ront **L**eft **W**heel

### Wheel blueprint

Inside the folder where you have your new vehicle, right click and choose to create a new blueprint class. Search for

![wheel_blueprint](img/tuto_content_authoring_vehicles/wheel_blueprint.png)

Double click on the blueprint to adjust it:

![wheel_blueprint_open](img/tuto_content_authoring_vehicles/wheel_bp_open.png)

### Collision mesh

Firstly, the default cylinder used for the collision mesh has a high polygon count, so we should replace this with a low polygon version. In the content browser locate the `CollisionWheel` mesh inside `Content > Carla > Blueprints > Vehicles`. Drag it onto the
`Collision Mesh` slot in the details panel of the blueprint. This will improve performance without any noticeable deficit to physics simulation.

### Tire configuration

Next, we  set the tire configuration. Inside `Content > Carla > Blueprints > Vehicles` locate the `CommonTireConfig` configuration and drag it onto the `Tire Config` section of the blueprint. If you double click on the Tire Config in the blueprint, you can adjust the Friction Scale, you can modify the behavior of the vehicle's road handling. By default it is set at 3.5, a value suitable for most vehicle use cases. However, if you wish to model for example a racing vehicle with slick tires, this would be the appropriate parameter to adjust.

### Wheel dimensions

Next, in your 3D application, measure the diameter of your wheel. In Blender, the dimensions can be viewed in the properties panel opened by pressing `n` in object mode.

![tire_dimensions](img/tuto_content_authoring_vehicles/wheel_dims.png)

Now plug these numbers into the `Wheel` section of the blueprint.Take care to remember to half the diameter for the radius and also that Unreal Editor works in units of centimeters. For the wheel mass, we recommend looking for specifications on the internet, find the right tire model or a similar one to estimate the correct mass (in kilograms).

![bp_wheel_dimensions](img/tuto_content_authoring_vehicles/bp_wheel_dimensions.png)


`Affected by handbrake` should be checked for both rear wheels.

`Steer angle` should be set to the maximum intended steer angle for both front wheels and set to zero for both rear wheels.

### __Suspension characteristics__

The default values here provide a reasonable starting point. View [__this guide__](tuto_D_customize_vehicle_suspension.md) to set suspension characteristics appropriate to your vehicle type.


## Emissive meshes

The last element to complete a realistic vehicle for CARLA is the lights, headlights, brake lights, blinkers etc. In your 3D modelling application, you should model some shapes that resemble the lights of the vehicle you are replicating. This would be flat discs or flat cuboid structures for most headlights. Some vehicles may also have strips of LEDs.

![lights_blender](img/tuto_content_authoring_vehicles/lights_blender.png)

### UV map

The different types of lights (headlights, blinkers, brake lights, etc.) are distinguished using a texture. You need to create a UV map in your 3D modelling application and position the lights to match up with the relevant region of the texture.

![lights_uv](img/tuto_content_authoring_vehicles/lights_uv_map.png)
<br>
You can download the texture for the emissive meshes’ UV layout here: 
<br>
<img src="img/WVTruck/emissive.png">.



### Importing

Import the light mesh into the Unreal Editor- After importing the light mesh:

- Drag the mesh item(s) into the **_Mesh (VehicleMesh) (Inherited)_** hierarchy in the **_Components_** panel.
- Select the extra meshes in the hierarchy and search for `Collision` in the **_Details_** panel.
- Set **_Collision Presets_** to `NoCollision`.
- Select any lights meshes in the hierarchy. Search for `Tag` in the **_Details_** panel and add the tag `emissive`.