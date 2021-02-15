# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    options.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2012-03-15


from __future__ import print_function
from __future__ import absolute_import
import sys
import subprocess
from collections import namedtuple
import re
from xml.sax import parse, handler
import argparse

_OPTIONS = [None]


def get_long_option_names(application):
    # @todo using option "--save-template stdout" and parsing xml would be prettier
    output = subprocess.check_output([application, '--help'], universal_newlines=True)
    reprog = re.compile(r'(--\S*)\s')
    result = []
    for line in output.splitlines():
        m = reprog.search(line)
        if m:
            result.append(m.group(1))
    return result


Option = namedtuple("Option", ["name", "value", "type", "help"])


class OptionReader(handler.ContentHandler):

    """Reads an option file"""

    def __init__(self):
        self.opts = []

    def startElement(self, name, attrs):
        if 'value' in attrs:
            self.opts.append(Option(name, attrs['value'], attrs.get('type'), attrs.get('help')))


def readOptions(filename):
    optionReader = OptionReader()
    parse(filename, optionReader)
    return optionReader.opts


def getOptions():
    # return global option value (after parse_args was called)
    return _OPTIONS[0]


class ArgumentParser(argparse.ArgumentParser):
    """Drop-in replacement for argparse.ArgumentParser that adds support for
    sumo-style config files.
    Inspired by https://github.com/bw2/ConfigArgParse
    """

    def __init__(self, *args, **kwargs):
        argparse.ArgumentParser.__init__(self, *args, **kwargs)
        self.add_argument('-c', '--configuration-file', help='read configuration from FILE', metavar="FILE")
        self.add_argument('-C', '--save-configuration', help='save configuration to FILE and exit', metavar="FILE")
        self.add_argument('--save-template', help='save configuration template to FILE and exit', metavar="FILE")

    def write_config_file(self, namespace, exit=True):
        if namespace.save_configuration:
            out_file = namespace.save_configuration
            print_template = False
        elif namespace.save_template:
            out_file = namespace.save_template
            print_template = True
        else:
            return
        with open(out_file, "w") as out:
            out.write('<configuration>\n')
            for k, v in vars(namespace).items():
                if k not in ("save_configuration", "save_template", "configuration_file"):
                    key = k
                    default = ''
                    help = ''
                    for a in self._actions:
                        if a.dest == k:
                            for s in a.option_strings:
                                if s.startswith("--"):
                                    key = s[2:]
                                    break
                            if print_template:
                                if a.default is not None:
                                    v = a.default
                                if a.help is not None:
                                    help = ' help="%s"' % a.help
                            break
                    if print_template or v != a.default:
                        if isinstance(v, list):
                            v = " ".join(map(str, v))
                        out.write('    <%s value="%s"%s%s/>\n' % (key, v, default, help))
            out.write('</configuration>\n')
        if exit:
            sys.exit()

    def parse_args(self, args=None, namespace=None):
        args, argv = self.parse_known_args(args, namespace)
        if argv:
            self.error('unrecognized arguments: %s' % ' '.join(argv))
        if _OPTIONS[0] is None:
            # only save the "outermost" option instance
            _OPTIONS[0] = args
        return args

    def parse_known_args(self, args=None, namespace=None):
        if args is None:
            args = sys.argv[1:]
        elif isinstance(args, str):
            args = args.split()
        else:
            args = list(args)
        idx = -1
        if '-c' in args:
            idx = args.index('-c') + 1
        if '--configuration-file' in args:
            idx = args.index('--configuration-file') + 1
        # add each config item to the commandline unless it's there already
        config_args = []
        if idx > 0:
            act_map = {}
            multi_value = set()
            for a in self._actions:
                for s in a.option_strings:
                    if s.startswith("--"):
                        act_map[s[2:]] = a.option_strings
                        if a.nargs:
                            multi_value.add(s[2:])
            for cfg_file in args[idx].split(","):
                for option in readOptions(cfg_file):
                    is_set = False
                    for s in act_map.get(option.name, []):
                        if s in args:
                            is_set = True
                            break
                    if not is_set:
                        if option.value == "True":
                            config_args += ["--" + option.name]
                        elif option.value != "False":
                            if option.name in multi_value:
                                config_args += ["--" + option.name] + option.value.split()
                            else:
                                config_args += ["--" + option.name, option.value]
        namespace, unknown_args = argparse.ArgumentParser.parse_known_args(
            self, args=args+config_args, namespace=namespace)
        self.write_config_file(namespace)
        return namespace, unknown_args
