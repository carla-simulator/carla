import socket
import json

if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect(('127.0.0.1', 9998))

        send_data = {"vehid": 1, "method": "veh_id"}

        s.sendall(bytes(json.dumps(send_data), "ascii"))
        recv_data = s.recv(1024)
        print(json.loads(str(recv_data, "ascii")))

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect(('127.0.0.1', 9998))

        send_data = {"vehid": 1, "method": "veh_info", "args": {}}
        send_data['args'] = {"aaa": "c" * 1024, "bbb": [[1,1,1], [2,2,2],[3,3,3]]}

        print(len(bytes(json.dumps(send_data), "ascii")))
        s.sendall(bytes(json.dumps(send_data), "ascii"))
        recv_data = s.recv(4096)
        print(json.loads(str(recv_data, "ascii")))
