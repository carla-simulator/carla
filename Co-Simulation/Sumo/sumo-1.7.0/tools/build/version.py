#!/usr/bin/env python
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

# @file    version.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @date    2007

"""
This script rebuilds "<BUILD_DIR>/src/version.h", the file which
 lets the applications know the version of their build.
It does this by parsing the output of git describe where the function is
 implemented in sumolib.
If the version file is newer than the .git index file or the revision cannot be
determined any existing version.h is kept.
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
from os.path import dirname, exists, getmtime, join, abspath

sys.path.append(dirname(dirname(abspath(__file__))))
import sumolib  # noqa


SUMO_ROOT = abspath(join(dirname(__file__), '..', '..'))


def get_version(padZero=True):
    return sumolib.version.gitDescribe(gitDir=join(SUMO_ROOT, ".git"), padZero=padZero)


def get_pep440_version():
    v = get_version(padZero=False)[1:-11].replace("_", ".").replace("+", ".post")
    vs = v.split(".")
    if len(vs) == 4 and vs[3] == "post0":
        return v[:-6]
    return v


def create_version_file(versionFile, revision):
    with open(versionFile, 'w') as f:
        print('#define VERSION_STRING "%s"' % revision, file=f)


def main():
    # determine output file
    if len(sys.argv) > 1:
        versionDir = sys.argv[1]
        if sys.argv[1] == "-":
            sys.stdout.write(get_version())
            return
    else:
        versionDir = join(SUMO_ROOT, "src")
    versionFile = join(versionDir, 'version.h')

    vcsFile = join(SUMO_ROOT, ".git", "index")
    try:
        if exists(vcsFile):
            if not exists(versionFile) or getmtime(versionFile) < getmtime(vcsFile):
                # vcsFile is newer. lets update the revision number
                print('generating %s from revision in %s' % (versionFile, vcsFile))
                create_version_file(versionFile, get_version())
        else:
            print("version control file '%s' not found" % vcsFile)
        if not exists(versionFile):
            print('trying to generate version file %s from existing header' % versionFile)
            create_version_file(versionFile, sumolib.version.fromVersionHeader())
    except Exception as e:
        print("Error creating", versionFile, e)
        try:
            # try at least to create something
            create_version_file(versionFile, "UNKNOWN")
        except Exception as ee:
            print("Error creating", versionFile, ee)
            pass


if __name__ == "__main__":
    main()
