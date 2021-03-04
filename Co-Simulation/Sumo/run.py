import argparse
import logging
import multiprocessing
import os
import signal
import subprocess
import sys
import time
import xmltodict

from subprocess import Popen
from multiprocessing import Process

from util.func import (
    copy_local_file_to_vagrant,
    check_port_listens,
    check_port_listens_by_port_scan,
    data_from_json,
    host_ip,
)

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import traci
else:
    sys.exit("Please declare environment variable 'SUMO_HOME'")

# ----- GLOBAL_VARS -----
CTRL_C_PRESSED_MESSAGE = "ctrl-c is pressed."

# ----- function -----
# ----- handling processes -----
def is_processes_alive(procs):
    is_alive = True

    for p in procs:
        if p.poll() is None:
            is_alive = is_alive & True
        else:
            is_alive = is_alive & False

    return is_alive


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


# ----- processes -----
def change_carla_map(args, env, sumo_files):
    return Popen(f"python config.py -p {args.carla_unrealengine_port} -m {args.carla_map_name} > /dev/null 2>&1", cwd=args.python_api_util_path, shell=True)


def start_carla_sumo_synchronization(args, env, sumo_files):
    return Popen(f"python my_synchronization.py {sumo_files['sumocfg']} --carla-port {args.carla_unrealengine_port} --sumo-port {args.carla_sumo_port} --sumo-host {args.sumo_host} ", shell=True)


def start_veins_sumo_synchronization(args, env, sumo_files):
    return Popen(f"vagrant ssh -c \"python /vagrant/my-sumo-launched.py --sh localhost --sp {args.veins_sumo_port}\"", cwd=args.veins_vagrant_path, shell=True)


def start_carla_veins_data_server(args, env, sumo_files):
    return Popen(f"python carla_veins_data_server.py --host {args.data_server_host} --port {args.data_server_port}", shell=True)


def start_sumo_for_carla(args, env, sumo_files):
    return Popen(f"{args.sumocmd} -c {sumo_files['sumocfg']} --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.sumo_step_length} --remote-port {args.carla_sumo_port} --num-clients 2 > /dev/null 2>&1", shell=True)


def start_sumo_for_veins(args, env, sumo_files):
    return Popen(f"vagrant ssh -c \"{args.sumocmd} -c {env['veins_ini_dir_in_vagrant']}/{env['sumo_files_name_in_veins']}.sumocfg --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.sumo_step_length} --remote-port {args.veins_sumo_port} --num-clients 2 > /dev/null 2>&1\"", cwd=args.veins_vagrant_path, shell=True)


def start_tracis_synchronization(args, env, sumo_files):
    if args.main_mobility_handler == "carla":
        return Popen(f"python run_tracis_synchronization.py --main_sumo_host_port 127.0.0.1:{args.carla_sumo_port} --other_sumo_host_ports {env['vagrant_ip']}:{args.veins_sumo_port} --sumo_order {2} ", shell=True)
    else:
        return Popen(f"python run_tracis_synchronization.py --main_sumo_host_port {env['vagrant_ip']}:{args.veins_sumo_port} --other_sumo_host_ports 127.0.0.1:{args.carla_sumo_port} --sumo_order {2}  ", shell=True)


class Main:
    def __init__(self, args, env):
        self.args = args
        self.env = env
        self.sumo_files = env["map_2_sumo_files"][args.carla_map_name]

        # ----- copy sumo files into Veins directory -----
        print("Initializing ...")
        copy_local_file_to_vagrant(self.sumo_files["rou"], f"{env['sumo_files_name_in_veins']}.rou.xml", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(self.sumo_files["net"], f"{env['sumo_files_name_in_veins']}.net.xml", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(self.sumo_files["poly"], f"{env['sumo_files_name_in_veins']}.poly.xml", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(self.sumo_files["sumocfg_for_veins"], f"{env['sumo_files_name_in_veins']}.sumocfg", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)


    def run(self):
        procs = []
        args = self.args
        env = self.env
        sumo_files = self.sumo_files

        try:
            print("Changing a carla map ...")
            if check_port_listens(args.carla_unrealengine_port) is False:
                raise Exception("Carla server is not standed.")
            p = change_carla_map(args, env, sumo_files)
            p.wait()

            print("Starting SUMO-traci servers ...")
            procs.append(start_sumo_for_carla(args, env, sumo_files))
            procs.append(start_sumo_for_veins(args, env, sumo_files))
            time.sleep(5)

            print("Starting synchronization processes ...")
            procs.append(start_carla_sumo_synchronization(args, env, sumo_files))
            procs.append(start_veins_sumo_synchronization(args, env, sumo_files))
            procs.append(start_carla_veins_data_server(args, env, sumo_files))
            procs.append(start_tracis_synchronization(args, env, sumo_files))

            print(f"Please run {env['ini_file_in_veins']} manually in Veins.")
            while is_processes_alive(procs) == True:
                pass

        except KeyboardInterrupt:
            logging.info(CTRL_C_PRESSED_MESSAGE)
            kill_processes(procs)

        except Exception as e:
            logging.error(e)
            kill_processes(procs)

        finally:
            kill_processes(procs)


# ----- main -----
if __name__ == '__main__':
    env = data_from_json("./env.json")
    maps = list(env["map_2_sumo_files"].keys())

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for Carla-Veins synchronization.')

    parser.add_argument('--python_api_util_path', default="./../../PythonAPI/util/")
    parser.add_argument('--carla_map_name', default=maps[0], choices=maps)

    parser.add_argument('--sumocmd', default="sumo")
    parser.add_argument('--sumo_host', default="127.0.0.1")
    parser.add_argument('--sumo_begin_time', default=0)
    parser.add_argument('--sumo_end_time', default=(24 * 60 * 60))
    parser.add_argument('--sumo_step_length', default=0.1)

    parser.add_argument('--carla_unrealengine_port', default=2000)
    parser.add_argument('--carla_sumo_port', default=env["carla_sumo_port"])

    parser.add_argument('--veins_vagrant_path', default="./../Veins")
    parser.add_argument('--veins_sumo_port', default=env["veins_sumo_port"])

    parser.add_argument('--data_server_host', default="localhost")
    parser.add_argument('--data_server_port', default=9998)

    parser.add_argument('--main_mobility_handler', default=env["mobility_handler_choices"][0], choices=env["mobility_handler_choices"])
    parser.add_argument('--log_file_path', default="./log/carla_veins_logger.log")

    args = parser.parse_args()

    # ----- set logging -----
    logging.basicConfig(
        handlers=[logging.FileHandler(filename=args.log_file_path), logging.StreamHandler(sys.stdout)],
        format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s: %(message)s',
        level=logging.DEBUG
    )

    Main(args, env).run()
