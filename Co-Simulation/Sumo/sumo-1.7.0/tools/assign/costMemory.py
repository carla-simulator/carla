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

# @file    costMemory.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2012-03-14

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
from collections import defaultdict
from xml.sax import make_parser, handler

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
from sumolib.net import readNet  # noqa


class EdgeMemory:

    def __init__(self, cost):
        self.cost = cost
        self.seen = True

    def update(self, cost, memory_weight, new_weight, pessimism):
        p = (cost / self.cost) ** pessimism if self.cost > 0 else 1
        memory_factor = memory_weight / (memory_weight + new_weight * p)
        self.cost = self.cost * memory_factor + cost * (1 - memory_factor)
        self.seen = True


class CostMemory(handler.ContentHandler):
    # memorize the weighted average of edge costs

    def __init__(self, cost_attribute, pessimism=0, network_file=None):
        # the cost attribute to parse (i.e. 'traveltime')
        self.cost_attribute = cost_attribute.encode('utf8')
        # the duaIterate iteration index
        self.iteration = None
        # the main data store: for every interval and edge id we store costs and
        # whether data was seen in the last call of load_costs()
        # start -> (edge_id -> EdgeMemory)
        self.intervals = defaultdict(dict)
        # the intervall length (only known for certain if multiple intervals
        # have been seen)
        self.interval_length = 214748  # SUMOTIME_MAXSTRING
        # the intervall currently being parsed
        self.current_interval = None
        # the combined weigth of all previously loaded costs
        self.memory_weight = 0.0
        # update is done according to: memory * memory_factor + new * (1 -
        # memory_factor)
        self.memory_factor = None
        # differences between the previously loaded costs and the memorized
        # costs
        self.errors = None
        # some statistics
        self.num_loaded = 0
        self.num_decayed = 0
        # travel times without obstructing traffic
        # XXX could use the minimum known traveltime
        self.traveltime_free = defaultdict(lambda: 0)
        if network_file is not None:
            # build a map of default weights for decaying edges assuming the
            # attribute is traveltime
            self.traveltime_free = dict([(e.getID(), e.getLength() / e.getSpeed())
                                         for e in readNet(network_file).getEdges()])
        self.pessimism = pessimism

    def startElement(self, name, attrs):
        if name == 'interval':
            self.current_interval = self.intervals[float(attrs['begin'])]
        if name == 'edge':
            id = attrs['id']
            # may be missing for some
            if self.cost_attribute.decode('utf-8') in attrs:
                self.num_loaded += 1
                cost = float(attrs[self.cost_attribute.decode('utf-8')])
                if id in self.current_interval:
                    edgeMemory = self.current_interval[id]
                    self.errors.append(edgeMemory.cost - cost)
                    edgeMemory.update(
                        cost, self.memory_weight, self.new_weight, self.pessimism)
                    # if id == "4.3to4.4":
                    #    with open('debuglog', 'a') as f:
                    #        print(self.memory_factor, edgeMemory.cost, file=f)
                else:
                    self.errors.append(0)
                    self.current_interval[id] = EdgeMemory(cost)

    def load_costs(self, dumpfile, iteration, weight):
        # load costs from dumpfile and update memory according to weight and
        # iteration
        if weight <= 0:
            sys.stderr.write(
                "Skipped loading of costs because the weight was %s but should have been > 0\n" % weight)
            return
        assert(weight > 0)
        if self.iteration is None and iteration != 0:
            print("Warning: continuing with empty memory")
        # update memory weights. memory is a weighted average across all runs
        self.new_weight = float(weight)
        self.iteration = iteration
        self.errors = []
        # mark all edges as unseen
        for edges in self.intervals.values():
            for edgeMemory in edges.values():
                edgeMemory.seen = False
        # parse costs
        self.num_loaded = 0
        parser = make_parser()
        parser.setContentHandler(self)
        parser.parse(dumpfile)
        # decay costs of unseen edges
        self.num_decayed = 0
        for edges in self.intervals.values():
            for id, edgeMemory in edges.items():
                if not edgeMemory.seen:
                    edgeMemory.update(
                        self.traveltime_free[id], self.memory_weight, self.new_weight, self.pessimism)
                    self.num_decayed += 1
                    # if id == "4.3to4.4":
                    #    with open('debuglog', 'a') as f:
                    #            print(self.memory_factor, 'decay', edgeMemory.cost, file=f)
        # figure out the interval length
        if len(self.intervals.keys()) > 1:
            sorted_begin_times = sorted(self.intervals.keys())
            self.interval_length = sorted_begin_times[
                1] - sorted_begin_times[0]
        self.memory_weight += self.new_weight

    def write_costs(self, weight_file):
        with open(weight_file, 'w') as f:
            f.write('<netstats>\n')
            for start, edge_costs in self.intervals.items():
                f.write('    <interval begin="%d" end="%d">\n' %
                        (start, start + self.interval_length))
                for id, edgeMemory in edge_costs.items():
                    f.write('        <edge id="%s" %s="%s"/>\n' %
                            (id, self.cost_attribute.decode('utf-8'), edgeMemory.cost))
                f.write('    </interval>\n')
            f.write('</netstats>\n')

    def avg_error(self, values=None):
        if not values:
            values = self.errors
        length = len(list(values))
        if length > 0:
            return (sum(list(values)) / length)
        else:
            return 0

    def avg_abs_error(self):
        return self.avg_error(list(map(abs, self.errors)))

    def mean_error(self, values=None):
        if not values:
            values = self.errors
        values.sort()
        if values:
            return values[len(values) // 2]

    def mean_abs_error(self):
        return self.mean_error(list(map(abs, self.errors)))

    def loaded(self):
        return self.num_loaded

    def decayed(self):
        return self.num_decayed
