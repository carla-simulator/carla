<h1>How to add custom props to the Blueprint Library</h1>

Users can register their own props with `.fbx` format and make these props available through the existing Carla Blueprint Library API. Also, these props can be spawned, maintaining compatibility with semantic segmentation tags.

To do so, the following steps should be done:

1. Create a folder called `FBXImporter` inside Carla root folder.
1. Place all your props in `.fbx` format and its textures inside `FBXImporter` folder.
1. Inside the newly created `FBXImporter` folder, create a JSON File with the name of your package and with extension `.PropRegistry.json`. For example, `YourPackageName.PropRegistry.json`. It is also possible to define multiple prop registry files inside that folder if it is needed to organize props in separate packages.
1. For each prop registry file, store a JSON Array called `definitions`, which will contain JSON Objects with relevant information for importing a prop such as:
    - name: The name of your prop, this name is the one that will be registered in the Carla Blueprint Library.
    - source: The path to your `.fbx` file.
    - size: The size of your prop in terms of dimensions. The possible values are: `tiny`, `small`, `medium`, `big` and `huge`.
    - tag: The semantic segmentation tag.

    An example of a `.PropRegistry.json` file would be:

        {
          "definitions":
          [
            {
              "name": "Prop01",
              "size": "medium",
              "source": "SM_Prop_1.fbx",
              "tag": "Pole"
            },
            {
              "name": "Prop02",
              "size": "medium",
              "source": "SM_Prop_2.fbx",
              "tag": "Roads"
            }
          ]
        }

1. Finally, run the Python script `import_fbx.py` located inside `Carla/Util` folder.
This script reads all `.PropRegistry.json` files inside the `FBXImporter` folder and, for each `.fbx` file defined in the registry, it will be cooked together with its textures by Unreal Engine and moved inside the Carla Content in the following path: `Content/YourPackageName/Static/PropTag/PropName/PropSource`.
Furthermore, each defined prop will be also included in the Carla Blueprint Library, so they can be queried and spawned using the `PythonAPI`.