#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    tls_buildTransitions.py
# @author  Jakob Erdmann
# @date    2019-11-09

"""
Create tlLogiic definitions with branching signal plans based on a simplified
input: named green phases and list of successor green phases names
The corresponding yellow and red phases will be build and the 'next' attribute
will be set to the appropriate transition phase.
"""

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import argparse
from collections import defaultdict
sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa


def get_options(args=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--description-file", dest="logic",
                        help="Input logic description file (mandatory)")
    parser.add_argument("-o", "--output-file", dest="out",
                        help="Output route file (mandatory)")
    parser.add_argument("-y", "--yellow-time", type=int, default=3, dest="yellowTime",
                        help="Duration of yellow phase")
    parser.add_argument("-r", "--red-time", type=int, default=0, dest="redTime",
                        help="Duration of all-red phase durating each transition")

    options = parser.parse_args()

    if options.logic is None or options.out is None:
        parser.print_help()
        sys.exit()

    return options


def main(options):
    with open(options.out, 'w') as outf:
        outf.write('<add>\n')
        for logic in sumolib.xml.parse(options.logic, 'tlLogic'):
            transitions = []
            phases = list(enumerate(logic.phase))
            index = len(phases)
            targets = defaultdict(lambda: [])
            for i, p in phases:
                nextNames = p.next.split()
                targets[i] = [-1] * len(nextNames)
                for i2, p2 in phases:
                    if i != i2:
                        state = ''
                        for old, new in zip(p.state, p2.state):
                            if old == 'r':
                                state += 'r'
                            elif (old == 'g' or old == 'G') and new == 'r':
                                state += 'y'
                            else:
                                state += old

                        name = "%s-%s" % (p.attr_name, p2.attr_name)
                        for iNext, nextName in enumerate(nextNames):
                            nextIndex = i2  # go directly to target phase
                            if options.redTime > 0:
                                nextIndex = index + 1  # go to red phase
                            if p2.attr_name in nextName:
                                transitions.append((options.yellowTime, state, nextIndex, name))
                                targets[i][iNext] = index
                                index += 1
                                if options.redTime > 0:
                                    transitions.append((options.redTime, 'r' * len(state), i2, name))
                                    index += 1
                                break

            for i, phase in enumerate(logic.phase):
                phase.next = ' '.join(map(str, targets[i]))

            for duration, state, nextIndex, name in transitions:
                logic.addChild('phase', {
                    'duration': duration,
                    'state': state,
                    'next': nextIndex,
                    'name': name})

            outf.write(logic.toXML(" " * 4))
            outf.write('\n')

        outf.write('</add>\n')


if __name__ == "__main__":
    options = get_options(sys.argv)
    main(options)
