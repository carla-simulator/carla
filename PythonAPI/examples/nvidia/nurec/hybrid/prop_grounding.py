"""Seat qualified construction props on the collision road, including its slope.

Scenic's 2D snap sets only the origin height to a lane-centre waypoint. It does
not tilt a prop onto the road or account for mesh vertices below its pivot.
Bounds below were measured from unrotated, live assets after a world tick;
CARLA's rotated static-prop bounds cannot be used as local mesh bounds.
"""
import math

import carla
import numpy as np

DRIVABLE_LANES = (carla.LaneType.Driving | carla.LaneType.OnRamp |
                   carla.LaneType.OffRamp | carla.LaneType.Entry | carla.LaneType.Exit)


def snap_ramp_vehicle(world, actor):
    """Correct Scenic's Driving-only spawn height when the actor is on a ramp."""
    if not isinstance(actor, carla.Vehicle):
        return
    tf = actor.get_transform()
    wp = world.get_map().get_waypoint(tf.location, lane_type=DRIVABLE_LANES)
    if wp is not None and wp.lane_type != carla.LaneType.Driving:
        tf.location.z = wp.transform.location.z + 0.5
        actor.set_transform(tf)

BASE_Z_M = {
    'static.prop.constructioncone': -0.004472524,
    'static.prop.streetbarrier': 0.000212133,
    'static.prop.warningconstruction': -0.000724494,
}


def road_plane(world, location, radius=0.7):
    """Return z at the prop origin and local dz/dx, dz/dy from actual road hits."""
    offsets = [(0., 0.), (radius, 0.), (-radius, 0.), (0., radius), (0., -radius)]
    heights = []
    for dx, dy in offsets:
        x, y = location.x + dx, location.y + dy
        hits = world.cast_ray(carla.Location(x, y, location.z + 2),
                              carla.Location(x, y, location.z - 2))
        roads = [hit for hit in hits if hit.label == carla.CityObjectLabel.Roads]
        if not roads:
            raise ValueError(f'No collision road under construction prop at ({x}, {y})')
        heights.append(min(roads, key=lambda hit: abs(hit.location.z-location.z)).location.z)
    A = np.array([[1., dx, dy] for dx, dy in offsets])
    plane = np.linalg.lstsq(A, heights, rcond=None)[0]
    if np.max(np.abs(A @ plane - heights)) > 0.02:
        raise ValueError('Construction prop footprint crosses a road discontinuity')
    return tuple(map(float, plane))


def grounded_rotation(yaw_degrees, dx, dy):
    yaw = math.radians(yaw_degrees)
    forward_slope = dx * math.cos(yaw) + dy * math.sin(yaw)
    right_slope = -dx * math.sin(yaw) + dy * math.cos(yaw)
    pitch = math.atan(forward_slope)
    roll = -math.atan(right_slope * math.cos(pitch))
    return carla.Rotation(pitch=math.degrees(pitch), yaw=yaw_degrees,
                          roll=math.degrees(roll))


def ground_prop(world, actor):
    """Correct a qualified prop in place; return reviewable placement measurements."""
    if actor.type_id not in BASE_Z_M:
        return None
    tf = actor.get_transform()
    before = [tf.location.z, tf.rotation.pitch, tf.rotation.roll]
    z, dx, dy = road_plane(world, tf.location)
    tf.rotation = grounded_rotation(tf.rotation.yaw, dx, dy)
    # Base plane offset is measured along the mesh's local up axis. Solving
    # against the road plane requires dividing by its normal's vertical term.
    tf.location.z = z - BASE_Z_M[actor.type_id] * math.sqrt(1 + dx*dx + dy*dy) + 0.001
    actor.set_transform(tf)
    return dict(blueprint=actor.type_id, actor_id=actor.id,
                before_z_pitch_roll=before,
                after_z_pitch_roll=[tf.location.z, tf.rotation.pitch, tf.rotation.roll],
                road_z=z, road_gradient=[dx,dy], base_z_m=BASE_Z_M[actor.type_id],
                clearance_m=0.001)
