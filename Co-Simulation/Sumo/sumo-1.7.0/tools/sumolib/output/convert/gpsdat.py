# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    gpsdat.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2013-01-15

"""
This module includes functions for converting SUMO's fcd-output into
csv data files used by dlr-fcd processing chain
"""
from __future__ import print_function
from __future__ import absolute_import
import datetime
import sumolib.output  # noqa

TAXI_STATUS_FREE_FLOW = "70"


def fcd2gpsdat(inpFCD, outSTRM, further):
    date = further["base-date"]
    for timestep in inpFCD:
        if timestep.vehicle:
            # does not work with subseconds
            mtime = str(
                date + datetime.timedelta(seconds=int(float(timestep.time))))
            for v in timestep.vehicle:
                print('%s\t%s\t%s\t%s\t%s\t%.3f' % (v.id, mtime, v.x, v.y,
                                                    TAXI_STATUS_FREE_FLOW, float(v.speed) * 3.6), file=outSTRM)
