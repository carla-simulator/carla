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

# @file    __init__.py
# @author  Daniel Krajzewicz
# @date    2014-07-01


from __future__ import absolute_import
from __future__ import print_function
import sumolib.net.generator.cross as netGenerator  # noqa
import sumolib.net.generator.demand as demandGenerator
import sumolib
import os


SANDBOX_PATH = os.path.join(os.path.dirname(__file__), "..", "sandbox")
REBUILD = False


def maxIndexValue(l):
    max_val = max(l)
    max_idx = l.index(max_val)
    return max_idx, max_val


def minIndexValue(l):
    min_val = min(l)
    min_idx = l.index(min_val)
    return min_idx, min_val


def maxIndexValue_unset(l, l2):
    i = 0
    max_val = None
    max_idx = -1
    while i < len(l):
        if l2[i] != 0:
            i = i + 1
            continue
        if max_val is None or max_val < l[i]:
            max_idx = i
            max_val = l[i]
        i = i + 1
    return max_idx, max_val


def minIndexValue_unset(l, l2):
    i = 0
    min_val = None
    min_idx = -1
    while i < len(l):
        if l2[i] != 0:
            i = i + 1
            continue
        if min_val is None or min_val > l[i]:
            min_idx = i
            min_val = l[i]
        i = i + 1
    return min_idx, min_val


def fileNeedsRebuild(filePath, app):
    print("fileNeedsRebuild> %s" % filePath)
    if REBUILD:
        return True
    if not os.path.exists(filePath):
        return True
    genAppPath = sumolib.checkBinary(app)
    tf = os.path.getmtime(filePath)
    ta = os.path.getmtime(genAppPath)
    return tf < ta


def split_by_proportions(total, proportions, mininum_values):
    """splits the given total by the given proportions but ensures that each value in
    the result has at least the given minimum value"""
    assert(len(proportions) == len(mininum_values))
    assert(total >= sum(mininum_values))
    assert(min(proportions) > 0)
    num = len(proportions)
    sumProportions = float(sum(proportions))
    fractions = [p / sumProportions for p in proportions]
    result = [max(m, int(round(total * f)))
              for f, m in zip(fractions, mininum_values)]
    delta = sum(result) - total
    correct = -1 if delta > 0 else 1
    i = 0
    while delta != 0:
        if result[i] + correct >= mininum_values[i]:
            result[i] += correct
            delta += correct
        i = (i + 1) % num

    assert(sum(result) == total)
    return result


def extrapolateDemand(stream, freq, probs, pivot=demandGenerator.PIVOT__PEAK, tBeg=0):
    ret = demandGenerator.Demand()
    if pivot == demandGenerator.PIVOT__PEAK:
        mmax = 0
        mpos = []
        for i, p in enumerate(probs):
            if p > mmax:
                mpos = i
                mmax = p
        # !!! should be done
        # if count(probs, p)>1:
        #  raise "more than one maximum value"
        # else:
        #  pivot = mpos
        pivot = mpos
    t = tBeg
    for i, p in enumerate(probs):
        # ok, this works just if _numberModel is a number
        num = float(stream._numberModel) * p / probs[pivot]
        ret.addStream(demandGenerator.Stream(stream.sid + "_" + str(i), t, t + freq,
                                             num, stream._departEdgeModel, stream._arrivalEdgeModel,
                                             stream._vTypeModel))
        t = t + freq
    return ret


class Scenario:

    def __init__(self, dataPath):
        self.dataPath = dataPath
        self.net = None
        self.netName = None
        self.demand = None
        self.demandName = None
        self.additional = {}
        self.conn = None
        self.addAdditionalFile("vtypes")
        try:
            os.makedirs(os.path.join(SANDBOX_PATH, self.name))
        except OSError:
            pass

    def addAdditionalFile(self, name):
        self.additional[name] = []

    def addAdditional(self, name, add):
        self.additional[name].append(add)

    def writeSUMOConfig(self, cfgName, addOptions={}):
        cfg = {}
        for a in addOptions:
            cfg[a] = addOptions[a]
        cfg["net-file"] = self.netName
        cfg["route-files"] = self.demandName
        if "vtypes" in self.additional:
            cfg["additional-files"] = "vtypes.add.xml"
        for a in self.additional:
            if a == "vtypes":
                continue
            fileName = a + ".add.xml"
            if len(self.additional[a]) > 0:
                sumolib.files.additional.write(fileName, self.additional[a])
            if "additional-files" not in cfg:
                cfg["additional-files"] = fileName
            else:
                cfg["additional-files"] = cfg["additional-files"] + \
                    "," + fileName
        fdo = open(cfgName, "w")
        fdo.write("<c>\n")
        for v in cfg:
            fdo.write('  <%s value="%s"/>\n' % (v, cfg[v]))
        fdo.write("</c>\n")
        fdo.close()

    def getNet(self):
        if self.net is not None:
            return self.net
        if self.netName is not None:
            self.net = sumolib.net.readNet(self.netName)
            return self.net
        raise RuntimeError("network is unknown")

    def fullPath(self, fileName):
        print("full >" + os.path.join(self.dataPath, fileName))
        return os.path.join(self.dataPath, fileName)

    def sandboxPath(self, fileName):
        print("sandbox >" + os.path.join(SANDBOX_PATH, fileName))
        return os.path.join(SANDBOX_PATH, fileName)

    def getOppositeFlows(self):
        fNS = [0] * 24
        fWE = [0] * 24
        for s in self.demand.streams:
            if s._departEdgeModel.startswith("em") or s._departEdgeModel.startswith("wm"):
                fWE[int(s._validFrom / 3600)
                    ] = fWE[int(s._validFrom / 3600)] + s._numberModel
            elif s._departEdgeModel.startswith("sm") or s._departEdgeModel.startswith("nm"):
                fNS[int(s._validFrom / 3600)
                    ] = fNS[int(s._validFrom / 3600)] + s._numberModel
        return (fNS, fWE)

    def getOppositeFlows2(self, ew, sn):
        fNS = [0] * 24
        fWE = [0] * 24
        for s in self.demand.streams:
            if s._departEdgeModel in ew:
                fWE[int(s._validFrom / 3600)
                    ] = fWE[int(s._validFrom / 3600)] + s._numberModel
            else:
                fNS[int(s._validFrom / 3600)
                    ] = fNS[int(s._validFrom / 3600)] + s._numberModel
        return (fNS, fWE)

    def buildWAUT(self, streamsNS, streamsWE):
        #
        rel = []
        ovr = []
        program = [0] * len(streamsNS)
        for i, x in enumerate(streamsNS):
            s = streamsNS[i] + streamsWE[i]
            if s != 0:
                rel.append(x / s)
            else:
                rel.append(0)
            ovr.append(s)
        #
        NIGHT = 1
        DAY = 2
        MORNING = 3
        AFTERNOON = 4
        NIGHT_MAX = 500

        MORNING_MIN = 500
        AFTERNOON_MIN = 500
        REL_THRESHOLD = .45
        IREL_THRESHOLD = 1. - REL_THRESHOLD

        i = 0
        # streamsNS[i]<NIGHT_MAX and streamsWE[i]<NIGHT_MAX:
        while i < len(streamsNS) and streamsNS[i] + streamsWE[i] < NIGHT_MAX:
            program[i] = NIGHT
            i = i + 1
        i = len(streamsNS) - 1
        # and streamsNS[i]<NIGHT_MAX and streamsWE[i]<NIGHT_MAX:
        while i > 0 and streamsNS[i] + streamsWE[i] < NIGHT_MAX:
            program[i] = NIGHT
            i = i - 1
        i = 0
        maxIdx, maxVal = maxIndexValue_unset(rel, program)
        minIdx, minVal = minIndexValue_unset(rel, program)
        print(program)
        print("morning max %s %s" % (maxIdx, maxVal))
        print("morning min %s %s" % (minIdx, minVal))
        print("%s %s" % (maxVal, 1. - minVal))
        if maxVal > 1. - minVal:
            i = maxIdx
            while i > 0 and rel[i] > IREL_THRESHOLD and program[i] == 0 and streamsNS[i] + streamsWE[i] > MORNING_MIN:
                program[i] = MORNING
                i = i - 1
            i = maxIdx + 1
            while i < len(rel) and rel[i] > IREL_THRESHOLD and program[
                    i] == 0 and streamsNS[i] + streamsWE[i] > MORNING_MIN:
                program[i] = MORNING
                i = i + 1
        else:
            i = minIdx
            print("!")
            print("  %s %s %s %s" %
                  (i, rel[i], program[i], streamsNS[i] + streamsWE[i]))
            while i > 0 and rel[i] < REL_THRESHOLD and program[i] == 0 and streamsNS[i] + streamsWE[i] > MORNING_MIN:
                program[i] = MORNING
                i = i - 1
                print("  %s %s %s %s" %
                      (i, rel[i], program[i], streamsNS[i] + streamsWE[i]))
            i = minIdx + 1
            print("  %s %s %s %s" %
                  (i, rel[i], program[i], streamsNS[i] + streamsWE[i]))
            while i < len(rel) and rel[i] < REL_THRESHOLD and program[
                    i] == 0 and streamsNS[i] + streamsWE[i] > MORNING_MIN:
                program[i] = MORNING
                i = i + 1
                print("  %s %s %s %s" %
                      (i, rel[i], program[i], streamsNS[i] + streamsWE[i]))

        print(rel)
        print(program)
        maxIdx, maxVal = maxIndexValue_unset(rel, program)
        minIdx, minVal = minIndexValue_unset(rel, program)
        if maxIdx != -1 and minIdx != -1:
            print("%s %s" % (maxIdx, maxVal))
            print("%s %s" % (minIdx, minVal))
            print("%s %s" % (maxVal, 1 - minVal))
            if maxVal > 1 - minVal:
                i = maxIdx
                # and rel[i]>.6
                while i > 0 and rel[i] > IREL_THRESHOLD and program[
                        i] == 0 and streamsNS[i] + streamsWE[i] > AFTERNOON_MIN:
                    program[i] = AFTERNOON
                    i = i - 1
                i = maxIdx + 1
                while i < len(rel) and rel[i] > IREL_THRESHOLD and program[
                        i] == 0 and streamsNS[i] + streamsWE[i] > AFTERNOON_MIN:
                    program[i] = AFTERNOON
                    i = i + 1
            else:
                i = minIdx
                while i > 0 and rel[i] < REL_THRESHOLD and program[
                        i] == 0 and streamsNS[i] + streamsWE[i] > AFTERNOON_MIN:
                    program[i] = AFTERNOON
                    i = i - 1
                i = minIdx + 1
                while i < len(rel) and rel[i] < REL_THRESHOLD and program[
                        i] == 0 and streamsNS[i] + streamsWE[i] > AFTERNOON_MIN:
                    program[i] = AFTERNOON
                    i = i + 1
        for i, x in enumerate(program):
            if program[i] == 0:
                program[i] = DAY
        fdo = open("waut.txt", "w")
        for i, x in enumerate(program):
            fdo.write("%s %s %s %s %s\n" %
                      (streamsNS[i], streamsWE[i], ovr[i], rel[i], program[i]))
        fdo.close()
        #
        ret1 = {}
        for i in range(1, 5):
            maxV = None
            maxI = 0
            for j, v in enumerate(rel):
                if program[j] != i:
                    continue
                if maxV is None:
                    maxV = rel[j]
                    maxI = j
                elif maxV < rel[j]:
                    maxV = rel[j]
                    maxI = j
                elif maxV < 1. - rel[j]:
                    maxV = 1. - rel[j]
                    maxI = j
            print("%s %s %s" % (i, maxV, maxI))
            greens = split_by_proportions(
                72, (rel[maxI], 1. - rel[maxI]), (10, 10))
            ret1[i] = greens
            print(greens)
        # build programs
        ret2 = []
        last = -1
        for j, v in enumerate(program):
            if last != v:
                ret2.append((j, v))
                last = v
        return (ret1, ret2)


def getScenario(which, useName, params, withDefaultDemand=True):
    if which == "RiLSA1":
        from . import rilsa1
        return rilsa1.Scenario_RiLSA1(useName, withDefaultDemand)
    elif which == "RiLSA1OutTLS":
        from . import rilsa1_out_tls
        return rilsa1_out_tls.Scenario_RiLSA1OutTLS(params, withDefaultDemand)
    elif which == "RiLSA1BothTLS":
        from . import rilsa1_both_tls
        return rilsa1_both_tls.Scenario_RiLSA1BothTLS(params, withDefaultDemand)
    elif which == "RiLSA1OutTLS24":
        from . import rilsa1_out_tls24
        return rilsa1_out_tls24.Scenario_RiLSA1OutTLS24(params, withDefaultDemand)
    elif which == "RiLSA1BothTLS24":
        from . import rilsa1_both_tls24
        return rilsa1_both_tls24.Scenario_RiLSA1BothTLS24(params, withDefaultDemand)
    elif which == "BasicCross":
        from . import basic_cross
        return basic_cross.Scenario_BasicCross(useName, withDefaultDemand)
    elif which == "BasicCrossL":
        from . import basic_crossl
        return basic_crossl.Scenario_BasicCrossL(useName, withDefaultDemand)
    elif which == "BasicCorridor":
        from . import basic_corridor
        return basic_corridor.Scenario_BasicCorridor(useName, params["xoff"], withDefaultDemand)
    elif which == "BasicNet":
        from . import basic_net
        return basic_net.Scenario_BasicNet(useName, params["rot"], withDefaultDemand)
    elif which == "RealWorld":
        from . import real_world
        return real_world.Scenario_RealWorld(useName, params["which"], withDefaultDemand)
    elif which == "BasicRiLSANet":
        from . import basic_rilsanet
        return basic_rilsanet.Scenario_BasicRiLSANet(useName, params, withDefaultDemand)
    elif which == "BasicRiLSANet2x2":
        from . import basic_rilsanet2x2
        return basic_rilsanet2x2.Scenario_BasicRiLSANet2x2(useName, params, withDefaultDemand)
    elif which == "BasicRiLSACorridor3":
        from . import basic_rilsacorridor3
        return basic_rilsacorridor3.Scenario_BasicRiLSACorridor3(useName, params, withDefaultDemand)
    raise RuntimeError("unknown scenario '%s'" % which)
