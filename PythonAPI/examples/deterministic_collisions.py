#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Deterministic collisions example for CARLA

Runs the same crash several times, a car hitting a bicycle that crosses its
lane in Town10HD_Opt, and reports whether every repetition ended the same.

By default the physics engine can detect a contact one frame earlier or later
depending on how fast the server runs, so the same crash can end differently.
carla.WorldSettings.deterministic_collisions makes it repeat exactly. It
applies when a map loads, so this script applies it and then reloads the map
keeping the settings (reset_settings=False).

Run it with --no-deterministic to see the default behaviour. With
deterministic collisions on, the script exits with 1 if the repetitions differ.
"""

import argparse
import sys

import carla

MAP = 'Town10HD_Opt'
TICKS = 60


def set_up_world(client, deterministic):
    """Synchronous, fixed time-step and deterministic collisions, then reload."""
    world = client.get_world()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    # Physics substepping set explicitly, not inherited from whatever the
    # previous client left: it changes how the crash ends.
    settings.substepping = True
    settings.max_substep_delta_time = 0.01
    settings.max_substeps = 10
    settings.deterministic_collisions = deterministic
    world.apply_settings(settings)
    # reset_settings=False keeps the settings just applied, and the reload
    # makes deterministic_collisions take effect.
    if world.get_map().name.split('/')[-1] == MAP:
        return client.reload_world(False)
    return client.load_world(MAP, False)


def run_crash(client):
    """One repetition: reload, spawn a car and a crossing bicycle, record both."""
    world = client.reload_world(False)
    for _ in range(5):
        world.tick()
    library = world.get_blueprint_library()
    car = carla.Transform(carla.Location(-80.2, 24.6, 0.04), carla.Rotation(yaw=0))
    bike = carla.Transform(carla.Location(-45.2, 38.6, 0.04), carla.Rotation(yaw=-90))
    # A batch spawns both actors on the same frame, with their initial speed.
    responses = client.apply_batch_sync([
        carla.command.SpawnActor(library.find('vehicle.lincoln.mkz'), car)
        .then(carla.command.ApplyTargetVelocity(carla.command.FutureActor, carla.Vector3D(30, 0, 0))),
        carla.command.SpawnActor(library.find('vehicle.gazelle.omafiets'), bike)
        .then(carla.command.ApplyTargetVelocity(carla.command.FutureActor, carla.Vector3D(0, -12, 0)))])
    actor_ids = [response.actor_id for response in responses]
    if any(response.error for response in responses):
        raise RuntimeError(f'spawn failed: {[response.error for response in responses]}')

    # Location and velocity of both actors on every frame, unrounded: the
    # runs only count as the same if they match exactly.
    actors = [world.get_actor(actor_id) for actor_id in actor_ids]
    trajectory = []
    for _ in range(TICKS):
        ticked = world.tick()
        if world.get_snapshot().frame != ticked:
            # Something else advanced the server: the samples no longer match
            # this client's ticks, so the runs cannot be compared.
            print(f'  warning: the server is at frame {world.get_snapshot().frame} '
                  f'after this client ticked frame {ticked}')
        frame = []
        for actor in actors:
            location, velocity = actor.get_location(), actor.get_velocity()
            frame += [location.x, location.y, location.z, velocity.x, velocity.y, velocity.z]
        trajectory.append(tuple(frame))

    client.apply_batch_sync([carla.command.DestroyActor(actor_id) for actor_id in actor_ids], True)
    return trajectory


def main():
    """Repeat the crash and report whether every repetition ended the same."""
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-n', '--repetitions',
        metavar='N',
        default=8,
        type=int,
        help='number of times to repeat the crash (default: 8)')
    argparser.add_argument(
        '--no-deterministic',
        action='store_true',
        help='keep the default collision detection, to compare')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)
    original_settings = client.get_world().get_settings()

    try:
        deterministic = not args.no_deterministic
        set_up_world(client, deterministic)
        print(f'deterministic_collisions = {deterministic}')

        trajectories = []
        for repetition in range(args.repetitions):
            trajectories.append(run_crash(client))
            end = trajectories[-1][-1]
            print(f'  repetition {repetition}: car ends at ({end[0]:.4f}, {end[1]:.4f}), '
                  f'bicycle at ({end[6]:.4f}, {end[7]:.4f})')

        outcomes = len(set(tuple(trajectory) for trajectory in trajectories))
        if outcomes == 1:
            print(f'All {args.repetitions} repetitions ended the same.')
            return 0
        print(f'{args.repetitions} repetitions gave {outcomes} different outcomes.')
        # Only a failure when deterministic collisions were asked for.
        return 1 if deterministic else 0

    finally:
        # Back to the original settings; the reload turns
        # deterministic_collisions back to what they had.
        world = client.get_world()
        world.apply_settings(original_settings)
        client.reload_world(False)


if __name__ == '__main__':
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        pass
