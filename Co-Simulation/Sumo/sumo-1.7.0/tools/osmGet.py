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

# @file    osmGet.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2009-08-01

from __future__ import absolute_import
from __future__ import print_function
import os
try:
    import httplib
    import urlparse
except ImportError:
    # python3
    import http.client as httplib
    import urllib.parse as urlparse

import optparse
import base64
from os import path

import sumolib  # noqa


def readCompressed(conn, urlpath, query, filename):
    conn.request("POST", "/" + urlpath, """
    <osm-script timeout="240" element-limit="1073741824">
    <union>
       %s
       <recurse type="node-relation" into="rels"/>
       <recurse type="node-way"/>
       <recurse type="way-relation"/>
    </union>
    <union>
       <item/>
       <recurse type="way-node"/>
    </union>
    <print mode="body"/>
    </osm-script>""" % query)
    response = conn.getresponse()
    print(response.status, response.reason)
    if response.status == 200:
        out = open(path.join(os.getcwd(), filename), "wb")
        out.write(response.read())
        out.close()


optParser = optparse.OptionParser()
optParser.add_option("-p", "--prefix", default="osm", help="for output file")
optParser.add_option("-b", "--bbox", help="bounding box to retrieve in geo coordinates west,south,east,north")
optParser.add_option("-t", "--tiles", type="int",
                     default=1, help="number of tiles the output gets split into")
optParser.add_option("-d", "--output-dir", help="optional output directory (must already exist)")
optParser.add_option("-a", "--area", type="int", help="area id to retrieve")
optParser.add_option("-x", "--polygon", help="calculate bounding box from polygon data in file")
optParser.add_option("-u", "--url", default="www.overpass-api.de/api/interpreter",
                     help="Download from the given OpenStreetMap server")
# alternatives: overpass.kumi.systems/api/interpreter, sumo.dlr.de/osm/api/interpreter


def get(args=None):
    (options, args) = optParser.parse_args(args=args)
    if not options.bbox and not options.area and not options.polygon:
        optParser.error("At least one of 'bbox' and 'area' and 'polygon' has to be set.")
    if options.polygon:
        west = 1e400
        south = 1e400
        east = -1e400
        north = -1e400
        for area in sumolib.output.parse_fast(options.polygon, 'poly', ['shape']):
            coordList = [tuple(map(float, x.split(',')))
                         for x in area.shape.split()]
            for point in coordList:
                west = min(point[0], west)
                south = min(point[1], south)
                east = max(point[0], east)
                north = max(point[1], north)
    if options.bbox:
        west, south, east, north = [float(v) for v in options.bbox.split(',')]
        if south > north or west > east:
            optParser.error("Invalid geocoordinates in bbox.")

    if options.output_dir:
        options.prefix = path.join(options.output_dir, options.prefix)

    if "http" in options.url:
        url = urlparse.urlparse(options.url)
    else:
        url = urlparse.urlparse("https://" + options.url)
    if os.environ.get("https_proxy") is not None:
        headers = {}
        proxy_url = urlparse.urlparse(os.environ.get("https_proxy"))
        if proxy_url.username and proxy_url.password:
            auth = '%s:%s' % (proxy_url.username, proxy_url.password)
            headers['Proxy-Authorization'] = 'Basic ' + base64.b64encode(auth)
        conn = httplib.HTTPSConnection(proxy_url.hostname, proxy_url.port)
        conn.set_tunnel(url.hostname, 443, headers)
    else:
        if url.scheme == "https":
            conn = httplib.HTTPSConnection(url.hostname, url.port)
        else:
            conn = httplib.HTTPConnection(url.hostname, url.port)

    if options.area:
        if options.area < 3600000000:
            options.area += 3600000000
        readCompressed(conn, url.path, '<area-query ref="%s"/>' %
                       options.area, options.prefix + "_city.osm.xml")
    if options.bbox or options.polygon:
        if options.tiles == 1:
            readCompressed(conn, url.path, '<bbox-query n="%s" s="%s" w="%s" e="%s"/>' %
                           (north, south, west, east), options.prefix + "_bbox.osm.xml")
        else:
            num = options.tiles
            b = west
            for i in range(num):
                e = b + (east - west) / float(num)
                readCompressed(conn, url.path, '<bbox-query n="%s" s="%s" w="%s" e="%s"/>' % (
                    north, south, b, e), "%s%s_%s.osm.xml" % (options.prefix, i, num))
                b = e

    conn.close()


if __name__ == "__main__":
    get()
