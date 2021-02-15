#!/usr/bin/env python
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

# @file    plot_tripinfo_distributions.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @date    2013-11-11

"""
This script plots measures from the tripinfo output, classified into bins
matplotlib (http://matplotlib.org/) has to be installed for this purpose
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys

sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa
from sumolib.visualization import helpers  # noqa
import matplotlib.pyplot as plt  # noqa


def main(args=None):
    """The main function; parses options and plots"""
    # ---------- build and read options ----------
    from optparse import OptionParser
    optParser = OptionParser()
    optParser.add_option("-i", "--tripinfos-inputs", dest="tripinfos", metavar="FILE",
                         help="Defines the tripinfo-output files to use as input")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="If set, the script says what it's doing")
    optParser.add_option("-m", "--measure", dest="measure",
                         default="duration", help="Define which measure to plot")
    optParser.add_option("--bins", dest="bins",
                         type="int", default=20, help="Define the bin number")
    optParser.add_option("--norm", dest="norm",
                         type="float", default=1., help="Read values will be devided by this number")
    optParser.add_option("--minV", dest="minV",
                         type="float", default=None, help="Define the minimum value boundary")
    optParser.add_option("--maxV", dest="maxV",
                         type="float", default=None, help="Define the maximum value boundary")
    # standard plot options
    helpers.addInteractionOptions(optParser)
    helpers.addPlotOptions(optParser)
    # parse
    options, _ = optParser.parse_args(args=args)

    if options.tripinfos is None:
        print("Error: at least one tripinfo file must be given")
        sys.exit(1)

    minV = options.minV
    maxV = options.maxV
    files = options.tripinfos.split(",")
    values = {}
    for f in files:
        if options.verbose:
            print("Reading '%s'..." % f)
        nums = sumolib.output.parse_sax__asList(
            f, "tripinfo", [options.measure])
        fvp = sumolib.output.toList(nums, options.measure)
        fv = [x / options.norm for x in fvp]
        sumolib.output.prune(fv, options.minV, options.maxV)

        values[f] = fv
        if minV is None:
            minV = fv[0]
            maxV = fv[0]
        minV = min(minV, min(fv))
        maxV = max(maxV, max(fv))

    hists = {}
    binWidth = (maxV - minV) / float(options.bins)
    for f in files:
        h = [0] * options.bins
        for v in values[f]:
            i = min(int((v - minV) / binWidth), options.bins - 1)
            h[i] = h[i] + 1
        hists[f] = h

    width = binWidth / float(len(files)) * .8
    offset = binWidth * .1
    center = []
    for j in range(0, options.bins):
        center.append(binWidth * j + offset)

    fig, ax = helpers.openFigure(options)
    for i, f in enumerate(files):
        c = helpers.getColor(options, i, len(files))
        plt.bar(center, hists[f], width=width, label=helpers.getLabel(f, i, options), color=c)
        for j in range(options.bins):
            center[j] += width
    helpers.closeFigure(fig, ax, options)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
