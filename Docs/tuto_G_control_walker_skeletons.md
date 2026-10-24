# Walker Bone Control

In this tutorial we describe how to manually control and animate the
skeletons of walkers from the CARLA Python API. The reference of
all classes and methods available can be found at
[Python API reference](python_api.md).

*   [__Walker skeleton structure__](#walker-skeleton-structure)  
*   [__Manually control walker bones__](#manually-control-walker-bones)  
	*   [Connect to the simulator](#connect-to-the-simulator)  
	*   [Spawn a walker](#spawn-a-walker)  
	*   [Control walker skeletons](#control-walker-skeletons)  

!!! note
    **This document assumes the user is familiar with the Python API**. <br>
    The user should read the first steps tutorial before reading this document.
    [Core concepts](core_concepts.md).

---
## Walker skeleton structure

All walkers have the same skeleton hierarchy and bone names. Below is an image of the skeleton
hierarchy.

```
crl_root
└── crl_hips__C
    ├── crl_spine__C
    │   └── crl_spine01__C
    │       ├── ctrl_shoulder__L
    │       │   └── crl_arm__L
    │       │       └── crl_foreArm__L
    │       │           └── crl_hand__L
    │       │               ├── crl_handThumb__L
    │       │               │   └── crl_handThumb01__L
    │       │               │       └── crl_handThumb02__L
    │       │               │           └── crl_handThumbEnd__L
    │       │               ├── crl_handIndex__L
    │       │               │   └── crl_handIndex01__L
    │       │               │       └── crl_handIndex02__L
    │       │               │           └── crl_handIndexEnd__L
    │       │               ├── crl_handMiddle_L
    │       │               │   └── crl_handMiddle01__L
    │       │               │       └── crl_handMiddle02__L
    │       │               │           └── crl_handMiddleEnd__L
    │       │               ├── crl_handRing_L
    │       │               │   └── crl_handRing01__L
    │       │               │       └── crl_handRing02__L
    │       │               │           └── crl_handRingEnd__L
    │       │               └── crl_handPinky_L
    │       │                   └── crl_handPinky01__L
    │       │                       └── crl_handPinky02__L
    │       │                           └── crl_handPinkyEnd__L
    │       ├── crl_neck__C
    │       │   └── crl_Head__C
    │       │       ├── crl_eye__L
    │       │       └── crl_eye__R
    │       └── crl_shoulder__R
    │           └── crl_arm__R
    │               └── crl_foreArm__R
    │                   └── crl_hand__R
    │                       ├── crl_handThumb__R
    │                       │   └── crl_handThumb01__R
    │                       │       └── crl_handThumb02__R
    │                       │           └── crl_handThumbEnd__R
    │                       ├── crl_handIndex__R
    │                       │   └── crl_handIndex01__R
    │                       │       └── crl_handIndex02__R
    │                       │           └── crl_handIndexEnd__R
    │                       ├── crl_handMiddle_R
    │                       │   └── crl_handMiddle01__R
    │                       │       └── crl_handMiddle02__R
    │                       │           └── crl_handMiddleEnd__R
    │                       ├── crl_handRing_R
    │                       │   └── crl_handRing01__R
    │                       │       └── crl_handRing02__R
    │                       │           └── crl_handRingEnd__R
    │                       └── crl_handPinky_R
    │                           └── crl_handPinky01__R
    │                               └── crl_handPinky02__R
    │                                   └── crl_handPinkyEnd__R
    ├── crl_thigh__L
    │   └── crl_leg__L
    │       └── crl_foot__L
    │           └── crl_toe__L
    │               └── crl_toeEnd__L
    └── crl_thigh__R
        └── crl_leg__R
            └── crl_foot__R
                └── crl_toe__R
                    └── crl_toeEnd__R
```

---
## Manually control walker bones

Following is a detailed step-by-step example of how to change the bone transforms of a walker
from the CARLA Python API

### Connect to the simulator

Import neccessary libraries used in this example

```py
import carla
import random
```

Initialize the carla client

```py
client = carla.Client('127.0.0.1', 2000)
client.set_timeout(2.0)
```

### Spawn a walker

Spawn a random walker at one of the map's spawn points

```py
world = client.get_world()
blueprint = random.choice(world.get_blueprint_library().filter('walker.pedestrian.*'))
spawn_points = world.get_map().get_spawn_points()
spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
walker = world.try_spawn_actor(blueprint, spawn_point)
if walker is None:
    raise RuntimeError('could not spawn the walker, try another location')
```

### Control walker skeletons

A walker's skeleton can be modified by passing an instance of the WalkerBoneControlIn class
to the walker's set_bones function. The WalkerBoneControlIn class contains the transforms
of the bones to be modified. Its bone_transforms member is a list of tuples of value pairs
where the first value is the bone name and the second value is the bone transform. The
set_bones function can be called on every tick to animate a walker's skeleton. The
location and rotation of each transform is relative to its parent. Therefore when a
parent bone's transform is modified, the transforms of the child bones in model space
will also be changed relatively.

The example below rotates both hands by 90 degrees around their forward axes while
preserving each bone's current location and other rotation components. The current
transforms are read with get_bones, and show_pose displays the custom pose.

```py
bones = walker.get_bones()
current = {bone.name: bone.relative for bone in bones.bone_transforms}

transforms = []
for name in ('crl_hand__R', 'crl_hand__L'):
    relative = current[name]
    transforms.append((name, carla.Transform(
        location=relative.location,
        rotation=carla.Rotation(pitch=relative.rotation.pitch,
                                yaw=relative.rotation.yaw,
                                roll=relative.rotation.roll + 90))))

control = carla.WalkerBoneControlIn()
control.bone_transforms = transforms
walker.set_bones(control)
walker.show_pose()
```
