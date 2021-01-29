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

# @file    plot_net_dump.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Robert Hilbrich
# @date    2016-08-05

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from xml.sax.handler import ContentHandler

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

import sumolib  # noqa
from sumolib.visualization import helpers  # noqa
import matplotlib.pyplot as plt  # noqa
import matplotlib  # noqa


class WeightsReader(ContentHandler):

    """Reads the dump file"""

    def __init__(self, value):
        self._edge2value = {}
        self._value = value
        self._intervals = []

    def startElement(self, name, attrs):
        if name == 'interval':
            self._time = float(attrs['begin'])
            self._edge2value[self._time] = {}
            self._intervals.append(self._time)

        elif name == 'edge':
            id = attrs['id']
            if self._value in attrs:
                self._edge2value[self._time][id] = float(attrs[self._value])


def main(args=None):
    """The main function; parses options and plots"""
    # ---------- build and read options ----------
    from optparse import OptionParser
    optParser = OptionParser()
    optParser.add_option("-n", "--net", dest="net", metavar="FILE",
                         help="Defines the network to read")
    optParser.add_option("-i", "--dump-inputs", dest="dumps", metavar="FILE",
                         help="Defines the dump-output files to use as input")
    optParser.add_option("-m", "--measures", dest="measures",
                         default="speed,entered", help="Define which measure to plot")
    optParser.add_option("--min-width", dest="minWidth",
                         type="float", default=.5, help="Defines the minimum edge width")
    optParser.add_option("--max-width", dest="maxWidth",
                         type="float", default=3, help="Defines the maximum edge width")
    optParser.add_option("--log-colors", dest="logColors", action="store_true",
                         default=False, help="If set, colors are log-scaled")
    optParser.add_option("--log-widths", dest="logWidths", action="store_true",
                         default=False, help="If set, widths are log-scaled")
    optParser.add_option("--min-color-value", dest="colorMin",
                         type="float", default=None,
                         help="If set, defines the minimum edge color value")
    optParser.add_option("--max-color-value", dest="colorMax",
                         type="float", default=None,
                         help="If set, defines the maximum edge color value")
    optParser.add_option("--min-width-value", dest="widthMin",
                         type="float", default=None,
                         help="If set, defines the minimum edge width value")
    optParser.add_option("--max-width-value", dest="widthMax",
                         type="float", default=None,
                         help="If set, defines the maximum edge width value")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False,
                         help="If set, the script says what it's doing")
    optParser.add_option("--color-bar-label", dest="colorBarLabel", default="",
                         help="The label to put on the color bar")

    # standard plot options
    helpers.addInteractionOptions(optParser)
    helpers.addPlotOptions(optParser)
    helpers.addNetOptions(optParser)

    # Override the help string for the output option
    outputOpt = optParser.get_option("--output")
    outputOpt.help = ("Comma separated list of filename(s) the figure shall be written to; " +
                      "for multiple time intervals use '%s' in the filename as a " +
                      "placeholder for the beginning of the time interval")

    # parse
    options, _ = optParser.parse_args(args=args)

    if options.net is None:
        print("Error: a network to load must be given.")
        return 1
    if options.verbose:
        print("Reading network from '%s'" % options.net)
    net = sumolib.net.readNet(options.net)

    if options.measures is None:
        print("Error: a dump file must be given.")
        return 1

    dumps = options.dumps.split(",")
    times = []
    hc = None
    colorDump = dumps[0]
    colorMeasure = options.measures.split(",")[0]
    if colorDump:
        if options.verbose:
            print("Reading colors from '%s'" % colorDump)
        hc = WeightsReader(colorMeasure)
        sumolib.output.parse_sax(colorDump, hc)
        times = hc._edge2value

    hw = None
    if len(dumps) > 1:
        widthDump = dumps[1]
        widthMeasure = options.measures.split(",")[1]
        if options.verbose:
            print("Reading widths from '%s'" % widthDump)
        hw = WeightsReader(widthMeasure)
        sumolib.output.parse_sax(widthDump, hw)
        times = hw._edge2value

    # Should we also save the figure to a file / list of files (comma
    # separated)? Then we need to check the output filename(s)
    if options.output:
        options.nolegend = True
        optOutputNames = options.output

        # If we have multiple intervals to be plotted, make sure we have
        # proper output filenames (with a %s as a placeholder in it)
        if len(times) > 1 and optOutputNames.find('%s') < 0:
            print('Warning: multiple time intervals detected, but ' +
                  'the output filename(s) do not contain a \'%s\' placeholder. ' +
                  'Continuing by using a default placeholder.')

            # Modify each filename by putting a '-%s' right before the
            # extension
            filenames = optOutputNames.split(',')
            for i in range(0, len(filenames)):
                filename, extension = os.path.splitext(filenames[i])
                filenames[i] = filename + '-%s' + extension
            optOutputNames = ','.join(filenames)

    # Now go through each time interval and create the figures
    for t in times:
        if options.verbose:
            print("Processing interval with a beginning of %s" % t)
        colors = {}
        maxColorValue = None
        minColorValue = None
        for e in net._id2edge:
            if hc and t in hc._edge2value and e in hc._edge2value[t]:
                if options.colorMax is not None and hc._edge2value[t][e] > options.colorMax:
                    hc._edge2value[t][e] = options.colorMax
                if options.colorMin is not None and hc._edge2value[t][e] < options.colorMin:
                    hc._edge2value[t][e] = options.colorMin
                if maxColorValue is None or maxColorValue < hc._edge2value[t][e]:
                    maxColorValue = hc._edge2value[t][e]
                if minColorValue is None or minColorValue > hc._edge2value[t][e]:
                    minColorValue = hc._edge2value[t][e]
                colors[e] = hc._edge2value[t][e]
        if options.colorMax is not None:
            maxColorValue = options.colorMax
        if options.colorMin is not None:
            minColorValue = options.colorMin
        if options.logColors:
            helpers.logNormalise(colors, maxColorValue)
        else:
            helpers.linNormalise(colors, minColorValue, maxColorValue)
        for e in colors:
            colors[e] = helpers.getColor(options, colors[e], 1.)
        if options.verbose:
            print("Color values are between %s and %s" %
                  (minColorValue, maxColorValue))

        widths = {}
        maxWidthValue = None
        minWidthValue = None
        for e in net._id2edge:
            if hw and t in hw._edge2value and e in hw._edge2value[t]:
                v = abs(hw._edge2value[t][e])
                if options.widthMax is not None and v > options.widthMax:
                    v = options.widthMax
                if options.widthMin is not None and v < options.widthMin:
                    v = options.widthMin
                if maxWidthValue is None or maxWidthValue < v:
                    maxWidthValue = v
                if minWidthValue is None or minWidthValue > v:
                    minWidthValue = v
                widths[e] = v
        if options.widthMax is not None:
            maxWidthValue = options.widthMax
        if options.widthMin is not None:
            minWidthValue = options.widthMin
        if options.logWidths:
            helpers.logNormalise(widths, options.colorMax)
        else:
            helpers.linNormalise(widths, minWidthValue, maxWidthValue)
        for e in widths:
            widths[e] = options.minWidth + widths[e] * \
                (options.maxWidth - options.minWidth)
        if options.verbose:
            print("Width values are between %s and %s" %
                  (minWidthValue, maxWidthValue))

        fig, ax = helpers.openFigure(options)
        ax.set_aspect("equal", None, 'C')
        helpers.plotNet(net, colors, widths, options)

        # drawing the legend, at least for the colors
        norm = matplotlib.colors.LogNorm if options.logColors else matplotlib.colors.Normalize
        sm = plt.cm.ScalarMappable(cmap=matplotlib.cm.get_cmap(options.colormap),
                                   norm=norm(vmin=minColorValue, vmax=maxColorValue))

        # "fake up the array of the scalar mappable. Urgh..."
        # (pelson, http://stackoverflow.com/questions/8342549/matplotlib-add-colorbar-to-a-sequence-of-line-plots)
        sm._A = []
        color_bar = plt.colorbar(sm)
        color_bar.set_label(options.colorBarLabel)

        # Should we also save the figure to a file / list of files (comma
        # separated)?
        if options.output:

            # If we have a "%s" in the name of the output then replace it with the
            # interval begin of the current interval
            expandedOutputNames = optOutputNames
            if expandedOutputNames.find('%s') >= 0:
                expandedOutputNames = expandedOutputNames.replace("%s", str(t))

            # Can be used to print additional text in the figure:
            #
            # m, s = divmod(int(t), 60)
            # h, m = divmod(m, 60)
            # timeStr = "%02d:%02d:%02d" % (h, m, s)
            # ax.text(0.2, 0.2, timeStr, bbox={
            #    'facecolor': 'white', 'pad': 10}, size=16)
            helpers.closeFigure(fig, ax, options, False, expandedOutputNames)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
