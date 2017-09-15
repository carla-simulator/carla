#!/usr/bin/env python2

# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)


"""Test suite for testing CARLAUE4."""


import argparse
import glob
import imp
import inspect
import logging
import os
import random
import time


from lib.carla_util import TestCarlaClientBase
from lib.util import StopWatch
from lib.util import make_client


from test import CarlaServerTest

# Modified by command-line args.
VERBOSE = False

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
    if not VERBOSE:
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

    folder = os.path.join(os.path.dirname(__file__), 'test')
    modules = glob.glob(os.path.join(folder, "*.py"))

    for module_name in set(strip_ext(m) for m in modules if not m.endswith('__init__.py')):
        logging.debug('parsing module %r', module_name)
        try:
            module_info = imp.find_module(module_name, [folder])
            # This do a reload if already imported.
            module = imp.load_module(module_name, *module_info)
            for name, declaration in inspect.getmembers(module, is_valid):
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
    try:
        timer = StopWatch()
        result = test.run()
        timer.stop()
    except Exception as exception:
        timer.stop()
        logging.error('exception: %s', exception)
        result = False
    log_test(OK if result else FAILED, '%s (%d ms)', test.name, timer.milliseconds())
    return result


def do_the_tests(args):
    tests = [t for t in iterate_tests()]
    random.shuffle(tests)
    succeeded = []
    failed = []
    log_test(SEP0, 'Running %d tests.', len(tests))
    for test in tests:
        if succeeded or failed:
            logging.info('waiting for the server to be ready again')
            time.sleep(5)
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
        help='print debug information to console instead of log file')
    argparser.add_argument(
        '-d', '--debug',
        action='store_true',
        help='print debug extra information to log')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default 2000)')

    args = argparser.parse_args()

    global VERBOSE
    VERBOSE = args.verbose

    logging_config = {
        'format': '%(levelname)s: %(message)s',
        'level': logging.DEBUG if args.debug else logging.INFO
    }
    if not args.verbose:
        logging_config['filename'] = 'test_suite.log'
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    print('Running the CARLAUE4 test suite (looks like GTest but is not).')
    do_the_tests(args)


if __name__ == '__main__':

    main()
