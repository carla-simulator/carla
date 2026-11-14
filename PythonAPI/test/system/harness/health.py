"""Post-test health checks applied to every test that uses the server."""

import carla

from . import world as W
from .server import MAP


def snapshot(server, client):
    """State captured before a test, compared against afterwards."""
    world = client.get_world()
    try:
        world.wait_for_tick(5.0)
    except RuntimeError:
        pass
    return {"ids": W.dynamic_ids(world), "rss": server.rss_mb(),
            "log": server.log_size()}


def check(server, client, before):
    """Return (issues, notes, metrics). Issues fail the test; notes don't.

    Leaves the world clean: leaked actors destroyed, async mode restored,
    default map reloaded if a test left another one loaded.
    """
    issues, notes = [], []
    metrics = {"rss_before_mb": before["rss"]}
    crashes = server.crash_lines(before["log"])
    if not server.alive():
        issues.append("server died or stopped answering RPC")
        if crashes:
            issues.append("server log: " + " | ".join(crashes[:3]))
        return issues, notes, metrics
    if crashes:
        issues.append("crash markers in server log: " + " | ".join(crashes[:3]))

    # Patient: a test that timed out on a map load leaves the server busy for a
    # while; the next test must not inherit that.
    client.set_timeout(300.0)
    world = client.get_world()
    s = world.get_settings()
    if s.synchronous_mode:
        issues.append("world left in synchronous mode")
    if s.no_rendering_mode:
        issues.append("world left in no-rendering mode")
    if s.synchronous_mode or s.no_rendering_mode:
        W.restore_settings(world, server.initial_settings)
    try:
        world.wait_for_tick(10.0)
    except RuntimeError:
        issues.append("world does not tick after the test")

    leaked = W.dynamic_ids(world) - before["ids"]
    if leaked:
        types = {}
        for a in world.get_actors(list(leaked)):
            k = a.type_id.split(".")[0]
            types[k] = types.get(k, 0) + 1
        issues.append(f"leaked {len(leaked)} actors {types}")
        W.destroy(client, leaked)

    if MAP not in world.get_map().name:
        notes.append(f"test left map {world.get_map().name}; reloading {MAP}")
        client.load_world(MAP)

    client.get_world().set_weather(carla.WeatherParameters.ClearNoon)
    client.set_timeout(30.0)
    metrics["rss_after_mb"] = server.rss_mb()
    return issues, notes, metrics
