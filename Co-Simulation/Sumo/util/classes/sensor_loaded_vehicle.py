class Vehicle:
    def __init__(self, vehid):
        self.vehid = vehid
        # self.obstacle_data = {}
        self.obstacle_data = []

    def veh_id(self):
        return self.vehid

    def veh_info(self, aaa, bbb):
        return {"id": self.vehid, "aaa": aaa, "bbb": bbb}

    def save_obstacle_data(self, data):
        # if str(listend_time) not in self.obstacle_data.keys():
        #     self.obstacle_data[str(listend_time)] = []
        self.obstacle_data.append(data)
        print(self.obstacle_data)
