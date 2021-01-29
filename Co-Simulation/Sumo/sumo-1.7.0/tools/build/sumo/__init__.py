# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2020-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    __init__.py
# @author  Michael Behrisch
# @date    2020-07-28

import os
import sys
import subprocess


SUMO_HOME = os.path.dirname(__file__)
ENV = os.environ.copy()
if "SUMO_HOME" not in ENV:
    ENV["SUMO_HOME"] = SUMO_HOME


def _makefunc(app):
    return lambda: sys.exit(subprocess.call([os.path.join(SUMO_HOME, 'bin', app)] + sys.argv[1:], env=ENV))


activitygen = _makefunc("activitygen")
dfrouter = _makefunc("dfrouter")
duarouter = _makefunc("duarouter")
emissionsDrivingCycle = _makefunc("emissionsDrivingCycle")
emissionsMap = _makefunc("emissionsMap")
jtrrouter = _makefunc("jtrrouter")
marouter = _makefunc("marouter")
netconvert = _makefunc("netconvert")
netedit = _makefunc("netedit")
netgenerate = _makefunc("netgenerate")
od2trips = _makefunc("od2trips")
polyconvert = _makefunc("polyconvert")
sumo = _makefunc("sumo")
sumo_gui = _makefunc("sumo-gui")
