"""Shared helpers for the CARLA -> Autoware map tools.

Coordinate convention (load-bearing, read this before touching any math):

* CARLA / Unreal is LEFT-handed: x forward/east, y RIGHT (south), z up.
* OpenDRIVE and the Autoware lanelet2/pcd map pair are RIGHT-handed.
  CARLA negates y when it imports an OpenDRIVE file, so a point at
  OpenDRIVE (x, y) shows up in CARLA at (x, -y).
* The prebuilt carla-simulator/autoware-contents maps follow the same
  convention: map frame y == -CARLA y ("y-axis-inverted relative to
  CARLA"). That IS what Autoware expects here.

Therefore:
* Anything derived directly from the OpenDRIVE string (lanelet2
  conversion) is already in the map frame -- do NOT flip it again.
* Anything sampled from live CARLA actors/sensors (lidar returns,
  traffic-light poses, waypoints) is in the CARLA frame and must go
  through :func:`carla_xyz_to_map` (y := -y) before being written into
  map artifacts.
"""

from __future__ import annotations

import sys
import time

CARLA_IMPORT_HELP = (
    "ERROR: the 'carla' Python package is not importable.\n"
    "These tools talk to a running CARLA server through the PythonAPI.\n"
    "Fix one of:\n"
    "  * pip install the wheel built from this repo (PythonAPI/carla/dist/carla-*.whl)\n"
    "    into your (virtual) environment, or\n"
    "  * export PYTHONPATH=<repo>/PythonAPI/carla:$PYTHONPATH if you use an egg/source layout.\n"
)


def import_carla():
    """Import and return the carla module, exiting with guidance if absent."""
    try:
        import carla  # noqa: PLC0415
    except ImportError:
        sys.stderr.write(CARLA_IMPORT_HELP)
        sys.exit(2)
    return carla


def connect(host, port, timeout=10.0):
    """Connect to a RUNNING CARLA server. Never launches one."""
    carla = import_carla()
    client = carla.Client(host, port)
    client.set_timeout(timeout)
    try:
        world = client.get_world()
    except RuntimeError as exc:
        sys.stderr.write(
            f"ERROR: could not reach a CARLA server at {host}:{port} ({exc}).\n"
            "Start the simulator first (these tools never launch it), or pass "
            "--host/--port for a server running elsewhere.\n"
        )
        sys.exit(2)
    return client, world


def carla_xyz_to_map(x, y, z):
    """CARLA left-handed world coords -> right-handed Autoware map frame."""
    return (x, -y, z)


class TickPolicy:
    """Frame-advance policy honouring the 'exactly one ticking client' rule.

    * async world              -> the server ticks itself; we just wait.
    * sync world + allow_tick  -> WE are the single ticker (world.tick()).
    * sync world, no tick flag -> some OTHER client must be ticking; we wait
                                  passively and time out with a clear message.
    """

    def __init__(self, world, allow_tick):
        self.world = world
        settings = world.get_settings()
        self.synchronous = bool(settings.synchronous_mode)
        self.fixed_delta = settings.fixed_delta_seconds or None
        self.we_tick = self.synchronous and allow_tick
        if self.synchronous and not allow_tick:
            print(
                "NOTE: world is synchronous and --tick was not given; this tool "
                "will NOT tick. It relies on exactly one other client ticking. "
                "If nothing ticks, sensor capture will time out.",
                flush=True,
            )

    def advance(self):
        """Advance (or wait for) one simulation frame."""
        if self.we_tick:
            self.world.tick()
        elif self.synchronous:
            # An external client is (supposedly) ticking; just yield.
            time.sleep(0.005)
        else:
            self.world.wait_for_tick()

    def timeout_hint(self):
        if self.synchronous and not self.we_tick:
            return (
                "no sensor data arrived: the world is synchronous and nothing is "
                "ticking it. Re-run with --tick to make this tool the single "
                "ticking client, or keep your existing ticking client running."
            )
        return "no sensor data arrived from the server (is the simulation stalled?)"
