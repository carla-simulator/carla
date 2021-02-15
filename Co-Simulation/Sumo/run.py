import argparse
import logging
import multiprocessing
import os
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
    kill_processes,
)

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import traci
else:
    sys.exit("Please declare environment variable 'SUMO_HOME'")

# ----- GLOBAL_VARS -----
CTRL_C_PRESSED_MESSAGE = "ctrl-c is pressed."

# ----- function -----

def run_carla_sumo_synchronization(args, env, sumo_files):
    return Popen(f"python run_synchronization.py {sumo_files['sumocfg']} --carla-port {args.carla_unrealengine_port} --sumo-port {args.carla_sumo_port} --sumo-host {args.sumo_host} > /dev/null 2>&1", shell=True)

def run_veins_sumo_synchronization(args, env, sumo_files):
    return Popen(f"vagrant ssh -c \"python /vagrant/my-sumo-launched.py --sh localhost --sp {args.veins_sumo_port}\"", cwd=args.veins_vagrant_path, shell=True)

def start_carla_veins_data_server(args, env, sumo_files):
    return Popen(f"python carla_veins_data_server --host {args.data_api_host} --port {args.data_api_port} > /dev/null 2>&1", shell=True)

def start_sumo_for_carla(args, env, sumo_files):
    return Popen(f"{args.sumocmd} -c {sumo_files['sumocfg']} --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.sumo_step_length} --remote-port {args.carla_sumo_port} --num-clients 2 > /dev/null 2>&1", shell=True)

def start_sumo_for_veins(args, env, sumo_files):
    return Popen(f"vagrant ssh -c \"{args.sumocmd} -c {env['veins_ini_dir_in_vagrant']}/{env['sumo_files_name_in_veins']}.sumocfg --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.sumo_step_length} --remote-port {args.veins_sumo_port} --num-clients 2 > /dev/null 2>&1\"", cwd=args.veins_vagrant_path, shell=True)
    # return Popen(f"{args.sumocmd} -c {sumo_files['sumocfg']} --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.sumo_step_length} --remote-port {args.veins_sumo_port} --num-clients 2 > /dev/null 2>&1", shell=True)

# ----- main -----
def main(args, env):
    procs = []
    sumo_files = env["map_2_sumo_files"][args.carla_map_name]

    try:
        # ----- copy sumo files into Veins directory -----
        logging.info("Coping sumo files (rou, net poly, sumocfg) into Veins directory.")
        copy_local_file_to_vagrant(sumo_files["rou"], f"{env['sumo_files_name_in_veins']}.rou.xml", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(sumo_files["net"], f"{env['sumo_files_name_in_veins']}.net.xml", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(sumo_files["poly"], f"{env['sumo_files_name_in_veins']}.poly.xml", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(sumo_files["sumocfg_for_veins"], f"{env['sumo_files_name_in_veins']}.sumocfg", env['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)

        # ----- start sumo servers -----
        logging.info(f"For Carla, Sumo reads {sumo_files['sumocfg']} and starts on {args.carla_sumo_port} port.")
        procs.append(start_sumo_for_carla(args, env, sumo_files))

        logging.info(f"For Veins, Sumo reads {sumo_files['sumocfg']} and starts on {args.veins_sumo_port} port.")
        procs.append(start_sumo_for_veins(args, env, sumo_files))

        # ----- start carla-sumo synchronizer -----
        logging.info(f"Checking that Carla server is started on {args.carla_unrealengine_port} port.")
        if check_port_listens(args.carla_unrealengine_port) is False:
            raise Exception("Carla server is not standed.")

        logging.info(f"Changing the Carla map to {args.carla_map_name}.")
        p = Popen(f"python config.py -p {args.carla_unrealengine_port} -m {args.carla_map_name} > /dev/null 2>&1", cwd=args.python_api_util_path, shell=True)
        p.wait()

        logging.info("Running Carla-Sumo synchronizer.")
        procs.append(run_carla_sumo_synchronization(args, env, sumo_files))

        # ----- start veins-sumo synchronizer -----
        logging.info("Running Veins-Sumo synchronizer.")
        procs.append(run_veins_sumo_synchronization(args, env, sumo_files))

        # ----- start carla-veins data synchronizer -----
        logging.info("Starting Carla-Veins data server.")
        procs.append(start_carla_veins_data_server(args, env, sumo_files))

        # ----- start two-traci synchronizer -----
        logging.info("Connecting Carla-Sumo and Veins-Sumo.")
        logging.info(f"Please run {env['ini_file_in_veins']} manually in Veins.")
        if args.main_mobility_handler == "carla":
            tracis_syncronizer_proc = Popen(f"python run_tracis_synchronization.py --main_sumo_host_port 127.0.0.1:{args.carla_sumo_port} --other_sumo_host_ports {env['vagrant_ip']}:{args.veins_sumo_port} --sumo_order {2} > /dev/null 2>&1", shell=True)
            procs.append(tracis_syncronizer_proc)
        else:
            tracis_syncronizer_proc = Popen(f"python run_tracis_synchronization.py --main_sumo_host_port {env['vagrant_ip']}:{args.veins_sumo_port} --other_sumo_host_ports 127.0.0.1:{args.carla_sumo_port} --sumo_order {2} > /dev/null 2>&1", shell=True)
            procs.append(tracis_syncronizer_proc)
        tracis_syncronizer_proc.wait()

    except KeyboardInterrupt:
        logging.info(CTRL_C_PRESSED_MESSAGE)
        kill_processes(procs)

    except Exception as e:
        logging.error(e)
        kill_processes(procs)

    finally:
        kill_processes(procs)



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

    parser.add_argument('--data_api_host', default="localhost")
    parser.add_argument('--data_api_port', default=9998)

    parser.add_argument('--main_mobility_handler', default=env["mobility_handler_choices"][0], choices=env["mobility_handler_choices"])
    parser.add_argument('--log_file_path', default="./log/carla_veins_logger.log")

    args = parser.parse_args()

    # ----- set logging -----
    logging.basicConfig(
        handlers=[logging.FileHandler(filename=args.log_file_path), logging.StreamHandler(sys.stdout)],
        format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s: %(message)s',
        level=logging.DEBUG
    )
    main(args, env)
