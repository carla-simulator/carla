#!/usr/bin/env python
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

# @file    analyzePersonPlans.py
# @author  Jakob Erdmann
# @date    2019-09-13

"""
Count the number of different person plans in a route file
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from argparse import ArgumentParser
from collections import defaultdict

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa


def get_options(args=None):
    parser = ArgumentParser(description="Analyze person plans")
    parser.add_argument("-r", "--route-files", dest="routeFiles", help="Input route files")
    parser.add_argument("-w", "--merge-walks", dest="mergeWalks", action="store_true", help="merge subsequent walks")
    parser.add_argument("-p", "--public-prefixes", dest="public", help="Distinguis public transport modes by prefix")
    parser.add_argument("-i", "--ids", dest="ids", default=0, type=int,
                        help="List the given number of person ids for each type of plan")
    options = parser.parse_args(args=args)
    if options.routeFiles is None:
        parser.print_help()
        sys.exit()
    if options.public:
        options.public = options.public.split(',')
    else:
        options.public = []
    return options


def stageName(options, person, stage):
    if stage.name == 'ride':
        if stage.lines.startswith(person.id):
            return 'car'
        else:
            if stage.intended is not None:
                for pType in options.public:
                    if pType in stage.intended:
                        return pType
            return 'public'
    else:
        return stage.name


def main(options):
    counts = defaultdict(lambda: list())
    for routeFile in options.routeFiles.split(','):
        for person in sumolib.xml.parse(routeFile, 'person'):
            stages = tuple([stageName(options, person, s) for s in person.getChildList()])
            if options.mergeWalks:
                filtered = []
                for s in stages:
                    if len(filtered) == 0 or s != filtered[-1]:
                        filtered.append(s)
                stages = tuple(filtered)
            counts[stages].append(person.id)

    numPersons = sum(map(len, counts.values()))
    print("Loaded %s persons" % numPersons)
    maxPadding = max(map(len, [' '.join(k) for k, v in counts.items()]))
    countSize = len(str(max([len(p) for k, p in counts.items()])))
    formatStr = "%" + str(countSize) + "s: %s%s"
    reverseCounts = [(len(p), k) for k, p in counts.items()]
    for count, k in sorted(reverseCounts):
        stages = ' '.join(k)
        examples = ""
        if options.ids > 0:
            padding = " " * (maxPadding - len(stages))
            examples = padding + " - " + ','.join(counts[k][:options.ids])
        print(formatStr % (count, stages, examples))


if __name__ == "__main__":
    main(get_options())
