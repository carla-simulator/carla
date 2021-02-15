#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    vehicle2flow.py
# @author  Michael Behrisch
# @date    2012-11-15

"""
This script replaces all vehicle definitions in a route file by
flow definitions, adding an XML ntity for the repeat interval for
easy later modification.
"""
from __future__ import absolute_import
import sys
import re
from optparse import OptionParser


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " <routefile> [options]"
    optParser = OptionParser()
    optParser.add_option("-o", "--outfile", help="name of output file")
    optParser.add_option(
        "-r", "--repeat", default=1000, type="float", help="repeater interval")
    optParser.add_option(
        "-e", "--end", default=2147483, type="float", help="end of the flow")
    optParser.add_option("-w", "--with-entities", action="store_true",
                         default=False, help="store repeat and end as entities")
    options, args = optParser.parse_args()
    try:
        options.routefile = args[0]
    except Exception:
        sys.exit(USAGE)
    if options.outfile is None:
        options.outfile = options.routefile + ".rou.xml"
    return options


def main():
    options = parse_args()
    with open(options.routefile) as f:
        with open(options.outfile, 'w') as outf:
            for line in f:
                if options.with_entities:
                    if "<routes " in line or "<routes>" in line:
                        outf.write("""<!DOCTYPE routes [
        <!ENTITY RepeatInterval "%s">
        <!ENTITY RepeatEnd "%s">
    ]>
    """ % (options.repeat, options.end))
                    line = re.sub(
                        r'<vehicle(.*)depart( ?= ?"[^"]*")', r'<flow\1begin\2 end="&RepeatEnd;" ' +
                        'period="&RepeatInterval;"', line)
                else:
                    line = re.sub(
                        r'<vehicle(.*)depart( ?= ?"[^"]*")', r'<flow\1begin\2 end="%s" period="%s"' %
                        (options.end, options.repeat), line)
                line = re.sub(r'</vehicle>', '</flow>', line)
                outf.write(line)


if __name__ == "__main__":
    main()
