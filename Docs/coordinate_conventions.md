# Coordinate conventions

CARLA's world is **left-handed**. Almost every robotics toolchain — ROS 2 / tf2,
[REP-103][rep103], scipy, Open3D, NVIDIA's ClipGT rigs — is **right-handed**.
This page states CARLA's convention exactly, gives the one conversion you need,
and points at the API that performs it for you.

[rep103]: https://www.ros.org/reps/rep-0103.html

!!! Important
    `carla.Transform.get_matrix()`, `get_forward_vector()`, `get_right_vector()`
    and `get_up_vector()` are expressed in **CARLA's frame**. They are not
    "uncorrected" right-handed math and must not be sign-flipped to make them
    look right-handed. Convert at the boundary instead, with
    [`to_right_handed()`](#converting-to-a-right-handed-frame).

---

## CARLA's frame

|            | axis      | positive direction |
|------------|-----------|--------------------|
| `x`        | forward   | out of the nose    |
| `y`        | **right** | out of the right door |
| `z`        | up        | out of the roof    |

Left-handed, Z-up: point your **left** thumb along `+z` and your fingers curl
from `+x` to `+y`. Distances are in metres, angles in degrees.

A `carla.Rotation` is written `Rotation(pitch, yaw, roll)` (that argument order
is CARLA's; the Unreal Editor shows the same three angles as *roll, pitch, yaw*).
It composes as

```
R = Rz(+yaw) · Ry(−pitch) · Rx(−roll)
```

with the **standard** (right-handed) elementary matrices `Rx`, `Ry`, `Rz`. The
minus signs on pitch and roll are what "left-handed" cashes out to here.

### The three sign rules

Everything else follows from these, and all three are verified against rendered
frames from the engine:

| angle    | what `+` does                    | consequence                                |
|----------|----------------------------------|--------------------------------------------|
| `+yaw`   | turns the nose to the **right**  | `forward = (cos yaw · cos pitch, sin yaw · cos pitch, …)` |
| `+pitch` | tilts the nose **up**            | `forward.z = +sin(pitch)`                  |
| `+roll`  | drops the **right** side         | `right.z = −cos(pitch) · sin(roll)`        |

A camera spawned at `Rotation(pitch=20)` fills its frame with sky. A camera at
`Rotation(roll=25)` sees more ground on the right-hand side of the frame.

### The matrix

`carla.Transform.get_matrix()` returns that rotation in the same frame, row
major, with the translation in the last column:

```
    ⎡ cp·cy   cy·sp·sr − sy·cr   −cy·sp·cr − sy·sr   x ⎤
M = ⎢ cp·sy   sy·sp·sr + cy·cr   −sy·sp·cr + cy·sr   y ⎥
    ⎢ sp      −cp·sr             cp·cr               z ⎥
    ⎣ 0       0                  0                   1 ⎦
```

(`cy = cos(yaw)`, `sp = sin(pitch)`, `sr = sin(roll)`, …). Its columns are
`get_forward_vector()`, `get_right_vector()` and `get_up_vector()`.

Read as right-handed math this matrix has `det(R) = −1`, which is the tell-tale
sign of the handedness difference — and the reason you cannot fix the mismatch
by permuting or negating a couple of Euler angles by hand.

---

## The right-handed (FLU) frame

ROS REP-103 calls the body frame **FLU**: x **F**orward, y **L**eft, z **U**p,
right-handed, every rotation by the right-hand rule, `roll`-`pitch`-`yaw`
composed as intrinsic Z-Y-X. That is what
`scipy.spatial.transform.Rotation.from_euler("xyz", …)`,
`tf2::Quaternion::setRPY` and NVIDIA's `rig.json` all mean.

The two frames differ by a **mirror of the Y axis**, `S = diag(1, −1, 1)`:

| quantity  | CARLA                 | right-handed / FLU               |
|-----------|-----------------------|----------------------------------|
| location  | `(x, y, z)`           | `(x, −y, z)`                     |
| rotation  | `(roll, pitch, yaw)`  | `(roll, −pitch, −yaw)`           |
| 3×3 / 4×4 | `M`                   | `S · M · S`                      |
| quaternion| —                     | from the FLU Euler triple above  |

**Pitch and yaw flip; roll does not.** Worked example: CARLA
`(roll, pitch, yaw) = (10, 20, 30)` is FLU `(10, −20, −30)`.

!!! Note
    The mapping is its own inverse: applying it twice gets you back where you
    started, so the same three sign flips convert in both directions.

Why those three and not some other set: mirroring Y turns
`Rz(+yaw) · Ry(−pitch) · Rx(−roll)` into
`Rz(−yaw) · Ry(+(−pitch)) · Rx(+roll)`. Roll is about the forward axis, which
the mirror leaves alone; pitch and yaw are about axes the mirror moves.

---

## Converting to a right-handed frame

Do not hand-roll the sign flips. Use the boundary adapters:

```py
transform = camera.get_transform()

flu = transform.to_right_handed()      # carla.RightHandedTransform

flu.location                           # carla.RightHandedVector3D (x, y, z)
flu.rotation                           # carla.RightHandedRotation (roll, pitch, yaw), degrees
flu.get_quaternion()                   # carla.Quaternion (x, y, z, w), right-handed
flu.get_matrix()                       # 4x4 row-major, == S @ get_matrix() @ S

back = carla.Transform.from_right_handed(flu)   # exact round trip
```

`carla.Location` and `carla.Rotation` carry the same pair:

```py
carla.Location(10, 20, 30).to_right_handed()          # (10, -20, 30)
carla.Rotation(pitch=20, yaw=30, roll=10).to_right_handed()
                                                      # roll=10, pitch=-20, yaw=-30

carla.Location.from_right_handed(rh_location)
carla.Rotation.from_right_handed(rh_rotation)
```

`carla.Quaternion` has always lived in the right-handed frame — it is the
quaternion the ROS 2 bridge publishes — so
`carla.Quaternion(rotation)` and `carla.Quaternion(rotation.to_right_handed())`
are the same value.

### Publishing a pose to ROS 2

```py
import carla

def to_ros_pose(transform: carla.Transform):
    """carla.Transform -> (translation, quaternion) in REP-103 FLU."""
    flu = transform.to_right_handed()
    q = flu.get_quaternion()
    return (
        (flu.location.x, flu.location.y, flu.location.z),
        (q.x, q.y, q.z, q.w),          # geometry_msgs/Quaternion order
    )
```

### Feeding scipy / an FLU rig file

```py
import numpy as np
from scipy.spatial.transform import Rotation

flu = transform.to_right_handed()
m = np.array(flu.get_matrix())                                # 4x4, FLU
rpy = Rotation.from_matrix(m[:3, :3]).as_euler("xyz", degrees=True)
# rpy == (flu.rotation.roll, flu.rotation.pitch, flu.rotation.yaw)
```

### C++

The same adapters exist in `LibCarla`:

```cpp
#include <carla/geom/Transform.h>

const carla::geom::RightHandedTransform flu = transform.ToRightHanded();
const carla::geom::Quaternion q = flu.GetQuaternion();
const std::array<float, 16> m = flu.GetMatrix();

const carla::geom::Transform back =
    carla::geom::Transform::FromRightHanded(flu);
```

`carla::geom::Location::ToRightHanded()` returns the existing
`carla::geom::RightHandedVector3D`; `carla::geom::Rotation::ToRightHanded()`
returns `carla::geom::RightHandedRotation`.

---

## Pitfalls

* **Do not "correct" `get_matrix()`.** It is left-handed on purpose. A patch
  that flips its pitch/roll signs makes LibCarla describe the mirror image of
  the pose the engine actually builds from the same `carla.Rotation`, and the
  break is invisible to anything that only ever uses yaw — traffic manager,
  basic vehicle control — while silently corrupting every camera, lidar and
  radar pose. This happened once already and had to be reverted.
* **Do not map Euler angles between the frames by ad-hoc reasoning.** The three
  sign flips above are the whole answer; anything else (swapping axes,
  negating roll, transposing) will pass a yaw-only smoke test and fail on a
  pitched or rolled sensor.
* **Check `det(R)`.** A pose that reached a right-handed consumer without the
  mirror has `det(R) = −1`. It is the cheapest possible assertion.
* **Units.** CARLA's Python and C++ APIs are in metres and degrees. The Unreal
  Editor and the engine's own `FVector` are in centimetres.

---

## Where this is enforced

| what | where |
|---|---|
| C++ adapters | `LibCarla/source/carla/geom/{Location,Rotation,Transform}.h`, `RightHandedRotation.h`, `RightHandedTransform.h`, `RightHandedVector3D.h` |
| C++ tests | `LibCarla/source/test/common/test_geom.cpp` (`right_handed_*`) |
| ROS 2 bridge | `LibCarla/source/carla/ros2/publishers/TransformQuaternion.h` — an independent implementation of the same mapping, cross-checked by the tests above |
| Python bindings | `PythonAPI/carla/src/Geometry.cpp` |
| Python tests | `PythonAPI/test/unit/test_transform.py` (`TestRightHandedBoundary`) |
