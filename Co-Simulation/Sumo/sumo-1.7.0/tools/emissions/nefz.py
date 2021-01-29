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

# @file    nefz.py
# @author  daniel.krajzewicz@dlr.de
# @date    2014-01-14

"""
Generates a ';'-separated file that contains the time line of the NEFZ
 driving cycle.
"""
from __future__ import print_function

NEFZ1 = [
    [11, 0, 0],
    [4, 1.04, -1], [0, 0, 15],
    [8, 0, 15],
    [2, -0.69, -1], [0, 0, 10],
    [3, -.93, -1], [0, 0, 0],
    [21, 0, 0],

    [5, 0.83, -1], [0, 0, 15],
    [2, 0, -1],
    [5, 0.94, -1], [0, 0, 32],
    [24, 0, 32],
    [8, -.76, -1], [0, 0, 10],
    [3, -.93, -1], [0, 0, 0],
    [21, 0, 0],

    [5, .83, -1], [0, 0, 15],
    [2, 0, -1],
    [9, .62, -1], [0, 0, 35],
    [2, 0, -1],
    [8, .52, -1], [0, 0, 50],
    [12, 0, 50],
    [8, -.52, -1], [0, 0, 35],
    [13, 0, 35],
    [2, 0, -1],
    [7, -.87, -1], [0, 0, 10],
    [3, -.93, -1], [0, 0, 0],

    [7, 0, 0],

]

NEFZ2 = [
    [20, 0, 0],
    [5, .83, -1], [0, 0, 15],
    [2, 0, -1],
    [9, .62, -1], [0, 0, 35],
    [2, 0, -1],
    [8, .52, -1], [0, 0, 50],
    [2, 0, -1],
    [13, .43, -1], [0, 0, 70],
    [50, 0, 70],
    [8, -.69, -1], [0, 0, 50],
    [69, 0, 50],
    [13, .43, -1], [0, 0, 70],
    [50, 0, 70],
    [35, .24, -1], [0, 0, 100],
    [30, 0, 100],
    [20, .28, -1], [0, 0, 120],
    [10, 0, 120],
    [16, -.69, -1], [0, 0, 80],
    [8, -1.04, -1], [0, 0, 50],
    [10, -1.39, -1], [0, 0, 0],
    [20, 0, 0]
]


def build(what):
    t = 0
    v = 0
    a = 0
    ts1 = []
    vs1 = []
    as1 = []
    ts2 = []
    vs2 = []
    as2 = []
    ts3 = []
    vs3 = []
    as3 = []
    ct = 0
    cv = 0
    lv = 0
    lt = 0
    ts1.append(0)
    as1.append(0)
    vs1.append(0)
    for tav in what:
        [t, a, v] = tav[:3]
        v = v / 3.6
        if v >= 0:
            # destination velocity
            if a != 0:
                print("ups %s" % tav)
            ts1.append(ct + t)
            as1.append(0)
            vs1.append(v)
        # via acceleration
        for it in range(0, t):
            ts2.append(ct + it)
            as2.append(a)
            mv = cv + a * float(it)
            if mv < 0:
                mv = 0
            vs2.append(mv)
        # via speed (if not None, otherwise "keep in mind")
        if v >= 0:
            dt = float((ct + t) - lt)
            if dt != 0:
                dv = float(v - lv)
                a = dv / float(dt)
                for it in range(lt, ct + t):
                    ts3.append(it)
                    as3.append(a)
                    vs3.append(lv + a * float(it - lt))

        ct = ct + t
        if v >= 0:
            cv = v
            lv = v
            lt = ct
    return [ts1, vs1, as1, ts2, vs2, as2, ts3, vs3, as3]


BASE = 3
ts = []
vs = []
ts1 = []
vs1 = []
ts2 = []
vs2 = []
ts3 = []
vs3 = []
t = 0
for c in [NEFZ1, NEFZ1, NEFZ1, NEFZ1, NEFZ2]:
    tmp = build(c)
    for i in range(0, len(tmp[BASE])):
        ts.append(tmp[BASE][i] + t)
        vs.append(tmp[BASE + 1][i])
        # ts1.append(tmp[0][i]+t)
        # vs1.append(tmp[0+1][i])
        # ts2.append(tmp[3][i]+t)
        # vs2.append(tmp[3+1][i])
        # ts3.append(tmp[6][i]+t)
        # vs3.append(tmp[6+1][i])
    t = t + tmp[BASE][-1] + 1

fdo = open("nefz.csv", "w")
pv = 0
for i in range(0, len(ts)):
    fdo.write("%s;%s;%s\n" % (ts[i], vs[i] * 3.6, pv - vs[i]))
    pv = vs[i]
fdo.close()
