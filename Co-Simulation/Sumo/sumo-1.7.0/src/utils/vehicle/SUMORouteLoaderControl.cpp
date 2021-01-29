/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMORouteLoaderControl.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Wed, 06 Nov 2002
///
// Class responsible for loading of routes from some files
/****************************************************************************/
#include <config.h>

#include <vector>
#include <utils/common/StdDefs.h>
#include "SUMORouteLoader.h"
#include "SUMORouteLoaderControl.h"


// ===========================================================================
// method definitions
// ===========================================================================
SUMORouteLoaderControl::SUMORouteLoaderControl(SUMOTime inAdvanceStepNo):
    myFirstLoadTime(SUMOTime_MAX),
    myCurrentLoadTime(-SUMOTime_MAX),
    myInAdvanceStepNo(inAdvanceStepNo),
    myRouteLoaders(),
    myLoadAll(inAdvanceStepNo <= 0),
    myAllLoaded(false) {
}


SUMORouteLoaderControl::~SUMORouteLoaderControl() {
    for (std::vector<SUMORouteLoader*>::iterator i = myRouteLoaders.begin();
            i != myRouteLoaders.end(); ++i) {
        delete (*i);
    }
}


void
SUMORouteLoaderControl::add(SUMORouteLoader* loader) {
    myRouteLoaders.push_back(loader);
}


void
SUMORouteLoaderControl::loadNext(SUMOTime step) {
    // check whether new vehicles shall be loaded
    //  return if not
    if (myAllLoaded) {
        return;
    }
    if (myCurrentLoadTime > step) {
        return;
    }
    const SUMOTime loadMaxTime = myLoadAll ? SUMOTime_MAX : MAX2(myCurrentLoadTime + myInAdvanceStepNo, step);
    myCurrentLoadTime = SUMOTime_MAX;
    // load all routes for the specified time period
    bool furtherAvailable = false;
    for (SUMORouteLoader* loader : myRouteLoaders) {
        myCurrentLoadTime = MIN2(myCurrentLoadTime, loader->loadUntil(loadMaxTime));
        if (loader->getFirstDepart() != -1) {
            myFirstLoadTime = MIN2(myFirstLoadTime, loader->getFirstDepart());
        }
        furtherAvailable |= loader->moreAvailable();
    }
    myAllLoaded = !furtherAvailable;
}


/****************************************************************************/
