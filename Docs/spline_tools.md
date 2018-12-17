<h1>SplineBased repeaters</h1>

We've improved our worldbuilding assets with new options and better overall performance. 

<h5> Shared variables:</h5>
  
  - StaticMesh: The mesh to use/repeat. Can be any mesh for the RepSpline but Wall and Spline blueprints have some specifications to work properly: Walls meshes to be used for those blueprints need their origin set into the start of the wall and be facing to the positive X (Standar Unreal facing direction)  

  

<h2>BP_Spline</h2>

 is our old SplineMeshRepeater. Las Release we made this asset to hierarchically Instance each mesh but we noticed this caused some problems in Linux so we stepped back and now uses standard meshes again.

!!! Bug
    See [#35 SplineMeshRepeater loses its collider mesh](https://github.com/carla-simulator/carla/issues/35)

<h4>Standard use:</h4>

SplineMeshRepeater "Content/Blueprints/SplineMeshRepeater" is a tool included in
the Carla Project to help building urban environments; It repeats and aligns a
specific chosen mesh along a
[Spline](https://docs.unrealengine.com/latest/INT/Engine/BlueprintSplines/Reference/SplineEditorTool/index.html)
(Unreal Component). Its principal function is to build Typically tiled and
repetitive structures as Walls, Roads, Bridges, Fences... Once the actor is
placed into the world the spline can be modified so the object gets the desired
form. Each Point Defining the spline Generates a new tile so that as more points
the Spline has, the more defined it will be, but also heavier on the world. This
actor is defined by the following parameters:

  - StaticMesh: The mesh to be repeated along the spline.
  - ForWardAxis: Changes the mesh axis to be aligned with the spline.
  - Material: Overrides the mesh' default material.
  - Collision Enabled: Chooses the type of collision to use.
  - Gap distance: Places a Gap between each repeated mesh, for repetitive non continuous walls: bush chains, bollards...

(Last three variables are specific for some particular assets to be defined in
the next point) A requisite to create assets compatibles with this component is
that all the meshes have their pivot placed wherever the repetition starts in
the lower point possible with the rest of the mesh pointing positive (Preferably
by the X axis)


<h4>Specific Walls (Dynamic material)</h4>

In the project folder "Content/Static/Walls" are included some specific assets
to be used with this SplineMeshRepeater with a series of special
characteristics. The UV space of this meshes and their materials are the same
for all of them, making them exchangeable. each material is composed of three
different surfaces the last three parameters slightly modify the color of this
surfaces:

  - MainMaterialColor: Change the main material of the Wall
  - DetailsColor: Change the color of the details (if any)
  - TopWallColor: Change the color of the wall cover (if any)

   To add elements that profit from this functions exist the GardenWallMask File that defines the uv space to place the materials: (Blue space: MainMaterial; green space: Details; red space TopWall).

Between the material masters is WallMaster which is going to be the master of
the materials using this function. An instance of this material will be created
and the correspondent textures will be added. This material includes the
following parameters to be modified by the material to use:

  - Normal Flattener: Slightly modifies the normal map values to exaggerate it or flatten it.
  - RoughnessCorrection: Changes the Roughness value given by the texture.

The rest of the parameters are the mask the textures and the color corrections
that won't be modified in this instance but in the blueprint that will be
launched into the world.

<h2>BP_Wall</h2>
This blueprint Is a much simpler verion of the BP_Spline With a usefull added feature. Instead of placing a mesh in each point of the intersection it calculates the number of meshes required to fill all space between two points of the spline and even scales the meshes if needed, very usefull for any kind of urban fences or wall that doesn't need much customization.

  - Vertically. Aligned Wether to vertically allign the meshes
  - Scale Offset. Adds an Offset to the scale of the meshes lenghtwise. 


<h2>BP_RepSpline</h2>
This blueprint specializes in instancing and repeating a mesh over a straight line given a fixed Gap. works well with things like lines of trees, bushes, benches or streetlights. Its Rotation and translation offsets allow for some level of randomization.  

  - Distance Between Meshes: The distance between each mesh spawned origin. Is not aware of the mesh lenght.
  - Fixed Rotation Offset: Add the chosen fixed rotation to every mesh.
  - Random Rotation: Rotates randomly over this given threshold.
  - Random Translation: Once "Real" position is set adds a random translation to each mesh.
  - Max Number of Meshes: Security number, the actor wont add more meshes once it reachs this number of meshes.
  - World Aligned ZY: Wether to vertically allign the meshes
  - Place Meshes on points: wether to place a mesh on each point of the spline.




