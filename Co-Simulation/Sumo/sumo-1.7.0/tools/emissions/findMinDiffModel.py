#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    findMinDiffModel.py
# @author  Michael Behrisch
# @date    2016-08-24

"""
This script takes the output file of the emission tests and tries to
find models most similar to a given model concerning the emission values.
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
import pprint

emissionFile = sys.argv[1]
for refModel in sys.argv[2:]:
    refValue = {}
    minDiff = {}
    findRef = False
    minRelDiff = None
    for line in open(emissionFile):
        ls = line.split(":")
        if line[:7] == "Running":
            model = line.split()[1][1:-1]
            if model == refModel:
                findRef = True
        elif line[:7] == "Success":
            findRef = False
        elif findRef and len(ls) > 1:
            refValue[ls[0]] = float(ls[1])
    for line in open(emissionFile):
        ls = line.split(":")
        if line[:7] == "Running":
            model = line.split()[1][1:-1]
            relDiff = 0.
            fuelDiff = 0.
        elif model != refModel and line[:7] == "Success":
            if "HDV" in model:
                print(refModel, model, relDiff / len(minDiff), fuelDiff)
            if minRelDiff is None or relDiff < minRelDiff[0]:
                minRelDiff = (relDiff, model)
        elif model != refModel and len(ls) > 1:
            emission = ls[0]
            if emission != "length" and emission != "electricity":
                diff = float(ls[1]) - refValue[emission]
                relDiff += abs(diff) / refValue[emission]
                if emission == "fuel":
                    fuelDiff = abs(diff) / refValue[emission]
                if emission not in minDiff or abs(diff) < abs(minDiff[emission][0]):
                    minDiff[emission] = (diff, model)
    print(refModel, minRelDiff[1], minRelDiff[0] / len(minDiff))
    pprint.pprint(refValue)
    pprint.pprint(minDiff)
