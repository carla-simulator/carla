# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    poi.py
# @author  Jakob Erdmann
# @author  Laura Bieker
# @date    2014-02-13

"""
This module includes functions for converting SUMO's fcd-output into
pois (useful for showing synthetic GPS disturtbances)
"""
from __future__ import absolute_import
from collections import defaultdict
from sumolib.shapes.poi import PoI
from sumolib.color import RGBAColor
from sumolib.miscutils import Colorgen

LAYER = 100  # show above everything else


def fcd2poi(inpFCD, outSTRM, ignored):
    colors = defaultdict(
        lambda: RGBAColor(*Colorgen(("random", 1, 1)).byteTuple()))
    outSTRM.write("<pois>\n")
    for timestep in inpFCD:
        for v in timestep.vehicle:
            outSTRM.write("    %s\n" % PoI("%s_%s" % (
                v.id, timestep.time), v.id, LAYER, colors[v.id], v.x, v.y, lonLat=True).toXML())
    outSTRM.write("</pois>\n")
