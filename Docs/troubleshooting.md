Troubleshooting
===============

#### Editor hangs after hitting Play

This is most probably happening because CARLA is started in server mode. Check
in your CarlaSettings.ini file ("./Unreal/CarlaUE4/Config/CarlaSettings.ini")
and set

```ini
[CARLA/Server]
UseNetworking=false
```

#### Very low FPS in editor when not in focus

UE4 Editor goes to a low performance mode when out of focus. It can be disabled
in the editor preferences. Go to "Edit->Editor Preferences->Performance" and
disable the "Use Less CPU When in Background" option.

#### Fatal error: file '/usr/include/linux/version.h' has been modified since the precompiled header

This happens from time to time due to Linux updates. It is possible to force a
rebuild of all the project files with

    $ cd Unreal/CarlaUE4/
    $ make CarlaUE4Editor ARGS=-clean
    $ make CarlaUE4Editor
