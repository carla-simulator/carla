/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2003-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    MSGlobals.h
/// @author  Daniel Krajzewicz
/// @author  Christian Roessel
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    late summer 2003
///
// Some static variables for faster access
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MELoop;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSGlobals
 * This class holds some static variables, filled mostly with values coming
 *  from the command line or the simulation configuration file.
 * They are stored herein to allow a faster access than from the options
 *  container.
 */
class MSGlobals {
public:
    /// Information whether empty edges shall be written on dump
    static bool gOmitEmptyEdgesOnDump;

    /* Allows switching between time step integration methods
     * "Semi-Implicit Euler" (default) and the ballistic update rule. */
    static bool gSemiImplicitEulerUpdate;

    /** Information how long the simulation shall wait until it recognizes
        a vehicle as a grid lock participant */
    static SUMOTime gTimeToGridlock;

    /** The time to detect grid locks on highways */
    static SUMOTime gTimeToGridlockHighways;

    /** Information how long a vehicle must wait for impatience to grow from 0 to 1 */
    static SUMOTime gTimeToImpatience;

    /// Information whether the simulation regards internal lanes
    static bool gUsingInternalLanes;

    /** Vehicles on internal lanes (even partially) with a waiting time that exceeds this threshold
     * no longer block cross-traffic on the junction */
    static SUMOTime gIgnoreJunctionBlocker;

    /** information whether the network shall check for collisions */
    static bool gCheck4Accidents;

    /** information whether the routes shall be checked for connectivity */
    static bool gCheckRoutes;

    /** information Duration of a lane change maneuver */
    static SUMOTime gLaneChangeDuration;

    /** Lateral resolution within a lane */
    static double gLateralResolution;

    /// Information whether a state has been loaded
    static bool gStateLoaded;

    /** Information whether mesosim shall be used */
    static bool gUseMesoSim;

    /** Information whether limited junction control shall be used */
    static bool gMesoLimitedJunctionControl;

    /** Information whether overtaking is enabled in the mesoscopic simulation */
    static bool gMesoOvertaking;

    /** scaling factor for macroscopic time penalty when passing tls controlled intersection */
    static double gMesoTLSPenalty;

    /** scaling factor for macroscopic headway penalty when passing tls controlled intersection */
    static double gMesoTLSFlowPenalty;

    /** penalty time for passing a minor link */
    static SUMOTime gMesoMinorPenalty;

    /// mesoscopic simulation infrastructure
    static MELoop* gMesoNet;

    /// length of memory for waiting times (in millisecs)
    static SUMOTime gWaitingTimeMemory;

    /// default value for the interval between two action points for MSVehicle (defaults to DELTA_T)
    static SUMOTime gActionStepLength;

    // whether Kirchhoff's laws are used for solving overhead wire circuit
    static bool gOverheadWireSolver;

    /// encoding of the string-option default.emergencydecel
    static double gDefaultEmergencyDecel;

    /// whether unit tests are being run
    static bool gUnitTests;

    /// whether the simulationLoop is in the lane changing phase
    static bool gComputeLC;

    /// how many threads to use for simulation
    static int gNumSimThreads;

    /// how many threads to use
    static int gNumThreads;

    /// treshold for warning about strong deceleration
    static double gEmergencyDecelWarningThreshold;

    /// time penalty for passing a minor link when routing
    static double gMinorPenalty;

    /// whether parking simulation includes manoeuver time and any associated lane blocking
    static bool gModelParkingManoeuver;

    /// whether sublane simulation is enabled (sublane model or continuous lanechanging)
    static bool gSublane;

    /// @brief The tolerance to apply when matching waiting persons and vehicles
    static double gStopTolerance;

    /// @brief Whether lefthand-drive is being simulated
    static bool gLefthand;
};
