import math

from collections import deque
from functools import reduce
from util.classes.constants import Constants
from util.classes.utils import Location

class dict2(dict):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__dict__ = self

class PerceivedObject:
    def __init__(self, time, location, speed, pseudonym=None):
        self.time = time
        self.location = dict2({
            "x": location.x,
            "y": location.y
        })
        self.speed = dict2({
            "x": speed.x,
            "y": speed.y,
            "z": speed.z
        })
        self.pseudonym = pseudonym



    def predicted_location(self, delta_t):
        return Location(self.location.x + delta_t * self.speed.x, self.location.y + delta_t * self.speed.y)


    def is_predictable_location(self, other):
        if self.time <= other.time:
            return (self.predicted_location(other.time - self.time).is_close_to(other.location, Constants.LOCATION_THRESHOLD))
        else:
            return (other.predicted_location(self.time - other.time).is_close_to(self.location, Constants.LOCATION_THRESHOLD))

    def dict_format(self):
        return vars(self)

class PerceivedObjectList:
    def __init__(self):
        self.list = []


    def is_the_same_object(self, new_obj):
        if len(self.list) <= 0:
            return True
        else:
            return new_obj.is_predictable_location(self.__time_near_obj(new_obj))


    def latest(self):
        return reduce((lambda x, y: x if y.time <= x.time else y), self.list, self.list[0])


    def save(self, new_obj):
        self.list.append(new_obj)


    def __time_near_obj(self, new_obj):
        return reduce((lambda x, y: x if math.fabs(x.time - new_obj.time) <= math.fabs(y.time - new_obj.time) else y), self.list, self.list[0])


class PerceivedObjectsHandler:
    def __init__(self):
        self.pseudonym2objects = {}


    def save(self, obj):
        obj.pseudonym = self.pseudonym(obj)
        self.pseudonym2objects[obj.pseudonym].save(obj)

    def remove_unused_objects(self, current_time, duration):
        unused_psues = [pseu for pseu, objects in self.pseudonym2objects.items() if max([obj.time for obj in objects.list]) + duration < current_time]

        for unsused_psue in unused_psues:
            self.pseudonym2objects.pop(unsused_psue)


    def pseudonym(self, new_obj):
        for pseu in self.pseudonym2objects.keys():
            if self.pseudonym2objects[pseu].is_the_same_object(new_obj):
                return pseu
            else:
                continue

        return self.__new_pseudonym()


    def __new_pseudonym(self):
        if len(self.pseudonym2objects.keys()) <= 0:
            new_pseudonym = 0
        else:
            new_pseudonym = max([int(k) for k in self.pseudonym2objects.keys()]) + 1

        self.pseudonym2objects[new_pseudonym] = PerceivedObjectList()
        return new_pseudonym
