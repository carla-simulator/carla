/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    StdDefs.cpp
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    2014-01-07
///
/****************************************************************************/
#include "StdDefs.h"


// set by option --precision (see SystemFrame.cpp)
int gPrecision = 2;
int gPrecisionGeo = 6;
bool gHumanReadableTime = false;
bool gSimulation = false;
double gWeightsRandomFactor = 1;

bool gDebugFlag1 = false;
bool gDebugFlag2 = false;
bool gDebugFlag3 = false;
bool gDebugFlag4 = false;

double truncate(double x, int fractionBits) {
    return ceil(x * (1 << fractionBits)) / (1 << fractionBits);
}

double roundBits(double x, int fractionBits) {
    const double x2 = x * (1 << fractionBits);
    const double rounded = x2 < 0 ? ceil(x2 - 0.5) : floor(x2 + 0.5);
    return rounded / (1 << fractionBits);
}


/****************************************************************************/
