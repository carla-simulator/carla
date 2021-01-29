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

# @file    plot_summary.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @date    2013-11-11

"""

This script plots a selected measure from a summary-output.
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


def readValues(files, verbose, measure):
    ret = {}
    for f in files:
        if verbose:
            print("Reading '%s'..." % f)
        ret[f] = sumolib.output.parse_sax__asList(f, "step", [measure])
    return ret


def main(args=None):
    """The main function; parses options and plots"""
    # ---------- build and read options ----------
    from optparse import OptionParser
    optParser = OptionParser()
    optParser.add_option("-i", "--summary-inputs", dest="summary", metavar="FILE",
                         help="Defines the summary-output files to use as input")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="If set, the script says what it's doing")
    optParser.add_option("-m", "--measure", dest="measure",
                         default="running", help="Define which measure to plot")
    # standard plot options
    helpers.addInteractionOptions(optParser)
    helpers.addPlotOptions(optParser)
    # parse
    options, _ = optParser.parse_args(args=args)

    if options.summary is None:
        print("Error: at least one summary file must be given")
        sys.exit(1)

    minV = 0
    maxV = 0
    files = options.summary.split(",")
    nums = readValues(files, options.verbose, options.measure)
    times = readValues(files, options.verbose, "time")
    for f in files:
        maxV = max(maxV, len(nums[f]))
    range(minV, maxV + 1)

    fig, ax = helpers.openFigure(options)
    for i, f in enumerate(files):
        v = sumolib.output.toList(nums[f], options.measure)
        t = sumolib.output.toList(times[f], "time")
        c = helpers.getColor(options, i, len(files))
        plt.plot(t, v, label=helpers.getLabel(f, i, options), color=c)
    helpers.closeFigure(fig, ax, options)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
