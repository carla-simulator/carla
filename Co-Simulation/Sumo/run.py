import argparse
import datetime
import glob
import logging
import multiprocessing
import os
import pathlib
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

def remove_cache(cache_dir):
    files = glob.glob(cache_dir + "/*")
    for file in files:
        if pathlib.Path(file).is_file():
            os.remove(file)
        elif pathlib.Path(file).is_symlink():
            os.unlink(file)

# ----- processes -----
def change_carla_map(args, env, sumo_files):
    if bool(args.is_carla_standalone_mode):
        return Popen(f"python config.py -p {args.carla_unrealengine_port} -x {env['map_2_sumo_files'][args.carla_map_name]['xodr']} > /dev/null 2>&1", cwd=args.python_api_util_path, shell=True)
    else:
        return Popen(f"python config.py -p {args.carla_unrealengine_port} -m {args.carla_map_name} > /dev/null 2>&1", cwd=args.python_api_util_path, shell=True)

def switch_carla_rendering(args, env, is_carla_rendering):
    if bool(is_carla_rendering):
        Popen(f"python config.py -p {args.carla_unrealengine_port} --rendering > /dev/null 2>&1", cwd=args.python_api_util_path, shell=True).wait()
    else:
        Popen(f"python config.py -p {args.carla_unrealengine_port} --no-rendering > /dev/null 2>&1", cwd=args.python_api_util_path, shell=True).wait()


def start_carla_sumo_synchronization(args, env, sumo_files):
    return Popen(f"python my_synchronization.py {sumo_files['sumocfg']} --step-length {args.time_step} --carla-port {args.carla_unrealengine_port} --sumo-port {args.carla_sumo_port} --sumo-host {args.sumo_host} --carla_veins_data_dir {args.data_dir} --time_to_start {args.time_to_start}", shell=True)


def start_veins_sumo_synchronization(args, env, sumo_files):
    return Popen(f"vagrant ssh -c \"python /vagrant/my-sumo-launched.py --sh localhost --sp {args.veins_sumo_port}\"", cwd=args.veins_vagrant_path, shell=True)


def start_carla_veins_data_server(args, env, sumo_files):
    return Popen(f"python ./synch/carla_veins_data_server.py --host {args.data_server_host} --port {args.data_server_port}", shell=True)


def start_sumo_for_carla(args, env, sumo_files, client_num):
    return Popen(f"{args.sumocmd} -c {sumo_files['sumocfg']} --seed {args.sumo_seed} --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.time_step} --remote-port {args.carla_sumo_port} --num-clients {client_num} --log logfile.txt > /dev/null 2>&1", shell=True)


def start_sumo_for_veins(args, env, sumo_files, client_num):
    return Popen(f"vagrant ssh -c \"sumo -c {env['in_vagrant']['veins_ini_dir_in_vagrant']}/{env['in_vagrant']['sumo_files_name_in_veins']}.sumocfg --seed {args.sumo_seed} --begin {args.sumo_begin_time} --end {args.sumo_end_time} --step-length {args.time_step} --remote-port {args.veins_sumo_port} --num-clients {client_num} --log logfile.txt > /dev/null 2>&1\"", cwd=args.veins_vagrant_path, shell=True)


def start_tracis_synchronization(args, env, sumo_files):
    dev_null = "> /dev/null 2>&1"

    if args.main_mobility_handler == "carla":
        return Popen(f"python ./synch/run_tracis_synchronization.py --main_sumo_host_port 127.0.0.1:{args.carla_sumo_port} --other_sumo_host_ports {env['vagrant_ip']}:{args.veins_sumo_port} --sumo_order {2} --time_to_start {args.time_to_start} {dev_null}", shell=True)
    else:
        return Popen(f"python ./synch/run_tracis_synchronization.py --main_sumo_host_port {env['vagrant_ip']}:{args.veins_sumo_port} --other_sumo_host_ports 127.0.0.1:{args.carla_sumo_port} --sumo_order {2} --time_to_start {args.time_to_start} {dev_null}", shell=True)

class VeinsStateHandler:
    class VeinsState:
        def __init__(self):
            self.is_running = False
            self.run_time = 0

        def update(self, is_running):
            if self.is_running is False and is_running is False:
                pass

            elif self.is_running is False and is_running is True:
                self.is_running = True

            elif self.is_running is True and is_running is True:
                pass

            elif self.is_running is True and is_running is False:
                self.is_running = False
                self.run_time = self.run_time + 1

            return self.is_running, self.run_time


    def __init__(self, veins_vagrant_path, ps_grep_terms, default_number_of_grep_results):
        self.veins_vagrant_path = veins_vagrant_path
        self.ps_grep_terms = ps_grep_terms
        self.default_number_of_grep_results = default_number_of_grep_results

        # state
        self.state = self.VeinsState()


    def run_time(self):
        is_running, run_time = self.state.update(self.__is_running())

        return run_time


    def __is_running(self):
        p = Popen(f"vagrant ssh -c \"ps awx {self.__grep_pipe(self.ps_grep_terms)} \"", cwd=self.veins_vagrant_path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        grep_results = p.communicate()[0].decode('utf-8').split("\n")

        return (self.default_number_of_grep_results < len(grep_results))


    def __grep_pipe(self, grep_terms):
        grep_pipe = ""

        for grep_term in grep_terms:
            grep_pipe = grep_pipe + f" | grep {grep_term} "

        return grep_pipe


class Main:
    def __init__(self, args, env):
        self.args = args
        self.env = env
        self.sumo_files = env["map_2_sumo_files"][args.carla_map_name]
        self.save_dir_name = str(datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S'))

        remove_cache(args.data_dir)

    def run(self):
        procs = []
        args = self.args
        env = self.env
        sumo_files = self.sumo_files

        try:
            print("Changing a carla map ...")
            if check_port_listens(args.carla_unrealengine_port) is False:
                raise Exception("Carla server is not standed.")
            if bool(args.is_carla_change_map):
                p = change_carla_map(args, env, sumo_files)
                p.wait()

            print("Starting SUMO-traci servers ...")
            procs.append(start_sumo_for_carla(args, env, sumo_files, 1))
            time.sleep(5)

            print("Starting synchronization processes ...")
            procs.append(start_carla_sumo_synchronization(args, env, sumo_files))

            switch_carla_rendering(args, env, args.is_carla_rendering)

            print("Wait for running ...")
            while is_processes_alive(procs) == True:
                pass

        except KeyboardInterrupt:
            logging.info(CTRL_C_PRESSED_MESSAGE)

        except Exception as e:
            logging.error(e)

        finally:
            self.finally_proccess(args, env, procs)

    def finally_proccess(self, args, env, procs):
        # save data if the simulation is statis.
        if bool(args.is_dynamic_simulation):
            pass
        else:
            static_save_dir = f"{args.data_dir}/../static_past/{self.save_dir_name}"
            Popen(f"mkdir {static_save_dir}", shell=True).wait()
            Popen(f"cp -rf {args.data_dir}/* {static_save_dir}", shell=True).wait()

        # switch on the calra rendering
        switch_carla_rendering(args, env, 1)
        kill_processes(procs)


class MainWithVeins(Main):
    def __init__(self, args, env):
        super().__init__(args, env)
        self.veins_state_handler = VeinsStateHandler(args.veins_vagrant_path, [env["in_vagrant"]["ini_file_in_veins"]], 3)

        # ----- copy sumo files into Veins directory -----
        print("Initializing ...")
        copy_local_file_to_vagrant(self.sumo_files["rou"], f"{env['in_vagrant']['sumo_files_name_in_veins']}.rou.xml", env['in_vagrant']['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(self.sumo_files["net"], f"{env['in_vagrant']['sumo_files_name_in_veins']}.net.xml", env['in_vagrant']['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(self.sumo_files["poly"], f"{env['in_vagrant']['sumo_files_name_in_veins']}.poly.xml", env['in_vagrant']['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)
        copy_local_file_to_vagrant(self.sumo_files["sumocfg_for_veins"], f"{env['in_vagrant']['sumo_files_name_in_veins']}.sumocfg", env['in_vagrant']['veins_ini_dir_in_vagrant'], args.veins_vagrant_path)

    def run(self):
        procs = []
        args = self.args
        env = self.env
        sumo_files = self.sumo_files

        try:
            print("Changing a carla map ...")
            if check_port_listens(args.carla_unrealengine_port) is False:
                raise Exception("Carla server is not standed.")
            if bool(args.is_carla_change_map):
                p = change_carla_map(args, env, sumo_files)
                p.wait()

            print("Starting SUMO-traci servers ...")
            procs.append(start_sumo_for_carla(args, env, sumo_files, 2))
            procs.append(start_sumo_for_veins(args, env, sumo_files, 2))
            time.sleep(5)

            print("Starting synchronization processes ...")
            procs.append(start_carla_sumo_synchronization(args, env, sumo_files))
            procs.append(start_veins_sumo_synchronization(args, env, sumo_files))
            procs.append(start_tracis_synchronization(args, env, sumo_files))

            switch_carla_rendering(args, env, args.is_carla_rendering)

            print("Please run Veins manually.")
            while is_processes_alive(procs) == True and self.veins_state_handler.run_time() <= 0:
                pass

        except KeyboardInterrupt:
            logging.info(CTRL_C_PRESSED_MESSAGE)

        except Exception as e:
            logging.error(e)

        finally:
            self.finally_proccess(args, env, procs)

    def finally_proccess(self, args, env, procs):
        # save veins result
        Popen(f"vagrant ssh -c \"mkdir {args.veins_result_aggrigation_dir}/{self.save_dir_name} \"", cwd=args.veins_vagrant_path, shell=True).wait()
        Popen(f"vagrant ssh -c \"cp -f {args.result_file_path_in_veins}/* {args.veins_result_aggrigation_dir}/{self.save_dir_name}/ \"", cwd=args.veins_vagrant_path, shell=True).wait()

        super().finally_proccess(args, env, procs)


# ----- main -----
if __name__ == '__main__':
    # env = data_from_json("./env.json")
    env = data_from_json("./env_for_veins.json")
    maps = list(env["map_2_sumo_files"].keys())

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for Carla-Veins synchronization.')

    parser.add_argument('--python_api_util_path', default="./../../PythonAPI/util/")
    parser.add_argument('--carla_map_name', default=maps[0], choices=maps)
    parser.add_argument('--time_step', default=float(env["time_step"]))
    parser.add_argument('--time_to_start', type=float, default=float(env["time_to_start"]))

    parser.add_argument('--sumocmd', default=env["sumocmd"], choices=env["sumocmd_choices"])
    parser.add_argument('--sumo_host', default="127.0.0.1")
    parser.add_argument('--sumo_begin_time', default=0)
    # parser.add_argument('--sumo_end_time', default=(24 * 60 * 60))
    parser.add_argument('--sumo_end_time', default=env["time_to_finish"])
    parser.add_argument('--sumo_seed', type=int, default=env["sumo_seed"])


    parser.add_argument('--carla_unrealengine_port', default=2000)
    parser.add_argument('--carla_sumo_port', default=env["carla_sumo_port"])

    parser.add_argument('--veins_vagrant_path', default=env["veins_vagrant_path"])
    parser.add_argument('--veins_sumo_port', default=env["veins_sumo_port"])

    parser.add_argument('--data_server_host', default="localhost")
    parser.add_argument('--data_server_port', default=9998)
    parser.add_argument('--data_dir', default=(env['carla_veins_dynamic_data_dir'] if bool(env['is_dynamic_simulation']) else env['carla_veins_static_data_dir']))


    parser.add_argument('--is_carla_change_map', type=int, default=env["is_carla_change_map"], choices=[0, 1])
    parser.add_argument('--is_carla_rendering', type=int, default=env["is_carla_rendering"], choices=[0, 1])
    parser.add_argument('--is_carla_standalone_mode', type=int, default=env["is_carla_standalone_mode"], choices=[0, 1])
    parser.add_argument('--is_dynamic_simulation', type=int, default=env["is_dynamic_simulation"], choices=[0, 1])
    parser.add_argument('--main_mobility_handler', default=env["mobility_handler_choices"][0], choices=env["mobility_handler_choices"])
    parser.add_argument('--log_file_path', default="./log/carla_veins_logger.log")
    parser.add_argument('--result_file_path_in_veins', default=env["in_vagrant"]["result_file_path_in_veins"])
    parser.add_argument('--veins_result_aggrigation_dir', default=env["in_vagrant"]["veins_result_aggrigation_dir"])

    args = parser.parse_args()

    # ----- set logging -----
    logging.basicConfig(
        handlers=[logging.FileHandler(filename=args.log_file_path), logging.StreamHandler(sys.stdout)],
        format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s: %(message)s',
        level=logging.DEBUG
    )

    if bool(args.is_dynamic_simulation):
        MainWithVeins(args, env).run()
    else:
        Main(args, env).run()
