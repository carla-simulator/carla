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

# @file    transportationTestProblems.py
# @author  Michael Behrisch
# @date    2015-02-24


from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import subprocess
import csv
sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
import sumolib  # noqa

csvfile = csv.DictReader(open(sys.argv[1]), delimiter='\t')
with open("nodes.nod.xml", "w") as nodeout:
    nodeout.write("<nodes>\n")
    for l in csvfile:
        nodeout.write('    <node id="%s" x="%s" y="%s"/>\n' %
                      (l['node'], l['X'], l['Y']))
    nodeout.write("</nodes>\n")

csvfile = csv.reader(open(sys.argv[2]), delimiter='\t')
with open("edges.edg.xml", "w") as edgeout:
    edgeout.write("<edges>\n")
    for l in csvfile:
        if l and l[0][0] not in "~<":
            edgeout.write(
                '    <edge id="%s_%s" from="%s" to="%s"/>\n' % (2 * (l[0], l[1])))
    edgeout.write("</edges>\n")

subprocess.call(
    [sumolib.checkBinary("netconvert"), "-n", nodeout.name, "-e", edgeout.name])
