<h1>How to make a new map in Carla with RoadRunner</h1>

![Town03](img/createmap_01.jpg)

<h2>RoadRunner</h2>

RoadRunner is a powerful software from Vector Zero to create road maps. Get the hang of RoadRunner is easy, in few steps you can create a map. 
You can download Free Trial on:

 ![vectorzero](img/logovectorzero.jpg) https://www.vectorzero.io/

 
<h2>Step 1 - Create your map in RoadRunner:</h2>

  - When you have become familiar with RoadRunner and You are sure which map you want to make.
	You should keep in mind:
	- Create map centered arround 0.0. This will help to work better in a future with unreal and Opendrive
	- Just starting the map select Tools/TransformScene and aply a 180º rotation. Currently the opendrive appear rotated 180º. We are working on that.
	- Check the all conections and geometries
	- Check the OpenDrive with button "OpenDrivePrevewTool"


!!! note
	While creating the map, Junctions (The union between two roads) Might give some errors when building the OpenDrive File (clicking in "OpenDrivePreviewTool" 
	lets you test the validity of the current map). If this happens you shoud click it with the "ManouverRoadsTool" and in the details window, under junction, 
	click the "RebuildManouverRoads" button. Even if nothing seems to happen the road should be fixed.



<h3>Exporting:</h3>

After verifying that everything is correct. Export to Unreal. You need export 2 files to create a map in Unreal.

OpendDrive (.xodr) Information that cars need to be able to circulate on the map.

Binaries (.fbx) All meshes you need to build the map; Roads, lanemarking, sidewalk, ect..


  - Select File - Export - OpenDrive (.xodr)
  - Select File - Export - Filmbox (.fbx)
- Choose your export folder and Keep Merge Roads, Merge Marking and Merge Terrain _Unchecked_.
- Under Options Check "ExportToTiles" to divide the map into chunks, this keeps the render a bit lighter; Be carefull though, subdividing the
	map too much will also be inefficient. some experimentation is needed here to get the perfect point and pretty much always will depend on the map beeing created.
- Leave "Export individual Tiles" _Unchecked_, this will give you only one fbx file with all the pieces, much easier to keep track of it.


 ![Tutorial_RoadRunner](img/Tutorial_Roadrunner.jpg)

 <h2>Step 2 - Unreal</h2>

<h2>Importing into Unreal:</h2>

  - Start dragging your file from where it's saved to the "content browser" in your unreal's project as you would do with any other asset. 
- In the Import window make sure the following options are _unchecked_: ForcefrontXAxis, Autogenerate collission, combine meshes. 
- Make sure that ConvertSceneUnit is _checked_. 
- You should Also _check_ "Import Materials" and "Import Textures" if you want to use the roadrunner materials or use the method 
	described at the end of the next step to change them in bunches.

  ![Tutorial_Roadrunner_UE4](img/Tutorial_Roadrunner_UE4.jpg)


<h2>Working with the map in Unreal:</h2>
	
First of all create a level with the Map's Name (This will be Important later) and save It inside Game/Carla/Maps/	

Once everything is loaded up you should end with many staticMeshes in the folder you've loaded the map into. You could drag them all to the 
level and you´ll have your map in unreal. Depends on how many pieces you cut the map will load faster or slower. All the pieces share the same center pivot, so if they are positioned in the same place they should fit as they did in the RoadRunner Editor. When unreal finish loading the meshes must be centered at point 0.0.0. 

  ![Transform_Map](img/transform.JPG)

<h3>Semantic Segmentation</h3>

Once with the correct map and materials, going to prepare the semantic segmantation. For the semantic segmentation to work, you'll need to get the different pieces of the level and save them in their corresponding folder. This is a bit difficult today as with the current version there is no name identification for the different pieces but this is a just temporal issue on Roadrunner's part. The most probably you need only 3 Folders: ../Content/Static/ 1- Road, 2-Terrain, 3- RoadLines.

<h3>How to put meshes in a correct folder</h3>

Select the material of the items you want to change folder. (Importing the assets creates a road material, a sidewalk material and so on) 

1. Once selected Rightclick and select "Asset Actions/Select actors Using this asset" All actors using that material should be highlighted in the viewport.

2. Rightclick on them in the world and select "Browse to asset" All assets using the first material you´ve chosen should be selected in the Content browser. 

3. Move them all to the corresponding folder and repeat for each category until every road asset is clasified.
	

Roadrunner roads come with premade materials, but for Carla's weather to work properly you'll need to change the pavement materials to some of our own
(Stored under Carla/Static/GenericMaterials/WetPavement) either go changing the materials one by one or you could remove the Roadrunner Materials.Triggering the replace references window in which you can select and change the new material for the meshes the last one was referencing.



<h2>Step 3 - Loading OpenDrive into Carla.</h2>
	
  - In Roadrunner go into File/Export and choose OpenDrive(.xodr), In the Export Window choose the save folder and name the file with the same name as the map
	let everything else as default and Export.


  - Copy the .xodr file inside the Content/Carla/Maps/OpenDrive folder.

  - Open Your Unreal's level, drag the OpenDriveActor inside the scene. This will Read the level's name, search the Opendrive file
	with the same name and Load it.
	
  - If everything went right you should have your Road information loaded into your map. Congratulations!

!!! note
	It is possible that the map that generates that of Opendrive, goes out turned. Maybe you didn't rotate before in RoadRunner. You only select all map import from RoadRunner and rotate Z:180.

  

<h2>Setting up the traffic based on OpenDrive:</h2>

!!! Note
    the current version of carla needs a CarlaMapGenerator Spawned in order for our vehicles to drive. This is already reported as a bug and will be fixed as soon as posible. For the moment Create a CarlaMapGenerator away from your city, never above or under it (Then this map would drive the vehicles too) and build its navigation. 


  - You'll need to place points for the vehicles to spawn. The actor that sets the spawn position of the vehicles is called VehicleSpawnPoint. Carla vehicles must
	overlap with one of the RoutePlanner's trigger box for them to be directed, otherwise they would drive straight until they find an obstacle. We suggest placing 
	the spawners 2 to 3 meters avobe each trigger box, not much higher as if the spawn is too high it is not able to measure if there is any vehicle under it before spawn and 
	they would end spawning on top of each other. Spawning the vehicles a few meters behind the trigger boxes would also work but that is a bit trickier. 

  - Spawners must always be oriented as the road dictates (this must be done by hand but we'll make some form of automation in the future)	

  - Traffic lights must be placed in every crossing in whitch vehicles might conflict. theese are the childs of "TrafficLightBase" and are stored
	inside Game/Carla/Static/TrafficSigns/Streetlights_01. 


<h2>Working with BP_TrafficLight and BP_TrafficLightGroups:</h2>

	
  - One trafficlight must be placed at every entrance that needs to be regulated and one TrafficLightGroup must be placed in every crossing that has street lights. Adjust
	The triggerbox of each traffic light until it covers the roads it must stop.

  - Once the actors of an Intersection are placed, open the corresponding group and under TrafficLights store all TrafficLight actors involved.
	
  - Trafficlight group works by Rotating whitch of the involved traffic light is green each moment. You can configure the timing the Lights stays in green (GreenTime), the time it stays yellow
	(YellowTime), and the time it takes between one trafficlight goes red and the next one goes green (ChangeTime).
	

  - This last steps are not full-proof and will require a good amount of trial and error from the user part. VehicleSpawner placement, Street light timing and Traffic light StopperBoxes might 
	need some tweaking and testing to fit perfectly into the built city.

	