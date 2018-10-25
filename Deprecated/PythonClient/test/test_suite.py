#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Test suite for testing CARLAUE4."""

import argparse
import glob
import imp
import inspect
import logging
import os
import random
import sys
import time

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(os.path.join(os.path.dirname(__file__), '.'))

import carla

from carla.tcp import TCPConnectionError
from carla.util import StopWatch

from suite import CarlaServerTest

# Modified by command-line args.
LOGGING_TO_FILE = False

# Output.
GREEN    = '\x1b[0;32m%s\x1b[0m'
YELLOW   = '\x1b[0;33m%s\x1b[0m'
RED      = '\x1b[0;31m%s\x1b[0m'

SEP0   = GREEN % '[==========]'
SEP1   = GREEN % '[----------]'
RUN    = GREEN % '[ RUN      ]'
OK     = GREEN % '[       OK ]'
PASSED = GREEN % '[  PASSED  ]'
FAILED = RED   % '[EXCEPTION ]'
FAILED = RED   % '[  FAILED  ]'


def log_test(prep, message, *args):
    message = prep + ' ' + message % args
    print(message)
    if LOGGING_TO_FILE:
        logging.info(message)


class TestProxy(object):
    def __init__(self, name, declaration, module_name):
        self.name = module_name + '.' + name
        self.declaration = declaration
        self.instance = None

    def instantiate(self, *args, **kwargs):
        self.instance = self.declaration(*args, **kwargs)

    def run(self, *args, **kwargs):
        if not hasattr(self.instance, 'run'):
            logging.error('%s: "run" method should be implemented in derived class', self.name)
            return False
        result = self.instance.run(*args, **kwargs)
        return True if result is None else result


def iterate_tests():
    interface = CarlaServerTest
    strip_ext = lambda f: os.path.splitext(os.path.basename(f))[0]
    is_valid = lambda obj: inspect.isclass(obj) and issubclass(obj, interface)

    folder = os.path.join(os.path.dirname(__file__), 'suite')
    modules = glob.glob(os.path.join(folder, "*.py"))

    for module_name in set(strip_ext(m) for m in modules if not m.startswith('_')):
        logging.debug('parsing module %r', module_name)
        try:
            module_info = imp.find_module(module_name, [folder])
            # This do a reload if already imported.
            module = imp.load_module(module_name, *module_info)
            for name, declaration in inspect.getmembers(module, is_valid):
                if not name.startswith('_'):
                    yield TestProxy(name, declaration, module_name)
        except Exception as exception:
            logging.error('failed to load module %r: %s', module_name, exception)
        finally:
            module_info[0].close()


def run_test(test, args):
    log_test(SEP1, 'Instantiating %s', test.name)
    try:
        test.instantiate(args)
    except Exception as exception:
        logging.error('exception instantiating %r: %s', test.name, exception)
        return False
    log_test(RUN, test.name)
    while True:
        try:
            timer = StopWatch()
            result = test.run()
            timer.stop()
            break
        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)
        except Exception as exception:
            timer.stop()
            logging.exception('exception: %s', exception)
            result = False
            break
    log_test(OK if result else FAILED, '%s (%d ms)', test.name, timer.milliseconds())
    return result


def do_the_tests(args):
    tests = [t for t in iterate_tests()]
    random.shuffle(tests)
    succeeded = []
    failed = []
    log_test(SEP0, 'Running %d tests.', len(tests))
    for test in tests:
        if run_test(test, args):
            succeeded.append(test)
        else:
            failed.append(test)
    log_test(SEP0, '%d tests ran.', len(tests))
    if succeeded:
        log_test(PASSED, '%d tests.', len(succeeded))
    if failed:
        log_test(FAILED, '%d tests.', len(failed))
        for test in failed:
            log_test(FAILED, test.name)
    return True


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--log',
        metavar='LOG_FILE',
        default=None,
        help='print output to file')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')

    args = argparser.parse_args()

    global LOGGING_TO_FILE
    LOGGING_TO_FILE = args.log is not None

    logging_config = {
        'format': '%(levelname)s: %(message)s',
        'level': logging.DEBUG if args.debug else logging.INFO
    }
    if args.log:
        logging_config['filename'] = args.log
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    logging.info('listening to server %s:%s', args.host, args.port)

    print('Running the CARLAUE4 test suite (looks like GTest but it\'s not).')
    do_the_tests(args)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
