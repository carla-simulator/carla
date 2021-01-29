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
/// @file    SUMORouteLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 6 Nov 2002
///
// A class that performs the loading of routes
/****************************************************************************/
#include <config.h>

#include <utils/xml/SUMOSAXReader.h>
#include <utils/xml/XMLSubSys.h>
#include "SUMORouteHandler.h"
#include "SUMORouteLoader.h"


// ===========================================================================
// method definitions
// ===========================================================================
SUMORouteLoader::SUMORouteLoader(SUMORouteHandler* handler)
    : myParser(nullptr), myMoreAvailable(true), myHandler(handler) {
    myParser = XMLSubSys::getSAXReader(*myHandler, false, true);
    if (!myParser->parseFirst(myHandler->getFileName())) {
        throw ProcessError("Can not read XML-file '" + myHandler->getFileName() + "'.");
    }
}


SUMORouteLoader::~SUMORouteLoader() {
    delete myParser;
    delete myHandler;
}


SUMOTime
SUMORouteLoader::loadUntil(SUMOTime time) {
    // read only when further data is available, no error occurred
    //  and vehicles may be found in the between the departure time of
    //  the last read vehicle and the time to read until
    if (!myMoreAvailable) {
        return SUMOTime_MAX;
    }
    // read vehicles until specified time or the period to read vehicles
    //  until is reached
    while (myHandler->getLastDepart() <= time) {
        if (!myParser->parseNext()) {
            // no data available anymore
            myMoreAvailable = false;
            return SUMOTime_MAX;
        }
    }
    return myHandler->getLastDepart();
}


bool
SUMORouteLoader::moreAvailable() const {
    return myMoreAvailable;
}


SUMOTime
SUMORouteLoader::getFirstDepart() const {
    return myHandler->getFirstDepart();
}


/****************************************************************************/
