"""World helpers: dynamic-actor bookkeeping, cleanup and settings restore."""

import time

import carla

DYNAMIC = ("vehicle.*", "walker.*", "sensor.*", "controller.*")


def dynamic_ids(world):
    ids = set()
    for f in DYNAMIC:
        ids.update(a.id for a in world.get_actors().filter(f))
    return ids


def count(world, pattern, role=None):
    actors = world.get_actors().filter(pattern)
    if role is not None:
        actors = [a for a in actors if a.attributes.get("role_name") == role]
    return len(actors)


def find_role(world, role, pattern="vehicle.*"):
    for a in world.get_actors().filter(pattern):
        if a.attributes.get("role_name") == role:
            return a
    return None


def speed(actor):
    v = actor.get_velocity()
    return (v.x ** 2 + v.y ** 2 + v.z ** 2) ** 0.5


def destroy(client, ids):
    """Stop walker controllers first, then destroy everything in one batch."""
    if not ids:
        return
    world = client.get_world()
    for a in world.get_actors(list(ids)).filter("controller.ai.walker"):
        try:
            a.stop()
        except RuntimeError:
            pass
    client.apply_batch_sync([carla.command.DestroyActor(i) for i in ids], False)


def restore_settings(world, initial):
    s = world.get_settings()
    s.synchronous_mode = False
    s.no_rendering_mode = False
    s.fixed_delta_seconds = initial.fixed_delta_seconds if initial else None
    world.apply_settings(s)


def sim_wait(world, seconds):
    """Wait `seconds` of wall time while the world runs (async) — keeps polling ticks."""
    end = time.time() + seconds
    while time.time() < end:
        try:
            world.wait_for_tick(2.0)
        except RuntimeError:
            time.sleep(0.2)
