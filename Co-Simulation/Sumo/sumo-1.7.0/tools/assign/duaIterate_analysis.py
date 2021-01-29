#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

# @file    duaIterate_analysis.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2012-09-06

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import re
import glob
from optparse import OptionParser
from collections import defaultdict
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from sumolib.miscutils import Statistics, uMax  # noqa
from sumolib.output import parse_fast  # noqa


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " <dua-log.txt> [options]"
    optParser = OptionParser()
    optParser.add_option("--stdout",
                         help="also parse the given FILE containing stdout of duaIterate")
    optParser.add_option("-o", "--output", default="plot",
                         help="output prefix for plotting with gnuplot")
    optParser.add_option("-l", "--label-size", default=40, dest="label_size",
                         help="limit length of the plot label to this size")
    optParser.add_option("--limit", type=int,  default=uMax,
                         help="only parse the first INT number of iterations")
    optParser.add_option("--teleports", default="teleplot",
                         help="output prefix for plotting teleport-prone edges")
    optParser.add_option(
        "--mpl", help="output prefix for matplotlib plots or SHOW for plotting to the display")
    options, args = optParser.parse_args()
    if len(args) != 1:
        sys.exit(USAGE)
    options.dualog = args[0]
    return options


def parse_dualog(dualog, limit):
    print("Parsing %s" % dualog)
    teleStats = Statistics('Teleports')
    header = ['#Inserted', 'Running', 'Waiting', 'Teleports', 'Loaded']
    step_values = []  # list of lists
    step_counts = []  # list of edge teleport counters
    reInserted = re.compile(r"Inserted: (\d*)")
    reLoaded = re.compile(r"Loaded: (\d*)")
    reRunning = re.compile(r"Running: (\d*)")
    reWaiting = re.compile(r"Waiting: (\d*)")
    reFrom = re.compile("from '([^']*)'")  # mesosim
    teleports = 0
    inserted = None
    loaded = None
    running = None
    waiting = None
    haveMicrosim = None
    counts = defaultdict(lambda: 0)
    for line in open(dualog):
        try:
            if "Warning: Teleporting vehicle" in line:
                if haveMicrosim is None:
                    if "lane='" in line:
                        haveMicrosim = True
                        reFrom = re.compile("lane='([^']*)'")
                    else:
                        haveMicrosim = False
                teleports += 1
                edge = reFrom.search(line).group(1)
                if ':' in edge:  # mesosim output
                    edge = edge.split(':')[0]
                counts[edge] += 1
            elif "Inserted:" in line:
                inserted = reInserted.search(line).group(1)
                if "Loaded:" in line:  # optional output
                    loaded = reLoaded.search(line).group(1)
                else:
                    loaded = inserted
            elif "Running:" in line:
                running = reRunning.search(line).group(1)
            elif "Waiting:" in line:
                iteration = len(step_values)
                if iteration > limit:
                    break
                waiting = reWaiting.search(line).group(1)
                teleStats.add(teleports, iteration)
                step_values.append(
                    [inserted, running, waiting, teleports, loaded])
                teleports = 0
                step_counts.append(counts)
                counts = defaultdict(lambda: 0)
        except Exception:
            sys.exit("error when parsing line '%s'" % line)

    print("  parsed %s steps" % len(step_values))
    print(teleStats)
    return [header] + step_values, step_counts


def parse_stdout(step_values, stdout):
    print("Parsing %s" % stdout)
    step_values[0] += ['routingMinutes', 'simMinutes', 'absAvgError']
    reDuration = re.compile("Duration: (.*)$")
    reError = re.compile(r"Absolute Error avg:(\d*)")

    def get_minutes(line):
        hours, minutes, seconds = reDuration.search(line).group(1).split(':')
        return int(hours) * 60 + int(minutes) + float(seconds) / 60
    step = 1
    routingMinutes = None
    simMinutes = None
    error = None
    for line in open(stdout):
        if "Duration" in line:
            if routingMinutes is None:
                routingMinutes = get_minutes(line)
            else:
                simMinutes = get_minutes(line)
        elif "Absolute" in line:
            error = reError.search(line).group(1)
            step_values[step] += [routingMinutes, simMinutes, error]
            step += 1
            if step >= len(step_values):
                break
            routingMinutes = None
    print("  parsed %s steps" % (step - 1))


def gnuplot_teleport_edges(plotfile, step_counts, xlabel):
    datafile = plotfile + '.data'
    # an edge is interesting if a large proportion of teleports happen on it
    interestingness = defaultdict(lambda: 0)
    for counts in step_counts:
        teleports = float(sum(counts.values()))
        if teleports == 0:
            continue
        for edge, count in counts.items():
            interestingness[edge] += count / teleports
    interesting = sorted([(c, e) for e, c in interestingness.items()])[-7:]
    print("most interesting edges:", interesting)
    if len(interesting) > 0:
        interesting = [e for c, e in interesting]
        with open(datafile, 'w') as f:
            print('#' + ' '.join(interesting), file=f)
            for counts in step_counts:
                values = [counts[e] for e in interesting]
                f.write(' '.join(map(str, values)) + '\n')
        # write plotfile
        with open(plotfile, 'w') as f:
            f.write("set xlabel '%s'\nplot \\\n" % xlabel)
            lines = ["'%s' using 0:%s title '%s' with lines" % (datafile, ii + 1, edge)
                     for ii, edge in enumerate(interesting)]
            f.write(', \\\n'.join(lines))


def parse_trip_durations():
    result = []
    for file in sorted(glob.glob("tripinfo_*.xml")):
        result.append([float(t.duration)
                       for t in parse_fast(file, 'tripinfo', ['duration'])])
    return result


def matplot(output):
    if output is not None:
        import matplotlib
        if output != 'SHOW':
            matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        plt.boxplot(parse_trip_durations())
        if output == 'SHOW':
            plt.show()
        else:
            plt.savefig(output)
            plt.close()


def gnuplot_vehicle_summary(plotfile, xlabel, step_values):
    datafile = plotfile + '.data'
    with open(datafile, 'w') as f:
        for values in step_values:
            f.write(' '.join(map(str, values)) + '\n')

    with open(plotfile, 'w') as f:
        f.write("""
set xlabel '%s'
plot \\
'%s' using 0:1 title 'inserted' with lines, \\
'%s' using 0:4 title 'teleports' with lines, \\
'%s' using 0:3 title 'waiting' with lines, \\
'%s' using 0:5 title 'loaded' with lines, \\
'%s' using 0:2 title 'running' with lines
""" % ((xlabel,) + (datafile,) * 5))


def main():
    options = parse_args()
    step_values, step_counts = parse_dualog(options.dualog, options.limit)
    if options.stdout is not None:
        parse_stdout(step_values, options.stdout)
    duaPath = os.path.dirname(
        os.path.abspath(options.dualog))[-options.label_size:]
    xlabel = 'Iterations in ' + duaPath

    gnuplot_vehicle_summary(options.output, xlabel, step_values)
    gnuplot_teleport_edges(options.teleports, step_counts, xlabel)
    matplot(options.mpl)


##################
if __name__ == "__main__":
    main()
