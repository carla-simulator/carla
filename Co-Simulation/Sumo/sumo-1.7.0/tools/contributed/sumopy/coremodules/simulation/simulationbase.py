# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
# SUMOPy module
# Copyright (C) 2012-2017 University of Bologna - DICAM
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    simulationbase.py
# @author  Joerg Schweizer
# @date


import numpy as np
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.misc import random_choice, get_inversemap


class SimobjMixin:
    def prepare_sim(self, process):
        return []  # [(steptime1,func1),(steptime2,func2),...]

    def config_simresults(self, results):
        # tripresults = res.Tripresults(          'tripresults', results,
        #                                        self,
        #                                        self.get_net().edges
        #                                        )
        #
        #
        #results.config(tripresults, groupnames = ['Trip results'])
        pass

    def process_results(self, results, process=None):
        pass
