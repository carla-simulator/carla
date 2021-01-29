#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    plot_csv_bars.py
# @author  Daniel Krajzewicz
# @date    2014-01-27

"""

This script plots name / value pairs from a given .csv file (';'-separated).
The values are plotted as bars.
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
    optParser.add_option("-c", "--column", dest="column",
                         type="int", default=1, help="Selects the column to read values from")
    optParser.add_option("-r", "--revert", dest="revert", action="store_true",
                         default=False, help="Reverts the order of read values")
    optParser.add_option("-w", "--width", dest="width",
                         type="float", default=.8, help="Defines the width of the bars")
    optParser.add_option("--space", dest="space",
                         type="float", default=.2, help="Defines the space between the bars")
    optParser.add_option("--norm", dest="norm",
                         type="float", default=1., help="Divides the read numbers by this value before plotting them")
    optParser.add_option("--show-values", dest="showValues", action="store_true",
                         default=False, help="Shows the values")
    optParser.add_option("--values-offset", dest="valuesOffset",
                         type="float", default=1., help="Position offset for values")
    optParser.add_option("--vertical", dest="vertical", action="store_true",
                         default=False, help="vertical bars are used")
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
    vlabels = []
    vals = []
    total = 0
    xs = []
    ts = []
    s = options.width + options.space
    t = options.width / 2. + options.space / 2.
    x = options.space / 2.
    for line in fd:
        v = line.strip().split(";")
        if len(v) < 2:
            continue
        labels.append(v[0].replace("\\n", "\n"))
        value = float(v[options.column]) / options.norm
        vals.append(value)
        vlabels.append(str(value) + "%")
        total += value
        xs.append(x)
        ts.append(t)
        x = x + s
        t = t + s

    if options.revert:
        labels.reverse()
        vals.reverse()
        vlabels.reverse()
    colors = []
    for i, e in enumerate(labels):
        colors.append(helpers.getColor(options, i, len(labels)))

    fig, ax = helpers.openFigure(options)
    if not options.vertical:
        rects = plt.barh(xs, vals, height=options.width)
        for i, rect in enumerate(rects):
            if options.showValues:
                width = rect.get_width()
                ax.text(width + options.valuesOffset, rect.get_y() +
                        rect.get_height() / 2., vlabels[i], va='center', ha='left')
            rect.set_color(colors[i])
            rect.set_edgecolor('k')
        plt.ylim(0, x)
        plt.yticks(ts, labels)
    else:
        rects = plt.bar(xs, vals, width=options.width)
        for i, rect in enumerate(rects):
            if options.showValues:
                height = rect.get_height()
                ax.text(rect.get_x() + rect.get_width() / 2., height +
                        options.valuesOffset, vlabels[i], ha='center', va='bottom')
            rect.set_color(colors[i])
            rect.set_edgecolor('k')
        plt.xlim(0, x)
        plt.xticks(ts, labels)
    helpers.closeFigure(fig, ax, options, False)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
