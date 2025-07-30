# NVIDIA SimReady

**SimReady** is a standard specification for information and metadata associated with 3D assets intended to be used in simulation. The SimReady standard ensures that a 3D asset is not only a high fidelity visual resource in a digital twin, but also has a well-organized metadata definition to include semantic labelling, dense captions, non-visual sensor information and physical properties.

* __[SimReady converter](#simready-converter)__  
* __[Exporting a SimReady asset](#exporting-an-asset-with-the-simready-converter)__
    * [Exporting a map](#exporting-a-map)
    * [Exporting a vehicle or pedestrian](#exporting-a-vehicle-or-a-pedestrian)
* __[SimReady export options](#simready-export-options)__
* __[Semantic labels and Q-codes](#semantic-labels-and-q-codes)__

## SimReady converter

The NVIDIA SimReady converter enables CARLA assets to be exported to the Universal Scene Description (USD) format in a SimReady compliant manner, facilitating import into other applications from the OmniVerse ecosystem such as Sensor RTX or the RTX renderer for co-simulation. 

The SimReady converter maps Unreal objects, properties and parameters to USD/SimReady compliant equivalents, where possible, and applies appropriate metadata (for example semantic labels) for simulation purposes.

### Exporting an asset with the SimReady converter

#### Exporting a map

To export a map, locate the appropriate map file in the CARLA content browser. For example, *Town10HD_Opt* in `Content > Carla > Maps`. 

![map_asset](img/nvidia/map_file.png)

Double click the map asset in the content browser to load it. Select `File > Export All`. Set `Save as type` to *Universal Scene Description binary SimReady assets for AV Sim (\*.usd)*.

![export_simready_map](img/nvidia/export_simready_map.png)

Choose a folder location for the asset. On clicking the *Save* option, an options dialogue will appear. Using the default options to export will work in most cases. See the [SimReady options](#simready-export-options) section for more details on the export options.

![options_simready_map](img/nvidia/simready_export_options_map.png)

#### Exporting a vehicle or a pedestrian

To export a vehicle or a pedestrian, locate the blueprint file. For example, *BP_Ford_Crown* in `Content >  Carla > Blueprints > Vehicles > Ford_Crown` or *BP_Walker* in `Content > Carla > Blueprints > Walkers`. Right click on the blueprint and select *Export as USD*.

![vehicle_bp](img/nvidia/bp_simready_export.png)

A file browser will open to choose a location for the exported USD file. After selecting *Save*, an options dialogue will open. The default options will work for most cases. 

![bp_export_options](img/nvidia/simready_bp_export_options.png)

For more details on SimReady export options please see the following section.

## SimReady export options

The two most important options are the following:

* `Include MDL` - selecting this option will convert Unreal materials into MDL materials. Ensure that the application you are exporting for is compatible with MDL materials. Omniverse Kit applications are compatible with MDL materials. 

* `Include USD Preview Surface` - selecting this option exports the USD with a USD preview surface which ensures a wider compatibility for visualisation, but with a more limited set of material features. This helps ensure that the asset can be viewed correctly in applications that do not support the full MDL specification.

The following table gives details about the available export options. Further details can be found in the Omniverse Connect documentation [here](https://docs.omniverse.nvidia.com/connect/latest/ue4/manual.html#exporting-to-omniverse-as-usd-and-mdl). 

| Option | Result | 
|--------|--------|
| Export Method | Shared organizes materials so they can be shared by many assets\n Modular exports a<br> material for every asset so that assets and their materials are all self-contained | 
|Include MDL | Export MDL materials
| MDL Dependency Option | Include Dependencies - Duplicates the MDL dependencies (templates) in Destination Folder<br> Use Core Library Dependencies - Core MDL Library templates are referenced | 
| Destination Unreal Template Path | Destination Folder on Omniverse Nucleus for the MDL dependencies (templates)<br> Note: A file must be selected, but only the folder is used |
| Texture Size | Use Texture Setting or Source as Exported Texture Size |
| Export Texture as DDS |  Save DDS Files to Destination Path and reference them from materials |
| Include USD Preview Surface | Export Preview Surfaces |
| Include Unreal Materials | Export a reference to an Unreal material (note, when importing/opening<br> the material must be available in the within the Unreal project) |
| Create a material overrides layer | Export a sublayer that contains all of the material reference overs for the stage. <br> Also create USD files for each material rather than including them in the prop stages. |
| Export physics data | Include collision data with static meshes (not available with Unreal Engine 5.0 due to USD Schema updates) |
| Export source geometry for<br> Nanite high-poly detailed mesh | Brute-force export the Nanite geometry as USD geometry data.<br> (only available with Unreal Engine 5.0, this generates a LOT of triangles) |
| USD Scenegraph Instancing | Make every referenced mesh in a stage an instance. If the stage has<br> duplicate meshes this can greatly reduce geometric GPU memory in<br> another renderer, but the editing workflow may be hampered. |
| Export as Payloads | Use Payloads instead of References in the stage |
| Export as Y-up axis | If checked, USD and MDL are exported with a Y-up<br> stage up-axis rather than the default Z-up |
| Export Sublevels as Sublayers | If checked, levels are exported as USD sublayers<br> and referenced in the root stage |
| Convert Skeleton root to identity transform | This option will convert a Skeleton’s root node to an identity transform to assist<br> downstream tools that require this. All animations exported during this<br> export will be transformed properly so they’ll work in downstream tools. |
| Include invisible levels | If checked, levels that are hidden will be included<br> in the export and marked as invisible |
| Export Decal Actors as planes | If checked, decal actors are exported as “floating”<br> plane meshes offset from their base mesh |
| Add extra extension to exports | Stage files will be named `.stage.usd`, props will be named `.prop.usd` |
| Checkpoint Comment (Optional) | Set the checkpoint comment on the Nucleus server for each exported file |

## Semantic labels and Q-codes

The SimReady standard uses [Q-codes](https://en.wikipedia.org/wiki/Q_code) for semantic labelling. The Q-code is a standardised collection of three-letter codes that all start with the letter Q. The SimReady exporter applies the appropriate Q-code for each semantically labelled item exported from CARLA as part of the object's metadata in the USD file. You can look up Q-code definitions on the [Wikidata site](https://www.wikidata.org/wiki/Wikidata:Main_Page).

The following table shows the mapping of CARLA semantic classes to Q-codes:

| Label | Q-code |
|-------|--------|
| Road | Q34442 |
| SideWalk | Q177749 |
| Building | Q41176 |
| Wall | Q42948 |
| Fence | Q148571 |
| Pole | Q2180428 |
| TrafficLight | Q8004 |
| TrafficSign | Q170285 |
| Vegetation | Q187997 |
| Terrain | Q186131 |
| Sky | Q527 |
| Pedestrian | Q221488 |
| Rider | Q11998775 |
| Car | Q1420 |
| Truck | Q43193 |
| Bus | Q5638 |
| Train | Q870 |
| Motorcycle | Q34493 |
| Bicycle | Q11442 |
| Static | Q2302426 |
| Dynamic | Q2302426 |
| Other | Q55107540 |
| Water | Q283 |
| RoadLine | Q1392287 |
| Ground | Q36133 |
| Bridge | Q12280 |
| RailTrack | Q57498564 |
| GuardRail | Q1188866 |







