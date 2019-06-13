<h1>Importing and exporting maps for distribution builds</h1>

!!! important
    The given scripts only works if the files are kept in the folders
    detailed below and on Linux.

This method and its scripts are aimed to reducing the size of the distribution
build, allowing to import maps on distributed builds with a script, exporting
them with yet another script from the editor.

<h4>How to export a map</h4>
The current script only work for content placed into the
"CarlaRoot/Unreal/CarlaUE4/Content/Carla/ExportedMaps" folder (this can be overriden with
--map flag), inside the unreal's project structure. Once all the changes are saved in the
editor and the maps to export are located in that folder, by the name of the map or the
name provided with the --file flag, so, running

```sh
make export-maps ARGS="--map=/Path/To/Awesome/Map --file=AwesomeMap"
```

!!! important
    The path specified in <b>--map</b> flag has to be in Unreal's way.
    For example:<br><i><b>/Game/Carla/Maps/AwesomeTownMap</b></i>

will clean, cook and pack the necessary assets with the maps. Please, note that
there is no way of knowing what assets are on the project-to-import-to, so the
final package will usually contain assets needed for the current project to work
even though they might not be directly used in the exported map. Also, the maps
referenced inside the Editor Properties will be exported due to the current way
of cooking provided by Unreal.

!!! important
    This will only work if "Use pak file" is unchecked under the Packaging settings

Once the script is completed, the packed file can be found under ExportedMaps in
the project's root.

<h4>How to import a map</h4>
In order to do so, the file generated in the import phase has to be placed in a folder
at the root of the distributed folder called "ExportedAssets". Once the zip is placed there,
by running

```sh
./ImportAssets.sh
```

the content will get extracted and located properly where the executable can reach it. All
the imported maps can be loaded by passing the convenient argument to the CarlaUE4.sh script.