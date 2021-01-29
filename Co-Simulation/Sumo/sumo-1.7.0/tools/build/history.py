#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    history.py
# @author  Michael Behrisch
# @date    2014-06-21

"""
This script builds all sumo versions in a certain revision range
and tries to eliminate duplicates afterwards.
"""
from __future__ import absolute_import

import subprocess
import optparse
import shutil
import os
import sys
import traceback

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib  # noqa


optParser = optparse.OptionParser()
optParser.add_option("-b", "--begin", default="v1_3_0", help="first revision to build")
optParser.add_option("-e", "--end", default="HEAD", help="last revision to build")
options, args = optParser.parse_args()

LOCK = "../history.lock"
if os.path.exists(LOCK):
    sys.exit("History building is still locked!")
open(LOCK, 'w').close()
try:
    subprocess.call(["git", "checkout", "-q", "master"])
    subprocess.call(["git", "pull"])
    commits = {}
    for line in subprocess.check_output(["git", "log", "%s..%s" % (options.begin, options.end)]).splitlines():
        if line.startswith("commit "):
            h = line.split()[1]
            commits[h] = sumolib.version.gitDescribe(h)
    haveBuild = False
    for h, desc in sorted(commits.items(), key=lambda x: x[1]):
        if not os.path.exists('../bin%s' % desc):
            ret = subprocess.call(["git", "checkout", "-q", h])
            if ret != 0:
                continue
            os.chdir("build/cmake-build")
            subprocess.call('make clean; make -j32', shell=True)
            os.chdir("../..")
            haveBuild = True
            shutil.copytree('bin', '../bin%s' % desc,
                            ignore=shutil.ignore_patterns('Makefile*', '*.bat', '*.jar'))
            subprocess.call('strip -R .note.gnu.build-id ../bin%s/*' % desc, shell=True)
            subprocess.call("sed -i 's/%s/%s/' ../bin%s/*" % (desc, len(desc) * "0", desc), shell=True)
    if haveBuild:
        for line in subprocess.check_output('fdupes -1 -q ../binv*', shell=True).splitlines():
            dups = line.split()
            for d in dups[1:]:
                subprocess.call('ln -sf %s %s' % (dups[0], d), shell=True)
    subprocess.call(["git", "checkout", "-q", "master"])
except Exception:
    traceback.print_exc()
os.remove(LOCK)
