<h3>Building master:</h3>

We have configured settings to reunite all building actors under a single BlueprintClass, named BuildingMaster. This class contains all the variables related to any kind of building we design. This blueprint is able to randomize materials and tweak colors; place decorative meshes randomly to break the repetitivity; control the height and the number of floors of the building and hopefully, more things yet to come. There is currently only a few buildings making full use of all this blueprint functionality, but we expect to keep adding new assets following this new method. Other buildings are created as children of this Blueprint but they do not make use of it yet. Those buildings are only added for categorization and as a requirement of the BuildingBlockGenerator asset.



<h4>Categories:</h4>

Under every child of the BuildingMaster class there is a variable called categories. This category system will be used when filtering is applied by the building block generator. The categories in ehich the building is included should be “ticked” first in the building blueprint in order to declare it as such.

<h4>Randomizating/Configuring Materials:</h4>

Our Buildings store a number of tileable materials that can be selected and parameterized, either at random or with a specific materials. By filling named material Id's in the models requires one material for the wall, one for the roof, another for details, and additional materials for specific assets. Variables for each of those materials are stored under their name in the blueprint as follows:

  - Edit (Materialname) Material: Wether to Edit the material or not (Edited materials consume more memory space)
  - Preset (Materialname): The list of materials to use as base.
  - TexturesIndex: The current index of the item of the later variable to be used.
  - Rotation: When edited, rotation of the material in the Uv space.
  - Scale: When edited, Scale df the material in the uv space.
  - MaterialColor: When Edited, Color to overimpose over the material by.
  - DetailColor: When avaliable, second color of the material to edit.
  - (MaterialName) Material: Current Material in use.

This parameters might be Randomized by clicking on (Randomize Materials) this randomizes all material parameters except Edit(MaterialName), Rotation and Scale

<h4>PropPlacement</h4>
Certain number of buildings will have automatic placement of props.

  - Spawn Any: Wether to spawn any mesh or not.
  - Items to spawn: The items avaliable to be spawned.
  - PositionsToSpawn: the current positions setted as spawn points for the props.
  - RandomPositions: Wether to spawn on random positions.
  - ItemChances: When random positions is true the chance for each position to spawn an item.



MultipleFloorBuilding
---------------------

MultipleFloorBuildings are children of BuildingMaster with some specific uses to make repeating and varying tall buildings a bit easier. Provided a Base, a MiddleFloor (and optionally a Roof); this blueprint repeats the middle floor to the desired number of stores and tops it with the last floor given some conditions:

  - All model pivots should be in the bottom center of the Specific mesh.
  - Al models must start and end exactly where the repetition occurs.

This blueprint is controlled by this 6 specific Parameters:

  - GroundFloor: The mesh to be placed in the base of the building.
  - Floor: The mesh to be repeated along the building.
  - Roof: Final mesh to top the building.
  - FloorNumber: Number of stores of the building.
  - FloorHeightOffset: Adjust The placement of every floor vertically.
  - RoofOffset: Adjust the placement of the roof vertically.

Besides this, all parameters from The BuildingMaster class apply here.

All of This parameters can be modified once this blueprint is placed in the world.
