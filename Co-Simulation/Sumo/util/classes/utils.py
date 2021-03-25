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
