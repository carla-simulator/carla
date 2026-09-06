# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Bhutan mobility-data and AV-evaluation toolkit for CARLA.

The package implements the real-to-synthetic-to-evaluation loop used by the
Bhutan pilot:

* ``taxonomy`` / ``scenario`` / ``library``: parameterized Bhutan-relevant
  scenario families and a deterministic generator for 100+ templates.
* ``weather``: Bhutan weather and lighting presets mapped to
  ``carla.WeatherParameters``.
* ``telemetry``: the unified timeline record schema shared with the
  Cloudflare Workers dashboard, a JSONL logger and an upload client.
* ``safety_rules``: critical safety-rule checks over telemetry and events.
* ``quality``: data-quality gates (missing data, timestamp drift, sensor
  gaps, privacy status) and segment acceptance.
* ``evaluation``: perception benchmark metrics by class, weather, lighting
  and route class, plus failure clustering and reproducibility manifests.
* ``runner``: executes a scenario template inside a running CARLA server
  and records telemetry, events and ground truth. Only this module imports
  ``carla``; everything else runs without a simulator.
"""

__version__ = "0.1.0"

from .scenario import ScenarioParameters, ScenarioTemplate, ActorSpec  # noqa: F401
from .taxonomy import Taxonomy, ScenarioFamily  # noqa: F401
from .library import generate_library  # noqa: F401
