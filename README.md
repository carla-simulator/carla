CARLA UE4 Plugin
================

Plugin for Unreal Engine 4.

See [CHANGELOG](CHANGELOG.md).

Settings
--------

See [Resources/Example.CarlaSettings.ini](Resources/Example.CarlaSettings.ini).

Compiling a UE4 project with CARLA plugin
-----------------------------------------

Clone or copy the files of this repository under the folder
`<project-root>/Plugins/Carla`.

Build CarlaServer located at `<project-root>/Plugins/Carla/Source/CarlaServer`,
follow the instructions [here](Source/CarlaServer/README.md) to build all the
necessary dependencies too.

Now compile the Unreal project.

#### Linux

Opening the .uproject file with UE4Editor usually triggers a recompile. If not
you can follow the next steps to do it manually.

Generate project files

    $ <unreal-installation-path>/GenerateProjectFiles.sh -project="<full-path-to-the-project-root-folder>/<project-name>.uproject" -game -engine

Build project in the in-editor configuration

    $ cd <project-root-folder>
    $ make <project-name>Editor

Now you can launch the editor.

#### Windows

Usually double-clicking the .uproject file triggers a recompile.

To build from Visual Studio, right-click the project file and select "Generate
Visual Studio Files". Open the generated solution (.sln) and compile. You can
change the build configuration to development/debug both in-editor and
standalone game.
