import argparse
import logging
import threading
import socket
import socketserver
import sys
import json

from copy import deepcopy

from util.classes.sensor_loaded_vehicle import (
    Vehicle,
)

VEHICLES = {}
PACKETS = {}
SENSOR_DATA = {}

class ThreadedTCPRequestHandler(socketserver.BaseRequestHandler):
    def handle(self):
        try:
            received_data = json.loads(str(self.__read_all_bytes(), 'ascii'))
            status, response_data = self.__response_handler(received_data)
            self.request.sendall(self.__formated_response(status, response_data))

        except Exception as e:
            logging.error(e)
            self.request.sendall(self.__formated_response(500, {}))

    def __args_from_dict(self, generated_args="", dict_args={}):
        if 0 < len(dict_args):
            item = dict_args.popitem()
            args = item[0]
            val = item[1] if type(item[1]) is not str else f"\"{item[1]}\""

            return self.__args_from_dict(f"{generated_args}{args}={val}{', ' if 0 < len(dict_args) else ''}", deepcopy(dict_args))
        else:
            return generated_args

    def __check_received_data_format(self, received_data):
        try:
            if "vehid" not in received_data.keys():
                return False
            if "method" not in received_data.keys():
                return False

            return True

        except Exception as e:
            logging.error(e)
            return False

    def __formated_response(self, status=200, response_data={}):
        resp_d = {
            "status": status,
            "data": response_data
        }

        return bytes(json.dumps(resp_d), 'ascii')

    def __get_vehicle_by_id(self, id):
        global VEHICLES

        if str(id) not in VEHICLES.keys():
            VEHICLES[str(id)] = Vehicle(id)

        return VEHICLES[str(id)]

    def __read_all_bytes(self, buffsize=65536):
        received_data = b''

        while True:
            data = self.request.recv(buffsize)
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

    def __response_handler(self, recv_data={}):
        try:
            if self.__check_received_data_format(recv_data) is False:
                return 500, {}

            else:
                vehicle = self.__get_vehicle_by_id(recv_data["vehid"])
                dict_args = {} if "args" not in recv_data.keys() else recv_data['args']

                return 200, eval(f"vehicle.{str(recv_data['method'])}({self.__args_from_dict('', deepcopy(dict_args))})")

        except Exception as e:
            logging.error(e)
            return 500, {}


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
