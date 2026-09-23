#!/usr/bin/env python3
"""Unit tests for generate_lanelet2_map._check_unique_opendrive_id(). Runs with
plain python or pytest (no live CARLA server or crdesigner needed):

    python3 PythonAPI/examples/av_stacks/autoware/map_tools/tests/test_generate_lanelet2_map.py
    pytest PythonAPI/examples/av_stacks/autoware/map_tools/tests/test_generate_lanelet2_map.py
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from generate_lanelet2_map import _check_unique_opendrive_id  # noqa: E402


def test_valid_numeric_id_is_accepted():
    seen = {}
    result = _check_unique_opendrive_id("stop sign", 1, "42", seen)
    assert result == 42
    assert seen == {42: 1}
    print("test_valid_numeric_id_is_accepted OK")


def test_empty_id_is_skipped_with_warning():
    seen = {}
    result = _check_unique_opendrive_id("stop sign", 2, "", seen)
    assert result is None
    assert seen == {}
    print("test_empty_id_is_skipped_with_warning OK")


def test_non_numeric_id_is_skipped_with_warning():
    seen = {}
    result = _check_unique_opendrive_id("traffic light", 3, "abc", seen)
    assert result is None
    assert seen == {}
    print("test_non_numeric_id_is_skipped_with_warning OK")


def test_duplicate_numeric_id_still_raises():
    seen = {}
    _check_unique_opendrive_id("stop sign", 4, "7", seen)
    try:
        _check_unique_opendrive_id("stop sign", 5, "7", seen)
    except RuntimeError:
        print("test_duplicate_numeric_id_still_raises OK")
        return
    raise AssertionError("expected RuntimeError for duplicate OpenDRIVE id")


if __name__ == "__main__":
    test_valid_numeric_id_is_accepted()
    test_empty_id_is_skipped_with_warning()
    test_non_numeric_id_is_skipped_with_warning()
    test_duplicate_numeric_id_still_raises()
    print("All generate_lanelet2_map tests passed.")
