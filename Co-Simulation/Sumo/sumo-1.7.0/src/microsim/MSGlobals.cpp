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
/// @file    MSGlobals.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    late summer 2003
///
// Some static variables for faster access
/****************************************************************************/
#include <config.h>

#include "MSGlobals.h"


// ===========================================================================
// static member variable definitions
// ===========================================================================
bool MSGlobals::gOmitEmptyEdgesOnDump;

bool MSGlobals::gUsingInternalLanes;
SUMOTime MSGlobals::gIgnoreJunctionBlocker;

SUMOTime MSGlobals::gTimeToGridlock;
SUMOTime MSGlobals::gTimeToGridlockHighways;
SUMOTime MSGlobals::gTimeToImpatience;

bool MSGlobals::gCheck4Accidents;

bool MSGlobals::gCheckRoutes;

SUMOTime MSGlobals::gLaneChangeDuration;

double MSGlobals::gLateralResolution;

bool MSGlobals::gStateLoaded;
bool MSGlobals::gUseMesoSim;
bool MSGlobals::gMesoLimitedJunctionControl;
bool MSGlobals::gMesoOvertaking;
double MSGlobals::gMesoTLSPenalty;
double MSGlobals::gMesoTLSFlowPenalty;
SUMOTime MSGlobals::gMesoMinorPenalty;
MELoop* MSGlobals::gMesoNet;

bool MSGlobals::gOverheadWireSolver;

bool MSGlobals::gSemiImplicitEulerUpdate;

SUMOTime MSGlobals::gWaitingTimeMemory;

SUMOTime MSGlobals::gActionStepLength;

double MSGlobals::gDefaultEmergencyDecel(-1); // default for unitTest

bool MSGlobals::gUnitTests(false);

bool MSGlobals::gComputeLC;

int MSGlobals::gNumSimThreads;
int MSGlobals::gNumThreads;

double MSGlobals::gEmergencyDecelWarningThreshold(1);

double MSGlobals::gMinorPenalty(0);

bool MSGlobals::gModelParkingManoeuver;

bool MSGlobals::gSublane(false);

double MSGlobals::gStopTolerance;

bool MSGlobals::gLefthand(false);


/****************************************************************************/
