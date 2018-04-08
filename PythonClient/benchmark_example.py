#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import logging
import time



from carla.client import make_carla_client
from carla.tcp import TCPConnectionError



from carla.agent_benchmark.agent_benchmark import AgentBenchmark
from carla.agent.forward import Forward
from carla.agent_benchmark.experiment_suite.corl_2017 import CoRL2017
from carla.agent_benchmark.experiment_suite.basic import Basic
import carla.agent_benchmark.results_printer as results_printer



def run_benchmark(full_benchmark):

    while True:
        try:

            with make_carla_client(args.host, args.port) as client:

                # We instantiate a forward agent, a simple policy that just set
                # acceleration as 0.9 and steering as zero
                agent = Forward()
                # We instantiate an experiment suite. Basically a set of experiments
                # that are going to be evaluated on this benchmark.
                if full_benchmark:
                    experiment_suite = CoRL2017()
                else:
                    experiment_suite = Basic()
                # We instantiate the agent benchmark, that is the engine used to
                # benchmark an agent. The instantiation starts the log process, setting
                # the city and log name.
                benchmark = AgentBenchmark(city_name=args.city_name,
                                           name_to_save=args.log_name
                                           + type(experiment_suite).__name__
                                           + '_' + args.city_name)
                # This function performs the benchmark. It returns a dictionary summarizing
                # the entire execution.

                benchmark_summary = benchmark.benchmark_agent(experiment_suite, agent, client)

                print("")
                print("")
                print("----- Printing results for training weathers (Seen in Training) -----")
                print("")
                print("")
                results_printer.print_summary(benchmark_summary, experiment_suite.train_weathers,
                                              benchmark.get_path())

                print("")
                print("")
                print("----- Printing results for test weathers (Unseen in Training) -----")
                print("")
                print("")

                results_printer.print_summary(benchmark_summary, experiment_suite.test_weathers,
                                              benchmark.get_path())

                break

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)





if __name__ == '__main__':

    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='verbose',
        help='print some extra status information')
    argparser.add_argument(
        '-db', '--debug',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host server (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-c', '--city-name',
        metavar='C',
        default='Town01',
        help='The town that is going to be used on benchmark'
        + '(needs to match active town in server, options: Town01 or Town02)')
    argparser.add_argument(
        '-n', '--log_name',
        metavar='T',
        default='test',
        help='The name of the log file to be created by the benchmark'
        )
    argparser.add_argument(
        '--corl-2017',
        action='store_true',
        help='print debug information'
    )


    args = argparser.parse_args()
    if args.debug:
        log_level = logging.DEBUG
    elif args.verbose:
        log_level = logging.INFO
    else:
        log_level = logging.WARNING

    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)
    logging.info('listening to server %s:%s', args.host, args.port)
    run_benchmark(args.corl_2017)