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

# @file    demand.py
# @author  Daniel Krajzewicz
# @date    2013-10-10

from __future__ import absolute_import
from __future__ import print_function
import random
import sumolib
import os
import subprocess
import math
import tempfile


PIVOT__PEAK = 10000


class LinearChange:

    def __init__(self, beginFlow, endFlow, beginTime, endTime):
        self.beginFlow = beginFlow / 3600.
        self.endFlow = endFlow / 3600.
        self.beginTime = beginTime
        self.endTime = endTime

    def depart(self, t):
        return random.random() < (
            self.beginFlow + (self.endFlow - self.beginFlow) / (self.endTime - self.beginTime) * (t - self.beginTime))


class WaveComposition:

    def __init__(self, offset, curves):
        self.offset = offset
        self.curves = curves

    def depart(self, t):
        v = self.offset
        for c in self.curves:
            dt = t - c[3]
            v = v + \
                c[0] * math.sin(2 * math.pi * dt * c[2]) + \
                c[1] * math.cos(2 * math.pi * dt * c[2])
        v = v / 3600.
        return random.random() < v


class Vehicle:

    def __init__(self, id, depart, fromEdge, toEdge, vType, via=None):
        self.id = id
        self.depart = depart
        self.fromEdge = fromEdge
        self.toEdge = toEdge
        self.vType = vType
        self._via = via


class Stream:

    def __init__(self, sid, validFrom, validUntil, numberModel,
                 departEdgeModel, arrivalEdgeModel, vTypeModel, via=None):
        self.sid = sid
        self._numberModel = numberModel
        self._departEdgeModel = departEdgeModel
        self._arrivalEdgeModel = arrivalEdgeModel
        self._vTypeModel = vTypeModel
        self._validFrom = validFrom
        self._validUntil = validUntil
        self._via = via

    def getVehicleDepartures(self, b, e, sampleFactor=None, seenRatio=None):
        if self._validFrom is not None and self._validUntil is not None and (
                e < self._validFrom or b > self._validUntil):
            return []
        ret = []
        for i in range(b, e):
            if self._validFrom is not None and self._validUntil is not None and (
                    i < self._validFrom or i > self._validUntil):
                continue
            depart = i
            if sampleFactor is not None:
                off = i % (sampleFactor * 24)
                if not off < sampleFactor:
                    continue
                depart = sampleFactor * int(i / (24 * sampleFactor)) + off
            if isinstance(self._numberModel, int) or isinstance(self._numberModel, float):
                if random.random() < float(self._numberModel) / 3600.:
                    ret.append(depart)
            elif self._numberModel.depart(i):
                ret.append(depart)
        return ret

    def getFrom(self, what, i, number):
        if isinstance(what, str):
            return what
        if isinstance(what, int):
            return what
        if isinstance(what, float):
            return what
        if isinstance(what, list):
            return what[i % len(what)]
        if isinstance(what, dict):
            r = random.random()
            s = 0
            for k in what:
                s = s + what[k]
                if s > r:
                    return k
            return None
        return what.get()

    def toVehicles(self, b, e, offset=0, sampleFactor=None, seenRatio=None):
        vehicles = []
        departures = self.getVehicleDepartures(b, e, sampleFactor, seenRatio)
        number = len(departures)
        for i, d in enumerate(departures):
            fromEdge = self.getFrom(self._departEdgeModel, i, number)
            toEdge = self.getFrom(self._arrivalEdgeModel, i, number)
            vType = self.getFrom(self._vTypeModel, i, number)
            sid = self.sid
            if sid is None:
                sid = fromEdge + "_to_" + toEdge + "_" + str(i)
            vehicles.append(
                Vehicle(sid + "#" + str(i + offset), int(d), fromEdge, toEdge, vType, self._via))
        return vehicles


class Demand:

    def __init__(self):
        self.streams = []

    def addStream(self, s):
        self.streams.append(s)

    def build(self, b, e, netName="net.net.xml", routesName="input_routes.rou.xml", sampleFactor=None):
        vehicles = []
        for s in self.streams:
            vehicles.extend(s.toVehicles(b, e, len(vehicles), sampleFactor))
        fdo = tempfile.NamedTemporaryFile(mode="w", delete=False)
        fdo.write("<routes>\n")
        for v in sorted(vehicles, key=lambda veh: veh.depart):
            via = ""
            if v._via is not None:
                via = ' via="%s"' % v._via
            if v.vType == "pedestrian":
                fdo.write('    <person id="%s" depart="%s" type="pedestrian"><walk from="%s" to="%s"/></person>\n' %
                          (v.id, v.depart, v.fromEdge, v.toEdge))
            else:
                fdo.write('    <trip id="%s" depart="%s" from="%s" to="%s" type="%s" %s/>\n' %
                          (v.id, v.depart, v.fromEdge, v.toEdge, v.vType, via))
        fdo.write("</routes>")
        fdo.close()
        duarouter = sumolib.checkBinary("duarouter")
        print("netName > %s" % netName)
        print("routesName > %s" % routesName)
        # aeh, implicitly setting --no-warnings is not nice, is it?; and the
        # need to dump generated vtypes to a temporary file as well
        subprocess.call([duarouter, "-v", "-n", netName, "-t", fdo.name, "-o", routesName,
                         "--no-warnings", "--additional-files", "vtypes.add.xml", "--vtype-output", "tmp.add.xml"])
        os.remove(fdo.name)
