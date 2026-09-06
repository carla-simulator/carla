# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Forward-looking regression guards for NumPy 2 compatibility.

NumPy 2 removed `np.bool` (in 1.20) and `np.matrix` (in 2.0). ue5-dev only
had one site that tripped on this: PythonAPI/test/API/prop_control.py used
`np.bool` in the DVS event dtype. This module pins that fix so a future edit
cannot silently reintroduce the deprecated alias.
"""

import os
import re
import unittest

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", ".."))

PROP_CONTROL = os.path.join(
    REPO_ROOT, "PythonAPI", "test", "API", "prop_control.py")


class TestPropControlDvsDtype(unittest.TestCase):
    """The DVS dtype in prop_control.py must use NumPy-2-safe aliases."""

    def setUp(self):
        self.assertTrue(
            os.path.exists(PROP_CONTROL),
            f"Expected prop_control.py at {PROP_CONTROL}")
        with open(PROP_CONTROL) as f:
            self.contents = f.read()

    def test_uses_bool_underscore(self):
        """np.bool_ (NumPy 2 safe) is present in the DVS dtype."""
        self.assertIn(
            "('pol', np.bool_)",
            self.contents,
            "prop_control.py DVS dtype is missing the np.bool_ field")

    def test_no_bare_np_bool(self):
        """`np.bool` (removed in NumPy 1.20+) must not appear anywhere."""
        # Match `np.bool` not followed by an identifier character (so np.bool_
        # and np.bool8 stay valid).
        self.assertIsNone(
            re.search(r"\bnp\.bool\b(?!_)", self.contents),
            "prop_control.py still contains a bare `np.bool` reference")

    def test_no_np_matrix(self):
        """`np.matrix` (removed in NumPy 2) must not appear anywhere."""
        self.assertNotIn(
            "np.matrix",
            self.contents,
            "prop_control.py still contains a `np.matrix` reference")


if __name__ == "__main__":
    unittest.main()
