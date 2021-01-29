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

# @file    run.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @author  Yun-Pang Floetteroed
# @date    2008-03-10

from __future__ import absolute_import
from __future__ import print_function
import glob
import os
import shutil
import subprocess
import time
import optparse


def detectCPUs():
    """Detects the number of effective CPUs in the system"""
    # for Linux, Unix and MacOS
    if hasattr(os, "sysconf"):
        if "SC_NPROCESSORS_ONLN" in os.sysconf_names:
            # Linux and Unix
            ncpus = os.sysconf("SC_NPROCESSORS_ONLN")
            if isinstance(ncpus, int) and ncpus > 0:
                return ncpus
        else:
            # MacOS X
            return int(os.popen2("sysctl -n hw.ncpu")[1].read())
    # for Windows
    if "NUMBER_OF_PROCESSORS" in os.environ:
        ncpus = int(os.environ["NUMBER_OF_PROCESSORS"])
        if ncpus > 0:
            return ncpus
    # return the default value
    return 1


def makeAndChangeDir(dirName):
    runID = 1
    fullName = "%s%03i" % (dirName, runID)
    while os.path.exists(fullName):
        runID += 1
        fullName = "%s%03i" % (dirName, runID)
    os.mkdir(fullName)
    os.chdir(fullName)
    return fullName


def execute(command):
    if options.verbose:
        print(command)
    os.system(command)


optParser = optparse.OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
optParser.add_option("-o", "--use-od2trips", action="store_true", dest="od2trips",
                     default=False, help="use od2trips instead of trips from incremental assignment")
optParser.add_option("-s", "--statistics", dest="stats", type="int",
                     default=0, help="use od2trips instead of trips from incremental assignment")
optParser.add_option("-d", "--dua-only", action="store_true", dest="duaonly",
                     default=False, help="just run dua with current routes from input")
optParser.add_option("-m", "--mesosim", action="store_true", dest="mesosim",
                     default=False, help="run in mesosim mode")
(options, args) = optParser.parse_args()

os.chdir("input")
netFile = "../input/" + glob.glob("*.net.xml")[0]
mtxNamesList = ",".join(
    ["../input/" + item for item in sorted(glob.glob("*.fma"))])

addFiles = ",".join(["../input/" + item for item in glob.glob("*.add.xml")])
pyAdds = ""
sumoAdds = ""
signalAdds = ""
if addFiles:
    pyAdds = "-+ %s" % addFiles
    sumoAdds = "-a %s" % addFiles
    signalAdds = "-s %s" % addFiles
if options.od2trips:
    trips = "trips"
else:
    trips = "successive"
routes = "../input/routes.rou.xml"
ncpus = detectCPUs()

mesoAppendix = ""
if options.mesosim:
    mesoAppendix = "meso_"

if options.stats == 0:
    if not options.duaonly:
        succDir = makeAndChangeDir("../" + mesoAppendix + "successive")
        execute("Assignment.py -e incremental -i 10 -d ../input/districts.xml -m %s -n %s" %
                (mtxNamesList, netFile))
        if not options.od2trips:
            shutil.copy("%s/routes.rou.xml" % succDir, routes)
            execute(
                "route2trips.py %s > ../input/successive.trips.xml" % routes)
    duaDir = makeAndChangeDir("../" + mesoAppendix + "dua")
    duaCall = "dua-iterate.py -e 90000 -C -n %s -t ../input/%s.trips.xml %s" % (
        netFile, trips, pyAdds)
    if options.mesosim:
        duaCall = duaCall + " --mesosim"
    duaProcess = subprocess.Popen(duaCall, shell=True)
    oneshotProcess = None
    if not options.duaonly:
        if options.od2trips:
            while not os.path.exists("%s/trips_0.rou.xml" % duaDir):
                time.sleep(1)
            shutil.copy("%s/trips_0.rou.xml" % duaDir, routes)
        shotDir = makeAndChangeDir("../" + mesoAppendix + "oneshot")
        shotCall = "one-shot.py -e 90000 -n %s -t %s %s" % (
            netFile, routes, pyAdds)
        if options.mesosim:
            shotCall = shotCall + " --mesosim"
        if ncpus > 2:
            oneshotProcess = subprocess.Popen(shotCall, shell=True)
        else:
            execute(shotCall)
        clogDir = makeAndChangeDir("../" + mesoAppendix + "clogit")
        execute("Assignment.py -i 60 -d ../input/districts.xml -m %s -n %s %s" %
                (mtxNamesList, netFile, signalAdds))
        lohseDir = makeAndChangeDir("../" + mesoAppendix + "lohse")
        execute("Assignment.py -e lohse -i 60 -d ../input/districts.xml -m %s -n %s %s" %
                (mtxNamesList, netFile, signalAdds))
    if oneshotProcess:
        oneshotProcess.wait()
    duaProcess.wait()
else:
    succDir = "../" + mesoAppendix + "successive%03i" % options.stats
    duaDir = "../" + mesoAppendix + "dua%03i" % options.stats
    clogDir = "../" + mesoAppendix + "clogit%03i" % options.stats
    lohseDir = "../" + mesoAppendix + "lohse%03i" % options.stats
    shotDir = "../" + mesoAppendix + "oneshot%03i" % options.stats

makeAndChangeDir("../" + mesoAppendix + "statistics")
tripinfos = ""
routes = []
for step in [0, 49]:
    tripinfofile = "tripinfo_dua_%s.xml" % step
    shutil.copy("%s/tripinfo_%s.xml" % (duaDir, step), tripinfofile)
    tripinfos += tripinfofile + ","
    execute("networkStatistics.py -t tripinfo_dua_%s.xml -o networkStatistics_%s_%s.txt" %
            (step, os.path.basename(duaDir), step))
    routes.append("%s/%s_%s.rou.xml" % (duaDir, trips, step))
if not options.duaonly:
    for step in [-1, 15]:
        tripinfofile = "tripinfo_oneshot_%s.xml" % step
        shutil.copy("%s/tripinfo_%s.xml" % (shotDir, step), tripinfofile)
        tripinfos += tripinfofile + ","
        routes.append("%s/vehroutes_%s.xml" % (shotDir, step))
    execute("sumo -W --no-step-log -n %s -e 90000 -r %s/routes.rou.xml --dump-basename dump_successive " +
            "--dump-intervals 900 --emissions emissions_successive.xml --tripinfo-output tripinfo_successive.xml " +
            "%s -l sumo_successive.log" % (netFile, succDir, sumoAdds))
    execute("sumo -W --no-step-log -n %s -e 90000 -r %s/routes.rou.xml --dump-basename dump_clogit " +
            "--dump-intervals 900 --emissions emissions_clogit.xml --tripinfo-output tripinfo_clogit.xml " +
            "%s -l sumo_clogit.log" % (netFile, clogDir, sumoAdds))
    execute("sumo -W --no-step-log -n %s -e 90000 -r %s/routes.rou.xml --dump-basename dump_lohse " +
            "--dump-intervals 900 --emissions emissions_lohse.xml --tripinfo-output tripinfo_lohse.xml %s -l " +
            "sumo_lohse.log" % (netFile, lohseDir, sumoAdds))
    tripinfos += tripinfofile + \
        ",tripinfo_successive.xml,tripinfo_clogit.xml,tripinfo_lohse.xml"
    execute(
        "networkStatistics.py -t %s -o networkStatisticsWithSgT.txt" % tripinfos)
    for dir in succDir, clogDir, lohseDir:
        routes.append(dir + "/routes.rou.xml")
# outfilename = "routecompare.txt"
# for idx, route1 in enumerate(routes):
#    for route2 in routes[idx+1:]:
#        outfile = open(outfilename, "a")
#        print >> outfile, route1, route2
#        outfile.close()
#        execute("routecompare.py -d ../input/districts.xml %s %s >> %s" % (route1, route2, outfilename))
os.chdir("..")
