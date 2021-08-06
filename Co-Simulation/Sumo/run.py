import argparse
import json

from subprocess import Popen

from util.func import (
    data_from_json,
)


# ----- main -----
if __name__ == "__main__":
    scenario2envfile_path = data_from_json("./envs/scenario2envfile.json")

    parser = argparse.ArgumentParser(description='This script is a launcher for Carla-Veins synchronization.')

    parser.add_argument('scenario', type=str, choices=scenario2envfile_path.keys())

    args = parser.parse_args()

    Popen(f"python -u run_procs.py {scenario2envfile_path[args.scenario]}", shell=True).wait()
