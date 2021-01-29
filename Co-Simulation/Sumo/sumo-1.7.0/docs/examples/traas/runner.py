#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2019-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    runner.py
# @author  Michael Behrisch
# @date    2019-05-01

from __future__ import absolute_import
from __future__ import print_function


import os
import subprocess
import sys
import time
if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")
from sumolib import checkBinary  # noqa

javac = "javac"
if 'JAVA_HOME' in os.environ:
    javac = os.path.join(os.environ['JAVA_HOME'], "bin", javac)

traasJar = os.path.join(os.environ['SUMO_HOME'], "bin", "TraaS.jar")
assert(os.path.exists(traasJar))

for f in sys.argv[1:]:
    subprocess.check_call([javac, "-cp", traasJar, "data/%s.java" % f])
procs = [subprocess.Popen(["java", "-cp", os.pathsep.join([traasJar, "data"]), sys.argv[1],
                           checkBinary('sumo'), "data/config.sumocfg"])]
if len(sys.argv) > 2:
    time.sleep(10)  # give sumo some time to start
    procs += [subprocess.Popen(["java", "-cp", os.pathsep.join([traasJar, "data"]), f]) for f in sys.argv[2:]]
for p in procs:
    p.wait()
