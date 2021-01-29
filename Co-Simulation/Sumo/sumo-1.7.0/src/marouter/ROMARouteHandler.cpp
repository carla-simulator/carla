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
/// @file    ROMARouteHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Parser and container for routes during their loading
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <od/ODMatrix.h>
#include "ROMARouteHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
ROMARouteHandler::ROMARouteHandler(ODMatrix& matrix) :
    SUMOSAXHandler(""), myMatrix(matrix) {
    if (OptionsCont::getOptions().isSet("taz-param")) {
        myTazParamKeys = OptionsCont::getOptions().getStringVector("taz-param");
    }
}


ROMARouteHandler::~ROMARouteHandler() {
}


void
ROMARouteHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    if (element == SUMO_TAG_TRIP || element == SUMO_TAG_VEHICLE) {
        myVehicleParameter = SUMOVehicleParserHelper::parseVehicleAttributes(element, attrs, true);
        if (!myVehicleParameter->wasSet(VEHPARS_FROM_TAZ_SET) && attrs.hasAttribute(SUMO_ATTR_FROM)) {
            myVehicleParameter->fromTaz = attrs.getString(SUMO_ATTR_FROM);
        }
        if (!myVehicleParameter->wasSet(VEHPARS_TO_TAZ_SET) && attrs.hasAttribute(SUMO_ATTR_TO)) {
            myVehicleParameter->toTaz = attrs.getString(SUMO_ATTR_TO);
        }
    } else if (element == SUMO_TAG_PARAM && !myTazParamKeys.empty()) {
        if (attrs.getString(SUMO_ATTR_KEY) == myTazParamKeys[0]) {
            myVehicleParameter->fromTaz = attrs.getString(SUMO_ATTR_VALUE);
            myVehicleParameter->parametersSet |= VEHPARS_FROM_TAZ_SET;
        }
        if (myTazParamKeys.size() > 1 && attrs.getString(SUMO_ATTR_KEY) == myTazParamKeys[1]) {
            myVehicleParameter->toTaz = attrs.getString(SUMO_ATTR_VALUE);
            myVehicleParameter->parametersSet |= VEHPARS_TO_TAZ_SET;
        }
    }
}


void
ROMARouteHandler::myEndElement(int element) {
    if (element == SUMO_TAG_TRIP || element == SUMO_TAG_VEHICLE) {
        if (myVehicleParameter->fromTaz == "" || myVehicleParameter->toTaz == "") {
            WRITE_WARNING("No origin or no destination given, ignoring '" + myVehicleParameter->id + "'!");
        } else {
            myMatrix.add(myVehicleParameter->id, myVehicleParameter->depart,
                         myVehicleParameter->fromTaz, myVehicleParameter->toTaz, myVehicleParameter->vtypeid,
                         !myVehicleParameter->wasSet(VEHPARS_FROM_TAZ_SET), !myVehicleParameter->wasSet(VEHPARS_TO_TAZ_SET));
        }
        delete myVehicleParameter;
    }
}


/****************************************************************************/
