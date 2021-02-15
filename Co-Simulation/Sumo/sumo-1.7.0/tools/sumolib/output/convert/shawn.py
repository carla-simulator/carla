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

# @file    shawn.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2013-01-15

"""
This module includes functions for converting SUMO's fcd-output into
data files read by Shawn.
"""
from __future__ import print_function
from __future__ import absolute_import
import datetime
import sumolib.output
import sumolib.net


def fcd2shawn(inpFCD, outSTRM, further):
    print('<?xml version="1.0" encoding="utf-8"?>', file=outSTRM)
    print('<!-- generated on %s by %s -->\n' %
          (datetime.datetime.now(), further["app"]), file=outSTRM)
    print('<scenario>', file=outSTRM)
    # is it necessary to convert the ids?
    vIDm = sumolib._Running(further["orig-ids"])
    for timestep in inpFCD:
        print('   <snapshot id="%s">' % timestep.time, file=outSTRM)
        if timestep.vehicle:
            for v in timestep.vehicle:
                nid = vIDm.g(v.id)
                print('     <node id="%s"> <location x="%s" y="%s" z="%s"/> </node>' %
                      (nid, v.x, v.y, v.z), file=outSTRM)
        print('   </snapshot>', file=outSTRM)
    print('</scenario>', file=outSTRM)
