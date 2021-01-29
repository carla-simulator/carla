# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    _reporting.py
# @author Leonhard Luecken
# @date   2017-04-09

from collections import deque
import sys
import traci

VERBOSITY = 1
WARNING_LOG = deque()
REPORT_LOG = deque()


def initDefaults():
    global VERBOSITY, MAX_LOG_SIZE, WARNING_LOG, REPORT_LOG
    # control level of verbosity
    # 0 - silent (only errors)
    # 1 - standard (errors and warnings)
    # 2 - log (additional status messages for platoons)
    # 3 - extended log (more status information)
    # 4 - insane (all kind of single vehicle detailed state infos)
    VERBOSITY = 1

    # log storage
    MAX_LOG_SIZE = 1000
    WARNING_LOG = deque()
    REPORT_LOG = deque()


# perform default init
initDefaults()


def simTime():
    return traci.simulation.getTime()


def array2String(a):
    if len(a) > 0 and hasattr(a[0], "getID"):
        return str([e.getID() for e in a])
    return str([str(e) for e in a])


class Warner(object):

    def __init__(self, domain):
        self._domain = domain

    def __call__(self, msg, omitReportTime=False):
        global MAX_LOG_SIZE, WARNING_LOG
        if len(WARNING_LOG) >= MAX_LOG_SIZE:
            WARNING_LOG.popleft()
        time = str(simTime())
        rep = "WARNING: " + str(msg) + " (" + self._domain + ")"
        if not omitReportTime:
            sys.stderr.write(time + ": " + rep + "\n")
        else:
            sys.stderr.write(rep + "\n")
        WARNING_LOG.append((time, rep))


class Reporter(object):

    def __init__(self, domain):
        self._domain = domain

    def __call__(self, msg, omitReportTime=False):
        global MAX_LOG_SIZE, REPORT_LOG
        if len(REPORT_LOG) >= MAX_LOG_SIZE:
            REPORT_LOG.popleft()
        time = str(simTime())
        rep = str(msg) + " (" + self._domain + ")"
        if not omitReportTime:
            sys.stdout.write(time + ": " + rep + "\n")
        else:
            sys.stdout.write(rep + "\n")
        REPORT_LOG.append((time, rep))
