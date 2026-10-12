# Sensors — creation detail

Detail layer for the `create-sensor` skill.

## Sensor families (0.9.16)

- **Cameras** `sensor.camera.*`: `rgb`, `depth`, `semantic_segmentation`,
  `instance_segmentation`, `optical_flow`, `normals`, `dvs` (event camera), plus
  `.wide_angle_lens` variants. Attributes: `image_size_x`, `image_size_y`, `fov`,
  `sensor_tick`, and many post-process controls.
- **Lidar** `sensor.lidar.ray_cast` (+ `ray_cast_semantic`): `range`,
  `points_per_second`, `channels`, `rotation_frequency`, `upper_fov`, `lower_fov`,
  `sensor_tick`.
- **Radar** `sensor.other.radar`: `horizontal_fov`, `vertical_fov`, `range`,
  `points_per_second`.
- **IMU** `sensor.other.imu`, **GNSS** `sensor.other.gnss`: `sensor_tick` (+ noise
  attrs).
- **Event sensors** `sensor.other.collision`, `.lane_invasion`, `.obstacle`: fire
  on an event rather than every tick (`obstacle` has `distance`, `hit_radius`).

`sensor_tick` is the minimum seconds between readings (0 = every frame). Raise it
to throttle a sensor's rate independently of the sim step.

## Attaching

`world.spawn_actor(bp, transform, attach_to=parent, attachment_type=...)`:

- **transform is relative to the parent** when attached — a windshield camera is
  roughly `x=1.5, z=1.3`; a roof lidar `z=2.4`.
- **AttachmentType**: `Rigid` (fixed to the parent), `SpringArm` /
  `SpringArmGhost` (spring-damped — smooth chase-cam motion, less jitter over
  bumps). Use SpringArm for third-person-style cameras, Rigid for measurement
  sensors you want perfectly fixed.
- Omit the parent for a **world-fixed** sensor at an absolute transform (e.g. a
  fixed surveillance camera).

## Lifecycle

The sensor is an actor and persists until destroyed. It produces data only while a
listener is attached (the read-sensor skill), but its transform tracks the parent
regardless. `destroy` stops any listener and removes the sensor; a map reload also
clears it. Attached sensors are destroyed with their parent if the parent goes.

## Choosing attributes

Cameras: bigger `image_size_*` and `fov` cost more; typical 800×600, fov 90.
Lidar: `points_per_second` × `1/rotation_frequency` = points per sweep; more
points and longer `range` cost more. Match `rotation_frequency` to your sim FPS in
sync mode so one sweep completes per frame.
