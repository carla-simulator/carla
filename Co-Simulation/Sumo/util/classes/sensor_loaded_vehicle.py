from copy import deepcopy

class Vehicle:
    def __init__(self, vehid):
        self.__vehid = vehid
        self.__received_CAMs = []
        self.__received_CPMs = []
        self.__reserved_CAMs = []
        self.__reserved_CPMs = []

    def veh_id(self):
        return self.__vehid

    def veh_info(self, aaa, bbb):
        return {"id": self.__vehid, "aaa": aaa, "bbb": bbb}

    def vehid(self):
        return self.__vehid

    # self.__received_CAMs
    def get_received_CAMs(self):
        result = deepcopy(self.__received_CAMs)
        self.__received_CAMs = []

        return result

    def post_received_CAMs(self, data):
        self.__received_CAMs = self.__received_CAMs + data


    # self.__received_CPMs
    def get_received_CPMs(self):
        result = deepcopy(self.__received_CPMs)
        self.__received_CPMs = []

        return result

    def post_received_CPMs(self, data):
        self.__received_CPMs = self.__received_CPMs + data


    # self.__reserved_CAMs
    def get_reserved_CAMs(self):
        result = deepcopy(self.__reserved_CAMs)
        self.__reserved_CAMs = []

        return result

    def post_reserved_CAMs(self, data):
        self.__received_CAMs = self.__received_CAMs + data


    # self.__reserved_CPMs
    def get_reserved_CPMs(self):
        result = deepcopy(self.__reserved_CPMs)
        self.__reserved_CPMs = []

        return result

    def post_reserved_CPMs(self, data):
        self.__reserved_CPMs = self.__reserved_CPMs + data
