import math
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
        self.raw_sensor_data = data

        # We use the three data.
        self.time = time
        self.l = location
        self.s = speed


    def location(self):
        return self.l


    def predicted_location(self, delta_t):
        speed = self.speed()
        location = location()

        return Location(location.x + delta_t * speed.x, location.y + delta_t * speed.y)


    def speed(self):
        return self.s


class ObstacleSensorDataHandler(SensorDataHandler):

    def perceived_objects(self, current_time, duration):
        new_perceived_objects = []


        while self.data:
            osd = self.data.popleft()   # obstacle_sensor_data (osd)

            # validation
            if osd.time + duration < current_time:
                continue

            if self.__is_futher_than_all_the_new_perceived_objects_more_than_or_equal_to_sensor_tick(osd, new_perceived_objects):
                new_perceived_objects.append(PerceivedObject(time=osd.time, location=osd.location(), speed=osd.speed()))

            elif not self.__is_predictable_location_compared_with_all_the_new_perceived_objects(osd, new_perceived_objects):
                new_perceived_objects.append(PerceivedObject(time=osd.time, location=osd.location(), speed=osd.speed()))

        self.clear_data()

        return new_perceived_objects

    def clear_data(self):
        self.data.clear()


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



class RaderSensorData(ObstacleSensorData):
    """
    In curent phases, we do not have needs to implement new features for rader sensors.
    Therefore, we only inheritance the ObstacleSensorData
    """
    pass


class RaderSensorDataHandler(ObstacleSensorDataHandler):
    """
    In curent phases, we do not have needs to implement new features for rader sensors.
    Therefore, we only inheritance the ObstacleSensorDataHandler
    """
    pass
