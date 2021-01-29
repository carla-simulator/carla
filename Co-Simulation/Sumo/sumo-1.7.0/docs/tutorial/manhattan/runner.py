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

# @file    runner.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-10-25

from __future__ import absolute_import
from __future__ import print_function


import os
from subprocess import call
import sys
try:
    sys.path.append(os.path.join(os.path.dirname(
        __file__), '..', '..', '..', '..', "tools"))  # tutorial in tests
    sys.path.append(os.path.join(os.environ.get("SUMO_HOME", os.path.join(
        os.path.dirname(__file__), "..", "..", "..")), "tools"))  # tutorial in docs
    from sumolib import checkBinary  # noqa
except ImportError:
    sys.exit("please declare environment variable 'SUMO_HOME'")

netgenBinary = checkBinary('netgenerate')
jtrrouterBinary = checkBinary('jtrrouter')
sumoBinary = checkBinary('sumo')
import randomTrips  # noqa

call([netgenBinary, '-c', 'data/manhattan.netgcfg'])
randomTrips.main(randomTrips.get_options([
    '--flows', '100',
    '-b', '0',
    '-e', '1',
    '-n', 'data/net.net.xml',
    '-o', 'data/flows.xml',
    '--jtrrouter',
    '--trip-attributes', 'departPos="random" departSpeed="max"']))
call([jtrrouterBinary, '-c', 'data/manhattan.jtrrcfg'])
call([sumoBinary, '-c', 'data/manhattan.sumocfg', '--duration-log.statistics', '-e' '1000'])
