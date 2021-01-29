#!/usr/bin/env python
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

# @file    addStopDelay.py
# @author  Jakob Erdmann
# @date    2020-08-25

"""
Increase stop duration by a fixed or random time
"""

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import random
import re

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa
from sumolib.miscutils import parseTime  # noqa


def get_options(args=None):
    parser = sumolib.options.ArgumentParser(description="Sample routes to match counts")
    parser.add_argument("-r", "--route-file", dest="routeFile",
                        help="Input route file")
    parser.add_argument("-o", "--output-file", dest="out", default="out.rou.xml",
                        help="Output route file")
    parser.add_argument("-s", "--seed", type=int, default=42,
                        help="random seed")
    parser.add_argument("-p", "--probability", type=float, default=1,
                        help="apply delay with the given probability ]0, 1]")
    parser.add_argument("--min", type=float, default=0,
                        help="minimum delay value (default 0)")
    parser.add_argument("--max", type=float, default=3600,
                        help="maximum delay value (default 3600)")
    parser.add_argument("--mean", type=float, default=0,
                        help="mean delay")
    parser.add_argument("--dev", type=float, default=60,
                        help="mean standard deviation of delay")

    options = parser.parse_args(args=args)
    if options.routeFile is None:
        parser.print_help()
        sys.exit()

    if options.min != options.max and (
            options.min > options.mean or options.mean > options.max):
        sys.stderr.write("mean should lie between min and max")
        sys.exit()

    return options


def main(options):
    if options.seed:
        random.seed(options.seed)

    with open(options.out, 'w') as outf:
        pat = re.compile(r'(.* duration=")([^"]*)(".*)')
        for line in open(options.routeFile):
            if "<stop" in line and "duration" in line:
                if random.random() < options.probability:
                    if options.min == options.max:
                        delay = options.min
                    else:
                        delay = None
                        while (delay is None
                                or delay < options.min
                                or delay > options.max):
                            delay = random.gauss(options.mean, options.dev)

                    mo = pat.match(line)
                    newDuration = int((parseTime(mo.group(2)) + delay))
                    line = mo.group(1) + str(newDuration) + mo.group(3) + "\n"
            outf.write(line)


if __name__ == "__main__":
    main(get_options())
