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

# @file    real_world.py
# @author  Daniel Krajzewicz
# @date    2014-09-01


from __future__ import absolute_import
from __future__ import print_function

from . import fileNeedsRebuild, Scenario
import os
import sumolib.net.generator.cross as netGenerator  # noqa
import sumolib.net.generator.demand as demandGenerator  # noqa


class Scenario_RealWorld(Scenario):
    NAME = "RealWorld"
    THIS_DIR = os.path.join(os.path.abspath(os.path.dirname(__file__)), NAME)

    def __init__(self, which, withDefaultDemand=True):
        Scenario.__init__(self, self.NAME + "/" + which)
        self.THIS_DIR = os.path.join(self.THIS_DIR, which)
        self.NAME = os.path.join(self.NAME, which)
        self.params = {}
        fd = open(os.path.join(self.THIS_DIR, "meta.txt"))
        for line in fd:
            line = line.strip()
            if len(line) == 0:
                continue
            (key, value) = line.split(":")
            self.params[key] = value
        fd.close()
        self.NET_FILE = os.path.join(self.THIS_DIR, self.params["net"])
        self.TLS_FILE = os.path.join(self.THIS_DIR, self.params["tls"])
        self.netName = self.fullPath(self.NET_FILE)
        self.demandName = os.path.join(self.THIS_DIR, self.params["rou"])
        for add in self.params["add"].split(","):
            self.addAdditionalFile(os.path.join(self.THIS_DIR, add))
        # network
        if fileNeedsRebuild(self.netName, "netconvert"):
            pass
        # demand
        print(withDefaultDemand)
        if not withDefaultDemand:
            print("Not supported")
