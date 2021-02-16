class Vehicle:
    def __init__(self, vehid):
        self.vehid = vehid

    def veh_id(self):
        return self.vehid

    def veh_info(self, aaa, bbb):
        return {"id": self.vehid, "aaa": aaa, "bbb": bbb}
