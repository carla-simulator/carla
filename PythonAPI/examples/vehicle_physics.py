#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Vehicle physics example for CARLA
Small example that shows the effect of different impulse and force application
methods to a vehicle.
"""

import argparse

import carla


def print_step_info(world, vehicle):
    snapshot = world.get_snapshot()
    print("%d %06.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f" %
            (snapshot.frame, snapshot.timestamp.elapsed_seconds, \
            vehicle.get_acceleration().x, vehicle.get_acceleration().y, vehicle.get_acceleration().z, \
            vehicle.get_velocity().x, vehicle.get_velocity().y, vehicle.get_velocity().z, \
            vehicle.get_location().x, vehicle.get_location().y, vehicle.get_location().z))


def wait(world, frames=100):
    for i in range(0, frames):
        world.tick()


def main(arg):
    """Main function of the script"""
    client = carla.Client(arg.host, arg.port)
    client.set_timeout(2.0)
    world = client.get_world()

    try:
        # Setting the world and the spawn properties
        original_settings = world.get_settings()
        settings = world.get_settings()

        delta = 0.1
        settings.fixed_delta_seconds = delta
        settings.synchronous_mode = True
        world.apply_settings(settings)

        blueprint_library = world.get_blueprint_library()
        vehicle_bp = blueprint_library.filter(arg.filter)[0]

        vehicle_transform = world.get_map().get_spawn_points()[0]
        vehicle_transform.location.z += 3
        vehicle = world.spawn_actor(vehicle_bp, vehicle_transform)

        physics_vehicle = vehicle.get_physics_control()
        car_mass = physics_vehicle.mass

        spectator_transform = carla.Transform(vehicle_transform.location, vehicle_transform.rotation)
        spectator_transform.location += vehicle_transform.get_forward_vector() * 20
        spectator_transform.rotation.yaw += 180
        spectator = world.get_spectator()
        spectator.set_transform(spectator_transform)


        # We let the vehicle stabilize and save the transform to reset it after each test.
        wait(world)
        vehicle.set_target_velocity(carla.Vector3D(0, 0, 0))
        vehicle_transform = vehicle.get_transform()
        wait(world)


        # Impulse/Force at the center of mass of the object
        impulse = 10 * car_mass

        print("# Adding an Impulse of %f N s" % impulse)
        vehicle.add_impulse(carla.Vector3D(0, 0, impulse))

        wait(world)
        vehicle.set_transform(vehicle_transform)
        vehicle.set_target_velocity(carla.Vector3D(0, 0, 0))
        wait(world)

        print("# Adding a Force of %f N" % (impulse / delta))
        # The add_force method should not be use for instantaneous forces like this one,
        # it is more useful for constant or variable forces acting in a finite amount of time.
        # In this script it is done with the proper scaling to show the equivalence
        # between the add_impulse and add_force methods.
        # As in this case the force is going to be applied during the whole step dt=delta
        # a force more or less equivalent is impulse / delta.
        vehicle.add_force(carla.Vector3D(0, 0, impulse / delta))

        wait(world)
        vehicle.set_transform(vehicle_transform)
        vehicle.set_target_velocity(carla.Vector3D(0, 0, 0))
        wait(world)

        wait(world, 500)


    finally:
        world.apply_settings(original_settings)

        vehicle.destroy()


if __name__ == "__main__":

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='model3',
        help='actor filter (default: "vehicle.*")')
    args = argparser.parse_args()

    try:
        main(args)
    except KeyboardInterrupt:
        print(' - Exited by user.')
