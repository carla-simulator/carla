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

# @file    _config.py
# @author Leonhard Luecken
# @date   2017-04-09


from collections import defaultdict
import os
import sys
import xml.etree.ElementTree as ET

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

from simpla._platoonmode import PlatoonMode  # noqa
import simpla._reporting as rp  # noqa
from simpla import SimplaException  # noqa

warn = rp.Warner("Config")
report = rp.Reporter("Config")


def initDefaults():
    '''
    Init default values for the configuration parameters.
    They are overriden by specification in a configuration file (see load() method).
    '''
    global CONTROL_RATE, VEH_SELECTORS, MAX_PLATOON_GAP, CATCHUP_DIST, PLATOON_SPLIT_TIME
    global VTYPE_FILE, PLATOON_VTYPES, LC_MODE, SPEEDFACTOR, SWITCH_IMPATIENCE_FACTOR

    # Rate for updating the platoon manager checks and advices
    CONTROL_RATE = 1.0

    # specify substring for vtypes, that should be controlled by platoon manager
    VEH_SELECTORS = [""]

    # Distance in meters below which a vehicle joins a leading platoon
    MAX_PLATOON_GAP = 15.0

    # Distance in meters below which a vehicle tries to catch up with a platoon in front
    CATCHUP_DIST = 50.0

    # Latency time in secs. until a platoon is split if vehicles exceed PLATOON_SPLIT_DISTANCE to their
    # leaders within a platoon (or if they are not the direct follower),
    # or drive on different lanes than their leader within the platoon
    PLATOON_SPLIT_TIME = 3.0

    # The switch impatience factor determines the magnitude of the effect
    # that an increasing waiting time has on the active speed factor of a vehicle:
    # activeSpeedFactor = modeSpecificSpeedFactor/(1+impatienceFactor*waitingTime)
    SWITCH_IMPATIENCE_FACTOR = 0.1

    # Lanechange modes for the different platooning modes
    LC_MODE = {
        # for solitary mode use default mode
        PlatoonMode.NONE: 0b1001010101,
        # for platoon leader use default mode
        PlatoonMode.LEADER: 0b1001010101,
        # for platoon follower do not change lanes, except for traci commands
        # of for strategic reasons (these override traci)
        PlatoonMode.FOLLOWER: 0b1000000010,
        # for platoon catchup as for follower
        PlatoonMode.CATCHUP: 0b1000000010,
        # for platoon catchup follower as for follower
        PlatoonMode.CATCHUP_FOLLOWER: 0b1000000010
    }

    # speedfactors for the different platooning modes
    SPEEDFACTOR = {
        PlatoonMode.NONE: None,  # is not altered
        PlatoonMode.LEADER: 1.0,
        PlatoonMode.FOLLOWER: 1.0,
        PlatoonMode.CATCHUP: 1.1,
        PlatoonMode.CATCHUP_FOLLOWER: None  # is set to the same as for catchup mode below if not explicitely set
    }

    # file with vtype maps for platoon types
    VTYPE_FILE = ""

    # map of original to platooning vTypes
    PLATOON_VTYPES = defaultdict(dict)


# perform initialization
initDefaults()


def loadVTypeMap(fn):
    '''loadVTypeMap(string) -> dict

    Reads lines of the form 'origMode:leadMode:followMode:catchupMode:catchupFollowMode' (last three elements
    can be omitted) from a given file and write corresponding key:value pairs to PLATOON_VTYPES
    '''
    global PLATOON_VTYPES

    with open(fn, "r") as f:
        #         if rp.VERBOSITY >= 2:
        if rp.VERBOSITY >= 2:
            report("Loading vehicle type mappings from file '%s'..." % fn, True)
        splits = [l.split(":") for l in f.readlines()]
        NrBadLines = 0
        for j, spl in enumerate(splits):
            if len(spl) >= 2 and len(spl) <= 5:
                stripped = list(map(lambda x: x.strip(), spl))
                origType = stripped[0]
                if origType == "":
                    raise SimplaException("Original vType must be specified in line %s of vType file '%s'!" % (j, fn))
                if rp.VERBOSITY >= 2:
                    report("original type: '%s'" % origType, True)

                leadType = stripped[1]
                if leadType == "":
                    raise SimplaException(
                        "Platoon leader vType must be specified in line %s of vType file '%s'!" % (j, fn))
                if rp.VERBOSITY >= 2:
                    report("platoon leader type: '%s'" % leadType, True)

                if (len(stripped) >= 3 and stripped[2] != ""):
                    followerType = stripped[2]
                    if rp.VERBOSITY >= 2:
                        report("platoon follower type: '%s'" % followerType, True)
                else:
                    followerType = None

                if (len(stripped) >= 4 and stripped[3] != ""):
                    catchupType = stripped[3]
                    if rp.VERBOSITY >= 2:
                        report("catchup type: '%s'" % catchupType, True)
                else:
                    catchupType = None

                if len(stripped) >= 5 and stripped[4] != "":
                    catchupFollowerType = stripped[4]
                    if rp.VERBOSITY >= 2:
                        report("catchup follower type: '%s'" % catchupFollowerType, True)
                else:
                    catchupFollowerType = None

                PLATOON_VTYPES[origType][PlatoonMode.NONE] = origType
                PLATOON_VTYPES[origType][PlatoonMode.LEADER] = leadType
                PLATOON_VTYPES[origType][PlatoonMode.FOLLOWER] = followerType
                PLATOON_VTYPES[origType][PlatoonMode.CATCHUP] = catchupType
                PLATOON_VTYPES[origType][PlatoonMode.CATCHUP_FOLLOWER] = catchupFollowerType
            else:
                NrBadLines += 1
        if NrBadLines > 0:
            if rp.VERBOSITY >= 1:
                warn(("vType file '%s' contained %d lines that were not parsed into a colon-separated " +
                      "sequence of strings!") % (fn, NrBadLines))


def load(filename):
    '''load(string)

    This loads configuration parameters from a file and overwrites default values.
    '''
    global CONTROL_RATE, VEH_SELECTORS, MAX_PLATOON_GAP, CATCHUP_DIST, PLATOON_SPLIT_TIME
    global VTYPE_FILE, PLATOON_VTYPES, LC_MODE, SPEEDFACTOR, SWITCH_IMPATIENCE_FACTOR

    configDir = os.path.dirname(filename)
    configElements = ET.parse(filename).getroot().getchildren()
    parsedTags = []
    for e in configElements:
        parsedTags.append(e.tag)
        if e.tag == "verbosity":
            if hasAttributes(e):
                verbosity = int(list(e.attrib.values())[0])
                if verbosity in range(5):
                    rp.VERBOSITY = verbosity
                else:
                    if rp.VERBOSITY >= 1:
                        warn("Verbosity must be one of %s! Ignoring given value: %s" %
                             (str(list(range(5))), verbosity), True)
        elif e.tag == "controlRate":
            if hasAttributes(e):
                rate = float(list(e.attrib.values())[0])
                if rate <= 0.:
                    if rp.VERBOSITY >= 1:
                        warn("Parameter controlRate must be positive. Ignoring given value: %s" % (rate), True)
                else:
                    CONTROL_RATE = float(rate)
        elif e.tag == "vehicleSelectors":
            if hasAttributes(e):
                VEH_SELECTORS = list(map(lambda x: x.strip(), list(e.attrib.values())[0].split(",")))
        elif e.tag == "maxPlatoonGap":
            if hasAttributes(e):
                maxgap = float(list(e.attrib.values())[0])
                if maxgap <= 0:
                    if rp.VERBOSITY >= 1:
                        warn("Parameter maxPlatoonGap must be positive. Ignoring given value: %s" % (maxgap), True)
                else:
                    MAX_PLATOON_GAP = maxgap
        elif e.tag == "catchupDist":
            if hasAttributes(e):
                dist = float(list(e.attrib.values())[0])
                if maxgap <= 0:
                    if rp.VERBOSITY >= 1:
                        warn("Parameter catchupDist must be positive. Ignoring given value: %s" % (dist), True)
                else:
                    CATCHUP_DIST = dist
        elif e.tag == "switchImpatienceFactor":
            if hasAttributes(e):
                impfact = float(list(e.attrib.values())[0])
                if impfact < 0:
                    if rp.VERBOSITY >= 1:
                        warn("Parameter switchImpatienceFactor must be non-negative. Ignoring given value: %s" %
                             (impfact), True)
                else:
                    SWITCH_IMPATIENCE_FACTOR = impfact
        elif e.tag == "platoonSplitTime":
            if hasAttributes(e):
                splittime = float(list(e.attrib.values())[0])
                if splittime < 0:
                    if rp.VERBOSITY >= 1:
                        warn("Parameter platoonSplitTime must be non-negative. Ignoring given value: %s" %
                             (splittime), True)
                else:
                    PLATOON_SPLIT_TIME = splittime
        elif e.tag == "lcMode":
            if hasAttributes(e):
                if ("leader" in e.attrib):
                    if isValidLCMode(int(e.attrib["leader"])):
                        LC_MODE[PlatoonMode.LEADER] = int(e.attrib["leader"])
                if ("follower" in e.attrib):
                    if isValidLCMode(int(e.attrib["follower"])):
                        LC_MODE[PlatoonMode.FOLLOWER] = int(e.attrib["follower"])
                if ("catchup" in e.attrib):
                    if isValidLCMode(int(e.attrib["catchup"])):
                        LC_MODE[PlatoonMode.CATCHUP] = int(e.attrib["catchup"])
                if ("catchupFollower" in e.attrib):
                    if isValidLCMode(int(e.attrib["catchupFollower"])):
                        LC_MODE[PlatoonMode.CATCHUP_FOLLOWER] = int(e.attrib["catchupFollower"])
                if ("original" in e.attrib):
                    if isValidLCMode(int(e.attrib["original"])):
                        LC_MODE[PlatoonMode.NONE] = int(e.attrib["original"])
        elif e.tag == "speedFactor":
            if hasAttributes(e):
                if ("leader" in e.attrib):
                    if isValidSpeedFactor(float(e.attrib["leader"])):
                        SPEEDFACTOR[PlatoonMode.LEADER] = float(e.attrib["leader"])
                if ("follower" in e.attrib):
                    if isValidSpeedFactor(float(e.attrib["follower"])):
                        SPEEDFACTOR[PlatoonMode.FOLLOWER] = float(e.attrib["follower"])
                if ("catchup" in e.attrib):
                    if isValidSpeedFactor(float(e.attrib["catchup"])):
                        SPEEDFACTOR[PlatoonMode.CATCHUP] = float(e.attrib["catchup"])
                if ("catchupFollower" in e.attrib):
                    if isValidSpeedFactor(float(e.attrib["catchupFollower"])):
                        SPEEDFACTOR[PlatoonMode.CATCHUP_FOLLOWER] = float(e.attrib["catchupFollower"])
                if ("original" in e.attrib):
                    if isValidSpeedFactor(float(e.attrib["original"])):
                        SPEEDFACTOR[PlatoonMode.NONE] = float(e.attrib["original"])
        elif e.tag == "vTypeMapFile":
            if hasAttributes(e):
                fn = os.path.join(configDir, list(e.attrib.values())[0])
                if not os.path.isfile(fn):
                    raise SimplaException("Given vTypeMapFile '%s' does not exist." % fn)
                VTYPE_FILE = fn
        elif e.tag == "vTypeMap":
            if hasAttributes(e):
                if "original" not in e.attrib:
                    warn("vTypeMap must specify original type. Ignoring malformed vTypeMap element.", True)
                else:
                    origType = e.attrib["original"]
                    PLATOON_VTYPES[origType][PlatoonMode.NONE] = origType
                    if ("leader" in e.attrib):
                        leaderType = e.attrib["leader"]
                        PLATOON_VTYPES[origType][PlatoonMode.LEADER] = leaderType
                        # report("Registering vtype map '%s':'%s'"%(origType,leaderType), True)
                    if ("follower" in e.attrib):
                        followerType = e.attrib["follower"]
                        PLATOON_VTYPES[origType][PlatoonMode.FOLLOWER] = followerType
                        # report("Registering vtype map '%s':'%s'"%(origType,followerType), True)
                    if ("catchup" in e.attrib):
                        catchupType = e.attrib["catchup"]
                        PLATOON_VTYPES[origType][PlatoonMode.CATCHUP] = catchupType
                        # report("Registering vtype map '%s':'%s'"%(origType,followerType), True)
                    if ("catchupFollower" in e.attrib):
                        catchupFollowerType = e.attrib["catchupFollower"]
                        PLATOON_VTYPES[origType][PlatoonMode.CATCHUP_FOLLOWER] = catchupFollowerType
                        # report("Registering vtype map '%s':'%s'"%(origType,followerType), True)
        elif rp.VERBOSITY >= 1:
            warn("Encountered unknown configuration parameter '%s'!" % e.tag, True)

    if "vTypeMapFile" in parsedTags:
        # load vType mapping from file
        loadVTypeMap(VTYPE_FILE)

    if SPEEDFACTOR[PlatoonMode.CATCHUP_FOLLOWER] is None:
        # if unset, set speedfactor for catchupfollower mode to the same as in catchup mode
        SPEEDFACTOR[PlatoonMode.CATCHUP_FOLLOWER] = SPEEDFACTOR[PlatoonMode.CATCHUP]


def hasAttributes(element):
    '''
    check if xml element has at least one attribute
    '''
    # print("Checking element {tag}:\n{attributes}".format(tag=element.tag, attributes=str(element.attrib)))
    if len(element.attrib) == 0:
        warn("No attributes found for tag '%s'." % element.tag, True)
        return False
    else:
        return True


def isValidLCMode(mode):
    if 0 <= mode <= 1023:
        return True
    else:
        warn("Given lane change mode '%d' lies out of admissible range [0,255]. Using default mode instead." % (
            mode), True)
        return False


def isValidSpeedFactor(value):
    if 0 < value:
        return True
    else:
        warn("Given speedFactor %s is invalid. Using default value." % (value), True)
        return False
