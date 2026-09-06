# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Ingestion and export adapters between external logging tools and the unified timeline.

* ``gpx``: GPX tracks from dashcams, phones and GPS loggers (standard library only).
* ``mcap``: MCAP files from ROS 2 / rosbag2 recorders and Foxglove (needs the ``mcap`` package).
* ``traccar``: Traccar position exports and API responses.

Every adapter returns plain sample dictionaries in the ``telemetry.Sample``
shape so ``convert_run.py`` can write a run directory that ``check_quality.py``
and ``upload_run.py`` accept unchanged.
"""

from .common import derive_motion, write_run_dir  # noqa: F401
