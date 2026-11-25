# Walker skeleton reference

## Bones

Every CARLA pedestrian shares one 66-bone skeleton. Names are `crl_` + bone + side (`__C` centre, `__L` left, `__R` right). The order below is the order `get_bone_names()` returned on ue58-dev ccc52d24d.

| Group | Bones |
|---|---|
| Body | `crl_root`, `crl_hips__C`, `crl_spine__C`, `crl_spine01__C`, `crl_neck__C`, `crl_Head__C`, `crl_eye__R`, `crl_eye__L` |
| Arm, each side | `crl_shoulder`, `crl_arm`, `crl_foreArm`, `crl_hand` |
| Fingers, each side | `crl_hand{Thumb,Index,Middle,Ring,Pinky}`, then `01`, `02` and `End` of each: 20 bones |
| Leg, each side | `crl_thigh`, `crl_leg`, `crl_foot`, `crl_toe`, `crl_toeEnd` |

The parent chain is root → hips → spine → spine01 → neck → Head, with the shoulders on spine01 and the thighs on hips. `pose.py` draws the 22 links in `LINKS`, which is the chain of `PythonAPI/examples/draw_skeleton.py`.

## Which call returns what

| Call | Frame |
|---|---|
| `actor.get_bone_names()` | names, in the order of the two calls below |
| `actor.get_bone_world_transforms()` | world |
| `actor.get_bone_relative_transforms()` | relative to the actor |
| `walker.get_bones().bone_transforms[i]` | `.name`, `.world`, `.component`, `.relative`: `.relative` is relative to the parent bone |
| `walker.set_bones(carla.WalkerBoneControlIn([(name, transform), ...]))` | transforms relative to the parent bone |

The first three are on `carla.Actor` and work on vehicles too (9 bones on the mkz: the base and the wheels).

## Projection

The pinhole model from the camera's `image_size_x`, `image_size_y` and `fov`: focal `f = w / (2 tan(fov/2))`, principal point at the image centre. A world point goes to the camera frame through `image.transform.get_inverse_matrix()`, from Unreal axes (x forward, y right, z up) to camera axes (x right, y down, z forward), then through the intrinsics. Points with z ≤ 0 in the camera frame are behind it and get `uv: null`. The camera has no distortion here: a fisheye or `rt_lens` camera needs its own model.

## Measured

- Two walkers, 800×600 camera at 3 m, synchronous at 0.05 s: 132 keypoints per frame, the skeleton on both bodies in the PNG, image frame equal to the bones frame.
- Rotating `crl_arm__R` and `crl_arm__L` by 70° of roll moved 42 to 46 bones by more than 1 cm.
- A synchronous client sees no actors before its first frame, so the script steps once on connect.
