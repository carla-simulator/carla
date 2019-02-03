import argparse
import importlib
import logging

from server_manager import *

def run_evaluation(args):
    """

    :param args:
    :return:
    """

    # first we instantiate the Agent
    agent_module = importlib.import_module(args.agent)
    agent_instance = agent_module()

    # instantiate a CARLA server manager
    carla_server = ServerManager({'CARLA_SERVER': args.server_path})

    # main loop
    carla_server.reset(map_id='Town04', track_id=Track.SENSORS, port=2000, human_flag=False)
    carla_server.wait_until_ready()


def main():
    argparser = argparse.ArgumentParser(description='CARLA automatic evaluation script')

    argparser.add_argument('-p', '--port', metavar='P', default=2000, type=int,
                           help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--server_path', help='Absolute path to CARLA server binary')
    argparser.add_argument("-a", "--agent", type=str, help="Path to Agent's py file to evaluate")
    args = argparser.parse_args()

    # CARLA Evaluation protocol
    run_evaluation(args)


if __name__ == '__main__':
    main()
