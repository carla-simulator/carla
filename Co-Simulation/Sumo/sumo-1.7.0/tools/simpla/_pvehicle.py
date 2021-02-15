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

# @file    _pvehicle.py
# @author Leonhard Luecken
# @date   2017-04-09

import os
import sys
from collections import defaultdict

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

import traci  # noqa
import traci.constants as tc  # noqa

from simpla._platoon import Platoon  # noqa
import simpla._reporting as rp  # noqa
import simpla._config as cfg  # noqa
from simpla._platoonmode import PlatoonMode  # noqa

warn = rp.Warner("PVehicle")
report = rp.Reporter("PVehicle")

# lookup table for vType parameters
vTypeParameters = defaultdict(dict)

WARNED_DEFAULT = dict([(mode, False) for mode in PlatoonMode])


class pVehicleState(object):

    def __init__(self, ID):
        self.speed = traci.vehicle.getSpeed(ID)
        self.edgeID = traci.vehicle.getRoadID(ID)
        self.laneID = traci.vehicle.getLaneID(ID)
        self.laneIX = traci.vehicle.getLaneIndex(ID)
        self.leaderInfo = traci.vehicle.getLeader(ID, 50.)
        # must be set by vehicle creator (PlatoonManager._addVehicle()) to guarantee function in first step
        self.leader = None
        # Whether a possible platooning partner for the vehicle is located further downstream within _catchupDistance
        # (though not necessarily being the immediate leader)
        self.connectedVehicleAhead = False


class PVehicle(object):

    '''
    Vehicle objects for platooning
    '''

    def __init__(self, ID, controlInterval):
        '''Constructor(string, float)

        Create a PVehicle representing a SUMOVehicle for the PlatoonManager. The controlInterval is only piped through
        to the singelton platoon created by the vehicle.
        '''
        # vehicle ID (should be the one used in SUMO)
        self._ID = ID
        # vehicle state (is updated by platoon manager in every step)
        self.state = pVehicleState(ID)

        # store the vehicle's vTypes, speedfactors and lanechangemodes
        self._vTypes = dict()
        self._speedFactors = dict()
        self._laneChangeModes = dict()
        # original vtype, speedFactor and lanechangemodes
        self._vTypes[PlatoonMode.NONE] = traci.vehicle.getTypeID(ID)
        self._speedFactors[PlatoonMode.NONE] = traci.vehicle.getSpeedFactor(ID)
        # This is the default mode
        self._laneChangeModes[PlatoonMode.NONE] = 0b1001010101

        # vTypes, speedFactors and lanechangemodes parametrizing the platoon behaviour
        for mode in [PlatoonMode.LEADER, PlatoonMode.FOLLOWER, PlatoonMode.CATCHUP, PlatoonMode.CATCHUP_FOLLOWER]:
            self._vTypes[mode] = self._determinePlatoonVType(mode)
            self._laneChangeModes[mode] = cfg.LC_MODE[mode]
            self._speedFactors[mode] = cfg.SPEEDFACTOR[mode]
        self._speedFactors[PlatoonMode.NONE] = traci.vehicletype.getSpeedFactor(self._vTypes[PlatoonMode.NONE])

        # Initialize platoon mode to none
        self._currentPlatoonMode = PlatoonMode.NONE
        # the active speed factor is decreased as the waiting time for a mode switch rises
        # (assuming that the main hindrance to switching is too close following)
        self._activeSpeedFactor = cfg.SPEEDFACTOR[self._currentPlatoonMode]
        # The switch impatience factor determines the magnitude of the effect
        # that an increasing waiting time has on the active speed factor:
        # activeSpeedFactor = modeSpecificSpeedFactor/(1+impatienceFactor*waitingTime)
        self._switchImpatienceFactor = cfg.SWITCH_IMPATIENCE_FACTOR
        # create a new platoon containing only this vehicle
        self._platoon = Platoon([self], controlInterval)
        # the time left until splitting from a platoon if loosing coherence as a follower
        self._timeUntilSplit = cfg.PLATOON_SPLIT_TIME
        # Whether split conditions are fulfilled (i.e. leader in th platoon
        # is not found directly in front of the vehicle)
        self._splitConditions = False
        # waiting time for switching into different modes
        self._switchWaitingTime = {}
        self.resetSwitchWaitingTime()

    def _determinePlatoonVType(self, mode):
        '''_determinePlatoonVType(PlatoonMode) -> string

        Returns the type ID corresponding to the given mode. Uses the vehicles vType and the global map PLATOON_VTYPES
        between original and platoon-vTypes. If the original vType is not mapped to any platoon-vtypes,
        the original vType is used for platooning as well
        '''
        global WARNED_DEFAULT
        # original vType
        origVType = self._vTypes[PlatoonMode.NONE]
        if origVType not in cfg.PLATOON_VTYPES \
                or mode not in cfg.PLATOON_VTYPES[origVType] \
                or cfg.PLATOON_VTYPES[origVType][mode] == "":
            if "default" in cfg.PLATOON_VTYPES and mode in cfg.PLATOON_VTYPES["default"]:
                if rp.VERBOSITY >= 1 and not WARNED_DEFAULT[mode]:
                    warn(("Using default vType '%s' for vehicle '%s' (PlatoonMode: '%s'). " +
                          "This warning is issued only once.") %
                         (cfg.PLATOON_VTYPES["default"][mode], self._ID, PlatoonMode(mode).name))
                    WARNED_DEFAULT[mode] = True
                return cfg.PLATOON_VTYPES["default"][mode]
            else:
                if rp.VERBOSITY >= 1 and not WARNED_DEFAULT[mode]:
                    warn(("No vType specified for PlatoonMode '%s' for vehicle '%s'. Behavior within " +
                          "platoon is NOT altered. This warning is issued only once.") % (
                        PlatoonMode(mode).name, self._ID))
                    WARNED_DEFAULT[mode] = True
                return origVType
        if rp.VERBOSITY >= 3:
            report("Using vType '%s' for vehicle '%s' (PlatoonMode: '%s')." %
                   (cfg.PLATOON_VTYPES[origVType][mode], self._ID, PlatoonMode(mode).name))
        return cfg.PLATOON_VTYPES[origVType][mode]

    def getID(self):
        '''getID() -> string

        Returns the vehicle's ID corresponding to the ID of the associated SUMOVehicle.
        '''
        return self._ID

    def getVType(self, mode):
        '''getVType(PlatoonMode) -> string

        Returns the vehicle type associated with the given platooning mode
        '''
        return self._vTypes[mode]

    def getCurrentVType(self):
        '''getCurrentVType() -> string

        Returns the vehicle type associated with the currently active platooning mode
        '''
        return self._vTypes[self._currentPlatoonMode]

    def setPlatoon(self, platoon):
        '''setPlatoon(_platoon)

        Sets the vehicle's platoon to the given.
        '''
        self._platoon = platoon

    def getPlatoon(self):
        '''getPlatoon() -> Platoon

        Returns the vehicle's platoon.
        '''
        return self._platoon

    def isLastInPlatoon(self):
        '''isLastInPlatoon() -> bool

        Returns whether the ego is the last vehicle in its platoon
        '''
        return self._platoon.getVehicles()[-1] == self

    def setPlatoonMode(self, mode):
        '''setPlatoonMode(PlatoonMode)

        Assign this vehicle the vType corresponding to the given 'mode'
        'mode' is from Globals.PlatoonMode Enum. (safety checks have to be done at caller site)
        '''
        if self._currentPlatoonMode == mode:
            # do nothing mode is already chosen
            return

        if rp.VERBOSITY >= 3:
            report("Vehicle '%s': Setting PlatoonMode '%s'" % (self._ID, PlatoonMode(mode).name))

        if self._vTypes[mode] != self._vTypes[self._currentPlatoonMode]:
            traci.vehicle.setType(self._ID, self._vTypes[mode])
        # if self._speedFactors[mode] != self._speedFactors[self._currentPlatoonMode]:
        # Safer to call always since active speed factor mechanism may have changed
        # current speed factor from basic speedfactor
        traci.vehicle.setSpeedFactor(self._ID, self._speedFactors[mode])
        if self._laneChangeModes[mode] != self._laneChangeModes[self._currentPlatoonMode]:
            traci.vehicle.setLaneChangeMode(self._ID, self._laneChangeModes[mode])

        self.resetSplitCountDown()
        self._splitConditions = False
        self._currentPlatoonMode = mode

    def getCurrentPlatoonMode(self):
        ''' getCurrentPlatoonMode() -> PlatoonMode

        Returns the current platoon mode of the vehicle
        '''
        return self._currentPlatoonMode

    def getSwitchWaitingTime(self, mode):
        '''getSwitchWaitingTime(PlatoonMode) -> float

        Returns the waiting time for a switch to the given mode.
        '''
        return self._switchWaitingTime[mode]

    def addSwitchWaitingTime(self, mode, increment):
        '''addSwitchWaitingTime(PlatoonMode, float) -> void

        Increases the mode-specific waiting time for a switch, and decreases the active speed factor accordingly
        '''
        self._switchWaitingTime[mode] += increment
        if rp.VERBOSITY >= 4:
            report("Vehicle '%s' increases switch waiting time for %s to %s" %
                   (self._ID, mode, self._switchWaitingTime[mode]), 3)
        self._setActiveSpeedFactor(self._switchWaitingTime[mode])

    def resetSwitchWaitingTime(self, mode=None):
        '''resetSwitchWaitingTime(PlatoonMode) -> void

        Resets waiting time for a switch to a mode to 0. or, if mode==None, all times are reset to 0.
        The active speed factor is also reset.
        '''
        if rp.VERBOSITY >= 4:
            report("Vehicle '%s' resets switch waiting time." % self._ID, 3)
        if mode is None:
            for e in PlatoonMode:
                self._switchWaitingTime[e] = 0.
        else:
            self._switchWaitingTime[mode] = 0.
        self._resetActiveSpeedFactor()

    def _setActiveSpeedFactor(self, switchWaitingTime):
        ''' setActiveSpeedFactor(float)

        Sets the active speed factor derived from the current vType's speed factor. The higher the
        switch waiting time, the lower the active speed factor (to induce a slowing down, which allows
        to execute the switch safely)
        TODO: This mechanism does not work on highways, where the vehicles maxspeed is determining
              the travel speed and not the road's speed limit.
        '''
        self._activeSpeedFactor = cfg.SPEEDFACTOR[self._currentPlatoonMode] \
            / (1. + self._switchImpatienceFactor * switchWaitingTime)
        traci.vehicle.setSpeedFactor(self._ID, self._activeSpeedFactor)

    def _resetActiveSpeedFactor(self):
        '''resetActiveSpeedFactor()

        Resets the active speed factor to the mode specific base value
        '''
        self._activeSpeedFactor = cfg.SPEEDFACTOR[self._currentPlatoonMode]
        if self._activeSpeedFactor is None:
            assert(self._currentPlatoonMode is PlatoonMode.NONE)
            self._activeSpeedFactor = self._speedFactors[self._currentPlatoonMode]
        traci.vehicle.setSpeedFactor(self._ID, self._activeSpeedFactor)

    def splitCountDown(self, dt):
        '''splitCountDown(double)

        Decreases the time until the vehicle is split from its platoon
        '''
        self._timeUntilSplit -= dt
        if rp.VERBOSITY >= 4:
            report("Time until split from platoon for veh '%s': %s" % (self._ID, self._timeUntilSplit))
        return self._timeUntilSplit

    def resetSplitCountDown(self):
        '''resetSplitCountDown(double)

        Resets the time until the vehicle is split from its platoon to PLATOON_SPLIT_TIME
        '''
        self._timeUntilSplit = cfg.PLATOON_SPLIT_TIME

    def setSplitConditions(self, b=True):
        ''' splitConditions(bool) -> void
        Sets whether splitConditions are satisfied.
        '''
        self._splitConditions = b

    def splitConditions(self):
        ''' splitConditions() -> bool
        Returns whether vehicle did not find its leader in its current platoon in this step.
        '''
        return self._splitConditions

    def isSwitchSafe(self, targetMode, switchImpatience=0.):
        '''isSwitchSafe(PlatoonMode, double) -> bool

        Checks whether it is safe for this vehicle to continue in the target mode.
        The parameter switchImpatience in [0,1] indicates the emergency of the switch
        and controls to which degree the vehicle is disposed to break harder than
        its preferred decel.
        '''
        global vTypeParameters

        # if target mode already equals the current, no safety check is required
        if targetMode == self._currentPlatoonMode:
            return True

        # Check value of switchImpatience
        if (switchImpatience > 1.):
            if rp.VERBOSITY >= 1:
                warn("Given parameter switchImpatience > 1. Assuming == 1.")
            switchImpatience = 1.
        elif (switchImpatience < 0.):
            if rp.VERBOSITY >= 1:
                warn("Given parameter switchImpatience < 0. Assuming == 0.")
            switchImpatience = 0.

        # obtain the preferred deceleration and the tau of the target vType
        decel = vTypeParameters[self._vTypes[targetMode]][tc.VAR_DECEL]
        tau = vTypeParameters[self._vTypes[targetMode]][tc.VAR_TAU]
        speed = self.state.speed

        # If time until switch decreases below 0, this indicates that a switch from platoon to normal mode is required
        maxDecel = vTypeParameters[self._vTypes[targetMode]][tc.VAR_EMERGENCY_DECEL] * switchImpatience \
            + (1. - switchImpatience) * decel

        # check whether a halt at the end of the lane would prohibit the switch to a lower deceleration
        # TODO: restrict check to situations where a halt is really required
        if vTypeParameters[self._vTypes[self._currentPlatoonMode]][tc.VAR_DECEL] > decel:
            distToLaneEnd = traci.lane.getLength(self.state.laneID) - traci.vehicle.getLanePosition(self._ID)
            if self.brakeGap(speed, maxDecel) > distToLaneEnd:
                return False

        # check whether the ego vehicle has a leader, which must be considered
        if self.state.leaderInfo is None:
            return True

        leader = self.state.leader
        gap = self.state.leaderInfo[1]
        minGapDifference = vTypeParameters[self._vTypes[targetMode]][
            tc.VAR_MINGAP] - vTypeParameters[self.getCurrentVType()][tc.VAR_MINGAP]
        gap -= minGapDifference

        if gap < 0.:
            # may arise when minGap of target type differs
            return False

        if leader is None:
            # This may occur if the leader is not connected, so no corresponding PVehicle exists
            # I'm not sure if this is ever called, but wouldn't exclude the possibility
            leaderDecel = traci.vehicle.getDecel(self.state.leaderInfo[0])
            leaderSpeed = traci.vehicle.getSpeed(self.state.leaderInfo[0])
        else:
            leaderDecel = vTypeParameters[leader.getCurrentVType()][tc.VAR_DECEL]
            leaderSpeed = leader.state.speed

        # simplified safety check: consider leaderDecel = max(leaderDecel, decel) and compare brakegaps
        leaderDecel = max(leaderDecel, decel)
        leaderBrakeGap = PVehicle.brakeGap(leaderSpeed, leaderDecel)
        headwayDist = speed * tau
        followerBrakeGap = PVehicle.brakeGap(speed, maxDecel)

        if rp.VERBOSITY >= 4:
            report("leaderSpeed = %s" % leaderSpeed +
                   "\nleaderDecel = %s" % leaderDecel +
                   "\ngap = %s" % gap +
                   "\nleaderBrakeGap = %s" % leaderBrakeGap +
                   "\nspeed = %s" % speed +
                   "\ndecel = %s" % decel +
                   "\nfollowerBrakeGap = %s" % followerBrakeGap +
                   "\nheadwayDist = %s" % headwayDist)

        # TODO: test without headway...
        return gap + leaderBrakeGap - followerBrakeGap - headwayDist > 0

    @staticmethod
    def brakeGap(speed, decel):
        '''_brakeGap(double) -> double

        Return the break gap given a constant deceleration
        '''
        if decel <= 0.:
            return float("inf")
        return speed * speed / (2.0 * decel)

    def __str__(self):
        return "<PVehicle '%s'>" % self._ID
