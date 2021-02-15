#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    patchTrafficLights.py
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2010-03-11

from __future__ import absolute_import
from __future__ import print_function
import re
import sys
from optparse import OptionParser

optParser = OptionParser()
optParser.add_option("-n", "--net-file", dest="netfile",
                     help="define the input net file (mandatory)")
optParser.add_option("-o", "--output-net-file", dest="outfile",
                     default="out.net.xml", help="define the output net filename")
optParser.add_option(
    "-s", "--switch", type="int", default=2, help="switch time")
(options, args) = optParser.parse_args()
if not options.netfile:
    optParser.print_help()
    sys.exit()

skip = False
tlid = ""
out = open(options.outfile, "w")
for line in open(options.netfile):
    if tlid and not skip:
        m = re.search('state="([^"]+)"', line)
        if m and len(m.group(1)) == 16:
            skip = True
            print("""      <phase duration="%s" state="rrrrrrrrrrrrrrrr"/>
      <phase duration="10000" state="rrrrGGggrrrrGGgg"/>
   </tlLogic>

   <tlLogic id="%s" type="static" programID="1" offset="0">
      <phase duration="%s" state="rrrrrrrrrrrrrrrr"/>
      <phase duration="10000" state="GGggrrrrGGggrrrr"/>""" % (options.switch, tlid, options.switch), file=out)
    m = re.search('<tlLogic id="([^"]+)"', line)
    if m:
        tlid = m.group(1)
    if re.search('</tlLogic>', line):
        skip = False
        tlid = ""
    if not skip:
        out.write(line)
