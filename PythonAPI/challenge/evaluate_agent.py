import argparse
import importlib
import os
import sys
sys.path.append('{}/PythonAPI'.format(os.getcwd()))

from challenge.server_manager import Track
from challenge.scenario_setup import Route, ScenarioSetup

def run_evaluation(args):

    # first we instantiate the Agent
    module_name = os.path.basename(args.agent).split('.')[0]
    module_spec = importlib.util.spec_from_file_location(module_name,
                                                         args.agent)
    foo = importlib.util.module_from_spec(module_spec)
    module_spec.loader.exec_module(foo)
    agent_instance = getattr(foo, foo.__name__)()

    # configure simulation
    scenario_manager = ScenarioSetup(args, agent_instance)

    for route in scenario_manager.get_routes():
        scenario_manager.reset(route.map, args.track, args.port)
        scenario_manager.reset_vehicle(route.start)
        scenario_manager.assign_global_plan(route)

        # main loop for the current scenario
        scenario_manager.start_scenario(route)
        while not scenario_manager.is_scenario_over():
            # as soon as the server is ready continue!
            if not scenario_manager.wait_for_tick():
                continue

            # act!
            action = agent_instance()
            scenario_manager.apply_control(action)

        # report statistics
        scenario_manager.report_statistics()
        # clean up!
        scenario_manager.stop_scenario()

    return 0

def main():
    argparser = argparse.ArgumentParser(description='CARLA automatic evaluation script')

    argparser.add_argument('-p', '--port', metavar='P', default=2000, type=int,
                           help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--server_path', help='Absolute path to CARLA server binary', required=True)
    argparser.add_argument("-a", "--agent", type=str, help="Path to Agent's py file to evaluate", required=True)
    argparser.add_argument("-r", "--routes", help="Path to routes file", required=True)
    argparser.add_argument("-t", "--track", type=str, choices=["Sensors",
                                                               "NoRendering"],
                           help="Select competition track",
                           default="Sensors")
    args = argparser.parse_args()

    if args.track is 'Sensors':
        args.track = Track.SENSORS
    else:
        args.track = Track.NO_RENDERING

    # CARLA Evaluation protocol
    run_evaluation(args)


if __name__ == '__main__':
    main()
