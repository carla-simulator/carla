Troubleshooting
===============

#### Editor hangs after hitting Play

By default, when CARLA is started it waits for a client to be connected and
control the vehicle. This is the intended behavior. This can be changed in
"./Unreal/CarlaUE4/Config/CarlaSettings.ini" changing `UseNetworking=false`.

#### Very low FPS in editor when not in focus

UE4 Editor goes to a low performance mode when out of focus. It can be disabled
in the editor preferences. Go to "Edit->Editor Preferences->Performance" and
disable the "Use Less CPU When in Background" option.
