<h1>Automatically generate a map from RoadRunner</h1>

!!! important
    The given scripts only works if the files are kept in the folders
    detailed below.

This script is aimed to reduce the time needed to setup a map inside Carla's project.
Also, in combination of the [import / export scripts](export_import_dist.md) will allow
the user to generate a simple map without opening Unreal Engine.

!!! important
    The script does heavy use of Unreal's capabilities, so it is needed to be installed
    and setup, as the import fbx action is done directly over the code project, not a
    distribution build.


<h4>How to import a fbx</h4>
The current script only work for files placed with the follow structure (RoadRunnerFiles is
located in the root folder):

```sh
RoadRunnerFiles/
├── README.txt
├── MapToImport01
│   ├── Asphalt1_Diff.png
│   ├── Asphalt1_Norm.png
│   ├── Asphalt1_Spec.png
│   ├── Grass1_Diff.png
│   ├── Grass1_Norm.png
│   ├── Grass1_Spec.png
│   ├── LaneMarking1_Diff.png
│   ├── LaneMarking1_Norm.png
│   ├── LaneMarking1_Spec.png
│   ├── MapToImport01.fbx
│   └── MapToImport01.xodr
└── MapToImport02
    ├── Asphalt1_Diff.png
    ├── Asphalt1_Norm.png
    ├── Asphalt1_Spec.png
    ├── Concrete1_Diff.png
    ├── Concrete1_Norm.png
    ├── Concrete1_Spec.png
    ├── Grass1_Diff.png
    ├── Grass1_Norm.png
    ├── Grass1_Spec.png
    ├── LaneMarking1_Diff.png
    ├── LaneMarking1_Norm.png
    ├── LaneMarking1_Spec.png
    ├── MapToImport02.fbx
    └── MapToImport02.xodr
```

Under RoadRunnerFiles, place each "Export" folder obtained from RoadRunner and
rename it with the name of the map to be imported. It have to match the <b>.fbx</b>
and <b>.xodr</b> files.

Now, simply go into the PythonAPI folder and run <b>generate_map.py</b>

!!! important
    The script, by default, checks the <i>/Unreal/CarlaUE4/Content/Carla/Maps</i> for matching names.
    If a map with the same name as a to-be-imported map is found, it will notify the user and won't do anything.
    To override this, <b>generate_map.py</b> can be invoked with the <b>--force</b> flag.

After the script finishes, all a map with the same name of the folder and files will be located under
<i>/Unreal/CarlaUE4/Content/Carla/Maps</i> with all the static meshes and placed, textures, waypoints and routes
generated.

!!! important
    There is a bug in RoadRunner that generates wrong materials automatically and they get broken inside Unreal.
    By adding the flag <b>--usecarlamats</b> the meshes will use the materials provided by Carla's project,
    that will also <b>react to the weather</b>.