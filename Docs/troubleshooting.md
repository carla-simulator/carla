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
