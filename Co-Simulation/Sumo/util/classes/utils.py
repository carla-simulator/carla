import math

class Location:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def is_close_to(self, other_location, threshold):
        diff_x = (self.x - other_location.x)
        diff_y = (self.y - other_location.y)

        return (math.sqrt(diff_x * diff_x + diff_y * diff_y) <= threshold)
