import math
import json
import socket

from util.classes.perceived_objects import PerceivedObject

class MessagesHandler:
    def __init__(self, data_server_host, data_server_port):
        self.data_server_host = data_server_host
        self.data_server_port = data_server_port

        self.received_messages = []
        self.reserved_messages = []

    def access_data_server(self, sumo_vehid, method_name, args=None):
        """
        This mathod make the data server call method specified by the method_name

        If the send_data is {"vehid": 1, "method_name": "reserved_CPMs", args: {"data": [some CPM messages]}},
        this method call Vehicle(vehid=1).reserved_CPMs(data=[some CPM messages]) in the data server.
        """
        received_data = b''

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((str(self.data_server_host), int(self.data_server_port)))
            send_data = {
                "vehid": str(sumo_vehid),
                "method": str(method_name),
            }

            if args is not None and type(args) is dict:
                send_data["args"] = args

            s.sendall(bytes(json.dumps(send_data), "ascii"))
            received_data = self.__read_all_bytes(s)

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

class CAM:
    def __init__(self):
        pass


class CAMsHandler(MessagesHandler):
    pass


class CPM:
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

    def dict_format(self):
        return vars(self)

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
        resp = json.loads(str(self.access_data_server(sumo_id, "get_received_CPMs"), 'ascii'))
        data = resp["data"]

        self.received_messages = self.received_messages + [CPM(**d) for d in data]

    def send(self, sumo_id, CPMs_list):
        resp = json.loads(str(self.access_data_server(sumo_id, "post_reserved_CPMs", {"data": [cpm.dict_format() for cpm in CPMs_list]}), 'ascii'))
        data = resp["data"]

        self.reserved_messages = self.reserved_messages + CPMs_list

    def is_already_sent(self, detected_object):
        is_already_sent = False
        delta_t = None
        delta_s = None
        delta_p = None

        for reserved_message in self.reserved_messages:
            for sent_perceived_object in reserved_message.perceived_objects():
                if str(detected_object.pseudonym) == str(sent_perceived_object.pseudonym):
                    is_already_sent = True

                    delta_t = detected_object.time - sent_perceived_object.time

                    delta_s_x = detected_object.speed.x - sent_perceived_object.speed.x
                    delta_s_y = detected_object.speed.x - sent_perceived_object.speed.x
                    delta_s_z = detected_object.speed.x - sent_perceived_object.speed.x
                    delta_s = math.sqrt(delta_s_x * delta_s_x + delta_s_y * delta_s_y * delta_s_z * delta_s_z)

                    delta_p_x = detected_object.location.x - detected_object.location.x
                    delta_p_y = detected_object.location.x - detected_object.location.x
                    delta_p = math.sqrt(delta_p_x * delta_p_x + delta_p_y * delta_p_y)

                if is_already_sent:
                    break

            if is_already_sent:
                break

        return is_already_sent, delta_t, delta_s, delta_p
