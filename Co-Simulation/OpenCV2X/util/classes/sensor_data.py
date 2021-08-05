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


class SensorDataHandler:
    def __init__(self):
        self.data = deque()
        # self.data_for_jit = []


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

    # def save_for_jit(self, data):
    #     self.data_for_jit.append(data)


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

    # def perceived_objects_by_jit(self, current_time, duration):
    #     new_perceived_objects = []
    #
    #     if len(self.data_for_jit) <= 0:
    #         pass
    #     else:
    #         for p in perceived_objects_by_jit(self.data_for_jit, current_time, duration, Constants.LOCATION_THRESHOLD):
    #             new_perceived_objects.append(
    #                 PerceivedObject(
    #                     time=p[4],
    #                     location=Location(p[0], p[1]),
    #                     speed=Speed(p[2], p[3])
    #                 )
    #             )
    #
    #     return new_perceived_objects


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

        # print(f"new_perceived_objects, t1: {t1 - start}, t2: {t2 - t1}, t3: {t3 - t2}")

        return new_perceived_objects


    def new_perceived_objects_stdout(self, current_time, duration):
        return print(self.new_perceived_objects(current_time, duration))

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
        # l = data.location()
        # s = data.speed()
        # super().save_for_jit([l.x, l.y, s.x, s.y, data.time])


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


class RadarSensorDataHandler(ObstacleSensorDataHandler):
    pass
