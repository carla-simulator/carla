import math
import os
import time
import numpy as np
from collections import deque
from functools import reduce
from copy import deepcopy


from util.classes.constants import Constants
from util.classes.perceived_objects import PerceivedObject
from util.classes.utils import Location
from util.classes.constants import Constants

from numba import jit, njit


os.environ['NUMBA_OPT'] = str(3)


@njit
def perceived_objects_by_jit(data, current_time, duration, distance_threshold):
    """
    sensor_data = [
        [x_pos, y_pos, x_speed, y_speed, time] of point 1,
        [x_pos, y_pos, x_speed, y_speed, time] of point 2,
        ...,
        [x_pos, y_pos, x_speed, y_speed, time] of point N,
    ]

    new_perceived_objects = [
        [x_pos, y_pos, x_speed, y_speed, time] of object 1,
        [x_pos, y_pos, x_speed, y_speed, time] of object 2,
        ...,
        [x_pos, y_pos, x_speed, y_speed, time] of object N,
    ]

    return perceived_objects
    """

    new_perceived_objects = [[0, 0, 0, 0]]
    new_perceived_objects.pop()
    tmp_groups = [[0, 0, 0, 0]]
    tmp_groups.pop()

    while data:
        osd = data.pop()  # obstacle_sensor_data (osd)

        # validation
        if data[4] + duration < current_time:
            continue

        is_in_group = False
        for tmp_group in tmp_groups:
            for nop in tmp_group:
                dT = nop[4] - osd[4]
                osd_x = osd[0] + osd[2] * dt
                osd_y = osd[1] + osd[3] * dt
                dx = osd_x - nop[0]
                dy = osd_y - nop[1]

                if math.sqrt(dx * dx + dy * dy) <= distance_threshold:
                    is_grouped = True
                    break
                else:
                    continue

            if is_grouped:
                tmp_group.append(osd)
                break
            else:
                continue

        if is_in_group:
            continue
        else:
            tmp_groups.append([osd])

    for tmp_group in tmp_groups:
        latest_data = [0, 0, 0, 0]
        total_x_pos = 0
        total_y_pos = 0
        group_length = 0

        for d in tmp_group:
            if latest_time < d[4]:
                latest_data = d

            group_length = group_length + 1
            total_x_pos = total_x_pos + d[0]
            total_y_pos = total_y_pos + d[1]

        new_perceived_objects.append([total_x_pos/group_length, total_y_pos/group_length, latest_data[2], latest_data[3], latest_data[4]])


    return new_perceived_objects


class SensorDataHandler:
    def __init__(self):
        self.data = deque()
        self.data_for_jit = []


    def data_num(self):
        return len(self.data)

    def perceived_objects(self, current_time, duration):
        """
        This method is used to create Percevec Objects (POs) based on SensorData.
        POs have to belong to PerceivedObject class.
        Furthermore, this method have to return the list of POs.
        """
        pass


    def save(self, data):
        self.data.append(data)

    def save_for_jit(self, data):
        self.data.append(data)


class ObstacleSensorData:
    """
    This class is the ease implimantation of radar sensor data.
    We assume that we can get obstacles' speeds and distances [1].
    The distances are used to calculate the obstacles' locations.

    [1] cite: https://carla.readthedocs.io/en/latest/ref_sensors/
    """
    def __init__(self, time, location, speed):
        # We use the three data.
        self.time = time
        self.l = location
        self.s = speed


    def distance_from_other_location(self, other_location):
        location = self.location()

        dx = location.x - other_location.x
        dy = location.y - other_location.y

        return math.sqrt(dx * dx + dy * dy)


    def location(self):
        return self.l


    def predicted_location(self, delta_t):
        speed = self.speed()
        location = self.location()

        return Location(location.x + delta_t * speed.x, location.y + delta_t * speed.y)


    def speed(self):
        return self.s


class ObstacleSensorDataHandler(SensorDataHandler):

    def perceived_objects_by_jit(self, current_time, duration):
        new_perceived_objects = []

        if len(self.data_for_jit) <= 0:
            pass
        else:
            for p in perceived_objects_by_jit(self.data_for_jit, current_time, duration, Constants.LOCATION_THRESHOLD):
                new_perceived_objects.append(
                    PerceivedObject(
                        time=p[4],
                        location=Location(p[0], p[1]),
                        speed=Speed(p[2], p[3])
                    )
                )

        return new_perceived_objects


    def perceived_objects(self, current_time, duration):
        # The original method becomes too slow when the number of data is increased.
        # Therefore, we utilize function with jit.
        # return self.perceived_objects_by_jit(current_time, duration)


        # ----- original codes -----
        start = time.time()
        new_perceived_objects = []
        data = deque(self.data)
        self.data = []

        tmp_groups = []
        t1 = time.time()
        while data:
            osd = data.popleft()  # obstacle_sensor_data (osd)

            # validation
            if osd.time + duration < current_time:
                continue

            is_in_group = False
            for tmp_group in tmp_groups:
                is_in_group = reduce((lambda x, y: x or y), [self.__is_predictable_location(osd, osd_in_group) for osd_in_group in tmp_group], False)

                if is_in_group:
                    tmp_group.append(osd)
                    break
                else:
                    continue

            if is_in_group:
                continue
            else:
                tmp_groups.append([osd])

        t2 = time.time()
        for tmp_group in tmp_groups:
            latest_time = 0
            latest_data = None
            total_x_pos = 0
            total_y_pos = 0

            for d in tmp_group:
                if latest_time < d.time:
                    latest_time = d.time
                    latest_data = d

                total_x_pos = total_x_pos + d.location().x
                total_y_pos = total_y_pos + d.location().y

            new_perceived_objects.append(
                PerceivedObject(
                    time=latest_time,
                    location=Location(total_x_pos / len(tmp_group), total_y_pos / len(tmp_group)),
                    speed=latest_data.speed()
                )
            )
        t3 = time.time()

        print(f"new_perceived_objects, t1: {t1 - start}, t2: {t2 - t1}, t3: {t3 - t2}")

        return new_perceived_objects

    def clear_data(self):
        self.data.clear()


    def get_grouped_indexes(self, current_time, duration, data):
        grouped_indexes = []
        grouped_map = self.grouped_map(current_time, duration, data)

        for target_index in range(0, len(data)):
            if reduce((lambda x, y: x or y), [(target_index in indexes) for indexes in grouped_indexes], False):
                continue
            else:
                grouped_indexes.append(list(self.grouped_indexes_by_index(target_index, grouped_map, data)))

        return grouped_indexes


    def grouped_map(self, current_time, duration, data):
        grouped_map = {}

        for target_index in range(0, len(data)):
            for compared_index in range(target_index, len(data)):
                is_grouped = self.__is_predictable_location(
                    data[target_index],
                    data[compared_index]
                )

                grouped_map[target_index, compared_index] = is_grouped
                grouped_map[compared_index, target_index] = is_grouped

        return grouped_map


    def grouped_indexes_by_index(self, start_index, grouped_map, data):
        result = {start_index}

        for target_index in range(start_index + 1, len(data)):
            if target_index in result:
                continue

            if grouped_map[start_index, target_index]:
                result = result | self.grouped_indexes_by_index(target_index, grouped_map, data)
            else:
                continue

        return result


    def save(self, data):
        super().save(data)
        l = data.location()
        s = data.speed()
        super().save_for_jit([l.x, l.y, s.x, s.y, data.time])


    def __is_predictable_location(self, osd, nop):
        if nop.time <= osd.time:
            return nop.predicted_location(osd.time - nop.time).is_close_to(osd.location(), Constants.LOCATION_THRESHOLD)
        else:
            return osd.predicted_location(nop.time - osd.time).is_close_to(nop.location(), Constants.LOCATION_THRESHOLD)

    def __is_futher_than_all_the_new_perceived_objects_more_than_or_equal_to_sensor_tick(self, osd, new_perceived_objects):
        return reduce((lambda x,y: x and y), [self.__is_futher_than_sensor_tick(osd, nop) for nop in new_perceived_objects], True)

    def __is_predictable_location_compared_with_all_the_new_perceived_objects(self, osd, new_perceived_objects):
        return reduce((lambda x,y: x or y), [self.__is_predictable_location(osd, nop) for nop in new_perceived_objects if math.fabs(osd.time - nop.time) < Constants.SENSOR_TICK] , False)

    def __is_futher_than_sensor_tick(self, osd, nop):
        return (Constants.SENSOR_TICK <= math.fabs(osd.time - nop.time))



class RadarSensorData(ObstacleSensorData):
    pass

    # ----- deplicated codes -----
    # def detected_depth(self):
    #     return self.raw_data.depth
    #
    # def detected_velocity(self):
    #     return self.raw_data.velocity
    #
    #
    # def dx_from_sensor(self):
    #     location = self.location()
    #     sensor_location = self.sensor_location()
    #
    #     return location.x - sensor_location.x
    #
    #
    # def dy_from_sensor(self):
    #     location = self.location()
    #     sensor_location = self.sensor_location()
    #
    #     return location.y - sensor_location.y
    #
    #
    # def norm_vector_from_sensor(self):
    #     dx = self.dx_from_sensor()
    #     dy = self.dy_from_sensor()
    #     dist = math.sqrt(dx * dx + dy * dy)
    #
    #     return [dx / dist, dy / dist]
    #
    #
    # def sensor_location(self):
    #     return self.sensor_transform.location
    #
    # ----- deplicated codes, end. -----





class RadarSensorDataHandler(ObstacleSensorDataHandler):
    pass

    # ----- deplicated nodes -----
    # def perceived_objects(self, current_time, duration):
    #     """
    #     Since Radar data does not return object speed vector, we have to estimate the vector from raw_data.
    #     """
    #     print("data size, before")
    #     print(len(self.data))
    #
    #     # ----- validation -----
    #     ISOs = self.indexes_of_same_objectes()
    #
    #     # Estimate Speed Vector
    #     for indexes_of_the_object in ISOs:
    #         print("data size, after")
    #         print(len(self.data))
    #         cos_sin_alfa_list = np.array([self.data[index].norm_vector_from_sensor() for index in indexes_of_the_object])
    #         detected_velocities = np.array([[self.data[index].detected_velocity()] for index in indexes_of_the_object])
    #
    #         # if len(cos_sin_alfa_list) <= 1:
    #         #     pass
    #         # else:
    #
    #         # estimated_velocity = vx * cos_alfa + vy * sim_alfa.
    #         # A = [[ca1, sa1], [ca2, si2], ...], V = [ev1, ev2, ...].
    #         # In general, [vx, vy] * A = V.
    #         # Therefore, we estimate [vx, vy] as:
    #         # [vx, vy] = V * A^T * (A * A^T)^(-1)
    #
    #         for index in indexes_of_the_object:
    #             d = self.data[index]
    #             location = self.data[index].location()
    #             s_l = self.data[index].sensor_location()
    #             print(f"(x, y), v, d, (s_x, s_y) = ({location.x}, {location.y}), {d.detected_velocity()}, {d.detected_depth()}, ({s_l.x}, {s_l.y})")
    #
    #         print(cos_sin_alfa_list)
    #
    #         estimated_velocity = np.dot(
    #             np.linalg.pinv(np.dot(cos_sin_alfa_list.T, cos_sin_alfa_list)),
    #             np.dot(cos_sin_alfa_list.T, detected_velocities)
    #         )
    #         # This exception causes when we cannot create inverse metrixex of cos_sin_alfa_list.
    #         # Especially, thie error is causes when cos_sin_alfa_list includes only one kind of data.
    #         # In this case, we templora
    #
    #         print("estimated_velocity")
    #         print(estimated_velocity)
    #         print("")
    #
    #
    #     self.data.clear()
    #
    #     return []
    #
    #     # super().perceived_objects(current_time, duration)
    #
    #
    # def indexes_of_same_objectes(self):
    #     ISOs = [] # Indexes of Same Objectes
    #
    #     for compared_index in range(0, len(self.data)):
    #         is_grouped = False
    #
    #         for indexes_of_the_object in ISOs:
    #             for index_of_the_object in indexes_of_the_object:
    #                 if self.data[index_of_the_object].distance_from_other_location(self.data[compared_index].location()) <= Constants.LOCATION_THRESHOLD:
    #                     indexes_of_the_object.append(compared_index)
    #                     is_grouped = True
    #                     break
    #
    #                 else:
    #                     continue
    #
    #             if is_grouped:
    #                 break
    #
    #             else:
    #                 continue
    #
    #         if is_grouped:
    #             continue
    #
    #         else:
    #             ISOs.append([compared_index])
    #
    #     return ISOs
    #
    # ----- deplicated codes, end. -----
