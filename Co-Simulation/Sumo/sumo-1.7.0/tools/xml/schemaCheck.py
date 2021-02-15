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

# @file    schemaCheck.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    03.12.2009

"""
Checks schema for files matching certain file names using either
lxml or SAX2Count.exe depending on availability.
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import subprocess
import glob
import traceback
try:
    from urllib.request import unquote
except ImportError:
    from urllib import unquote
try:
    from lxml import etree
    haveLxml = True
    schemes = {}
except ImportError:
    haveLxml = False


def validate(root, f):
    root = os.path.abspath(root)
    normalized = os.path.abspath(f)[len(root) + 1:].replace('\\', '/')
    try:
        if os.path.getsize(f) < 80:
            # this is probably a texttest place holder file
            # it is definitely too small to contain a schema
            return
        doc = etree.parse(f)
        schemaLoc = doc.getroot().get(
            '{http://www.w3.org/2001/XMLSchema-instance}noNamespaceSchemaLocation')
        if schemaLoc and '/xsd/' in schemaLoc:
            localSchema = os.path.join(os.path.dirname(
                __file__), '..', '..', 'data', schemaLoc[schemaLoc.find('/xsd/') + 1:])
            if os.path.exists(localSchema):
                schemaLoc = localSchema
# if schemaLoc not in schemes: // temporarily disabled due to lxml bug
# https://bugs.launchpad.net/lxml/+bug/1222132
            schemes[schemaLoc] = etree.XMLSchema(etree.parse(schemaLoc))
            schemes[schemaLoc].validate(doc)
            for entry in schemes[schemaLoc].error_log:
                s = unquote(str(entry))
                # remove everything before (and including) the filename
                s = s[s.find(f.replace('\\', '/')) + len(f):]
                print(normalized + s, file=sys.stderr)
    except Exception:
        print("Error on parsing '%s'!" % normalized, file=sys.stderr)
        traceback.print_exc()


def main(srcRoot, toCheck, err):
    if not toCheck:
        toCheck = ["*.edg.xml", "*.nod.xml", "*.con.xml", "*.typ.xml",
                   "*.net.xml", "*.rou.xml", "*.add.xml", "*.????cfg",
                   "net.netgen", "net.netconvert",
                   "net.scenario", "tls.scenario",
                   "routes.duarouter", "alts.duarouter", "routes.jtrrouter", "routes.marouter",
                   "vehroutes.sumo", "vehroutes.sumo.meso", "trips.od2trips",
                   "*.turns.xml"]
    sax2count = "SAX2Count.exe"
    if 'XERCES_64' in os.environ:
        sax2count = os.path.join(os.environ['XERCES_64'], "bin", sax2count)
    elif 'XERCES' in os.environ:
        sax2count = os.path.join(os.environ['XERCES'], "bin", sax2count)

    fileNo = 0
    if os.path.exists(srcRoot):
        if os.path.isdir(srcRoot):
            for root, dirs, _ in os.walk(srcRoot):
                for pattern in toCheck:
                    for name in glob.glob(os.path.join(root, pattern)):
                        if haveLxml:
                            validate(srcRoot, name)
                        elif os.name != "posix":
                            subprocess.call(sax2count + " " + name, stdout=open(os.devnull), stderr=err)
                        fileNo += 1
                    if '.svn' in dirs:
                        dirs.remove('.svn')
        else:
            if haveLxml:
                validate("", srcRoot)
            elif os.name != "posix":
                subprocess.call(
                    sax2count + " " + srcRoot, stdout=open(os.devnull), stderr=err)
            fileNo += 1
    else:
        print("cannot open", srcRoot, file=err)
        return 1
    print("%s files checked" % fileNo)

    if haveLxml:
        for scheme in schemes.values():
            if scheme.error_log:
                print(scheme.error_log, file=err)
                return 1
    return 0


if __name__ == "__main__":
    if os.name == "posix" and not haveLxml:
        print("neither SAX2Count nor lxml available, exiting", file=sys.stderr)
        sys.exit(1)
    srcRoot = "."
    if len(sys.argv) > 1:
        srcRoot = sys.argv[1]
        if "$SUMO_HOME" in srcRoot:
            srcRoot = srcRoot.replace("$SUMO_HOME",
                                      os.path.join(os.path.dirname(__file__), '..', '..'))
    toCheck = None
    if len(sys.argv) > 2:
        toCheck = sys.argv[2].split(",")
    sys.exit(main(srcRoot, toCheck, sys.stderr))
