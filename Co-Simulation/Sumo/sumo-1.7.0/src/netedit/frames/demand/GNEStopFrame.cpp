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
/// @file    GNEStopFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    March 2019
///
// The Widget for add Stops elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/elements/additional/GNEAdditional.h>
#include <netedit/elements/demand/GNEDemandElement.h>
#include <netedit/elements/demand/GNERouteHandler.h>
#include <netedit/elements/network/GNELane.h>
#include <utils/gui/div/GUIDesigns.h>

#include "GNEStopFrame.h"

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEStopFrame::HelpCreation - methods
// ---------------------------------------------------------------------------

GNEStopFrame::HelpCreation::HelpCreation(GNEStopFrame* StopFrameParent) :
    FXGroupBox(StopFrameParent->myContentFrame, "Help", GUIDesignGroupBoxFrame),
    myStopFrameParent(StopFrameParent) {
    myInformationLabel = new FXLabel(this, "", 0, GUIDesignLabelFrameInformation);
}


GNEStopFrame::HelpCreation::~HelpCreation() {}


void
GNEStopFrame::HelpCreation::showHelpCreation() {
    // first update help cration
    updateHelpCreation();
    // show modul
    show();
}


void
GNEStopFrame::HelpCreation::hideHelpCreation() {
    hide();
}


void
GNEStopFrame::HelpCreation::updateHelpCreation() {
    // create information label
    std::ostringstream information;
    // set text depending of selected Stop type
    switch (myStopFrameParent->myStopTagSelector->getCurrentTagProperties().getTag()) {
        case SUMO_TAG_STOP_BUSSTOP:
            information
                    << "- Click over a bus stop\n"
                    << "  to create a stop.";
            break;
        case SUMO_TAG_STOP_CONTAINERSTOP:
            information
                    << "- Click over a container stop\n"
                    << "  to create a stop.";
            break;
        case SUMO_TAG_STOP_CHARGINGSTATION:
            information
                    << "- Click over a charging \n"
                    << "  station to create a stop.";
            break;
        case SUMO_TAG_STOP_PARKINGAREA:
            information
                    << "- Click over a parking area\n"
                    << "  to create a stop.";
            break;
        case SUMO_TAG_STOP_LANE:
            information
                    << "- Click over a lane to\n"
                    << "  create a stop.";
            break;
        default:
            information
                    << "- No stop parents in\n"
                    << "  current network.";
            break;
    }
    // set information label
    myInformationLabel->setText(information.str().c_str());
}

// ---------------------------------------------------------------------------
// GNEStopFrame - methods
// ---------------------------------------------------------------------------

GNEStopFrame::GNEStopFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Stops") {

    // Create Stop parent selector
    myStopParentSelector = new GNEFrameModuls::DemandElementSelector(this, {GNETagProperties::TagType::PERSON, GNETagProperties::TagType::VEHICLE, GNETagProperties::TagType::ROUTE});

    // Create item Selector modul for Stops
    myStopTagSelector = new GNEFrameModuls::TagSelector(this, GNETagProperties::TagType::STOP);

    // Create Stop parameters
    myStopAttributes = new GNEFrameAttributesModuls::AttributesCreator(this);

    // Create Netedit parameter
    myNeteditAttributes = new GNEFrameAttributesModuls::NeteditAttributes(this);

    // Create Help Creation Modul
    myHelpCreation = new HelpCreation(this);

    // refresh myStopParentMatchBox
    myStopParentSelector->refreshDemandElementSelector();
}


GNEStopFrame::~GNEStopFrame() {}


void
GNEStopFrame::show() {
    // first check if stop frame moduls can be shown
    bool validStopParent = false;
    // check if at least there an item that supports an stop
    for (auto i = myStopParentSelector->getAllowedTags().begin(); (i != myStopParentSelector->getAllowedTags().end()) && (validStopParent == false); i++) {
        if (myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(*i).size() > 0) {
            validStopParent = true;
        }
    }
    // show or hidde moduls depending of validStopParent
    if (validStopParent) {
        myStopParentSelector->showDemandElementSelector();
        myStopTagSelector->showTagSelector();
        // refresh vType selector
        myStopParentSelector->refreshDemandElementSelector();
        // refresh item selector
        myStopTagSelector->refreshTagProperties();
    } else {
        // hide moduls (except help creation)
        myStopParentSelector->hideDemandElementSelector();
        myStopTagSelector->hideTagSelector();
        myStopAttributes->hideAttributesCreatorModul();
        myNeteditAttributes->hideNeteditAttributesModul();
        // show help creation modul
        myHelpCreation->showHelpCreation();
    }
    // show frame
    GNEFrame::show();
}


bool
GNEStopFrame::addStop(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const GNEViewNetHelper::KeyPressed& keyPressed) {
    // check if we're selecting a new stop parent
    if (keyPressed.shiftKeyPressed()) {
        if (objectsUnderCursor.getDemandElementFront() &&
                (objectsUnderCursor.getDemandElementFront()->getTagProperty().isVehicle() || objectsUnderCursor.getDemandElementFront()->getTagProperty().getTag() == SUMO_TAG_ROUTE)) {
            myStopParentSelector->setDemandElement(objectsUnderCursor.getDemandElementFront());
            WRITE_WARNING("Selected " + objectsUnderCursor.getDemandElementFront()->getTagStr() + " '" + objectsUnderCursor.getDemandElementFront()->getID() + "' as stop parent.");
            return true;
        } else {
            WRITE_WARNING("Selected Stop parent isn't valid.");
            return false;
        }

    } else {
        // now check if stop parent selector is valid
        if (myStopParentSelector->getCurrentDemandElement() == nullptr) {
            WRITE_WARNING("Current selected Stop parent isn't valid.");
            return false;
        }
        // declare a Stop
        SUMOVehicleParameter::Stop stopParameter;
        // check if stop parameters was sucesfully obtained
        if (getStopParameter(stopParameter, myStopTagSelector->getCurrentTagProperties().getTag(),
                             myViewNet, myStopAttributes, myNeteditAttributes,
                             objectsUnderCursor.getLaneFront(), objectsUnderCursor.getAdditionalFront())) {
            // create it in RouteFrame
            GNERouteHandler::buildStop(myViewNet->getNet(), true, stopParameter, myStopParentSelector->getCurrentDemandElement());
            // stop sucesfully created, then return true
            return true;
        } else {
            return false;
        }
    }
}

bool
GNEStopFrame::getStopParameter(SUMOVehicleParameter::Stop& stop, const SumoXMLTag stopTag, GNEViewNet* /* viewNet */,
                               GNEFrameAttributesModuls::AttributesCreator* stopAttributes,
                               const GNEFrameAttributesModuls::NeteditAttributes* myNeteditAttributes,
                               const GNELane* lane, const GNEAdditional* stoppingPlace) {
    // first check that current selected Stop is valid
    if (stopTag == SUMO_TAG_NOTHING) {
        WRITE_WARNING("Current selected Stop type isn't valid.");
        return false;
    } else if (stopTag == SUMO_TAG_STOP_LANE) {
        if (lane) {
            stop.lane = lane->getID();
        } else {
            WRITE_WARNING("Click over a " + toString(SUMO_TAG_LANE) + " to create a stop placed in a " + toString(SUMO_TAG_LANE));
            return false;
        }
    } else if (stopTag == GNE_TAG_PERSONSTOP_EDGE) {
        if (lane) {
            stop.edge = lane->getParentEdge()->getID();
        } else {
            WRITE_WARNING("Click over a " + toString(SUMO_TAG_EDGE) + " to create a stop placed in a " + toString(SUMO_TAG_EDGE));
            return false;
        }
    } else if (stoppingPlace) {
        if (stoppingPlace->getTagProperty().getTag() == SUMO_TAG_BUS_STOP) {
            if ((stopTag != SUMO_TAG_STOP_BUSSTOP) && (stopTag != GNE_TAG_PERSONSTOP_BUSSTOP)) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a " + stoppingPlace->getTagProperty().getTagStr());
                return false;
            } else {
                stop.busstop = stoppingPlace->getID();
                stop.startPos = 0;
                stop.endPos = 0;
            }
        } else if (stoppingPlace->getTagProperty().getTag() == SUMO_TAG_CONTAINER_STOP) {
            if (stopTag != SUMO_TAG_STOP_CONTAINERSTOP) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a " + stoppingPlace->getTagProperty().getTagStr());
                return false;
            } else {
                stop.containerstop = stoppingPlace->getID();
                stop.startPos = 0;
                stop.endPos = 0;
            }
        } else if (stoppingPlace->getTagProperty().getTag() == SUMO_TAG_CHARGING_STATION) {
            if (stopTag != SUMO_TAG_STOP_CHARGINGSTATION) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a " + stoppingPlace->getTagProperty().getTagStr());
                return false;
            } else {
                stop.chargingStation = stoppingPlace->getID();
                stop.startPos = 0;
                stop.endPos = 0;
            }
        } else if (stoppingPlace->getTagProperty().getTag() == SUMO_TAG_PARKING_AREA) {
            if (stopTag != SUMO_TAG_STOP_PARKINGAREA) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a " + stoppingPlace->getTagProperty().getTagStr());
                return false;
            } else {
                stop.parkingarea = stoppingPlace->getID();
                stop.startPos = 0;
                stop.endPos = 0;
            }
        }
    } else {
        if (stopTag == SUMO_TAG_STOP_BUSSTOP) {
            WRITE_WARNING("Click over a " + toString(SUMO_TAG_STOP_BUSSTOP) + " to create a stop placed in a " + toString(SUMO_TAG_STOP_BUSSTOP));
        } else if (stopTag == SUMO_TAG_STOP_CONTAINERSTOP) {
            WRITE_WARNING("Click over a " + toString(SUMO_TAG_CONTAINER_STOP) + " to create a stop placed in a " + toString(SUMO_TAG_CONTAINER_STOP));
        } else if (stopTag == SUMO_TAG_CHARGING_STATION) {
            WRITE_WARNING("Click over a " + toString(SUMO_TAG_CHARGING_STATION) + " to create a stop placed in a " + toString(SUMO_TAG_CHARGING_STATION));
        } else if (stopTag == SUMO_TAG_STOP_PARKINGAREA) {
            WRITE_WARNING("Click over a " + toString(SUMO_TAG_PARKING_AREA) + " to create a stop placed in a " + toString(SUMO_TAG_PARKING_AREA));
        } else if (stopTag == GNE_TAG_PERSONTRIP_EDGE_BUSSTOP) {
            WRITE_WARNING("Click over a " + toString(SUMO_TAG_STOP_BUSSTOP) + " to create a person stop placed in a " + toString(SUMO_TAG_STOP_BUSSTOP));
        }
        return false;
    }
    // check if stop attributes are valid
    if (!stopAttributes->areValuesValid()) {
        stopAttributes->showWarningMessage();
        return false;
    }
    // declare map to keep attributes from Frames from Frame
    std::map<SumoXMLAttr, std::string> valuesMap = stopAttributes->getAttributesAndValues(false);
    // add netedit values
    if (!stop.lane.empty()) {
        myNeteditAttributes->getNeteditAttributesAndValues(valuesMap, lane);
        // check if start position can be parsed
        if (GNEAttributeCarrier::canParse<double>(valuesMap[SUMO_ATTR_STARTPOS])) {
            stop.startPos = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_STARTPOS]);
            stop.parametersSet |= STOP_START_SET;
        }
        // check if end position can be parsed
        if (GNEAttributeCarrier::canParse<double>(valuesMap[SUMO_ATTR_ENDPOS])) {
            stop.endPos = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_ENDPOS]);
            stop.parametersSet |= STOP_END_SET;
        }
    }
    // obtain friendly position
    if (valuesMap.count(SUMO_ATTR_FRIENDLY_POS) > 0) {
        stop.friendlyPos = GNEAttributeCarrier::parse<bool>(valuesMap.at(SUMO_ATTR_FRIENDLY_POS));
    }
    // obtain actType
    if (valuesMap.count(SUMO_ATTR_ACTTYPE) > 0) {
        stop.actType = valuesMap.at(SUMO_ATTR_ACTTYPE);
    }
    // fill rest of parameters depending if it was edited
    if (valuesMap.count(SUMO_ATTR_DURATION) > 0) {
        stop.duration = string2time(valuesMap.at(SUMO_ATTR_DURATION));
        stop.parametersSet |= STOP_DURATION_SET;
    } else {
        stop.duration = -1;
        stop.parametersSet &= ~STOP_DURATION_SET;
    }
    if (valuesMap.count(SUMO_ATTR_UNTIL) > 0) {
        stop.until = string2time(valuesMap[SUMO_ATTR_UNTIL]);
        stop.parametersSet |= STOP_UNTIL_SET;
    } else {
        stop.until = -1;
        stop.parametersSet &= ~STOP_UNTIL_SET;
    }
    if (valuesMap.count(SUMO_ATTR_EXTENSION) > 0) {
        stop.extension = string2time(valuesMap.at(SUMO_ATTR_EXTENSION));
        stop.parametersSet |= STOP_EXTENSION_SET;
    }
    if (valuesMap.count(SUMO_ATTR_TRIGGERED) > 0) {
        stop.triggered = GNEAttributeCarrier::parse<bool>(valuesMap.at(SUMO_ATTR_TRIGGERED));
        stop.parametersSet |= STOP_TRIGGER_SET;
    }
    if (valuesMap.count(SUMO_ATTR_CONTAINER_TRIGGERED) > 0) {
        stop.containerTriggered = GNEAttributeCarrier::parse<bool>(valuesMap.at(SUMO_ATTR_CONTAINER_TRIGGERED));
        stop.parametersSet |= STOP_CONTAINER_TRIGGER_SET;
    }
    if (valuesMap.count(SUMO_ATTR_PARKING) > 0) {
        stop.parking = GNEAttributeCarrier::parse<bool>(valuesMap.at(SUMO_ATTR_PARKING));
        stop.parametersSet |= STOP_PARKING_SET;
    }
    if (valuesMap.count(SUMO_ATTR_EXPECTED) > 0) {
        stop.awaitedPersons = GNEAttributeCarrier::parse<std::set<std::string> >(valuesMap.at(SUMO_ATTR_EXPECTED));
        stop.parametersSet |= STOP_EXPECTED_SET;
    }
    if (valuesMap.count(SUMO_ATTR_EXPECTED_CONTAINERS) > 0) {
        stop.awaitedContainers = GNEAttributeCarrier::parse<std::set<std::string> >(valuesMap.at(SUMO_ATTR_EXPECTED_CONTAINERS));
        stop.parametersSet |= STOP_EXPECTED_CONTAINERS_SET;
    }
    if (valuesMap.count(SUMO_ATTR_TRIP_ID) > 0) {
        stop.tripId = valuesMap.at(SUMO_ATTR_TRIP_ID);
        stop.parametersSet |= STOP_TRIP_ID_SET;
    }
    if (valuesMap.count(SUMO_ATTR_INDEX) > 0) {
        if (valuesMap[SUMO_ATTR_INDEX] == "fit") {
            stop.index = STOP_INDEX_FIT;
        } else if (valuesMap[SUMO_ATTR_INDEX] == "end") {
            stop.index = STOP_INDEX_END;
        } else {
            stop.index = GNEAttributeCarrier::parse<int>(valuesMap[SUMO_ATTR_INDEX]);
        }
    } else {
        stop.index = STOP_INDEX_END;
    }
    // refresh stop attributes
    stopAttributes->refreshRows();
    // all ok, then return true
    return true;
}

// ===========================================================================
// protected
// ===========================================================================

void
GNEStopFrame::tagSelected() {
    if (myStopTagSelector->getCurrentTagProperties().getTag() != SUMO_TAG_NOTHING) {
        // show Stop type selector modul
        myStopAttributes->showAttributesCreatorModul(myStopTagSelector->getCurrentTagProperties(), {});
        myNeteditAttributes->showNeteditAttributesModul(myStopTagSelector->getCurrentTagProperties());
        myHelpCreation->showHelpCreation();
    } else {
        // hide all moduls if stop parent isn't valid
        myStopAttributes->hideAttributesCreatorModul();
        myNeteditAttributes->hideNeteditAttributesModul();
        myHelpCreation->hideHelpCreation();
    }
}


void
GNEStopFrame::demandElementSelected() {
    // show or hidde moduls depending if current selected stop parent is valid
    if (myStopParentSelector->getCurrentDemandElement()) {
        myStopTagSelector->showTagSelector();
        if (myStopTagSelector->getCurrentTagProperties().getTag() != SUMO_TAG_NOTHING) {
            // show moduls
            myStopAttributes->showAttributesCreatorModul(myStopTagSelector->getCurrentTagProperties(), {});
            myNeteditAttributes->showNeteditAttributesModul(myStopTagSelector->getCurrentTagProperties());
            myHelpCreation->showHelpCreation();
        } else {
            myStopAttributes->hideAttributesCreatorModul();
            myNeteditAttributes->hideNeteditAttributesModul();
            myHelpCreation->hideHelpCreation();
        }
    } else {
        // hide moduls
        myStopTagSelector->hideTagSelector();
        myStopAttributes->hideAttributesCreatorModul();
        myNeteditAttributes->hideNeteditAttributesModul();
        myHelpCreation->hideHelpCreation();
    }
}


/****************************************************************************/
