class Vehicle:
    def __init__(self, vehid):
        self.vehid = vehid
        self.obstacle_data = []

    def veh_id(self):
        return self.vehid

    def veh_info(self, aaa, bbb):
        return {"id": self.vehid, "aaa": aaa, "bbb": bbb}

    def save_obstacle_data(self, data):
        self.obstacle_data.append(data)

    def obstacle_data(self):
        return self.obstacle_data
