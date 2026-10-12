# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""``dict_to_camera_spec`` builds every lens model the render engine accepts.

``CameraSpec`` is a oneof over three lens models and NRE serves all three, but this function
used to build ``FthetaCameraParam`` and reject everything else -- while its own docstring
documented ``opencv_pinhole`` and ``opencv_fisheye``.  That made two things impossible: the
``lens="pinhole"`` mode of ``carla_cosmos.nurec``, and rendering any camera the scene does not
calibrate with an f-theta (a nominal AV-7 slot, or a plain CARLA sensor described by its FOV).

The distortion vectors are the other half.  ``ncore`` asserts a fixed shape on each of them, so
an undistorted lens is a vector of zeros and *not* an empty list: passing ``radial_coeffs=[]``
made the server raise ``AssertionError`` inside ``render_rgb`` and answer an opaque
``StatusCode.UNKNOWN``.  These tests pin the lengths the server demands: 6 radial, 2 tangential,
4 thin-prism.
"""

import math
import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from nurec_integration import dict_to_camera_spec

RES = {"resolution_w": 1280, "resolution_h": 720}


def test_defaults_to_ftheta():
    spec = dict_to_camera_spec({**RES, "pixeldist_to_angle_poly": [0.0, 0.5]})
    assert spec.HasField("ftheta_param")
    assert list(spec.ftheta_param.pixeldist_to_angle_poly) == [0.0, 0.5]
    assert spec.ftheta_param.max_angle == pytest.approx(math.pi)
    assert spec.ftheta_param.principal_point_x == 640.0


def test_pinhole_is_built_not_rejected():
    spec = dict_to_camera_spec({**RES, "camera_type": "opencv_pinhole",
                                "focal_length_x": 600.0, "focal_length_y": 610.0})
    assert spec.HasField("opencv_pinhole_param") and not spec.HasField("ftheta_param")
    p = spec.opencv_pinhole_param
    assert (p.focal_length_x, p.focal_length_y) == (600.0, 610.0)
    assert (p.principal_point_x, p.principal_point_y) == (640.0, 360.0)


def test_pinhole_focal_length_can_come_from_a_field_of_view():
    """What a CARLA sensor knows about itself is an FOV, not a focal length."""
    spec = dict_to_camera_spec({**RES, "camera_type": "opencv_pinhole", "fov": 90.0})
    assert spec.opencv_pinhole_param.focal_length_x == pytest.approx(640.0)
    assert spec.opencv_pinhole_param.focal_length_y == pytest.approx(640.0)


def test_a_projective_lens_without_a_focal_length_or_a_fov_is_refused():
    with pytest.raises(ValueError, match="focal_length_x"):
        dict_to_camera_spec({**RES, "camera_type": "opencv_pinhole"})


@pytest.mark.parametrize("field,length", [("radial_coeffs", 6), ("tangential_coeffs", 2),
                                          ("thin_prism_coeffs", 4)])
def test_pinhole_distortion_vectors_are_padded_to_the_length_ncore_asserts(field, length):
    """An empty list is not "no distortion" to the server -- it is a shape assertion failure."""
    p = dict_to_camera_spec({**RES, "camera_type": "opencv_pinhole",
                             "focal_length_x": 600.0}).opencv_pinhole_param
    assert len(getattr(p, field)) == length
    assert all(c == 0.0 for c in getattr(p, field))


def test_given_coefficients_are_kept_and_only_the_tail_is_padded():
    p = dict_to_camera_spec({**RES, "camera_type": "opencv_pinhole", "focal_length_x": 600.0,
                             "radial_coeffs": [0.1, -0.2]}).opencv_pinhole_param
    assert list(p.radial_coeffs) == [0.1, -0.2, 0.0, 0.0, 0.0, 0.0]


def test_too_many_coefficients_is_an_error_rather_than_a_silent_truncation():
    with pytest.raises(ValueError, match="at most 6 radial_coeffs"):
        dict_to_camera_spec({**RES, "camera_type": "opencv_pinhole", "focal_length_x": 600.0,
                             "radial_coeffs": [0.0] * 7})


def test_fisheye_is_built_with_its_max_angle():
    spec = dict_to_camera_spec({**RES, "camera_type": "opencv_fisheye", "fov": 180.0,
                                "max_angle": 2.0})
    assert spec.HasField("opencv_fisheye_param")
    assert spec.opencv_fisheye_param.max_angle == pytest.approx(2.0)
    assert len(spec.opencv_fisheye_param.radial_coeffs) == 6


def test_an_unknown_lens_still_names_the_ones_that_work():
    with pytest.raises(ValueError, match="opencv_pinhole"):
        dict_to_camera_spec({**RES, "camera_type": "orthographic"})


def test_carla_style_parameters_still_produce_an_ftheta():
    """The documented convenience path is unchanged: image_size + fov -> ftheta."""
    spec = dict_to_camera_spec({"image_size_x": 800, "image_size_y": 600, "fov": 90.0})
    assert spec.HasField("ftheta_param")
    assert (spec.resolution_w, spec.resolution_h) == (800, 600)
