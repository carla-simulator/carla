import math
import json
import os
import socket

from functools import reduce
from util.classes.perceived_objects import PerceivedObject

def unlock(dst):
    os.unlink(dst)


def make_symlink(src, dst):
    try:
        os.symlink(src, dst)
        return True
    except Exception as e:
        # print(e)
        return False

def lock(src, dst):
    while make_symlink(src, dst) is False:
        continue



class MessagesHandler:
    def __init__(self, data_server_host, data_server_port, data_sync_dir):
        self.data_server_host = data_server_host
        self.data_server_port = data_server_port
        self.data_sync_dir = data_sync_dir

        self.received_messages = []
        self.reserved_messages = []

    def sensor_data_file_path(self, veh_id):
        return self.data_sync_dir + f"{veh_id}_sensor.json"

    def sensor_lock_file_path(self, veh_id):
        return self.data_sync_dir + f"{veh_id}_sensor.json.lock"

    def packet_data_file_path(self, veh_id):
        return self.data_sync_dir + f"{veh_id}_packet.json"

    def packet_lock_file_path(self, veh_id):
        return self.data_sync_dir + f"{veh_id}_packet.json.lock"


    def receive(self, data_file, lock_file):
        data = []
        # ----- file base -----
        lock(data_file, lock_file)
        try:
            with open(data_file) as f:
                data = f.readlines()
        except:
            pass

        with open(data_file, mode="w") as f:
            f.write("")

        unlock(lock_file)

        return data

    def send(self, data_file, lock_file, data):
        # ----- file base -----
        lock(data_file, lock_file)
        with open(data_file, mode="a") as f:
            f.write(data + "\n")
        unlock(lock_file)

    def access_data_server(self, sumo_vehid, method_name, args=None):
        """
        This mathod make the data server call method specified by the method_name

        If the send_data is {"vehid": 1, "method_name": "reserved_CPMs", args: {"data": [some CPM messages]}},
        this method call Vehicle(vehid=1).reserved_CPMs(data=[some CPM messages]) in the data server.
        """
        received_data = b''

        # with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        #     s.connect((str(self.data_server_host), int(self.data_server_port)))
        #     send_data = {
        #         "vehid": str(sumo_vehid),
        #         "method": str(method_name),
        #     }
        #
        #     if args is not None and type(args) is dict:
        #         send_data["args"] = args
        #
        #     s.sendall(bytes(json.dumps(send_data), "ascii"))
        #     received_data = self.__read_all_bytes(s)


        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((str(self.data_server_host), int(self.data_server_port)))
        send_data = {
            "vehid": str(sumo_vehid),
            "method": str(method_name),
        }

        if args is not None and type(args) is dict:
            send_data["args"] = args

        s.sendall(bytes(json.dumps(send_data), "ascii"))
        received_data = self.__read_all_bytes(s)
        s.close()

        return received_data


    def __read_all_bytes(self, socket, buffsize=65536):
        received_data = b''

        while True:
            data = socket.recv(buffsize)
            received_data = received_data + data

            if len(data) < buffsize:
                break
            else:
                try:
                    eval(str(received_data), "ascii")
                    break
                except Exception:
                    continue

        return received_data


class Message:
    def dict_format(self):
        return vars(self)


class CAM(Message):
    def __init__(self, timestamp, ITS_PDU_Header={}, Basic_Container={}, HF_Container={}, LF_Container={}, Special_Vehicle_Container={}, option):
        # Easy implementation of a Cooprative Perception Message (CAM).
        # If you want to use detail information of CPM, you should include the information in CAM.
        self.timestamp = timestamp
        self.ITS_PDU_Header = ITS_PDU_Header
        self.Basic_Container = Basic_Container
        self.HF_Container = HF_Container
        self.LF_Container = LF_Container
        self.Special_Vehicle_Container = Special_Vehicle_Container

        # For convenience, we add optional data like payload size
        self.option = option
        self.update_option()


    def update_option(self):
        """
        We fix the CAM size to 196 Byte.
        """

        self.option["size"] = 190

class CAMGenerateHandler:
    def __init__(self, init_time, init_location, init_speed):
        self.init_location = init_location
        self.init_speed = speed

    def is_ready(self, current_time, current_location, current_speed):
        pass

class CAMsHandler(MessagesHandler):
    pass


class CPM(Message):
    MAX_SIZE = 800

    def __init__(self, timestamp, ITS_PDU_Header={}, Management_Container={}, Station_Data_Container={}, Sensor_Information_Container=[], Perceived_Object_Container=[], option={}):
        # Easy implementation of a Cooprative Perception Message (CPM).
        # If you want to use detail information of CPM, you should include the information in CPM.
        self.timestamp = timestamp
        self.ITS_PDU_Header = ITS_PDU_Header
        self.Management_Container = Management_Container
        self.Station_Data_Container = Station_Data_Container
        self.Sensor_Information_Container = Sensor_Information_Container
        self.Perceived_Object_Container = Perceived_Object_Container

        # For convenience, we add optional data like payload size
        self.option = option
        self.update_option()


    def perceived_objects(self):
        return [PerceivedObject(**po) for po in self.Perceived_Object_Container]


    def update_option(self):
        """
        ITS_PDU_Header + Management_Container + Station_Data_Container: 121 (Byte)
        Sensor_Information_Container:                                   35 (Byte/data)
        Perceived_Object_Container:                                     35 (Byte/data)
        cite from:                                                      Thandavarayan, G., Sepulcre, M., & Gozalvez, J. (2020). Cooperative Perception for Connected and Automated Vehicles: Evaluation and Impact of Congestion Control. IEEE Access, 8, 197665–197683. https://doi.org/10.1109/access.2020.3035119
        """

        self.option["size"] = 121 + 35 * len(self.Sensor_Information_Container) + 35 * len(self.Perceived_Object_Container)


class CPMsHandler(MessagesHandler):
    def receive(self, sumo_id):
        # ----- socket base -----
        # resp = json.loads(str(self.access_data_server(sumo_id, "get_received_CPMs"), 'ascii'))
        # data = resp["data"]
        #
        # self.received_messages = self.received_messages + [CPM(**d) for d in data]

        # ----- file base -----
        data = super().receive(self.packet_data_file_path(sumo_id), self.packet_lock_file_path(sumo_id))
        dict_data = [json.loads(d) for d in data]
        self.received_messages = self.received_messages + [CPM(**d) for d in dict_data]

    def send(self, sumo_id, cpm):
        # ----- socket base -----
        # resp = json.loads(str(self.access_data_server(sumo_id, "post_reserved_CPMs", {"data": [cpm.dict_format() for cpm in CPMs_list]}), 'ascii'))
        # data = resp["data"]
        #
        # self.reserved_messages = self.reserved_messages + CPMs_list

        # ----- file base -----
        self.reserved_messages = self.reserved_messages + [cpm]
        super().send(self.sensor_data_file_path(sumo_id), self.sensor_lock_file_path(sumo_id), json.dumps(cpm.dict_format()))

    def similar_reserved_perceived_object(self, new_time, new_pseudonym):
        srpo = None
        diff_time = float('inf')

        for reserved_message in self.reserved_messages:
            for reserved_perceived_object in reserved_message.perceived_objects():
                if str(new_pseudonym) == str(reserved_perceived_object.pseudonym) and math.fabs(new_time - reserved_perceived_object.time) < diff_time:
                    srpo = reserved_perceived_object
                    diff_time = math.fabs(new_time - reserved_perceived_object.time)
                else:
                    continue

        return srpo


    def is_already_sent(self, detected_object):
        is_already_sent = False
        delta_t = 0
        delta_s = 0
        delta_p = 0

        sent_perceived_object = self.similar_reserved_perceived_object(detected_object.time, detected_object.pseudonym)

        if sent_perceived_object is not None:
            is_already_sent = True

            delta_t = detected_object.time - sent_perceived_object.time

            delta_s_x = detected_object.speed.x - sent_perceived_object.speed.x
            delta_s_y = detected_object.speed.y - sent_perceived_object.speed.y
            delta_s = math.sqrt(delta_s_x * delta_s_x + delta_s_y * delta_s_y)

            delta_p_x = detected_object.location.x - detected_object.location.x
            delta_p_y = detected_object.location.y - detected_object.location.y
            delta_p = math.sqrt(delta_p_x * delta_p_x + delta_p_y * delta_p_y)

        return is_already_sent, delta_t, delta_s, delta_p
