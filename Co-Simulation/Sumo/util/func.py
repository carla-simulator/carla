import json
import os
import psutil
import signal
import socket
import shutil

from functools import reduce
from subprocess import Popen

def check_port_listens(checked_port):
    return (checked_port in [conn.laddr.port for conn in psutil.net_connections() if conn.status == 'LISTEN'])

def check_port_listens_by_port_scan(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    return_code = sock.connect_ex((host, port))
    sock.close()

    if return_code == 0:
        return True
    else:
        return False

def copy_local_file_to_vagrant(local_file_path, copied_file_name, target_vagrant_dir, shared_dir):
    try:
        shutil.copyfile(local_file_path, f"{shared_dir}/{copied_file_name}")
        p = Popen(f"vagrant scp {copied_file_name} :{target_vagrant_dir}/{copied_file_name} > /dev/null 2>&1", cwd=shared_dir, shell=True)
        p.wait()

    except Exception as e:
        print(e)
        os.remove(f"{shared_dir}/{copied_file_name}")
        raise e

    finally:
        os.remove(f"{shared_dir}/{copied_file_name}")


def data_from_json(json_file_path):
    with open(json_file_path) as f:
        data = json.load(f)

    return data


def host_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('10.255.255.255', 1))
        ip = s.getsockname()[0]
    except Exception:
        ip = '127.0.0.1'
    finally:
        s.close()

    return ip
