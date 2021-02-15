#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    timingStats.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @date    2010-10-15

"""
This script executes a config repeatedly and measures the execution time,
 computes the mean values and deviation.
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import subprocess
import numpy
from optparse import OptionParser
from datetime import datetime

# initialise
optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true",
                     default=False, help="tell me what you are doing")
optParser.add_option("-c", "--configuration",
                     help="sumo configuration to run", metavar="FILE")
optParser.add_option("-r", "--repeat", type="int", default="20",
                     help="how many times to run")
# parse options
(options, args) = optParser.parse_args()

sumoBinary = os.environ.get("SUMO_BINARY", os.path.join(
    os.path.dirname(sys.argv[0]), '..', '..', 'bin', 'sumo'))
elapsed = []
for run in range(options.repeat):
    before = datetime.now()
    subprocess.call([sumoBinary, '-c', options.configuration])
    td = datetime.now() - before
    elapsed.append(td.microseconds + 1000000 * td.seconds)
a = numpy.array(elapsed)
print("%.4f %.4f" % (a.mean() / 1000, a.std() / 1000))
