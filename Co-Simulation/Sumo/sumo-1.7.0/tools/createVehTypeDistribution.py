#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    createVehTypeDistribution.py
# @author  Mirko Barthauer (Technische Universitaet Braunschweig, Institut fuer Verkehr und Stadtbauwesen)
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2016-06-09

"""
Creates a vehicle type distribution with a number of representative car-following parameter sets.
Optional parameters can be viewed by using the --help switch.
Mandatory input:
path to config file - defines the car-following model parameter distributions for one single vehicle type distribution

In the config file, one line is used per vehicle type attribute. The syntax is:
nameOfAttribute; valueOfAttribute [; limits]

ValueOfAttribute can be a string, a scalar value or a distribution definition.
All parameters are floating point numbers.
Available distributions and their syntax are:
"normal(mu,sd)": Normal distribution with mean mu and standard deviation sd.
"lognormal(mu,sd)": Log-Normal distribution with mean mu and standard deviation sd.
"uniform(a,b)": Uniform distribution between a and b.
"gamma(alpha,beta)": Gamma distribution.

Limits are optional and defined as the allowed interval: e.g. "[0,1]" or "[3.5,5.0]".
By default, no negative values are accepted but have to be enabled by
a negative lower limit.
"""

import os
import sys
import csv
import re
import xml.dom.minidom
import random
import argparse


class FixDistribution(object):

    def __init__(self, params, isNumeric=True):
        if isNumeric:
            self._params = tuple([float(p) for p in params])
        else:
            self._params = params
        self._limits = (0, None)
        self._isNumeric = isNumeric
        self._maxSampleAttempts = 10

    def setMaxSamplingAttempts(self, n):
        if n is not None:
            self._maxSampleAttempts = n

    def setLimits(self, limits):
        self._limits = limits

    def sampleValue(self):
        if self._isNumeric:
            value = None
            nrSampleAttempts = 0
            # Sample until value falls into limits
            while nrSampleAttempts < self._maxSampleAttempts \
                    and (value is None or (self._limits[1] is not None and value > self._limits[1]) or
                         (self._limits[0] is not None and value < self._limits[0])):
                value = self._sampleValue()
                nrSampleAttempts += 1
            # Eventually apply fallback cutting value to limits
            if self._limits[0] is not None and value < self._limits[0]:
                value = self._limits[0]
            elif self._limits[1] is not None and value > self._limits[1]:
                value = self._limits[1]
        else:
            value = self._sampleValue()
        return value

    def sampleValueString(self, decimalPlaces):
        if self._isNumeric:
            decimalPattern = "%." + str(decimalPlaces) + "f"
            return decimalPattern % self.sampleValue()
        return self.sampleValue()

    def _sampleValue(self):
        return self._params[0]


class NormalDistribution(FixDistribution):

    def __init__(self, loc, scale):
        FixDistribution.__init__(self, (loc, scale))

    def _sampleValue(self):
        return random.normalvariate(self._params[0], self._params[1])


class LogNormalDistribution(FixDistribution):

    def __init__(self, loc, scale):
        FixDistribution.__init__(self, (loc, scale))

    def _sampleValue(self):
        return random.lognormvariate(self._params[0], self._params[1])


class NormalCappedDistribution(FixDistribution):

    def __init__(self, loc, scale, cutLow, cutHigh):
        FixDistribution.__init__(self, (loc, scale, cutLow, cutHigh))
        if loc < cutLow or loc > cutHigh:
            sys.stderr.write("mean %s is outside cutoff bounds [%s, %s]" % (
                loc, cutLow, cutHigh))
            sys.exit()

    def _sampleValue(self):
        while True:
            cand = random.normalvariate(self._params[0], self._params[1])
            if cand >= self._params[2] and cand <= self._params[3]:
                return cand


class UniformDistribution(FixDistribution):

    def __init__(self, lower, upper):
        FixDistribution.__init__(self, (lower, upper))

    def _sampleValue(self):
        return random.uniform(self._params[0], self._params[1])


class GammaDistribution(FixDistribution):

    def __init__(self, loc, scale):
        FixDistribution.__init__(self, (loc, 1.0 / scale))

    def _sampleValue(self):
        return random.gammavariate(self._params[0], self._params[1])


def get_options(args=None):
    argParser = argparse.ArgumentParser()
    argParser.add_argument(
        "configFile", help="file path of the config file which defines the car-following parameter distributions")
    argParser.add_argument(
        "-o", "--output-file", dest="outputFile", default="vTypeDistributions.add.xml", help="file path of the " +
        "output file (if the file already exists, the script tries to insert the distribution node into it)")
    argParser.add_argument(
        "-n", "--name", dest="vehDistName", default="vehDist", help="alphanumerical ID used for the created " +
        "vehicle type distribution")
    argParser.add_argument(
        "-s", "--size", type=int, default=100, dest="vehicleCount", help="number of vTypes in the distribution")
    argParser.add_argument(
        "-d", "--decimal-places", type=int, default=3, dest="decimalPlaces", help="number of decimal places for " +
        "numeric attribute values")
    argParser.add_argument(
        "--resampling", type=int, default=100, dest="nrSamplingAttempts", help="number of attempts to resample a " +
        "value until it lies in the specified bounds")
    argParser.add_argument("--seed", type=int, help="random seed", default=42)

    options = argParser.parse_args()
    return options


def readConfigFile(options):
    filePath = options.configFile
    result = {}
    floatRegex = [r'\s*(-?[0-9]+(\.[0-9]+)?)\s*']
    distSyntaxes = {'normal': r'normal\(%s\)' % (",".join(2 * floatRegex)),
                    'lognormal': r'lognormal\(%s\)' % (",".join(2 * floatRegex)),
                    'normalCapped': r'normalCapped\(%s\)' % (",".join(4 * floatRegex)),
                    'uniform': r'uniform\(%s\)' % (",".join(2 * floatRegex)),
                    'gamma': r'gamma\(%s\)' % (",".join(2 * floatRegex))}

    with open(filePath) as f:
        reader = csv.reader(f, delimiter=';')
        for row in reader:
            attName = None
            lowerLimit = 0
            upperLimit = None
            value = None

            if len(row) >= 2:
                if len(row[0].strip()) > 0:
                    attName = row[0].strip()
                    if attName == "param":
                        # this indicates that a parameter child-element is to be created for the vTypes
                        isParameter = True
                        del row[0]
                        if len(row) < 2:
                            # a parameter needs a name and a value specification
                            continue
                        attName = row[0].strip()
                    else:
                        isParameter = False
                    # check if attribute value matches given distribution
                    # syntax
                    attValue = row[1].strip()
                    distFound = False
                    for distName, distSyntax in distSyntaxes.items():
                        items = re.findall(distSyntax, attValue)
                        distFound = len(items) > 0
                        if distFound:  # found distribution
                            distPar1 = float(items[0][0])
                            distPar2 = float(items[0][2])

                            if distName == 'normal':
                                value = NormalDistribution(distPar1, distPar2)
                            if distName == 'lognormal':
                                value = LogNormalDistribution(distPar1, distPar2)
                            elif distName == 'normalCapped':
                                cutLow = float(items[0][4])
                                cutHigh = float(items[0][6])
                                value = NormalCappedDistribution(distPar1, distPar2, cutLow, cutHigh)
                            elif distName == 'uniform':
                                value = UniformDistribution(distPar1, distPar2)
                            elif distName == 'gamma':
                                value = GammaDistribution(distPar1, distPar2)
                            break

                    if not distFound:
                        if attName == "emissionClass":
                            isNumeric = False
                        else:
                            isNumeric = len(re.findall(r'^(-?[0-9]+(\.[0-9]+)?)$', attValue)) > 0
                        value = FixDistribution((attValue,), isNumeric)

                    # get optional limits
                    if len(row) == 3:
                        limitValue = row[2].strip()
                        items = re.findall(r'\[\s*(-?[0-9]+(\.[0-9]+)?)\s*,\s*(-?[0-9]+(\.[0-9]+)?)\s*\]', limitValue)
                        if len(items) > 0:
                            lowerLimit = float(items[0][0])
                            upperLimit = float(items[0][2])
                    value.setLimits((lowerLimit, upperLimit))
                    value.setMaxSamplingAttempts(options.nrSamplingAttempts)
                    res = {"value": value, "isParameter": isParameter}
                    result[attName] = res
    return result


def main(options):
    if options.seed:
        random.seed(options.seed)
    vTypeParameters = readConfigFile(options)
    useExistingFile = False
    if os.path.exists(options.outputFile):
        try:
            domTree = xml.dom.minidom.parse(options.outputFile)
        except Exception as e:
            sys.exit("Cannot parse existing %s. Error: %s" %
                     (options.outputFile, str(e)))
        useExistingFile = True
    else:
        domTree = xml.dom.minidom.Document()
    vTypeDistNode = domTree.createElement("vTypeDistribution")
    vTypeDistNode.setAttribute("id", options.vehDistName)

    for i in range(0, options.vehicleCount):
        vTypeNode = domTree.createElement("vType")
        vTypeNode.setAttribute("id", options.vehDistName + str(i))
        for attName, d in vTypeParameters.items():
            attValue = d["value"]
            isParameter = d["isParameter"]
            if isParameter:
                paramNode = domTree.createElement("param")
                paramNode.setAttribute("key", attName)
                paramNode.setAttribute("value", attValue.sampleValueString(options.decimalPlaces))
                vTypeNode.appendChild(paramNode)
            else:
                vTypeNode.setAttribute(
                    attName, attValue.sampleValueString(options.decimalPlaces))
        vTypeDistNode.appendChild(vTypeNode)

    existingDistNodes = domTree.getElementsByTagName("vTypeDistribution")
    replaceNode = None
    for existingDistNode in existingDistNodes:
        if existingDistNode.hasAttribute("id") and existingDistNode.getAttribute("id") == options.vehDistName:
            replaceNode = existingDistNode
            break
    if useExistingFile:
        if replaceNode is not None:
            replaceNode.parentNode.replaceChild(vTypeDistNode, replaceNode)
        else:
            domTree.documentElement.appendChild(vTypeDistNode)
    else:
        additionalNode = domTree.createElement("additional")
        additionalNode.setAttribute(
            "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
        additionalNode.setAttribute(
            "xsi:noNamespaceSchemaLocation", "http://sumo.dlr.de/xsd/additional_file.xsd")
        additionalNode.appendChild(vTypeDistNode)
        domTree.appendChild(additionalNode)
    fileHandle = open(options.outputFile, "w")
    domTree.documentElement.writexml(
        fileHandle, addindent="    ", newl="\n")
    fileHandle.close()
    sys.stdout.write("Output written to %s" % options.outputFile)


if __name__ == "__main__":
    options = get_options(sys.argv)
    main(options)
