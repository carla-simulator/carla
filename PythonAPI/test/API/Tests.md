This is a recopilation of the tests done to the PythonAPI

### 1) Testing the spawning of several props

```sh
python3 spawn_actor.py -t -18 130 1 0 0 0 -b *prop*
python3 spawn_actor.py -t -18 130 1 0 0 0 -b fountain
python3 spawn_actor.py -t -18 130 1 0 0 0 -b vendingmachine
python3 spawn_actor.py -t -18 130 1 0 0 0 -b clothcontainer
python3 spawn_actor.py -t -18 130 1 0 0 0 -b clothcontainer --sync
```


### 2) Testing sensors

```sh
python3 prop_control.py
```

Sensors:
- Cameras:
    - Fail due to missmatch in the data, missing 12 pixels (regardless of resolution)
    - Aren't updated, always returning the first image
    - ColorConverter.Raw works fine, the others crash the client with different malloc() errors (invalid size, corrupted top size...). The image seems to be correctly created as it is sometimes shown for a frame before crashing.
- LIDAR, RADAR, GNSS and DVS: All good
- IMU:
    - Good compass
    - rest is untested as props have no physics
- Semantic LIDAR, collision, LaneInvasion and ObstacleDetection: untested for now
- Optical flow crashes the engine

```sh
LoginId:6686efa0c1944a169dc02763faebae04-000003e9
EpicAccountId:

Caught signal 11 Segmentation fault

libUnrealEditor-Carla.so!AOpticalFlowCamera::PostPhysTick(UWorld*, ELevelTick, float) [/home/glopez/carla-ue5/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Sensor/OpticalFlowCamera.cpp:33]
libUnrealEditor-Carla.so!ASensor::PostPhysTickInternal(UWorld*, ELevelTick, float) [/home/glopez/carla-ue5/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Sensor/Sensor.cpp:136]
libUnrealEditor-Carla.so!FSensorManager::PostPhysTick(UWorld*, ELevelTick, float) [/home/glopez/carla-ue5/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Sensor/SensorManager.cpp:24]
libUnrealEditor-Carla.so!FCarlaEngine::OnPostTick(UWorld*, ELevelTick, float) [/home/glopez/carla-ue5/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Game/CarlaEngine.cpp:375]
libUnrealEditor-Carla.so!TBaseRawMethodDelegateInstance<false, FCarlaEngine, void (UWorld*, ELevelTick, float), FDefaultDelegateUserPolicy>::ExecuteIfSafe(UWorld*, ELevelTick, float) const [/home/glopez/UnrealEngine_5.3/Engine/Source/Runtime/Core/Public/Delegates/DelegateInstancesImpl.h:532]
libUnrealEditor-Engine.so!void TMulticastDelegateBase<FDefaultDelegateUserPolicy>::Broadcast<IBaseDelegateInstance<void (UWorld*, ELevelTick, float), FDefaultDelegateUserPolicy>, UWorld*, ELevelTick, float>(UWorld*, ELevelTick, float) const [/home/glopez/UnrealEngine_5.3/Engine/Source/Runtime/Core/Public/Delegates/MulticastDelegateBase.h:254]
libUnrealEditor-Engine.so!UWorld::Tick(ELevelTick, float) [/home/glopez/UnrealEngine_5.3/Engine/Source/./Runtime/Engine/Private/LevelTick.cpp:1653]
libUnrealEditor-UnrealEd.so!UEditorEngine::Tick(float, bool) [/home/glopez/UnrealEngine_5.3/Engine/Source/./Editor/UnrealEd/Private/EditorEngine.cpp:1922]
libUnrealEditor-UnrealEd.so!UUnrealEdEngine::Tick(float, bool) [/home/glopez/UnrealEngine_5.3/Engine/Source/./Editor/UnrealEd/Private/UnrealEdEngine.cpp:528]
UnrealEditor!FEngineLoop::Tick() [/home/glopez/UnrealEngine_5.3/Engine/Source/./Runtime/Launch/Private/LaunchEngineLoop.cpp:5819]
UnrealEditor!GuardedMain(char16_t const*) [/home/glopez/UnrealEngine_5.3/Engine/Source/./Runtime/Launch/Private/Launch.cpp:190]
libUnrealEditor-UnixCommonStartup.so!CommonUnixMain(int, char**, int (*)(char16_t const*), void (*)()) [/home/glopez/UnrealEngine_5.3/Engine/Source/./Runtime/Unix/UnixCommonStartup/Private/UnixCommonStartup.cpp:269]
libc.so.6!__libc_start_main(+0xf2)
UnrealEditor!_start()
```