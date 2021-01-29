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

# @file    tileGet.py
# @author  Michael Behrisch
# @date    2019-12-11

from __future__ import absolute_import
from __future__ import print_function
from __future__ import division
import math
import os

try:
    # python3
    import urllib.request as urllib
    from urllib.error import HTTPError as urlerror
except ImportError:
    import urllib
    from urllib2 import HTTPError as urlerror

import optparse

import sumolib  # noqa

MERCATOR_RANGE = 256
MAX_TILE_SIZE = 640
MAPQUEST_TYPES = {"roadmap": "map", "satellite": "sat", "hybrid": "hyb", "terrain": "sat"}


def fromLatLonToPoint(lat, lon):
    # inspired by https://stackoverflow.com/questions/12507274/how-to-get-bounds-of-a-google-static-map
    x = lon * MERCATOR_RANGE / 360
    siny = math.sin(math.radians(lat))
    y = 0.5 * math.log((1 + siny) / (1 - siny)) * -MERCATOR_RANGE / (2 * math.pi)
    return x, y


def fromLatLonToTile(lat, lon, zoom):
    # inspired by https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Python
    n = 2.0 ** zoom
    xtile = int((lon + 180.0) / 360.0 * n)
    ytile = int((1.0 - math.asinh(math.tan(math.radians(lat))) / math.pi) / 2.0 * n)
    return xtile, ytile


def fromTileToLatLon(xtile, ytile, zoom):
    n = 2.0 ** zoom
    lon = xtile / n * 360.0 - 180.0
    lat = math.degrees(math.atan(math.sinh(math.pi * (1 - 2 * ytile / n))))
    return lat, lon


def getZoomWidthHeight(south, west, north, east, maxTileSize):
    center = ((north + south) / 2, (east + west) / 2)
    centerPx = fromLatLonToPoint(*center)
    nePx = fromLatLonToPoint(north, east)
    zoom = 20
    width = (nePx[0] - centerPx[0]) * 2**zoom * 2
    height = (centerPx[1] - nePx[1]) * 2**zoom * 2
    while width > maxTileSize or height > maxTileSize:
        zoom -= 1
        width /= 2
        height /= 2
    return center, zoom, width, height


def retrieveMapServerTiles(url, tiles, west, south, east, north, decals, prefix, net, layer, output_dir):
    zoom = 20
    numTiles = tiles + 1
    while numTiles > tiles:
        zoom -= 1
        sx, sy = fromLatLonToTile(north, west, zoom)
        ex, ey = fromLatLonToTile(south, east, zoom)
        numTiles = (ex - sx + 1) * (ey - sy + 1)
    for x in range(sx, ex + 1):
        for y in range(sy, ey + 1):
            request = "%s/%s/%s/%s" % (url, zoom, y, x)
#            print(request)
#            opener = urllib.build_opener()
#            opener.addheaders = [('User-agent', 'Mozilla/5.0')]
#            urllib.install_opener(opener)
            try:
                urllib.urlretrieve(request, "%s%s_%s.jpeg" % (os.path.join(output_dir, prefix), x, y))
                lat, lon = fromTileToLatLon(x, y, zoom)
                upperLeft = net.convertLonLat2XY(lon, lat)
                lat, lon = fromTileToLatLon(x + 0.5, y + 0.5, zoom)
                center = net.convertLonLat2XY(lon, lat)
                print('    <decal file="%s%s_%s.jpeg" centerX="%s" centerY="%s" width="%s" height="%s" layer="%d"/>' %
                      (prefix, x, y, center[0], center[1],
                       2 * (center[0] - upperLeft[0]), 2 * (upperLeft[1] - center[1]), layer), file=decals)
            except urlerror as e:
                print("Tile server returned HTTP response code: " + str(e.code))
                raise ValueError


optParser = optparse.OptionParser()
optParser.add_option("-p", "--prefix", default="tile", help="for output file")
optParser.add_option("-b", "--bbox", help="bounding box to retrieve in geo coordinates west,south,east,north")
optParser.add_option("-t", "--tiles", type="int",
                     default=1, help="maximum number of tiles the output gets split into")
optParser.add_option("-d", "--output-dir", default=".", help="optional output directory (must already exist)")
optParser.add_option("-s", "--decals-file", default="settings.xml", help="name of decals settings file")
optParser.add_option("-l", "--layer", type="int", default=0,
                     help="(int) layer at which the image will appear, default 0")
optParser.add_option("-x", "--polygon", help="calculate bounding box from polygon data in file")
optParser.add_option("-n", "--net", help="get bounding box from net file")
optParser.add_option("-k", "--key", help="API key to use")
optParser.add_option("-m", "--maptype", default="satellite", help="map type (roadmap, satellite, hybrid, terrain)")
optParser.add_option("-u", "--url",
                     default="services.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile",
                     help="Download from the given tile server")
# alternatives: open.mapquestapi.com/staticmap/v4/getmap, maps.googleapis.com/maps/api/staticmap


def get(args=None):
    options, _ = optParser.parse_args(args=args)
    if not options.bbox and not options.net and not options.polygon:
        optParser.error("At least one of 'bbox' and 'net' and 'polygon' has to be set.")
    if not options.url.startswith("http"):
        options.url = "https://" + options.url
    bbox = ((0, 0), (0, 0))
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
    net = None
    if options.net:
        net = sumolib.net.readNet(options.net)
        bbox = net.getBBoxXY()
        west, south = net.convertXY2LonLat(*bbox[0])
        east, north = net.convertXY2LonLat(*bbox[1])

    prefix = os.path.join(options.output_dir, options.prefix)
    mapQuest = "mapquest" in options.url
    with open(os.path.join(options.output_dir, options.decals_file), "w") as decals:
        sumolib.xml.writeHeader(decals, root="viewsettings")
        if "MapServer" in options.url:
            retrieveMapServerTiles(options.url, options.tiles, west, south, east, north,
                                   decals, options.prefix, net, options.layer, options.output_dir)
        else:
            b = west
            for i in range(options.tiles):
                e = b + (east - west) / options.tiles
                offset = (bbox[1][0] - bbox[0][0]) / options.tiles
                c, z, w, h = getZoomWidthHeight(south, b, north, e, 2560 if mapQuest else 640)
                if mapQuest:
                    size = "size=%d,%d" % (w, h)
                    maptype = 'imagetype=png&type=' + MAPQUEST_TYPES[options.maptype]
                else:
                    size = "size=%dx%d" % (w, h)
                    maptype = 'maptype=' + options.maptype
                request = ("%s?%s&center=%.6f,%.6f&zoom=%s&%s&key=%s" %
                           (options.url, size, c[0], c[1], z, maptype, options.key))
    #            print(request)
                try:
                    urllib.urlretrieve(request, "%s%s.png" % (prefix, i))
                    print('    <decal file="%s%s.png" centerX="%s" centerY="%s" width="%s" height="%s" layer="%d"/>' %
                          (options.prefix, i, bbox[0][0] + (i + 0.5) * offset, (bbox[0][1] + bbox[1][1]) / 2,
                           offset, bbox[1][1] - bbox[0][1], options.layer), file=decals)
                    b = e
                except urlerror as e:
                    print("Tile server returned HTTP response code: " + str(e.code))
                    raise ValueError
        print("</viewsettings>", file=decals)


if __name__ == "__main__":
    get()
