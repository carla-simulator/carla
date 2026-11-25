#!/usr/bin/env python3
"""Probe a containerised CARLA server: versions, map, and one camera frame streamed back to the host."""
import queue
import sys

import carla

port = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
client = carla.Client("127.0.0.1", port)
client.set_timeout(60.0)
sv, cv = client.get_server_version(), client.get_client_version()
print(f"[probe] server {sv}, client {cv}{'' if sv == cv else '  MISMATCH: install the wheel from this image'}")
world = client.get_world()
print(f"[probe] map {world.get_map().name}, {len(world.get_map().get_spawn_points())} spawn points, {len(client.get_available_maps())} maps")
# A camera frame proves the streaming port (RPC + 1) reaches the host, not just the RPC port.
q = queue.Queue()
cam = world.spawn_actor(world.get_blueprint_library().find("sensor.camera.rgb"), carla.Transform(carla.Location(0, 0, 10)))
try:
    cam.listen(q.put)
    img = q.get(timeout=30)
    print(f"[probe] camera frame {img.frame} ({img.width}x{img.height}) received")
finally:
    cam.stop()
    cam.destroy()
