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
/// @file    ODAmitranHandler.cpp
/// @author  Michael Behrisch
/// @date    27.03.2014
///
// An XML-Handler for Amitran OD matrices
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>
#include "ODMatrix.h"
#include "ODAmitranHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
ODAmitranHandler::ODAmitranHandler(ODMatrix& matrix, const std::string& file)
    : SUMOSAXHandler(file), myMatrix(matrix) {}


ODAmitranHandler::~ODAmitranHandler() {}


void
ODAmitranHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    bool ok = true;
    switch (element) {
        case SUMO_TAG_ACTORCONFIG:
            myVehicleType = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            break;
        case SUMO_TAG_TIMESLICE:
            myBegin = attrs.get<int>(SUMO_ATTR_STARTTIME, myVehicleType.c_str(), ok);
            myEnd = myBegin + attrs.get<int>(SUMO_ATTR_DURATION, myVehicleType.c_str(), ok);
            if (myBegin >= myEnd) {
                WRITE_ERROR("Invalid duration for timeSlice starting " + toString(myBegin) + ".");
            }
            break;
        case SUMO_TAG_OD_PAIR:
            myMatrix.add(attrs.get<double>(SUMO_ATTR_AMOUNT, myVehicleType.c_str(), ok),
                         myBegin, myEnd, attrs.get<std::string>(SUMO_ATTR_ORIGIN, myVehicleType.c_str(), ok),
                         attrs.get<std::string>(SUMO_ATTR_DESTINATION, myVehicleType.c_str(), ok), myVehicleType);
            break;
        default:
            break;
    }
}


/****************************************************************************/
