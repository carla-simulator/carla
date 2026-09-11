#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Shared pytest CLI options for lens_validation/. pytest only picks up
pytest_addoption from conftest.py (not from arbitrary test_*.py modules),
so all custom flags used across test_calibration_refit.py and
test_ab_wideangle.py are registered here."""


def pytest_addoption(parser):
    parser.addoption('--host', action='store', default='127.0.0.1',
                      help='CARLA server host (server-dependent tests only)')
    parser.addoption('--port', action='store', type=int, default=2000,
                      help='CARLA server port (server-dependent tests only)')
    parser.addoption('--capture-dir', action='store', default=None,
                      help='directory written by calibration_scene.py (contains manifest.json)')
    parser.addoption('--tolerance-px', action='store', type=float, default=0.1,
                      help='max allowed reprojection RMS in pixels at the captured resolution')
    parser.addoption('--feature-tolerance-px', action='store', type=float, default=1.5,
                      help='max allowed rt_lens vs wide_angle feature-point deviation in pixels')
    parser.addoption('--curve-tolerance-px', action='store', type=float, default=0.5,
                      help='max allowed detected-vs-analytic straight-edge curve deviation in pixels')
