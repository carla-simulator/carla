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

# @file    checkFlows.py
# @author  Daniel Krajzewicz
# @date    2014-09-11


from __future__ import absolute_import
import sys
import sumolib.output
from pylab import legend, plot, show


f = {}
pd = sumolib.output.parse(sys.argv[1], "vehicle")
for v in pd:
    t = int(float(v.depart))
    e = v["route"][0].edges.split(" ")[0]
    if e not in f:
        f[e] = [0] * 86400
    f[e][t] = f[e][t] + 1


AGG = 3600
fa = {}
for e in f:
    fa[e] = [0] * (86400 / AGG)
    for th in range(0, 86400 / AGG):
        for tl in range(0, AGG):
            fa[e][th] = fa[e][th] + f[e][th * AGG + tl]
ts = range(0, 86400 / AGG)


for e in f:
    plot(ts, fa[e], 'o-', label=e)
legend()
show()
