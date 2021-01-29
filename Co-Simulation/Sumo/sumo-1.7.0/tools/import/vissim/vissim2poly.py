#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    vissim2poly.py
# @author  Jakob Erdmann
# @date    2020-01-30

"""
Parses a vissim .inpx file and exports link geometrys as <poly> elements
Usage: vissim2poly.py inpxFile outputFile
"""
import os
import sys
sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa


vissimfile, outfile = sys.argv[1:]
with open(outfile, 'w') as outf:
    outf.write('<add>\n')
    for link in sumolib.xml.parse(vissimfile, 'link'):
        if link.geometry is None or link.geometry[0].linkPolyPts is None:
            print("no polypoints for link %s" % link.no)
        points = [(p.x, p.y, p.z) for p in link.geometry[0].linkPolyPts[0].linkPolyPoint]
        shape = ' '.join([','.join(filter(None, xyz)) for xyz in points])
        outf.write('    <poly id="%s" shape="%s" color="blue"/>\n' % (
            link.no, shape))
    outf.write('</add>\n')
