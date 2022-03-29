# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla

class TestCollisionSensor(SyncSmokeTest):
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()

    def collision_callback(self, event, event_list):
        event_list.append(event)

    def run_collision_single_car_against_wall(self, bp_vehicle):
        veh_transf = carla.Transform(carla.Location(30, -6, 1), carla.Rotation(yaw=-90))
        vehicle = self.world.spawn_actor(bp_vehicle, veh_transf)

        bp_col_sensor = self.world.get_blueprint_library().find('sensor.other.collision')
        col_sensor = self.world.spawn_actor(bp_col_sensor, carla.Transform(), attach_to=vehicle)

        event_list = []
        col_sensor.listen(lambda data: self.collision_callback(data, event_list))

        self.wait(100)
        vehicle.set_target_velocity(10.0*veh_transf.rotation.get_forward_vector())

        self.wait(100)

        col_sensor.destroy()
        vehicle.destroy()

        return event_list

    def test_single_car(self):
        print("TestCollisionSensor.test_single_car")

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        cars_failing = ""
        for bp_veh in bp_vehicles:
            # Run collision agains wall
            event_list = self.run_collision_single_car_against_wall(bp_veh)

            if len(event_list) == 0:
                cars_failing += " %s" % bp_veh.id

        # Check result events
        if cars_failing != "":
            self.fail("The collision sensor have failed for the cars: %s" % cars_failing)
