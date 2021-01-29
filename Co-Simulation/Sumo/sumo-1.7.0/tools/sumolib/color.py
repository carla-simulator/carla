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

# @file    color.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2012-12-04

from __future__ import absolute_import


class RGBAColor:

    def __init__(self, r, g, b, a=None):
        self.r = r
        self.g = g
        self.b = b
        self.a = a

    def toXML(self):
        if self.a is not None:
            return "%s,%s,%s,%s" % (self.r, self.g, self.b, self.a)
        else:
            return "%s,%s,%s" % (self.r, self.g, self.b)


def decodeXML(c):
    return RGBAColor(*[float(x) for x in c.split(",")])
