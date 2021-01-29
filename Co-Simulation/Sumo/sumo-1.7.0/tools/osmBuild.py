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

# @file    osmBuild.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2009-08-01

from __future__ import absolute_import

import os
import optparse
import subprocess
from os import path

import sumolib  # noqa


vclassRemove = {"passenger": ["--keep-edges.by-vclass", "passenger"],
                "publicTransport": ["--keep-edges.by-vclass", "passenger,bus,tram,rail_urban,rail"],
                "road": ["--remove-edges.by-vclass", "tram,rail_urban,rail_electric,bicycle,pedestrian"],
                "all": []}
possibleVClassOptions = '|'.join(vclassRemove.keys())

DEFAULT_NETCONVERT_OPTS = '''--geometry.remove,--roundabouts.guess,--ramps.guess,-v,--junctions.join,\
--tls.guess-signals,--tls.discard-simple,--tls.join,--output.original-names,--junctions.corner-detail,\
5,--output.street-names'''


optParser = optparse.OptionParser()
optParser.add_option("-p", "--prefix", default="osm", help="for output file")
# don't know whether area or bbox call was used
optParser.add_option(
    "-f", "--osm-file", help="full name of the osm file to import")
optParser.add_option("-m", "--typemap", default=None,
                     help="typemap file for the extraction of colored areas (optional)")
optParser.add_option("--netconvert-typemap", default=None,
                     help="typemap files for netconverter (optional)")
optParser.add_option("-o", "--oldapi-prefix", default=None,
                     help="prefix that was used for retrieval with the old API")
optParser.add_option("-t", "--tiles", type="int", default=1,
                     help="number of tiles used for retrieving OSM-data via the old api")
optParser.add_option("-c", "--vehicle-classes", default='all',
                     help="[(%s)]extract network for a reduced set of vehicle classes" % possibleVClassOptions)
optParser.add_option("-d", "--output-directory", default=os.getcwd(),
                     help="directory in which to put the output files")
optParser.add_option("-n", "--netconvert-options",
                     default=DEFAULT_NETCONVERT_OPTS, help="comma-separated options for netconvert")
optParser.add_option("--pedestrians", action="store_true",
                     default=False, help="add pedestrian infrastructure to the network")
optParser.add_option("-y", "--polyconvert-options",
                     default="-v,--osm.keep-full-type", help="comma-separated options for polyconvert")


def getRelative(dirname, option):
    ld = len(dirname)
    if option[:ld] == dirname:
        return option[ld+1:]
    else:
        return option


def build(args=None, bindir=None):
    (options, args) = optParser.parse_args(args=args)

    if ((options.oldapi_prefix and options.osm_file) or
            not (options.oldapi_prefix or options.osm_file)):
        optParser.error(
            "exactly one of the options --osm-file and --oldapi-prefix must be supplied")
    if options.typemap and not path.isfile(options.typemap):
        # fail early because netconvert may take a long time
        optParser.error('typemap file "%s" not found' % options.typemap)
    if not (options.vehicle_classes in vclassRemove):
        optParser.error('invalid vehicle class "%s" given' %
                        options.vehicle_classes)
    if not path.isdir(options.output_directory):
        optParser.error('output directory "%s" does not exist' %
                        options.output_directory)

    netconvert = sumolib.checkBinary('netconvert', bindir)
    polyconvert = sumolib.checkBinary('polyconvert', bindir)

    netconvertOpts = [netconvert]
    if options.pedestrians:
        netconvertOpts += ['--sidewalks.guess', '--crossings.guess']
    if options.netconvert_typemap:
        netconvertOpts += ["-t", options.netconvert_typemap]
    netconvertOpts += options.netconvert_options.split(',') + ['--osm-files']
    polyconvertOpts = ([polyconvert] + options.polyconvert_options.split(',') +
                       ['--type-file', options.typemap, '--osm-files'])

    prefix = options.oldapi_prefix
    if prefix:  # used old API
        num = options.tiles
        tiles = ",".join(["%s%s_%s.osm.xml" % (prefix, i, num)
                          for i in range(num)])
        netconvertOpts += [tiles]
        polyconvertOpts += [tiles]
    else:  # used new API
        netconvertOpts += [options.osm_file]
        polyconvertOpts += [options.osm_file]
        prefix = path.basename(options.osm_file).replace('.osm.xml', '')

    if options.prefix:
        prefix = options.prefix

    netfile = prefix + '.net.xml'
    netconvertOpts += vclassRemove[options.vehicle_classes] + ["-o", netfile]

    # write config
    cfg = prefix + ".netccfg"
    # use relative paths where possible
    netconvertOpts = [getRelative(options.output_directory, o) for o in netconvertOpts]
    subprocess.call(netconvertOpts + ["--save-configuration", cfg], cwd=options.output_directory)
    subprocess.call([netconvert, "-c", cfg], cwd=options.output_directory)

    if options.typemap:
        # write config
        cfg = prefix + ".polycfg"
        polyconvertOpts += ["-n", netfile, "-o", prefix + '.poly.xml']
        # use relative paths where possible
        polyconvertOpts = [getRelative(options.output_directory, o) for o in polyconvertOpts]
        subprocess.call(polyconvertOpts + ["--save-configuration", cfg], cwd=options.output_directory)
        subprocess.call([polyconvert, "-c", cfg], cwd=options.output_directory)


if __name__ == "__main__":
    build()
