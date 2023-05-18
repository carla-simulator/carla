!!! note
	The __Procedural Building Tool__ is currently an __experimental feature__ and is not considered *production ready* at this stage. Hence it should only be used for experimental research projects.

# Procedural Building Tool

- [__Opening the tool__](#opening-the-procedural-building-tool)
- [__Mesh parameters__](#mesh-parameters)
- [__Base parameters__](#base-parameters)

The __Procedural Building Tool__ facilitates the generation of virtual 3D buildings for which the dimensions and decoration styles can be modulated to create a near infinite array of variations through a simple interface. The footprint dimensions and height in stories can be chosen through the interface. Then users can select a variety of styles for the building lobby, the body and the top floor or penthouse. A variety of facia element styles can be chosen for features such as corners windows and balconies from the CARLA asset library. 

## Opening the Procedural Building Tool

Firstly, you need to add a procedural building actor to the scene. Navigate to `Content>Carla>Blueprints>LevelDesign` and drag the BP_ProceduralBuilding blueprint into your map. Then to open the tool, navigate to the CarlaToolsContent and open the Procedural Building tool by right clicking on the UW_ProceduralBuilding and selecting *Run editor utility widget* from the context menu. This will open the tool's interface. 

## Mesh parameters

Firstly, we should select some meshes to style the building's exterior. Click on the Mesh Parameters tab in the interface. There are 5 categories of mesh pieces for different parts of the building:

- __Bottom Meshes__: mesh pieces to decorate the lobby of the building
- __Middle meshes__: mesh pieces to decorate the midsection of the building, every floor between the lobby and the top floor.
- __Top meshes__: mesh pieces to decorate the penthouse level.
- __Door meshes__: mesh pieces to add doors to the lobby
- __Wall meshes__: mesh pieces to decorate the walls of the building

In each category you will find numerous options for mesh pieces. Click on one or more such that they turn red, these will be added to the respective section of your building. If you choose more than one, the tool will randomly alternate between the selections. 

## Base parameters

Once we have selected our preferred mesh pieces, we chose the basic parameters of the building. The parameters are as follows:

- __Seed__: sets the random seed for the procedural generation, this enables variations on the building with the same settings
- __Num floors__: sets the number of stories or floors the building will have, and henceforth defines the height of the building
- __Length X/Y__: defines the size of the footprint of the building in the X and Y dimensions
- __Create automatically__: if this option is selected, the building will automatically update in the viewport so you can see the effect of your adjustments
- __Corners__: allows corner pieces to be added to the building, you can choose these pieces in the Mesh Parameters section.
- __Walls__: replace the middle pieces of the left/right/front/back of the building with alternate pieces that can be selected using the Mess parameters menu
- __Doors__: array allowing the placement of a door in the lobby level. The door is placed at the chosen index position.

## Detailed mesh parameters

In this section you can select the more detailed elements of the building such as windows, columns, plant pots, air conditioning units and antennas. Each type of decoration has slightly different properties,. The parameters work as follows:

* __Percentage__: controls the amount of the pieces that will be placed on the building
* __Offset__: offset of the piece from the body of the building

* Blinds and curtains have a specific parameter:
	* __Min/Max size__: Only for the blinds/curtains section, selects the min/max size of the blinds or curtains with random variation between the values.

* Pipes and wires have specific parameters:
	* __Index__: defines the face of the building where the pipe will be placed
	* __Offset side/front__: adds a spatial offset to the piece from the center of the face of the building

## Facade materials

In the facade materials tab, you can select the material that you want to decorate the walls of your building.

## Cooking

In the cooking tab, you initiate the combination of all the mesh pieces and materials you've selected into a single static mesh with associated materials and textures. An LOD texture of the building will also be created for creating the LOD. Specify a folder name in the interface where the assets for your new building will be saved.

Once you have cooked your building, you will then be able to place instances of the building in the map just like any other CARLA assets.












