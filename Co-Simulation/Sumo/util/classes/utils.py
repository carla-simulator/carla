import math

class Location:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def is_close_to(self, other_location, threshold):
        diff_x = (self.x - other_location.x)
        diff_y = (self.y - other_location.y)

        return (math.sqrt(diff_x * diff_x + diff_y * diff_y) <= threshold)

class Speed:
    def __init__(self, x, y, z=0):
        self.x = x
        self.y = y
        self.z = z

class VehicleData:
    """
    This class is essential for obtaining veihicle data such as location and speed of vehicles.
    In Co-simulation of Carla and SUMO, we cannot obtain vehicle speeds from Carla.
    Specifically, Carla always returns [0, 0, 0] as vehicle speed.
    """

    def __init__(self, time, actor):
        self.data = []
        self.tick(time, actor)

    def latest(self):
        if len(self.data) <= 0:
            return None
        else:
            return self.data[-1]

    def tick(self, time, actor):
        al = actor.get_transform().location
        yaw = actor.get_transform().rotation.yaw

        if len(self.data) <= 0:
            self.data.append(self.formatted_data(time, Location(al.x, al.y), Speed(0, 0), yaw))
        else:
            dT = time - self.data[-1]["time"]
            dX = al.x - self.data[-1]["location"].x
            dY = al.y - self.data[-1]["location"].y

            self.data.append(self.formatted_data(time, Location(al.x, al.y), Speed(dX / dT, dY / dT), yaw))

    def formatted_data(self, time, location, speed, yaw):
        return {"time": time, "location": location, "speed": speed, "yaw": yaw}

def location(actor, distance):
    x = actor.get_transform().location.x + distance * math.cos(math.radians(actor.get_transform().rotation.yaw))
    y = actor.get_transform().location.y + distance * math.sin(math.radians(actor.get_transform().rotation.yaw))

    return Location(x, y)

def speed(actor, abs_speed):
    x = abs_speed * math.cos(math.radians(actor.get_transform().rotation.yaw))
    y = abs_speed * math.sin(math.radians(actor.get_transform().rotation.yaw))

    return Speed(x, y)
