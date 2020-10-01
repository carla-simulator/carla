# Create semantic tags

Learn how to define customized tags for semantic segmentation. These can additionally be used to filter the bounding boxes that [carla.World](python_api.md#carla.World) retrieves.  

*   [__Create a new semantic tag__](#add-a-4-wheeled-vehicle)  
*   [__Add a tag to carla.CityObjectLabel__](#add-a-2-wheeled-vehicle)  

---

## Create a new semantic tag

### 1. Create the tag ID

__Open `ObjectLabel.h`__ in `LibCarla/source/carla/rpc`. Add your new tag by the end of the enum using the same formatting as the rest.  

![object_label_h](img/tuto_D_create_semantic_tags/01_objectlabel_tag.jpg)

!!! Note
    Tags do not have to appear in order. However, it is good practice to list them in order. 

### 2. Create the UE folder for assets

__Open the Unreal Engine Editor__ and go to `Carla/Static`. Create a new folder named as your tag. 

![ue_folder](img/tuto_D_create_semantic_tags/02_ue_folder.jpg)

!!! Note
    The UE folder and the tag do not necessarily have to be named the same. However, it is good practice to do so.  

### 3. Create two-way correspondence between UE and the code tag

__3.1. Open `Tagger.cpp`__ in `Unreal/CarlaUE4/Plugins/Carla/Source/Carla/Game`. Go to __`GetLabelByFolderName`__ Add the your tag by the end of the list.  

![tagger_cpp](img/tuto_D_create_semantic_tags/03_tagger_cpp.jpg)

__3.2. Go to `GetTagAsString`__ in the same `Tagger.cpp`. Add the new tag by the end of the switch.  

![tagger_cpp_02](img/tuto_D_create_semantic_tags/04_tagger_cpp_02.jpg)

### 4. Add the color code

__Open `CityScapesPalette.h`__ in `LibCarla/source/carla/image`. Add the color code of your new tag by the end of the array. 

![city_scapes_palette_h](img/tuto_D_create_semantic_tags/05_city_scapes_palette_h.jpg)

!!! Warning
    The position in the array must correspond with the tag ID, in this case, `23u`. 

### 5. Add the tagged elements

The new semantic tag is ready to be used. Only the elements stored inside the UE folder of a tag are tagged as such. Move or import the corresponding elements to the new folder, in order for the to be tagged properly. 

---

## Add a tag to carla.CityObjectLabel

This step is not directly related with semantic segmentation. However, these tags can be used to filter the bounding box query in [carla.World](python_api.md#carla.World). In order to do this, the tag must be added to the [carla.CityObjectLabel]() enum in the PythonAPI. 