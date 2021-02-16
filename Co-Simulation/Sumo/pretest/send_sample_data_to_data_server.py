import socket
import json

if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect(('127.0.0.1', 9998))

        send_data = {"vehid": 1, "method": "veh_id", "args": {}}

        s.sendall(bytes(json.dumps(send_data), "ascii"))
        recv_data = s.recv(1024)
        print(json.loads(str(recv_data, "ascii")))

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect(('127.0.0.1', 9998))

        send_data = {"vehid": 1, "method": "veh_id", "args": {}}
        send_data['args'] = {"aaa": "ccc", "bbb": "ddd"}
        
        s.sendall(bytes(json.dumps(send_data), "ascii"))
        recv_data = s.recv(1024)
        print(str(recv_data, "ascii"))
        print(json.loads(str(recv_data, "ascii")))
