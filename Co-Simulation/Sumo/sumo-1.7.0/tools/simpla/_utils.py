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

# @file    _utils.py
# @author Leonhard Luecken
# @date   2018-06-26

'''
Utility functions and classes for simpla
'''

import traci


class SimplaException(Exception):
    '''
    Simple exception raised by simpla.
    '''

    def __init__(self, *args, **kwargs):
        super(SimplaException, self).__init__(*args, **kwargs)


_activeGapControllers = {}
DEBUG_GAP_CONTROL = False


class GapController(traci.StepListener):
    '''
    class that manages opening a gap in front of its assigned vehicle
    '''

    def __init__(self, vehID, desiredGap, desiredSpeedDiff, maximumDecel, duration):
        '''
        GapController()

        @see openGap()
        '''
        self._desiredGap = float(desiredGap)
        self._desiredSpeedDiff = float(desiredSpeedDiff)
        self._maximumDecel = float(maximumDecel)
        self._duration = float(duration)
        self._vehID = vehID

        # gains for linear controller that should stabilize the desired gap
#         self._gapGain = 0.23
#         self._speedGain = 0.07
        self._gapGain = 0.04
        self._speedGain = 0.8

        # leaderHorizon coefficients assures that the leader is detected even if
        # the leader is farther away than the desired gap. The look ahead distance
        # given to traci.vehicle.getLeader is min(desiredGap*horizonFactor, minHorizon)
        self._leaderHorizonFactor = 1.5
        self._minimalLeaderHorizon = 50

        # step-length of simulation
        self._TS = traci.simulation.getDeltaT()
        # last received speed info
        self._leaderSpeed = None
        self._egoSpeed = None
        # whether controlled vehicle has an active setSpeed command
        self._speedControlActive = False

        if DEBUG_GAP_CONTROL:
            print("Created GapController for vehicle %s" % self._vehID)
            print("desiredGap=%s, desiredSpeedDiff=%s, maximumDecel=%s, duration=%s" % (
                  desiredGap, desiredSpeedDiff, maximumDecel, duration))

    def step(self, s=0):
        '''
        Perform one control step and count down the activity period for the controller
        '''
        self._applyControl()

        self._duration -= self._TS

        if DEBUG_GAP_CONTROL:
            print("Step of gap controller with listenerID=%s" % self.getID())
            print("Remaining activity period: %s" % self._duration)

        return self._duration > 0

    def _applyControl(self):
        leaderInfo = traci.vehicle.getLeader(self._vehID, max(self._desiredGap * self._leaderHorizonFactor,
                                                              self._minimalLeaderHorizon))
#         leaderInfo = traci.vehicle.getLeader(self._vehID, self._desiredGap)
        if leaderInfo is None:
            # no leader
            self._releaseControl()
            print("Lost leader...")
            return

        (leaderID, gap) = leaderInfo
        assert(leaderID != "")

        self._egoSpeed = traci.vehicle.getSpeed(self._vehID)
        self._leaderSpeed = traci.vehicle.getSpeed(leaderID)
        speedDiff = self._leaderSpeed - self._egoSpeed

        # first determine a desired acceleration from the controller
        accel = self._accel(speedDiff, gap)

        if DEBUG_GAP_CONTROL:
            print("GapController's acceleration control for veh '%s':" % self._vehID)
            print("accel(speedDiff=%s, gapError=%s) = %s" % (speedDiff, gap - self._desiredGap, accel))

        # assure that the speedDifference is not increased above the desired value
        if (accel <= 0 and speedDiff - self._TS * accel >= self._desiredSpeedDiff):
            accel = (speedDiff - self._desiredSpeedDiff) / self._TS
        elif (accel >= 0 and speedDiff - self._TS * accel <= -self._desiredSpeedDiff):
            accel = (speedDiff + self._desiredSpeedDiff) / self._TS
        if DEBUG_GAP_CONTROL:
            print("Truncating to prevent exceeding desired speedDiff results in\n   accel=%s" % accel)

        # assure it does not exceed the [maxdecel, maxaccel] bounds (assuming maximumAccel = maximumDecel)
        maximumAccel = self._maximumDecel
        accel = max(min(accel, maximumAccel), -self._maximumDecel)
        if DEBUG_GAP_CONTROL:
            print("Truncating to maximal decel/accel results in\n   accel=%s" % accel)

        # apply the calculated acceleration
        self._imposeSpeed(self._egoSpeed + self._TS * accel)

    def _accel(self, speedDiff, gap):
        '''
        Returns the acceleration computed by a linear controller
        '''
        gapError = gap - self._desiredGap
        return self._gapGain * gapError + self._speedGain * speedDiff

    def _releaseControl(self):
        '''
        Releases the vehicle's speed control such that sumo may take over again
        '''
        if self._speedControlActive:
            traci.vehicle.setSpeed(self._vehID, -1)
            self._speedControlActive = False

    def _imposeSpeed(self, speed):
        '''
        Sends a setSpeed command to the vehicle via traci
        '''
        traci.vehicle.setSpeed(self._vehID, max(speed, 0))
        self._speedControlActive = True

    def cleanUp(self):
        global _activeGapControllers
        del _activeGapControllers[self._vehID]
        traci.StepListener.cleanUp(self)
        self._releaseControl()
        if DEBUG_GAP_CONTROL:
            print("Cleaned up stepListener %s." % self.getID())


def openGap(vehID, desiredGap, desiredSpeedDiff, maximumDecel, duration):
    '''
    openGap(string, float>0, float>0, float>0, float>0)

    vehID - ID of the vehicle to be controlled
    desiredGap - gap that shall be established
    desiredSpeedDiff - rate at which the gap is open if possible
    maximumDecel - maximal deceleration at which the desiredSpeedDiff is tried to be approximated
    duration - The period for which the gap control should be active

    This methods adds a controller for the opening of a gap in front of the given vehicle.
    The controller stays active for a period of the given duration.
    If a leader is closer than the desiredGap, the controller tries to establish the desiredGap by inducing the
    given speedDifference, while not braking harder than maximumDecel.
    An object of the class GapCreator is created to manage the vehicle state and is added to traci as a stepListener.
    '''
    global _activeGapControllers

    if DEBUG_GAP_CONTROL:
        print("openGap()")

    # Check type error
    errorMsg = None
    if desiredGap <= 0:
        errorMsg = "simpla.openGap(): Parameter desiredGap has to be a positive float (given value = %s)." % desiredGap
    elif desiredSpeedDiff <= 0:
        errorMsg = "simpla.openGap(): Parameter desiredSpeedDiff has to be a positive float (given value = %s)." % (
            desiredSpeedDiff)
    elif maximumDecel <= 0:
        errorMsg = "simpla.openGap(): Parameter maximumDecel has to be a positive float (given value = %s)." % (
            maximumDecel)
    elif duration <= 0:
        errorMsg = "simpla.openGap(): Parameter duration has to be a positive float (given value = %s)." % duration
    if errorMsg is not None:
        raise SimplaException(errorMsg)

    # remove any previous controller attached to the vehicle
    removeGapController(vehID)
    gc = GapController(vehID, desiredGap, desiredSpeedDiff, maximumDecel, duration)
    listenerID = traci.addStepListener(gc)
    _activeGapControllers[vehID] = listenerID
    if DEBUG_GAP_CONTROL:
        print("Active controllers: %s." % (_activeGapControllers))


def removeGapController(vehID):
    '''
    Removes any current gap controller
    '''
    global _activeGapControllers
    if DEBUG_GAP_CONTROL:
        print("removeGapController(%s)\nactive: %s." % (vehID, _activeGapControllers))
    if vehID in _activeGapControllers:
        listenerID = _activeGapControllers[vehID]
        traci.removeStepListener(listenerID)
