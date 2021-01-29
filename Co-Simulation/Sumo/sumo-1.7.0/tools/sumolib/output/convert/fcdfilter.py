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

# @file    fcdfilter.py
# @author  Evamarie Wiessner
# @author  Michael Behrisch
# @date    2017-08-15

from __future__ import print_function
from __future__ import absolute_import


class Filter:
    def __init__(self, opts, begin, end, bbox, outfile):
        self._begin = float(begin)
        self._end = float(end)
        self._bbox = [float(b) for b in bbox.split(",")]
        self._out = open(outfile, "w")
        if opts.get("comment") is not None:
            k, v = opts.get("comment").split(":")
            self._out.write("""<!--
    <time>
        <begin value="%s"/>
        <end value="%s"/>
        <%s value="%s"/>
    </time>
-->
""" % (begin, end, k, v))
        self._out.write("<fcd-export>\n")
        self._type = opts.get("type")
        self._active = False

    def done(self, t):
        if t >= self._end:
            self.close()
            return True
        return False

    def checkTime(self, t):
        if self._active:
            self._out.write('    </timestep>\n')
        self._active = (t >= self._begin and t < self._end)
        if self._active:
            self._out.write('    <timestep time="%s">\n' % t)
        return self._active

    def write(self, v):
        lon, lat = float(v.x), float(v.y)
        if lon >= self._bbox[0] and lat >= self._bbox[1] and lon < self._bbox[2] and lat < self._bbox[3]:
            if self._type is not None:
                v.type = self._type
            self._out.write(v.toXML(8 * " "))

    def close(self):
        if self._active:
            self._out.write('    </timestep>\n')
        self._out.write("</fcd-export>\n")
        self._out.close()


def fcdfilter(inpFCD, outSTRM, further):
    filters = []
    for line in open(further["filter"]):
        filters.append(Filter(further, *line.strip().split(";")))
    for timestep in inpFCD:
        t = float(timestep.time)
        filters = [f for f in filters if not f.done(t)]
        if not filters:
            break
        active = [f for f in filters if f.checkTime(t)]
        if timestep.vehicle and active:
            for v in timestep.vehicle:
                for f in active:
                    f.write(v)
    for f in filters:
        f.close()
