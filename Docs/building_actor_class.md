<h3>Building master:</h3>

We've setted things up to reunite all building actors under a single BlueprintClass, this is BuildingMaster and contains al the variables relative to any kind of building we imagine. This blueprint is able to randomize
materials and tweak colors; place decorative meshes randomly to break the repetitivity, control the height or the number of floors of the building and hopefully, more things yet to come. There is currently only one building fully making use of all this blueprint current functionality but we expect to keep adding our assets under this building method. Other buildings are created as children of this Blueprint but dont make really any use of it yet, those are only added for categorization and as a requirement of the BuildingBlockGenerator asset.



<h4>Categories:</h4>

Under every child of the BuildingMaster there is a variable called categories. This category system will be used when filtering is made by the building block generator. The categories in wich the building is included should be ticked first in the building blueprint declaring it as such.

<h4>Randomizating/Configuring Materials:</h4>

Our Buildings store a number of tileable materials that can be selected and parametrized either at random or intentionaly chosen, filling specifically named Material Id's in the models involved one for the wall, one for the roof, another for details, and aditional materials for specific assets. Variables for each of those materials are stored under their name in the blueprint and for each of them are as follows:

  - Edit (Materialname) Material: Wether to Edit the material or not (Edited materials consume more memory space)
  - Preset (Materialname): The list of materials to use as base.
  - TexturesIndex: The current index of the item of the later variable to be used.
  - Rotation: When edited, rotation of the material in the Uv space.
  - Scale: When edited, Scale df the material in the uv space.
  - MaterialColor: When Edited, Color to overimpose over the material by.
  - DetailColor: When avaliable, second color of the material to edit.
  - (MaterialName) Material: Current Material in use.

This parameters might be Randomized by clicking on (Randomize Materials) this randomizes all material parameters except *Edit(MaterialName)*, *Rotation* and *Scale*

<h4>PropPlacement</h4>
Certain number of buildings will have automaticed placement of props configured.

  - Spawn Any: Wether to spawn any mesh or not.
  - Items to spawn: The items avaliable to be spawned.
  - PositionsToSpawn: the current positions setted as spawn points for the props.
  - RandomPositions: Wether to spawn on random positions.
  - ItemChances: When random positions is true the chance for each position to spawn an item.



MultipleFloorBuilding
---------------------

MultipleFloorBuildings are childs of BuildingMaster with some specific uses to make repeating and varying tall buildings a bit easier. Provided a Base, a MiddleFloor (And optionally a Roof); this blueprint repeats the middle floor to the desired number of stores and tops it with the last floor
given some conditions:

  - All model pivots should be in the bottom center of the Specific mesh.
  - Al models must start and end exactly where the repetition happen.

This blueprint is controlled by this 6 specific Parameters:

  - GroundFloor: The mesh to be placed in the base of the building.
  - Floor: The mesh to be repeated along the building.
  - Roof: Final mesh to top the building.
  - FloorNumber: Number of stores of the building.
  - FloorHeightOffset: Adjust The placement of every floor vertically.
  - RoofOffset: Adjust the placement of the roof vertically.

Besides this. All Parameters from The BuildingMaster also applies to them.

All of This parameters can be modified once this blueprint is placed in the world.