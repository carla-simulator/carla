#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    netcheck.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Jakob Erdmann
# @author  Greg Albiston
# @date    2007-03-20

"""
This script does simple check for the network.
It tests whether the network is (weakly) connected.
It needs one parameter, the SUMO net (.net.xml).
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from optparse import OptionParser
from itertools import chain
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " <net> <options>"
    optParser = OptionParser()
    optParser.add_option("-s", "--source",
                         default=False, help="List edges reachable from the source")
    optParser.add_option("-d", "--destination",
                         default=False, help="List edges which can reach the destination")
    optParser.add_option("-o", "--selection-output",
                         help="Write output to file(s) as a loadable selection")
    optParser.add_option("--ignore-connections", action="store_true",
                         default=False,
                         help="Assume full connectivity at each node when computing all connected components")
    optParser.add_option("-l", "--vclass", help="Include only edges allowing VCLASS")
    optParser.add_option("-c", "--component-output",
                         default=None, help="Write components of disconnected network to file - not compatible " +
                                            "with --source or --destination options")
    optParser.add_option("-r", "--results-output",
                         default=None, help="Write results summary of disconnected network to file - not compatible " +
                         "with --source or --destination options")

    options, args = optParser.parse_args()
    if len(args) != 1:
        sys.exit(USAGE)
    options.net = args[0]
    return options


def getWeaklyConnected(net, vclass=None, ignore_connections=False):
    components = []
    edgesLeft = set(net.getEdges())
    queue = list()
    while len(edgesLeft) != 0:
        component = set()
        queue.append(edgesLeft.pop())
        while not len(queue) == 0:
            edge = queue.pop(0)
            if vclass is None or edge.allows(vclass):
                component.add(edge.getID())
                if ignore_connections:
                    for n in (edge.getFromNode().getOutgoing() +
                              edge.getFromNode().getIncoming() +
                              edge.getToNode().getOutgoing() +
                              edge.getToNode().getIncoming()):
                        if n in edgesLeft:
                            queue.append(n)
                            edgesLeft.remove(n)
                else:
                    for n in edge.getOutgoing():
                        if n in edgesLeft:
                            queue.append(n)
                            edgesLeft.remove(n)
                    for n in edge.getIncoming():
                        if n in edgesLeft:
                            queue.append(n)
                            edgesLeft.remove(n)
        if component:
            components.append(sorted(component))
    return components


def getReachable(net, source_id, options, useIncoming=False):
    if not net.hasEdge(source_id):
        sys.exit("'{}' is not a valid edge id".format(source_id))
    source = net.getEdge(source_id)
    if options.vclass is not None and not source.allows(options.vclass):
        sys.exit("'{}' does not allow {}".format(source_id, options.vclass))
    fringe = [source]
    found = set()
    found.add(source)
    while len(fringe) > 0:
        new_fringe = []
        for edge in fringe:
            if options.vclass == "pedestrian":
                cands = chain(chain(*edge.getIncoming().values()), chain(*edge.getOutgoing().values()))
            else:
                cands = chain(*(edge.getIncoming().values() if useIncoming else edge.getOutgoing().values()))
            # print("\n".join(map(str, list(cands))))
            for conn in cands:
                if options.vclass is None or (
                        conn.getFromLane().allows(options.vclass)
                        and conn.getToLane().allows(options.vclass)):
                    for reachable in [conn.getTo(), conn.getFrom()]:
                        if reachable not in found:
                            # print("added %s via %s" % (reachable, conn))
                            found.add(reachable)
                            new_fringe.append(reachable)
        fringe = new_fringe

    if useIncoming:
        print("{} of {} edges can reach edge '{}':".format(
            len(found), len(net.getEdges()), source_id))
    else:
        print("{} of {} edges are reachable from edge '{}':".format(
            len(found), len(net.getEdges()), source_id))

    ids = sorted([e.getID() for e in found])
    if options.selection_output:
        with open(options.selection_output, 'w') as f:
            for e in ids:
                f.write("edge:{}\n".format(e))
    else:
        print(ids)


if __name__ == "__main__":
    options = parse_args()

    net = sumolib.net.readNet(options.net,
                              withInternal=(options.vclass == "pedestrian"),
                              withPedestrianConnections=(options.vclass == "pedestrian"))
    if options.source:
        getReachable(net, options.source, options)
    elif options.destination:
        getReachable(net, options.destination, options, True)
    else:
        components = getWeaklyConnected(
            net, options.vclass, options.ignore_connections)
        if len(components) != 1:
            print("Warning! Net is not connected.")

        total = 0
        max = 0
        max_idx = ""
        # Stores the distribution of components by edge counts - key: edge
        # counts - value: number found
        edge_count_dist = {}
        output_str_list = []
        dist_str_list = []

        # Iterate through components to output and summarise
        for idx, comp in enumerate(sorted(components, key=lambda c: next(iter(c)))):
            if options.selection_output:
                with open("{}comp{}.txt".format(options.selection_output, idx), 'w') as f:
                    for e in comp:
                        f.write("edge:{}\n".format(e))

            edge_count = len(comp)
            total += edge_count
            if edge_count > max:
                max = edge_count
                max_idx = idx

            if edge_count not in edge_count_dist:
                edge_count_dist[edge_count] = 0
            edge_count_dist[edge_count] += 1
            output_str = "Component: #{} Edge Count: {}\n {}\n".format(
                idx, edge_count, " ".join(comp))
            print(output_str)
            output_str_list.append(output_str)

        # Output the summary of all edges checked and largest component
        # To avoid divide by zero error if total is 0 for some reason.
        coverage = 0.0
        if total > 0:
            coverage = round(max * 100.0 / total, 2)
        summary_str = "Total Edges: {}\nLargest Component: #{} Edge Count: {} Coverage: {}%\n".format(
            total, max_idx, max, coverage)
        print(summary_str)
        dist_str = "Edges\tIncidence"
        print(dist_str)
        dist_str_list.append(dist_str)

        # Output the distribution of components by edge counts
        for key, value in sorted(edge_count_dist.items()):
            dist_str = "{}\t{}".format(key, value)
            print(dist_str)
            dist_str_list.append(dist_str)

        # Check for output of components to file
        if options.component_output is not None:
            print("Writing component output to: {}".format(
                options.component_output))
            with open(options.component_output, 'w') as f:
                f.write("\n".join(output_str_list))

        # Check for output of results summary to file
        if options.results_output is not None:
            print(
                "Writing results output to: {}".format(options.results_output))
            with open(options.results_output, 'w') as r:
                r.write(summary_str)
                r.write("\n".join(dist_str_list))
