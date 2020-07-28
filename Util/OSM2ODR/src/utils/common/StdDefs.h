/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    StdDefs.h
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Fri, 29.04.2005
///
//
/****************************************************************************/
#pragma once
#include <string>
#include <cmath>
#include <limits>

/* avoiding compiler warning unreferenced parameter */
#define UNUSED_PARAMETER(x)  ((void)(x))

#ifdef _MSC_VER
#define FALLTHROUGH /* do nothing */
#elif __GNUC__ < 7
#define FALLTHROUGH /* do nothing */
#else
#define FALLTHROUGH __attribute__((fallthrough))
#endif

/// @brief the maximum number of connections across an intersection
#define  SUMO_MAX_CONNECTIONS 256

class RGBColor;

/* -------------------------------------------------------------------------
 * some constant defaults used by SUMO
 * ----------------------------------------------------------------------- */
const double SUMO_const_laneWidth = (double) 3.2;
const double SUMO_const_laneOffset = (double) 0;
const double SUMO_const_halfLaneWidth = SUMO_const_laneWidth / 2;
const double SUMO_const_quarterLaneWidth = SUMO_const_laneWidth / 4;
const double SUMO_const_laneWidthAndOffset = SUMO_const_laneWidth + SUMO_const_laneOffset;
const double SUMO_const_halfLaneAndOffset = SUMO_const_halfLaneWidth + SUMO_const_laneOffset;
const double SUMO_const_laneMarkWidth = (double) 0.1;
const double SUMO_const_waitingPersonWidth = 0.8;
const double SUMO_const_waitingPersonDepth = 0.67;
const double SUMO_const_waitingContainerWidth = 2.4;
const double SUMO_const_waitingContainerDepth = 6.1;

/// @brief the speed threshold at which vehicles are considered as halting
const double SUMO_const_haltingSpeed = (double) 0.1;

const double INVALID_DOUBLE = std::numeric_limits<double>::max();

/// @brief version for written networks and default version for loading
const double NETWORK_VERSION = 1.6;


/* -------------------------------------------------------------------------
 * templates for mathematical functions missing in some c++-implementations
 * ----------------------------------------------------------------------- */
template<typename T>
inline T
MIN2(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
inline T
MAX2(T a, T b) {
    return a > b ? a : b;
}


template<typename T>
inline T
MIN3(T a, T b, T c) {
    return MIN2(c, a < b ? a : b);
}


template<typename T>
inline T
MAX3(T a, T b, T c) {
    return MAX2(c, a > b ? a : b);
}


template<typename T>
inline T
MIN4(T a, T b, T c, T d) {
    return MIN2(MIN2(a, b), MIN2(c, d));
}


template<typename T>
inline T
MAX4(T a, T b, T c, T d) {
    return MAX2(MAX2(a, b), MAX2(c, d));
}


template<typename T>
inline T
ISNAN(T a) {
    volatile T d = a;
    return d != d;
}


/// the precision for floating point outputs
extern int gPrecision;
extern int gPrecisionGeo; // for lon,lat
extern bool gHumanReadableTime;
extern bool gSimulation; // whether the current application is sumo or sumo-gui (as opposed to a router)
extern double gWeightsRandomFactor; // randomization for edge weights


/// @brief global utility flags for debugging
extern bool gDebugFlag1;
extern bool gDebugFlag2;
extern bool gDebugFlag3;
extern bool gDebugFlag4;

// synchronized output to stdout with << (i.e. DEBUGOUT(SIMTIME << " var=" << var << "\n")
#define DEBUGOUT(msg) {std::ostringstream oss; oss << msg; std::cout << oss.str();}

/// @brief discrds mantissa bits beyond the given number
double truncate(double x, int fractionBits);

/// @brief round to the given number of mantissa bits beyond the given number
double roundBits(double x, int fractionBits);
