#!/usr/bin/env python

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Tester for vehicle physics utility for CARLA
Uses:
    Basics scenearios (acceleration, brake, u-turn):
        python vehicle_physics_tester.py --filter vehicle_id --basics
    High-speed (100km/h) turn sceneario:
        python vehicle_physics_tester.py --filter vehicle_id --turn
"""

import argparse
import time
import numpy as np

import carla

class VehicleControlStop:
    def __init__(self, x_min = -100000, x_max = +100000, y_min = -100000, y_max = +100000,
            yaw_min = -500, yaw_max = +500, speed_min = -1, speed_max = +100000):

        self.x_min = x_min
        self.x_max = x_max
        self.y_min = y_min
        self.y_max = y_max
        self.yaw_min = yaw_min
        self.yaw_max = yaw_max
        self.speed_min = speed_min
        self.speed_max = speed_max

    def stop_control(self, vehicle):

        loc = vehicle.get_location()
        if loc.x > self.x_max :
            return True
        if loc.x < self.x_min:
            return True

        if loc.y > self.y_max :
            return True
        if loc.y < self.y_min:
            return True

        rot = vehicle.get_transform().rotation

        if rot.yaw > self.yaw_max :
            return True
        if rot.yaw < self.yaw_min:
            return True

        speed = norm(vehicle.get_velocity())
        if speed > self.speed_max :
            return True
        if speed < self.speed_min:
            return True

        return False


def change_physics_control(vehicle, tire_friction = None, drag = None, wheel_sweep = None, 
    max_rpm = None):

    physics_control = vehicle.get_physics_control()

    if drag is not None:
        physics_control.drag_coefficient = drag

    if wheel_sweep is not None:
        physics_control.use_sweep_wheel_collision = wheel_sweep

    if max_rpm is not None:
        physics_control.max_rpm = max_rpm

    if len(physics_control.forward_gears) == 1:
        gear = 1.0
        physics_control.forward_gears = [gear]

    if len(physics_control.reverse_gears) == 1:
        gear = 1.0
        physics_control.reverse_gears = [gear]

    front_left_wheel = physics_control.wheels[0]
    front_right_wheel = physics_control.wheels[1]
    rear_left_wheel = physics_control.wheels[2]
    rear_right_wheel = physics_control.wheels[3]

    if tire_friction is not None:
        front_left_wheel.tire_friction = tire_friction
        front_right_wheel.tire_friction = tire_friction
        rear_left_wheel.tire_friction = tire_friction
        rear_right_wheel.tire_friction = tire_friction

    wheels = [front_left_wheel, front_right_wheel, rear_left_wheel, rear_right_wheel]
    physics_control.wheels = wheels

    return physics_control

def print_step_info(world, vehicle):
    snapshot = world.get_snapshot()
    print("%d %06.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f" %
            (snapshot.frame, snapshot.timestamp.elapsed_seconds, \
            vehicle.get_acceleration().x, vehicle.get_acceleration().y, vehicle.get_acceleration().z, \
            vehicle.get_velocity().x, vehicle.get_velocity().y, vehicle.get_velocity().z, \
            vehicle.get_location().x, vehicle.get_location().y, vehicle.get_location().z))

def wait(world, frames=100):
    for _i in range(0, frames):
        world.tick()

def norm(vec):
    return np.sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z)

class TelemetryPoint:
    def __init__(self, curr_time=None, location=None, rotation=None, velocity=None):
        self.time = curr_time
        self.location = location
        self.rotation = rotation
        self.velocity = velocity

    def __str__(self):
        return "%.2f %s %s %s" % (self.time, str(self.location), str(self.rotation), str(self.velocity))

    def __sub__(self, other):
        t = self.time - other.time
        l = self.location - other.location
        r = carla.Rotation()
        v = self.velocity - other.velocity
        return TelemetryPoint(t, l, r, v)

class TelemetryData:
    def __init__(self, curr_time, vehicle):
        self.list_of_telemetries = []
        location = vehicle.get_location()
        rotation = vehicle.get_transform().rotation
        velocity = vehicle.get_velocity()
        self.list_of_telemetries.append(TelemetryPoint(curr_time, location, rotation, velocity))

    def __str__(self):
        ret_str = ""
        for idx, val in enumerate(self.list_of_telemetries):
            ret_str += "%d: %s\n" % (idx, str(val))

        return ret_str

    def add_telemetry(self, curr_time, vehicle):
        location = vehicle.get_location()
        rotation = vehicle.get_transform().rotation
        velocity = vehicle.get_velocity()
        self.list_of_telemetries.append(TelemetryPoint(curr_time, location, rotation, velocity))

    def number_of_telemetries(self):
        return len(self.list_of_telemetries)

    def get_telemetry(self, index):
        if index >= self.number_of_telemetries():
            return TelemetryPoint()

        return self.list_of_telemetries[index]

    def get_telemetry_delta(self, index):
        if index >= (self.number_of_telemetries()-1):
            return TelemetryPoint()

        return self.list_of_telemetries[index+1] - self.list_of_telemetries[index]

    def get_scalar_delta(self, index):
        if index >= (self.number_of_telemetries()-1):
            return TelemetryPoint()

        delta = self.list_of_telemetries[index+1] - self.list_of_telemetries[index]

        return delta.time, norm(delta.location), norm(delta.velocity)


def run_scenario(world, bp_veh, init_loc, init_speed = 0.0, init_frames=10,
        controls=[(150, carla.VehicleControl(), VehicleControlStop())],
        apply_phys_control = None):

    veh_transf = init_loc

    vehicle = world.spawn_actor(bp_veh, veh_transf)
    wait(world, 10)

    data = TelemetryData(world.get_snapshot().elapsed_seconds, vehicle)

    # Initialization at init_speed
    vehicle.enable_constant_velocity(carla.Vector3D(init_speed, 0, 0))
    wait(world, init_frames)
    vehicle.disable_constant_velocity()
    wait(world, 1)

    for i_control in controls:
        control_frames = i_control[0]
        control = i_control[1]
        stopper = i_control[2]

        data.add_telemetry(world.get_snapshot().elapsed_seconds, vehicle)

        # Apply control
        vehicle.apply_control(control)
        for _i in range(0, control_frames):
            world.tick()
            check = stopper.stop_control(vehicle)
            if check:
                break

    data.add_telemetry(world.get_snapshot().elapsed_seconds, vehicle)

    wait(world, 10)
    vehicle.destroy()

    return data

def brake_scenario(world, bp_veh, speed):

    spectator_transform = carla.Transform(carla.Location(20, -190, 10), carla.Rotation(yaw=67, pitch=-13))
    try:
        spectator = world.get_spectator()
        spectator.set_transform(spectator_transform)
    except:
        print("No spectator")

    init_loc = carla.Transform(carla.Location(32, -180, 0.5), carla.Rotation(yaw=90))

    controls = [
        (1000, carla.VehicleControl(brake=1.0), VehicleControlStop(speed_min=0.1))]
    
    data = run_scenario(world, bp_veh, init_loc, init_speed=speed/3.6, controls=controls)

    delta = data.get_scalar_delta(1)
    end_vel = 3.6*norm(data.get_telemetry(2).velocity)
    print("  %.0f -> 0 km/h: (%.1f s, %.1f m)" % (speed, delta[0], delta[1]), end="")

def accel_scenario(world, bp_veh, max_vel):

    spectator_transform = carla.Transform(carla.Location(20, -190, 10), carla.Rotation(yaw=67, pitch=-13))
    try:
        spectator = world.get_spectator()
        spectator.set_transform(spectator_transform)
    except:
        print("No spectator")

    init_loc = carla.Transform(carla.Location(32, -180, 0.5), carla.Rotation(yaw=90))

    controls = [
        (1000, carla.VehicleControl(throttle=1.0), VehicleControlStop(speed_max=max_vel/3.6))]

    data = run_scenario(world, bp_veh, init_loc=init_loc, controls=controls)

    delta = data.get_scalar_delta(1)
    end_vel = 3.6*norm(data.get_telemetry(2).velocity)
    print("  0 -> %.0f km/h: (%.1f s, %.1f m)" % (max_vel, delta[0], delta[1]), end="")

def uturn_scenario(world, bp_veh):

    spectator_transform = carla.Transform(carla.Location(30, -180, 20), carla.Rotation(yaw=-140, pitch=-36))
    try:
        spectator = world.get_spectator()
        spectator.set_transform(spectator_transform)
    except:
        pass

    init_pos = carla.Transform(carla.Location(15, -190, 0.2), carla.Rotation(yaw=0))
    controls = [
        (1000, carla.VehicleControl(throttle=1.0), VehicleControlStop(x_max=19)),
        (1000, carla.VehicleControl(throttle=0.25, steer=-0.4), VehicleControlStop(yaw_min=-170)),
        (100, carla.VehicleControl(throttle=0.4), VehicleControlStop(x_min =10))
        ]

    data = run_scenario(world, bp_veh, init_loc=init_pos, controls=controls)
    end_vel = 3.6*norm(data.get_telemetry(3).velocity)

def highspeed_turn_scenario(world, bp_veh, steer):
    spectator_transform = carla.Transform(carla.Location(70, -200, 15), carla.Rotation(yaw=0, pitch=-12))

    try:
        spectator = world.get_spectator()
        spectator.set_transform(spectator_transform)
    except:
        pass

    init_pos = carla.Transform(carla.Location(50, -204, 0.2), carla.Rotation(yaw=0))
    init_frames = 2
    init_speed = 100 / 3.6
    controls = [(100, carla.VehicleControl(throttle=0.5), VehicleControlStop(x_max=100)),
        (200, carla.VehicleControl(throttle=1.0, steer = steer), VehicleControlStop(yaw_max=45, speed_min=3)),
        (200, carla.VehicleControl(brake=1), VehicleControlStop())]

    data = run_scenario(world, bp_veh, init_loc=init_pos, init_speed=init_speed, init_frames = init_frames, controls=controls)

    time.sleep(1)

def main(arg):
    """Main function of the script"""
    client = carla.Client(arg.host, arg.port)
    client.set_timeout(30.0)
    world = client.get_world()

    try:
        # Setting the world and the spawn properties
        original_settings = world.get_settings()
        settings = world.get_settings()

        delta = 0.05
        settings.fixed_delta_seconds = delta
        settings.synchronous_mode = True
        world.apply_settings(settings)

        if world.get_map().name != "Town05":
            client.load_world("Town05", False)

        for bp_veh in world.get_blueprint_library().filter(args.filter):
            print("-------------------------------------------")
            print(bp_veh.id, end="", flush=True)

            if args.show_physics_control:
                try:
                    veh_transf = carla.Transform()
                    veh_transf.location.z = 100
                    vehicle = world.spawn_actor(bp_veh, veh_transf)
                    print()
                    print(vehicle.get_physics_control())
                    vehicle.destroy()
                except:
                    pass

            if args.none:
                continue

            if args.accel or args.all:
                accel_scenario(world, bp_veh, 50)
                accel_scenario(world, bp_veh, 100)

            if args.brake or args.all:
                brake_scenario(world, bp_veh, 80)
                brake_scenario(world, bp_veh, 100)

            if args.uturn or args.all:
                uturn_scenario(world, bp_veh)

            if args.turn or args.all:
                highspeed_turn_scenario(world, bp_veh, 0.2)

            print()

        print("-------------------------------------------")


    finally:
        world.apply_settings(original_settings)



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
        default='vehicle.*',
        help='actor filter (default: "vehicle.*")')
    argparser.set_defaults(accel=False)
    argparser.add_argument(
        '--accel',
        dest='accel',
        action='store_true',
        help='Execute accel scenarios')
    argparser.set_defaults(brake=False)
    argparser.add_argument(
        '--brake',
        dest='brake',
        action='store_true',
        help='Execute brake scenarios')
    argparser.set_defaults(uturn=False)
    argparser.add_argument(
        '--uturn',
        dest='uturn',
        action='store_true',
        help='Execute brake scenarios')
    argparser.set_defaults(turn=False)
    argparser.add_argument(
        '--turn',
        dest='turn',
        action='store_true',
        help='Execute basic scenarios')
    argparser.set_defaults(all=True)
    argparser.add_argument(
        '--all',
        dest='all',
        action='store_true',
        help='Execute all scenarios')
    argparser.set_defaults(none=False)
    argparser.add_argument(
        '--none',
        dest='none',
        action='store_true',
        help='Do not execute any scenarios')
    argparser.set_defaults(show_physics_control=False)
    argparser.add_argument(
        '--show_physics_control',
        dest='show_physics_control',
        action='store_true',
        help='Show default physics control of cars')

    args = argparser.parse_args()
    if args.accel or args.brake or args.uturn or args.turn:
        args.all = False

    try:
        main(args)
    except KeyboardInterrupt:
        print(' - Exited by user.')
