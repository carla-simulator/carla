import argparse
import logging
import threading
import socket
import socketserver
import sys
import json

from copy import deepcopy


PACKETS = {}
SENSOR_DATA = {}

class ThreadedTCPRequestHandler(socketserver.BaseRequestHandler):

    def handle(self):
        try:
            received_data = json.loads(str(self.request.recv(1024), 'ascii'))
            status, response_data = self.__response_handler(received_data)
            print(response_data)
            self.request.sendall(self.__formated_response(status, response_data))

        except Exception as e:
            logging.error(e)
            self.request.sendall(self.__formated_response(500, {}))


    def __formated_response(self, status=200, response_data={}):
        resp_d = {
            "status": status,
            "data": response_data
        }

        return bytes(json.dumps(resp_d), 'ascii')

    def __response_handler(self, recv_data={}):
        try:
            print(recv_data["method"])
            if False:
                return 500, {}

            elif recv_data["method"] == "get_sensor_data":
                return 200, self.__get_sensor_data(recv_data["veh_id"])

            elif recv_data["method"] == "get_packets":
                return 200, self.__get_packets(recv_data["veh_id"])

            elif recv_data["method"] == "post_packets":
                return 200, self.__post_packets(recv_data["veh_id"], recv_data["packets"])

            elif recv_data["method"] == "post_sensor_data":
                return 200, self.__post_sensor_data(recv_data["veh_id"], recv_data["sensor_data"])

            else:
                return 500, {}

        except Exception as e:
            logging.error(e)
            return 500, {}

    def __get_sensor_data(self, veh_id):
        global SENSOR_DATA

        if str(veh_id) not in SENSOR_DATA:
            SENSOR_DATA[str(veh_id)] = []

        resp_data = SENSOR_DATA[str(veh_id)]
        SENSOR_DATA[str(veh_id)] = []

        return resp_data

    def __get_packets(self, veh_id):
        print("!")
        global PACKETS

        if str(veh_id) not in PACKETS:
            PACKETS[str(veh_id)] = []

        resp_data = PACKETS[str(veh_id)]
        PACKETS[str(veh_id)] = []

        return resp_data

    def __post_sensor_data(self, veh_id, sensor_data):
        global SENSOR_DATA

        if str(veh_id) not in SENSOR_DATA:
            SENSOR_DATA[str(veh_id)] = []

        SENSOR_DATA[str(veh_id)].append(sensor_data)

        return {}

    def __post_packets(self, veh_id, packets):
        global PACKETS

        if str(veh_id) not in PACKETS:
            PACKETS[str(veh_id)] = []

        PACKETS[str(veh_id)].append(packets)
        print(PACKETS)

        return {}


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    pass


if __name__ == "__main__":
    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for carla and veins data synchronization.')
    # Port 0 means to select an arbitrary unused port
    parser.add_argument('--port', type=int, default=9998)
    parser.add_argument('--host', default='localhost')
    parser.add_argument('--log_file_path', default="./log/data_server_logger.log")
    args = parser.parse_args()

    # ----- set logging -----
    logging.basicConfig(
        handlers=[logging.FileHandler(filename=args.log_file_path), logging.StreamHandler(sys.stdout)],
        format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s: %(message)s',
        level=logging.DEBUG
    )

    # ----- start server -----
    server = ThreadedTCPServer((args.host, args.port), ThreadedTCPRequestHandler)
    with server:
        ip, port = server.server_address

        # Start a thread with the server -- that thread will then start one
        # more thread for each request
        server_thread = threading.Thread(target=server.serve_forever)
        # Exit the server thread when the main thread terminates
        server_thread.daemon = True
        server_thread.start()
        server_thread.join()
        server.shutdown()
