# Bounding boxes — detail

Detail layer for the `bounding-boxes` skill. Follows CARLA's
`client_bounding_boxes.py` / `lidar_to_camera.py`.

## Box sources

- **Actor box**: `actor.bounding_box` — a `carla.BoundingBox` in the actor's local
  frame (`location` offset, `extent` = half-size, `rotation`).
  `bounding_box.get_world_vertices(actor.get_transform())` gives the 8 corners in
  world space; `get_local_vertices()` gives them local.
- **Level box**: `world.get_level_bbs(CityObjectLabel)` — static geometry boxes
  (buildings, signs, ...), already in world space (see world-data).

Dimensions: `extent` is half-width, so length×width×height = `2*extent.x` ×
`2*extent.y` × `2*extent.z`.

## Drawing 3D boxes in the world

`world.debug.draw_box(world_bbox, rotation, thickness, color, life_time)`. The box
must be in world space: place the actor box at the actor
(`transform.transform(bb.location)`) and use the actor's rotation. Overlay only
shows on a rendered server (see debug-draw).

## Projecting to a camera (2D boxes)

The pinhole projection used by `project`:

1. **Intrinsics** from the camera's `fov` and resolution:
   `f = w / (2*tan(fov*pi/360))`, `K = [[f,0,w/2],[0,f,h/2],[0,0,1]]`.
2. **Extrinsics**: `world_2_cam = np.array(camera.get_transform().get_inverse_matrix())`.
3. For each 3D vertex `v` (world): `p = world_2_cam @ [v.x,v.y,v.z,1]`.
4. **Axis change** UE4 (x fwd, y right, z up) → camera (x right, y down, z fwd):
   `cam_pt = [p.y, -p.z, p.x]`. Drop vertices with `cam_pt.z <= 0` (behind camera).
5. **Project**: `img = K @ cam_pt`, pixel `= img[:2] / img[2]`.
6. The 2D box is the min/max of the projected pixels; discard fully off-frame or
   too-distant (`--max-dist`) actors.

Capture the frame and read the actor poses close together (ideally one sync tick)
so projection and geometry match. The JSON sidecar lists `{id, type, bbox:[x0,y0,
x1,y1]}` per actor — ready for detection-dataset tooling.

## lidar-to-camera

The same intrinsics/extrinsics project lidar points into the image (colour points
by depth), and conversely camera pixels can be lifted with depth. This skill ships
the actor-box projection; the point-projection variant is a short extension using
the same `K` and `world_2_cam`.
