import argparse
import logging
import multiprocessing
import os
import psutil
import signal
import socket
import subprocess
import sys
import time

from subprocess import Popen
from multiprocessing import Process

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import traci
else:
    sys.exit("Please declare environment variable 'SUMO_HOME'")

# ----- GLOBAL_VARS -----
CTRL_C_PRESSED_MESSAGE = "ctrl-c is pressed."
SECONDS_OF_ONE_DAY = 24 * 60 * 60


# ----- functions -----
def check_port_listens(checked_port):
    return (checked_port in [conn.laddr.port for conn in psutil.net_connections() if conn.status == 'LISTEN'])

def host_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

def kill_processes(procs):
    try:
        if 0 < len(procs):
            os.killpg(os.getpgid(procs[0].pid), signal.SIGTERM)
            procs.pop(0)
            return kill_processes(procs)
        else:
            return procs

    except Exception as e:
        logging.error(e)
        logging.error(f"Pids: {[proc.pid for proc in procs]} are remained.")
        return procs

def main(args):
    procs = []

    try:
        # ----- start sumo servers -----
        logging.info(f"For Carla, Sumo reads {args.sumocfg_path} and starts on {args.carla_sumo_port} port.")
        carla_sumo_proc = Popen(f"{args.sumocmd} -c {args.sumocfg_path} --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.sumo_step_length} --remote-port {args.carla_sumo_port} --num-clients 2 > /dev/null 2>&1", shell=True)
        procs.append(carla_sumo_proc)

        logging.info(f"For Veins, Sumo reads {args.sumocfg_path} and starts on {args.veins_sumo_port} port.")
        veins_sumo_proc = Popen(f"{args.sumocmd} -c {args.sumocfg_path} --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.sumo_step_length} --remote-port {args.veins_sumo_port} --num-clients 2 > /dev/null 2>&1", shell=True)
        procs.append(veins_sumo_proc)

        logging.info(f"Waiting until Sumo servers start on {args.carla_sumo_port} and {args.veins_sumo_port} ports.")
        while True:
            if check_port_listens(args.carla_sumo_port) and check_port_listens(args.veins_sumo_port):
                break

        # ----- start carla-sumo synchronizer -----
        logging.info(f"Checking that Carla server is started on {args.carla_unrealengine_port} port.")
        if check_port_listens(args.carla_unrealengine_port) is False:
            raise Exception("Carla server is not standed.")

        logging.info(f"Changing the Carla map to {args.carla_map_name}.")
        p = Popen(f"python config.py -p {args.carla_unrealengine_port} -m {args.carla_map_name}", cwd=args.python_api_util_path, shell=True)
        p.wait()

        logging.info("Running Carla-Sumo synchronizer.")
        carla_sumo_synchronizer_proc = Popen(f"python run_synchronization.py {args.sumocfg_path} --carla-port {args.carla_unrealengine_port} --sumo-port {args.carla_sumo_port} --sumo-host {args.sumo_host}", shell=True)
        procs.append(carla_sumo_synchronizer_proc)

        # ----- start veins-sumo synchronizer -----
        logging.info("Running Veins-Sumo synchronizer.")
        veins_sumo_synchronizer_proc = Popen(f"vagrant ssh -c \"python /vagrant/my-sumo-launched.py --sh {host_ip()} --sp {args.veins_sumo_port}\"", cwd=args.veins_vagrant_path, shell=True)
        procs.append(veins_sumo_synchronizer_proc)

        # ----- start carla-veins data synchronizer -----
        logging.info("Running Carla-Veins data synchronizer.")
        data_server_proc = Popen(f"python carla_veins_data_server --host {args.data_api_host} --port {args.data_api_port} > /dev/null 2>&1", shell=True)
        # data_server_proc = Popen(f"uvicorn --host {args.data_api_host} --port {args.data_api_port} api_server_for_carla_veins_data:app --reload > /dev/null 2>&1", shell=True)
        procs.append(data_server_proc)

        # ----- start two-traci synchronizer -----
        logging.info("Connecting Carla-Sumo and Veins-Sumo.")
        logging.info("Please start your Veins manually.")
        if args.main_mobility_handler == "carla":
            tracis_syncronizer_proc = Popen(f"python run_tracis_synchronization.py --main_sumo_port {args.carla_sumo_port} --other_sumo_ports {args.veins_sumo_port} --sumo_order {2} > /dev/null 2>&1", shell=True)
            procs.append(tracis_syncronizer_proc)
        else:
            tracis_syncronizer_proc = Popen(f"python run_tracis_synchronization.py --main_sumo_port {args.veins_sumo_port} --other_sumo_ports {args.carla_sumo_port} --sumo_order {2} > /dev/null 2>&1", shell=True)
            procs.append(tracis_syncronizer_proc)
        tracis_syncronizer_proc.wait()

        # ----- simulation finish -----
        kill_processes(procs)

    except KeyboardInterrupt:
        logging.info(CTRL_C_PRESSED_MESSAGE)
        kill_processes(procs)

    except Exception as e:
        logging.error(e)
        kill_processes(procs)


if __name__ == '__main__':
    mobility_handler_choices = ["carla", "veins"]

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for Carla-Veins synchronization.')

    parser.add_argument('--python_api_util_path', default="./../../PythonAPI/util/")
    parser.add_argument('--carla_map_name', default="Town04")

    parser.add_argument('--sumocmd', default="sumo")
    parser.add_argument('--sumocfg_path', default="./examples/Town04.sumocfg")
    parser.add_argument('--sumo_host', default="localhost")
    parser.add_argument('--sumo_begin_time', default=0)
    parser.add_argument('--sumo_end_time', default=SECONDS_OF_ONE_DAY)
    parser.add_argument('--sumo_step_length', default=0.1)

    parser.add_argument('--carla_unrealengine_port', default=2000)
    parser.add_argument('--carla_sumo_port', default=10001)

    parser.add_argument('--veins_vagrant_path', default="./../Veins")
    parser.add_argument('--veins_sumo_port', default=10002)

    parser.add_argument('--data_api_host', default="localhost")
    parser.add_argument('--data_api_port', default=9998)

    parser.add_argument('--main_mobility_handler', default=mobility_handler_choices[0], choices=mobility_handler_choices)
    parser.add_argument('--log_file_path', default="./log/carla_veins_logger.log")

    args = parser.parse_args()

    # ----- set logging -----
    logging.basicConfig(
        handlers=[logging.FileHandler(filename=args.log_file_path), logging.StreamHandler(sys.stdout)],
        format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s: %(message)s',
        level=logging.DEBUG
    )
    main(args)
