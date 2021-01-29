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

# @file    selection.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2013-05-06


def read(file, lanes2edges=True):
    ret = {}
    fd = open(file)
    for line in fd:
        vals = line.strip().split(":")
        if lanes2edges and vals[0] == "lane":
            vals[0] = "edge"
            vals[1] = vals[1][:vals[1].rfind("_")]
        if vals[0] not in ret:
            ret[vals[0]] = set()
        ret[vals[0]].add(vals[1])
    fd.close()
    return ret


def write(fdo, entries):
    for t in entries:
        writeTyped(fdo, t, entries[t])


def writeTyped(fdo, typeName, entries):
    for e in entries:
        fdo.write("%s:%s\n" % (typeName, e))
