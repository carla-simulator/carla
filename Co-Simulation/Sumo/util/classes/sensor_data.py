import math
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


class ObstacleSensorData:
    """
    This class is the ease implimantation of radar sensor data.
    We assume that we can get obstacles' speeds and distances [1].
    The distances are used to calculate the obstacles' locations.

    [1] cite: https://carla.readthedocs.io/en/latest/ref_sensors/
    """
    def __init__(self, data, time, location, speed):
        self.raw_data = data

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

    def perceived_objects(self, current_time, duration):
        new_perceived_objects = []
        data = self.data
        self.data = []

        for indexes in self.get_grouped_indexes(current_time, duration, data):
            latest_time = 0
            latest_data = None
            total_x_pos = 0
            total_y_pos = 0
            
            for index in indexes:
                d = data[index]

                if latest_time < d.time:
                    latest_time = d.time
                    latest_data = d

                total_x_pos = total_x_pos + d.location().x
                total_y_pos = total_y_pos + d.location().y

            if latest_time + duration < current_time:
                continue

            else:
                new_perceived_objects.append(
                    PerceivedObject(
                        time=latest_time,
                        location=Location(total_x_pos / len(indexes), total_y_pos / len(indexes)),
                        speed=latest_data.speed()
                    )
                )


        # ----- deplicated codes -----
        # while self.data:
        #     osd = self.data.popleft()   # obstacle_sensor_data (osd)
        #
        #     # validation
        #     if osd.time + duration < current_time:
        #         continue
        #
        #     if self.__is_futher_than_all_the_new_perceived_objects_more_than_or_equal_to_sensor_tick(osd, new_perceived_objects):
        #         new_perceived_objects.append(PerceivedObject(time=osd.time, location=osd.location(), speed=osd.speed()))
        #
        #     elif not self.__is_predictable_location_compared_with_all_the_new_perceived_objects(osd, new_perceived_objects):
        #         new_perceived_objects.append(PerceivedObject(time=osd.time, location=osd.location(), speed=osd.speed()))
        #
        # ----- deplicated codes, end -----


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
    def __init__(self, data, time, location, speed, sensor_transform):
        self.sensor_transform = sensor_transform

        super().__init__(data, time, location, speed)

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
