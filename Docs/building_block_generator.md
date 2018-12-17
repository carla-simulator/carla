<h2>BuildingBlockGenerator</h2>

This handy tool allows for the quick creation of building blocks based on our trusty asset library. With multiple options to choose from. Buildings are categorized so you can exclude them depending on your needs or you could hand pick them any way you want. Choose a range of floor numbers to be randomized along all the block, rotate them at a fixed angle or aply a random offset to the façade position to create organic looking streets.

Creating a block with this tool is fairly easy. Just drop one BuildingBlockGenerator on the map, modify the spline until you get the block shape you need (You might add any number of points that you need) and hit the SpawnBuildings button. Please mind that the buildings won't follow the spline per se, they will only follow straight lines from one point to the next.


<h4>Filter system:</h4>

  In the Building block Blueprint there is a variable called InitialBuildingPool. This variable contains all building actors at the builders disposition (All of them children of BuildingMaster). If you want to add buildings to the library, this is the place to do it. Once every building is added you can filter them by the categories determined in their own blueprint The filter works in two steps:  The first one is the additive filter, It passes by all of the buildings stored in the initial building pool, adding the ones that mathch the categories you've selected under	the FilterIn variable and ignoring the ones who not. Then, the second Step takes every building added in the first step and discard the ones that match the categories selected in the FilterOut Variable. This way you can add every residential building but exclude any Comercial one, for example. Both this steps are triggered when you select your filters and clic the "filter" button.

		

<h4>Randomization:</h4>
A lot of randomization comes from the type of buildings created by the blueprint but some comes from the buildings themselves Selecting "RandomizeMaterials" will make each building to choose between a pool of materials and colors. This option can be deactivated once the building is spawned if you aren't convinced by the random results. Each building comes also with a set of props to spawn and some positions to spawn them at random. Both this options can be chosen for each block or after spawning
from each individual building.

<h4>Spawning events:</h4>

  - Delete last batch: The spawner saves references to the last set spawned in the world. Triggering delete last batch removes this last set of buildings.
 
  - Filter: Trigers the filter system. If no filter is used it will use al avaliable buildings in the 
		initial building pool.

  -	Get New Seed:
		Generates a new seed for the next batch of buildings, two blocks with the same form and the same seed should allways give the same sequence of buildings.

  -	RandomizeFloorNumberBlock:
		Gets a new range of floor numbers for each building Spawned under further spawn events.

  -	SpawnBuildings: 
		Spawns buildings based on all parameters previously setted.

  -	Reroll:
	Removes last spawned batch, gets a new seed and spawns a new batch of buildings.

  -	Generate seed and Spawn:
	generates seed and spawns a new batch.
		
		
		

<h4>Variables:</h4>


  - FinalBuildingPool: Once filtered, the list of the buildings that will be used to spawn. 
		
  - StoredBuildings: Lists the references to the last buildings spawned.

  -  InitialBuildingPool: The list of buildings before filtering.

  - BuildingAlignment: Choose between stacking buildings to the front, the back or the middle of the mesh.

  - RandomizeMaterials: Choose if you want to randomize the building Materials or not.

  - SpawnItems: Choose if you want to spawn Items with every building.

  -	ZrotationOffset: Adds a set rotation on the Z axis.

  - Gap: Adds a fixed Gap between all buildings spawned, Notice that this Gap will be changed if 
		"RearrangeBuildingsToLevelGap" is set to true.

  - MaxNumberOfFloors: Chooses the Max number of stores for the buildings that have multiple of them

  - MinNumberOfFloors: Chooses the Min number of stores for the buildings that have multiple of them

  - MaxRandomYPosition: Adds a random location transform on the Y axis on the choosen range.

  - RearrangeBuildingsToLevelGap: If true, once each segment is finished rearranges all buildings along 
		it to make all gaps Equal 
		
  - CutStartandEndOfSegment: Removes the last building of each line, giving more air to the corner ones

