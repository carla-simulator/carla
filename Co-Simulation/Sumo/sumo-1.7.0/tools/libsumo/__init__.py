# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2018-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    __init__.py
# @author  Benjamin Striner
# @author  Michael Behrisch
# @date    2018-06-05

from functools import wraps
import sys
from .libsumo import vehicle, simulation
from .libsumo import *  # noqa

_traceFile = [None]


def isLibsumo():
    return True


def hasGUI():
    return False


def init(port):
    print("Warning! To make your code usable with traci and libsumo, please use traci.start instead of traci.init.")


def load(args):
    simulation.load(args)


def getVersion():
    return simulation.getVersion()


def close():
    simulation.close()
    if _traceFile[0]:
        _traceFile[0].close()


def start(args, traceFile=None, traceGetters=True):
    if traceFile is not None:
        _startTracing(traceFile, args, traceGetters)
    simulation.load(args[1:])
    return simulation.getVersion()


def setLegacyGetLeader(enabled):
    vehicle._legacyGetLeader = enabled


def _startTracing(traceFile, cmd, traceGetters):
    _traceFile[0] = open(traceFile, 'w')
    _traceFile[0].write("traci.start(%s)\n" % repr(cmd))
    self = sys.modules[__name__]
    # simulationStep shows up as simulation.step
    for m in ["close", "load"]:
        setattr(self, m, self._addTracing(getattr(self, m)))
    for domain in [
            busstop,  # noqa
            calibrator,  # noqa
            chargingstation,  # noqa
            edge,  # noqa
            # gui,  # noqa
            inductionloop,  # noqa
            junction,  # noqa
            lanearea,  # noqa
            lane,  # noqa
            multientryexit,  # noqa
            overheadwire,  # noqa
            parkingarea,  # noqa
            person,  # noqa
            poi,  # noqa
            polygon,  # noqa
            route,  # noqa
            simulation,  # noqa
            trafficlight,  # noqa
            vehicle,  # noqa
            vehicletype,  # noqa
    ]:
        for attrName in dir(domain):
            if not attrName.startswith("_"):
                attr = getattr(domain, attrName)
                if (callable(attr)
                        and attrName not in [
                            "wrapper",
                            "getAllSubscriptionResults",
                            "getAllContextSubscriptionResults",
                            "close",
                            "load",
                            "getVersion",
                ]
                        and not attrName.endswith('makeWrapper')
                        and (traceGetters or not attrName.startswith("get"))):
                    setattr(domain, attrName, _addTracing(attr, domain.__name__))


def _addTracing(method, domain=None):
    if domain:
        # replace first underscore with '.' because the methods name includes
        # the domain as prefix
        name = method.__name__.replace('_', '.', 1)
    else:
        name = method.__name__

    @wraps(method)
    def tracingWrapper(*args, **kwargs):
        _traceFile[0].write("traci.%s(%s)\n" % (
            name,
            ', '.join(list(map(repr, args)) + ["%s=%s" % (n, repr(v)) for n, v in kwargs.items()])))
        return method(*args, **kwargs)
    return tracingWrapper
