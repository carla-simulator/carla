import math
import json
import os
import socket

from functools import reduce
from util.classes.perceived_objects import PerceivedObject
from util.classes.constants import (
    Constants,
)
from util.classes.utils import (
    Location,
    Speed,
)

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


    def __tmp_data(self):
        return {"tmp": "tmp"}


class Message:
    def dict_format(self):
        return vars(self)


class CAM(Message):
    def __init__(self, timestamp, ITS_PDU_Header={}, Basic_Container={}, HF_Container={}, LF_Container={}, Special_Vehicle_Container={}, option={}):
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
        self.option["type"] = "CAM"

# class CAMGenerateHandler:
#     def __init__(self, init_time, init_location, init_speed, init_yaw):
#         self.init_time = init_time
#         self.init_location = init_location
#         self.init_speed = init_speed
#         self.init_yaw = init_yaw
#
#
#     def generate(self, timestamp, current_location, current_speed, current_yaw):
#         self.init_time = timestamp
#         self.init_location = current_location
#         self.init_speed = current_speed
#         self.init_yaw = current_yaw
#
#         return CAM(timestamp, self.__tmp_data(), self.__tmp_data(), self.__tmp_data(), self.__tmp_data(), self.__tmp_data())
#
#
#     def is_ready(self, current_time, current_location, current_speed, current_yaw):
#         return True
#
#
#     def __get_delta(self, current_time, current_location, current_speed, current_yaw):
#         delta_t = current_time - self.init_time
#
#         delta_y = current_yaw - self.init_yaw
#
#         delta_s_x = current_speed.x - self.init_speed.x
#         delta_s_y = current_speed.y - self.init_speed.y
#         delta_s = math.sqrt(delta_s_x * delta_s_x + delta_s_y * delta_s_y)
#
#         delta_p_x = current_location.x - self.init_location.x
#         delta_p_y = current_location.y - self.init_location.y
#         delta_p = math.sqrt(delta_p_x * delta_p_x + delta_p_y * delta_p_y)
#
#         # print(f"{current_time}, {self.init_time}, {vars(current_location)}, {vars(self.init_location)}, {vars(current_speed)}, {vars(self.init_speed)}, {current_yaw}, {self.init_yaw}")
#         # print(f"delta_t: {delta_t}, delta_s: {delta_s}, delta_p: {delta_p}")
#         return delta_t, delta_s, delta_p, delta_y
#
#
#     def __tmp_data(self):
#         return {"tmp": "tmp"}


class CAMsHandler(MessagesHandler):
    def generate(self, current_time, current_location, current_speed, current_yaw):
        return CAM(
            timestamp=timestamp,
            ITS_PDU_Header=self.__tmp_data(),
            Basic_Container=self.__tmp_data(),
            HF_Container=self.__new_hf_container(current_location, current_speed, current_yaw),
            LF_Container=self.__tmp_data(),
            Special_Vehicle_Container=self.__tmp_data()
        )

    def is_generate(self, current_time, current_location, current_speed, current_yaw):
        pass

    def receive(self, sumo_id):
        # ----- file base -----
        data = super().receive(self.packet_data_file_path(sumo_id), self.packet_lock_file_path(sumo_id))
        dict_data = [json.loads(d) for d in data]
        self.received_messages = self.received_messages + [CAM(**d) for d in dict_data if d["option"]["type"] == "CAM"]


    def send(self, sumo_id, cam):
        # ----- file base -----
        self.reserved_messages = self.reserved_messages + [cam]
        super().send(self.sensor_data_file_path(sumo_id), self.sensor_lock_file_path(sumo_id), json.dumps(cam.dict_format()))


    def has_latest_cam(self):
        if len(self.reserved_messages) <= 0:
            return False
        else:
            return True

    def get_delta_by_latest_cam(self, current_time, current_location, current_speed, current_yaw):
        latest_cam = self.__latest_cam()

        hf = latest_cam.HF_Container

        delta_t = current_time - latest_cam.timestamp

        delta_s_x = current_speed.x - hf["speed"][0]
        delta_s_y = current_speed.y - hf["speed"][1]
        delta_s = math.sqrt(delta_s_x * delta_s_x + delta_s_y * delta_s_y)

        delta_p_x = current_location.x - hf["location"][0]
        delta_p_y = current_location.y - hf["location"][1]
        delta_p = math.sqrt(delta_p_x * delta_p_x + delta_p_y * delta_p_y)

        delta_y = current_yaw - hf["yaw"]

        return delta_t, delta_s, delta_p, delta_y

    def __latest_cam(self):
        return reduce(lambda c_a, c_b: c_b if c_a.timestamp <= c_b.timestamp else c_a, self.reserved_messages)

    def __new_hf_container(self, location, speed, yaw):
        return {
            "location": [location.x, location.y],
            "speed": [speed.x, speed.y],
            "yaw": yaw
        }

class CAMsHandlerWithNoSend(CAMsHandler):
    """
    This class is used when users do not want to send CAMs.
    """
    def is_generate(self, current_time, current_location, current_speed, current_yaw):
        return False

class CAMsHandlerWithEtsi(CAMsHandler):
    def is_generate(self, current_time, current_location, current_speed, current_yaw):
        if self.has_latest_cam():
            delta_t, delta_s, delta_p, delta_y = self.get_delta_by_latest_cam(current_time, current_location, current_speed, current_yaw)

            # ETSI Standard
            if Constants.CAM_DELTA_T_MIN <= delta_t:
                return Constants.CAM_DELTA_T_MAX <= delta_t or Constants.CAM_DELTA_S < delta_s or Constants.CAM_DELTA_P < delta_p or Constants.CAM_DELTA_Y < delta_y
            else:
                return False

        else:
            return True


class CAMsHandlerWithInterval(CAMsHandler):
    INTERVAL = 0.1

    def is_generate(self, current_time, current_location, current_speed, current_yaw):
        if self.has_latest_cam():
            delta_t, delta_s, delta_p, delta_y = self.get_delta_by_latest_cam(current_time, current_location, current_speed, current_yaw)

            return (CAMsHandlerWithInterval.INTERVAL <= delta_t)

        else:
            return True

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
        self.option["type"] = "CPM"


class CPMsHandler(MessagesHandler):
    def new_perceived_object_container(self, new_perceived_objects_with_pseudonym):
        pass


    def receive(self, sumo_id):
        # ----- file base -----
        data = super().receive(self.packet_data_file_path(sumo_id), self.packet_lock_file_path(sumo_id))
        dict_data = [json.loads(d) for d in data]
        self.received_messages = self.received_messages + [CPM(**d) for d in dict_data if d["option"]["type"] == "CPM"]


    def send(self, sumo_id, cpm):
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


class CPMsHandlerWithNoSend(CPMsHandler):
    """
    This class is used when users do not want to send CAMs.
    """
    def new_perceived_object_container(self, new_perceived_objects_with_pseudonym):
        return []


class CPMsHandlerWithEtsi(CPMsHandler):
    def new_perceived_object_container(self, new_perceived_objects_with_pseudonym):
        """
        This method will be called every SENSOR_TICK(= 0.1 sec).
        """

        detected_objects_for_new_CPM = []

        for the_latest_detected_object in new_perceived_objects_with_pseudonym:
            is_already_sent, delta_t, delta_s, delta_p = self.is_already_sent(the_latest_detected_object)

            if is_already_sent:
                # ETSI Standard
                if Constants.CPM_DELTA_T_MAX <= delta_t or Constants.CPM_DELTA_S < delta_s or Constants.CPM_DELTA_P < delta_p:
                    detected_objects_for_new_CPM.append(the_latest_detected_object)
                else:
                    pass

            else:
                detected_objects_for_new_CPM.append(the_latest_detected_object)

        return [obj.dict_format() for obj in detected_objects_for_new_CPM]


class CPMsHandlerWithInterval(CPMsHandler):
    def new_perceived_object_container(self, new_perceived_objects_with_pseudonym):
        """
        This method will be called every SENSOR_TICK(= 0.1 sec).
        """

        return [obj.dict_format() for obj in new_perceived_objects_with_pseudonym]
