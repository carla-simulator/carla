#!/usr/bin/env python
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

# @file    plot_csv_pie.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @date    2014-01-15

"""

This script plots name / value pairs from a given .csv file (';'-separated).
The values are plotted as a pie diagram.
matplotlib (http://matplotlib.org/) has to be installed for this purpose

"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
import sumolib  # noqa
from sumolib.visualization import helpers  # noqa
import matplotlib.pyplot as plt  # noqa


def main(args=None):
    """The main function; parses options and plots"""
    # ---------- build and read options ----------
    from optparse import OptionParser
    optParser = OptionParser()
    optParser.add_option("-i", "--input", dest="input", metavar="FILE",
                         help="Defines the csv file to use as input")
    optParser.add_option("-p", "--percentage", dest="percentage", action="store_true",
                         default=False, help="Interprets read measures as percentages")
    optParser.add_option("-r", "--revert", dest="revert", action="store_true",
                         default=False, help="Reverts the order of read values")
    optParser.add_option("--no-labels", dest="nolabels", action="store_true",
                         default=False, help="Does not plot the labels")
    optParser.add_option("--shadow", dest="shadow", action="store_true",
                         default=False, help="Puts a shadow below the circle")
    optParser.add_option("--startangle", dest="startangle",
                         type="float", default=0, help="Sets the start angle")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="If set, the script says what it's doing")
    # standard plot options
    helpers.addInteractionOptions(optParser)
    helpers.addPlotOptions(optParser)
    # parse
    options, remaining_args = optParser.parse_args(args=args)

    if options.input is None:
        print("Error: at least one csv file must be given")
        sys.exit(1)

    fd = open(options.input)
    labels = []
    vals = []
    total = 0
    for line in fd:
        v = line.strip().split(";")
        if len(v) < 2:
            continue
        labels.append(v[0].replace("\\n", "\n"))
        vals.append(float(v[1]))
        total += float(v[1])

    if options.revert:
        labels.reverse()
        vals.reverse()
    colors = []
    for i, e in enumerate(labels):
        colors.append(helpers.getColor(options, i, len(labels)))

    fig, ax = helpers.openFigure(options)
    if options.nolabels:
        labels = None
    shadow = options.shadow
    if options.percentage:
        def autopct(p):
            return '{:.1f}%'.format(p)
        # autopct = lambda p: '{:.1f}%'.format(p)
    else:
        def autopct(p):
            return '{:.0f}'.format(p * total / 100)
        # autopct = lambda p: '{:.0f}'.format(p * total / 100)
    patches, texts, autotexts = plt.pie(
        vals, labels=labels, autopct=autopct, colors=colors, shadow=shadow, startangle=options.startangle)
    helpers.closeFigure(fig, ax, options)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
