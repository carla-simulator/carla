"""PhysicalAI-AV camera ring used by Alpamayo 2 Super."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class CameraSpec:
    index: int
    name: str
    display_name: str
    x: float
    y_left: float
    z: float
    yaw_left_deg: float
    pitch_up_deg: float
    fov_deg: float
    roll_carla_deg: float = 0.0
    focal_length_px: float | None = None
    kannala_brandt: tuple[float, float, float, float] | None = None
    principal_point_px: tuple[float, float] | None = None

    def carla_pose(self, rear_axle_x: float) -> tuple[float, float, float, float, float, float]:
        """Return x/y/z and pitch/yaw/roll in CARLA's left-handed local frame."""
        return (
            rear_axle_x + self.x,
            -self.y_left,
            self.z,
            -self.pitch_up_deg,
            -self.yaw_left_deg,
            self.roll_carla_deg,
        )


# Pose and lens values come from the public Alpamayo 2 validation calibration.
# The fitted Kannala-Brandt coefficients reproduce each source F-theta curve.
CAMERA_RING: tuple[CameraSpec, ...] = (
    CameraSpec(
        0, "camera_cross_left_120fov", "Front left",
        2.472564, 0.937820, 0.917814, 66.083303, -0.909268, 120.0,
        0.289044, 921.978679,
        (-0.020325173, 0.015561344, -0.019556958, 0.008121544),
        (965.79980657, 546.77842273),
    ),
    CameraSpec(
        1, "camera_front_wide_120fov", "Front wide",
        1.696904, -0.010188, 1.435701, -0.489230, -0.443195, 120.0,
        -0.155228, 925.540742,
        (-0.041555584, 0.067146887, -0.078102726, 0.031954250),
        (957.85292258, 537.02554301),
    ),
    CameraSpec(
        2, "camera_cross_right_120fov", "Front right",
        2.478202, -0.954542, 0.928658, -66.837341, 1.410818, 120.0,
        0.175652, 917.283211,
        (-0.007380577, -0.015709779, 0.017383842, -0.007038902),
        (960.73577164, 539.01147371),
    ),
    CameraSpec(
        3, "camera_rear_left_70fov", "Rear left",
        2.028755, 1.039446, 0.885386, 151.680637, -3.134268, 70.0,
        -0.666026, 1570.851575,
        (-0.033947816, 0.401793972, -1.702209054, 2.126361486),
        (976.85215933, 562.69828079),
    ),
    CameraSpec(
        4, "camera_rear_tele_30fov", "Rear tele (context)",
        0.382798, 0.300216, 1.433513, 179.318246, -0.600805, 30.0,
        0.272256, 3682.876071,
        (0.231522708, -8.797345185, 159.2215684, -1036.335972),
        (951.05683958, 548.15984186),
    ),
    CameraSpec(
        5, "camera_rear_right_70fov", "Rear right",
        2.007933, -1.005456, 0.896404, -162.200838, -1.211118, 70.0,
        -0.535919, 1560.629773,
        (0.009383454, 0.079461877, -0.582353954, 0.775536694),
        (970.96068574, 549.93512723),
    ),
    CameraSpec(
        6, "camera_front_tele_30fov", "Front tele",
        1.667030, 0.073855, 1.443884, -0.872023, 0.575408, 30.0,
        0.244831, 3687.248370,
        (0.058971681, -1.359812221, 23.21701466, -149.2217783),
        (966.87830919, 548.33546092),
    ),
)

# This CARLA-only chase camera is displayed in the dashboard but is never sent
# to Alpamayo. It turns the canonical seven-camera payload into a balanced 2x4
# wall without altering the model schema.
CARLA_THIRD_PERSON_CAMERA = CameraSpec(
    7,
    "carla_top_third_person",
    "CARLA third person",
    -6.0,
    0.0,
    5.0,
    0.0,
    32.0,
    90.0,
)
DISPLAY_CAMERA_RING = CAMERA_RING + (CARLA_THIRD_PERSON_CAMERA,)

CAMERA_INDICES = tuple(camera.index for camera in CAMERA_RING)
CAMERA_NAMES = tuple(camera.name for camera in CAMERA_RING)
DISPLAY_CAMERA_INDICES = tuple(camera.index for camera in DISPLAY_CAMERA_RING)
CAMERAS_BY_INDEX = {camera.index: camera for camera in DISPLAY_CAMERA_RING}

# The trajectory profile consumes these six cameras. Camera 4 remains in the
# canonical source payload because upstream validates the seven-camera ring.
TRAJECTORY_CAMERA_INDICES = (0, 1, 2, 3, 5, 6)

# Front views on row one; rear/context and the CARLA chase view on row two.
DASHBOARD_CAMERA_INDICES = (0, 1, 2, 6, 3, 4, 5, 7)

# These source poses fall inside the MKZ roof/windshield envelope. At runtime
# only their height is clamped above the actual vehicle bounding box; their
# calibrated yaw, pitch, roll, and horizontal position remain unchanged.
BODY_CLEARANCE_CAMERA_INDICES = (1, 6)


def body_clearance_z(
    calibrated_z: float,
    bounding_box_location_z: float,
    bounding_box_extent_z: float,
    clearance_m: float,
) -> float:
    """Raise a camera only as much as needed to clear the vehicle body."""
    return max(
        calibrated_z,
        bounding_box_location_z + bounding_box_extent_z + clearance_m,
    )
