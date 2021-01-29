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

# @file    plot_net_selection.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2014-02-19

from __future__ import absolute_import
from __future__ import print_function

import os
import sys

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
import sumolib  # noqa
from sumolib.visualization import helpers  # noqa


def main(args=None):
    """The main function; parses options and plots"""
    # ---------- build and read options ----------
    from optparse import OptionParser
    optParser = OptionParser()
    optParser.add_option("-n", "--net", dest="net", metavar="FILE",
                         help="Defines the network to read")
    optParser.add_option("-i", "--selection", dest="selection", metavar="FILE",
                         help="Defines the selection to read")
    optParser.add_option("--selected-width", dest="selectedWidth",
                         type="float", default=1, help="Defines the width of selected edges")
    optParser.add_option("--color", "--selected-color", dest="selectedColor",
                         default='r', help="Defines the color of selected edges")
    optParser.add_option("--edge-width", dest="defaultWidth",
                         type="float", default=.2, help="Defines the width of not selected edges")
    optParser.add_option("--edge-color", dest="defaultColor",
                         default='#606060', help="Defines the color of not selected edges")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="If set, the script says what it's doing")
    # standard plot options
    helpers.addInteractionOptions(optParser)
    helpers.addPlotOptions(optParser)
    # parse
    options, remaining_args = optParser.parse_args(args=args)

    if options.net is None:
        print("Error: a network to load must be given.")
        return 1
    if options.selection is None:
        print("Error: a selection to load must be given.")
        return 1
    if options.verbose:
        print("Reading network from '%s'" % options.net)
    net = sumolib.net.readNet(options.net)
    selection = sumolib.files.selection.read(options.selection)

    colors = {}
    widths = {}
    for e in selection["edge"]:
        colors[e] = options.selectedColor
        widths[e] = options.selectedWidth

    fig, ax = helpers.openFigure(options)
    ax.set_aspect("equal", None, 'C')
    helpers.plotNet(net, colors, widths, options)
    options.nolegend = True
    helpers.closeFigure(fig, ax, options)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
