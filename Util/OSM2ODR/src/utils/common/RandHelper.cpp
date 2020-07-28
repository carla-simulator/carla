/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    RandHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Tue, 29.05.2005
///
//
/****************************************************************************/
#include <config.h>

#include <ctime>
#include <utils/options/OptionsCont.h>
#include <utils/common/SysUtils.h>
#include "RandHelper.h"


// ===========================================================================
// static member variables
// ===========================================================================
std::mt19937 RandHelper::myRandomNumberGenerator;
#ifdef DEBUG_RANDCALLS
std::map<std::mt19937*, int> RandHelper::myCallCount;
std::map<std::mt19937*, int> RandHelper::myRngId;
int RandHelper::myDebugIndex(7);
#endif


// ===========================================================================
// member method definitions
// ===========================================================================
void
RandHelper::insertRandOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    // registers random number options
    oc.addOptionSubTopic("Random Number");

    oc.doRegister("random", new Option_Bool(false));
    oc.addSynonyme("random", "abs-rand", true);
    oc.addDescription("random", "Random Number", "Initialises the random number generator with the current system time");

    oc.doRegister("seed", new Option_Integer(23423));
    oc.addSynonyme("seed", "srand", true);
    oc.addDescription("seed", "Random Number", "Initialises the random number generator with the given value");
}


void
RandHelper::initRand(std::mt19937* which, const bool random, const int seed) {
    if (which == nullptr) {
        which = &myRandomNumberGenerator;
    }
#ifdef DEBUG_RANDCALLS
    myRngId[which] = myRngId.size();
#endif
    if (random) {
        which->seed((unsigned long)time(nullptr));
    } else {
        which->seed(seed);
    }
}


void
RandHelper::initRandGlobal(std::mt19937* which) {
    OptionsCont& oc = OptionsCont::getOptions();
    initRand(which, oc.getBool("random"), oc.getInt("seed"));
}


/****************************************************************************/
