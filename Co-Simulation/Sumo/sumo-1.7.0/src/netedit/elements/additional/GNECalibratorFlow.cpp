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
/// @file    GNECalibratorFlow.cpp
/// @author  Pablo Alvarez Lopez
/// @date    March 2016
///
//
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/changes/GNEChange_Attribute.h>

#include "GNECalibratorFlow.h"


// ===========================================================================
// member method definitions
// ===========================================================================


GNECalibratorFlow::GNECalibratorFlow(GNEAdditional* calibratorParent) :
    GNEAdditional(calibratorParent->getNet(), GLO_CALIBRATOR, SUMO_TAG_FLOW_CALIBRATOR, "", false,
{}, {}, {}, {calibratorParent}, {}, {}, {}, {}),
myVehicleType(calibratorParent->getNet()->retrieveDemandElement(SUMO_TAG_VTYPE, DEFAULT_VTYPE_ID)),
myRoute(calibratorParent->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_ROUTE).begin()->second) {
    // fill calibrator flows with default values
    setDefaultValues();
}


GNECalibratorFlow::GNECalibratorFlow(GNEAdditional* calibratorParent, GNEDemandElement* vehicleType, GNEDemandElement* route, const std::string& vehsPerHour, const std::string& speed,
                                     const RGBColor& color, const std::string& departLane, const std::string& departPos, const std::string& departSpeed, const std::string& arrivalLane,
                                     const std::string& arrivalPos, const std::string& arrivalSpeed, const std::string& line, int personNumber, int containerNumber, bool reroute,
                                     const std::string& departPosLat, const std::string& arrivalPosLat, SUMOTime begin, SUMOTime end) :
    GNEAdditional(calibratorParent->getNet(), GLO_CALIBRATOR, SUMO_TAG_FLOW_CALIBRATOR, "", false,
{}, {}, {}, {calibratorParent}, {}, {}, {}, {}),
myVehicleType(vehicleType),
myRoute(route),
myVehsPerHour(vehsPerHour),
mySpeed(speed),
myColor(color),
myDepartLane(departLane),
myDepartPos(departPos),
myDepartSpeed(departSpeed),
myArrivalLane(arrivalLane),
myArrivalPos(arrivalPos),
myArrivalSpeed(arrivalSpeed),
myLine(line),
myPersonNumber(personNumber),
myContainerNumber(containerNumber),
myReroute(reroute),
myDepartPosLat(departPosLat),
myArrivalPosLat(arrivalPosLat),
myBegin(begin),
myEnd(end) {
}


GNECalibratorFlow::~GNECalibratorFlow() {}


void
GNECalibratorFlow::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNECalibratorFlow::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


void
GNECalibratorFlow::updateGeometry() {
    // This additional doesn't own a geometry
}


Position
GNECalibratorFlow::getPositionInView() const {
    return getParentAdditionals().at(0)->getPositionInView();
}


Boundary
GNECalibratorFlow::getCenteringBoundary() const {
    return getParentAdditionals().at(0)->getCenteringBoundary();
}


void
GNECalibratorFlow::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


std::string
GNECalibratorFlow::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNECalibratorFlow::drawGL(const GUIVisualizationSettings& /* s */) const {
    // Currently This additional isn't drawn
}


std::string
GNECalibratorFlow::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_TYPE:
            return myVehicleType->getID();
        case SUMO_ATTR_ROUTE:
            return myRoute->getID();
        case SUMO_ATTR_VEHSPERHOUR:
            return myVehsPerHour;
        case SUMO_ATTR_SPEED:
            return mySpeed;
        case SUMO_ATTR_COLOR:
            return toString(myColor);
        case SUMO_ATTR_BEGIN:
            return time2string(myBegin);
        case SUMO_ATTR_END:
            return time2string(myEnd);
        case SUMO_ATTR_DEPARTLANE:
            return myDepartLane;
        case SUMO_ATTR_DEPARTPOS:
            return myDepartPos;
        case SUMO_ATTR_DEPARTSPEED:
            return myDepartSpeed;
        case SUMO_ATTR_ARRIVALLANE:
            return myArrivalLane;
        case SUMO_ATTR_ARRIVALPOS:
            return myArrivalPos;
        case SUMO_ATTR_ARRIVALSPEED:
            return myArrivalSpeed;
        case SUMO_ATTR_LINE:
            return myLine;
        case SUMO_ATTR_PERSON_NUMBER:
            return toString(myPersonNumber);
        case SUMO_ATTR_CONTAINER_NUMBER:
            return toString(myContainerNumber);
        case SUMO_ATTR_REROUTE:
            return toString(myReroute);
        case SUMO_ATTR_DEPARTPOS_LAT:
            return myDepartPosLat;
        case SUMO_ATTR_ARRIVALPOS_LAT:
            return myArrivalPosLat;
        case GNE_ATTR_PARENT:
            return getParentAdditionals().at(0)->getID();
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNECalibratorFlow::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_BEGIN:
            return STEPS2TIME(myBegin);
        case SUMO_ATTR_END:
            return STEPS2TIME(myEnd);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNECalibratorFlow::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_TYPE:
        case SUMO_ATTR_ROUTE:
        case SUMO_ATTR_COLOR:
        case SUMO_ATTR_VEHSPERHOUR:
        case SUMO_ATTR_SPEED:
        case SUMO_ATTR_BEGIN:
        case SUMO_ATTR_END:
        case SUMO_ATTR_DEPARTLANE:
        case SUMO_ATTR_DEPARTPOS:
        case SUMO_ATTR_DEPARTSPEED:
        case SUMO_ATTR_ARRIVALLANE:
        case SUMO_ATTR_ARRIVALPOS:
        case SUMO_ATTR_ARRIVALSPEED:
        case SUMO_ATTR_LINE:
        case SUMO_ATTR_PERSON_NUMBER:
        case SUMO_ATTR_CONTAINER_NUMBER:
        case SUMO_ATTR_REROUTE:
        case SUMO_ATTR_DEPARTPOS_LAT:
        case SUMO_ATTR_ARRIVALPOS_LAT:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNECalibratorFlow::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_TYPE:
            return SUMOXMLDefinitions::isValidTypeID(value) && (myNet->retrieveDemandElement(SUMO_TAG_VTYPE, value, false) != nullptr);
        case SUMO_ATTR_ROUTE:
            return SUMOXMLDefinitions::isValidVehicleID(value) && (myNet->retrieveAdditional(SUMO_TAG_ROUTE, value, false) != nullptr);
        case SUMO_ATTR_VEHSPERHOUR:
            if (value.empty()) {
                // speed and vehsPerHour cannot be empty at the same time
                if (mySpeed.empty()) {
                    return false;
                } else {
                    return true;
                }
            } else if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_SPEED:
            if (value.empty()) {
                // speed and vehsPerHour cannot be empty at the same time
                if (myVehsPerHour.empty()) {
                    return false;
                } else {
                    return true;
                }
            } else if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_COLOR:
            return canParse<RGBColor>(value);
        case SUMO_ATTR_BEGIN:
            return canParse<SUMOTime>(value);
        case SUMO_ATTR_END:
            return canParse<SUMOTime>(value);
        case SUMO_ATTR_DEPARTLANE:
            if ((value == "random") || (value == "free") || (value == "allowed") || (value == "best") || (value == "first")) {
                return true;
            } else {
                return (myNet->retrieveLane(value, false) != nullptr);
            }
        case SUMO_ATTR_DEPARTPOS:
            if ((value == "random") || (value == "free") || (value == "random_free") || (value == "base") || (value == "last")) {
                return true;
            } else {
                return canParse<double>(value);
            }
        case SUMO_ATTR_DEPARTSPEED:
            if ((value == "random") || (value == "max")) {
                return true;
            } else {
                return canParse<double>(value);
            }
        case SUMO_ATTR_ARRIVALLANE:
            if (value == "current") {
                return true;
            } else {
                return (myNet->retrieveLane(value, false) != nullptr);
            }
        case SUMO_ATTR_ARRIVALPOS:
            if ((value == "random") || (value == "max")) {
                return true;
            } else {
                return canParse<double>(value);
            }
        case SUMO_ATTR_ARRIVALSPEED:
            if (value == "current") {
                return true;
            } else {
                return canParse<double>(value);
            }
        case SUMO_ATTR_LINE:
            return true;
        case SUMO_ATTR_PERSON_NUMBER:
            return canParse<int>(value) && parse<int>(value) >= 0;
        case SUMO_ATTR_CONTAINER_NUMBER:
            return canParse<int>(value) && parse<int>(value) >= 0;
        case SUMO_ATTR_REROUTE:
            return canParse<bool>(value);
        case SUMO_ATTR_DEPARTPOS_LAT:
            return SUMOXMLDefinitions::LateralAlignments.hasString(value);
        case SUMO_ATTR_ARRIVALPOS_LAT:
            return SUMOXMLDefinitions::LateralAlignments.hasString(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNECalibratorFlow::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNECalibratorFlow::getPopUpID() const {
    return getTagStr();
}


std::string
GNECalibratorFlow::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_BEGIN) + " -> " + getAttribute(SUMO_ATTR_END);
}

// ===========================================================================
// private
// ===========================================================================

void
GNECalibratorFlow::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_TYPE:
            myVehicleType = myNet->retrieveDemandElement(SUMO_TAG_VTYPE, value);
            break;
        case SUMO_ATTR_ROUTE:
            myRoute = myNet->retrieveDemandElement(SUMO_TAG_ROUTE, value);
            break;
        case SUMO_ATTR_VEHSPERHOUR:
            myVehsPerHour = value;
            break;
        case SUMO_ATTR_SPEED:
            mySpeed = value;
            break;
        case SUMO_ATTR_COLOR:
            myColor = parse<RGBColor>(value);
            break;
        case SUMO_ATTR_BEGIN:
            myBegin = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_END:
            myEnd = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_DEPARTLANE:
            myDepartLane = value;
            break;
        case SUMO_ATTR_DEPARTPOS:
            myDepartPos = value;
            break;
        case SUMO_ATTR_DEPARTSPEED:
            myDepartSpeed = value;
            break;
        case SUMO_ATTR_ARRIVALLANE:
            myArrivalLane = value;
            break;
        case SUMO_ATTR_ARRIVALPOS:
            myArrivalPos = value;
            break;
        case SUMO_ATTR_ARRIVALSPEED:
            myArrivalSpeed = value;
            break;
        case SUMO_ATTR_LINE:
            myLine = value;
            break;
        case SUMO_ATTR_PERSON_NUMBER:
            myPersonNumber = parse<int>(value);
            break;
        case SUMO_ATTR_CONTAINER_NUMBER:
            myContainerNumber = parse<int>(value);
            break;
        case SUMO_ATTR_REROUTE:
            myReroute = parse<bool>(value);
            break;
        case SUMO_ATTR_DEPARTPOS_LAT:
            myDepartPosLat = value;
            break;
        case SUMO_ATTR_ARRIVALPOS_LAT:
            myArrivalPosLat = value;
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
