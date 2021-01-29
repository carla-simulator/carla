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
/// @file    GNEVehicleFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2018
///
// The Widget for add Vehicles/Flows/Trips/etc. elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/elements/demand/GNERouteHandler.h>
#include <netedit/elements/demand/GNEVehicle.h>
#include <netedit/elements/network/GNEEdge.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <utils/xml/SUMOSAXAttributesImpl_Cached.h>

#include "GNEVehicleFrame.h"

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEVehicleFrame::HelpCreation - methods
// ---------------------------------------------------------------------------

GNEVehicleFrame::HelpCreation::HelpCreation(GNEVehicleFrame* vehicleFrameParent) :
    FXGroupBox(vehicleFrameParent->myContentFrame, "Help", GUIDesignGroupBoxFrame),
    myVehicleFrameParent(vehicleFrameParent) {
    myInformationLabel = new FXLabel(this, "", 0, GUIDesignLabelFrameInformation);
}


GNEVehicleFrame::HelpCreation::~HelpCreation() {}


void
GNEVehicleFrame::HelpCreation::showHelpCreation() {
    // first update help cration
    updateHelpCreation();
    // show modul
    show();
}


void
GNEVehicleFrame::HelpCreation::hideHelpCreation() {
    hide();
}

void
GNEVehicleFrame::HelpCreation::updateHelpCreation() {
    // create information label
    std::ostringstream information;
    // set text depending of selected vehicle type
    switch (myVehicleFrameParent->myVehicleTagSelector->getCurrentTagProperties().getTag()) {
        case SUMO_TAG_VEHICLE:
            information
                    << "- Click over a route to\n"
                    << "  create a vehicle.";
            break;
        case SUMO_TAG_TRIP:
            information
                    << "- Select two edges to\n"
                    << "  create a Trip.";
            break;
        case GNE_TAG_FLOW_ROUTE:
            information
                    << "- Click over a route to\n"
                    << "  create a routeFlow.";
            break;
        case SUMO_TAG_FLOW:
            information
                    << "- Select two edges to\n"
                    << "  create a flow.";
            break;
        default:
            break;
    }
    // set information label
    myInformationLabel->setText(information.str().c_str());
}

// ---------------------------------------------------------------------------
// GNEVehicleFrame - methods
// ---------------------------------------------------------------------------

GNEVehicleFrame::GNEVehicleFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Vehicles") {

    // Create item Selector modul for vehicles
    myVehicleTagSelector = new GNEFrameModuls::TagSelector(this, GNETagProperties::TagType::VEHICLE);

    // Create vehicle type selector
    myVTypeSelector = new GNEFrameModuls::DemandElementSelector(this, SUMO_TAG_VTYPE);

    // Create vehicle parameters
    myVehicleAttributes = new GNEFrameAttributesModuls::AttributesCreator(this);

    // create PathCreator Modul
    myPathCreator = new GNEFrameModuls::PathCreator(this);

    // Create Help Creation Modul
    myHelpCreation = new HelpCreation(this);

    // set Vehicle as default vehicle
    myVehicleTagSelector->setCurrentTag(SUMO_TAG_VEHICLE);
}


GNEVehicleFrame::~GNEVehicleFrame() {}


void
GNEVehicleFrame::show() {
    // refresh item selector
    myVehicleTagSelector->refreshTagProperties();
    // show frame
    GNEFrame::show();
}


void
GNEVehicleFrame::hide() {
    // reset candidate edges
    for (const auto& edge : myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
        edge.second->resetCandidateFlags();
    }
    // hide frame
    GNEFrame::hide();
}


bool
GNEVehicleFrame::addVehicle(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const GNEViewNetHelper::KeyPressed& keyPressed) {
    // obtain tag (only for improve code legibility)
    SumoXMLTag vehicleTag = myVehicleTagSelector->getCurrentTagProperties().getTag();
    const bool addEdge = ((vehicleTag == SUMO_TAG_TRIP) || (vehicleTag == GNE_TAG_VEHICLE_WITHROUTE) || (vehicleTag == SUMO_TAG_FLOW) || (vehicleTag == GNE_TAG_FLOW_WITHROUTE));
    // first check that current selected vehicle is valid
    if (vehicleTag == SUMO_TAG_NOTHING) {
        myViewNet->setStatusBarText("Current selected vehicle isn't valid.");
        return false;
    }
    // now check if VType is valid
    if (myVTypeSelector->getCurrentDemandElement() == nullptr) {
        myViewNet->setStatusBarText("Current selected vehicle type isn't valid.");
        return false;
    }
    // now check if parameters are valid
    if (!myVehicleAttributes->areValuesValid()) {
        myVehicleAttributes->showWarningMessage();
        return false;
    }
    // Declare map to keep attributes from Frames from Frame
    std::map<SumoXMLAttr, std::string> valuesMap = myVehicleAttributes->getAttributesAndValues(false);
    // Check if ID has to be generated
    if (valuesMap.count(SUMO_ATTR_ID) == 0) {
        valuesMap[SUMO_ATTR_ID] = myViewNet->getNet()->generateDemandElementID(vehicleTag);
    }
    // add VType
    valuesMap[SUMO_ATTR_TYPE] = myVTypeSelector->getCurrentDemandElement()->getID();
    // set route or edges depending of vehicle type
    if ((vehicleTag == SUMO_TAG_VEHICLE) || (vehicleTag == GNE_TAG_FLOW_ROUTE)) {
        if (objectsUnderCursor.getDemandElementFront() && (objectsUnderCursor.getDemandElementFront()->getTagProperty().isRoute())) {
            // obtain route
            valuesMap[SUMO_ATTR_ROUTE] = (objectsUnderCursor.getDemandElementFront()->getTagProperty().getTag() == SUMO_TAG_ROUTE) ? objectsUnderCursor.getDemandElementFront()->getID() : "embedded";
            // check if departLane is valid
            if ((objectsUnderCursor.getDemandElementFront()->getTagProperty().getTag() == SUMO_TAG_ROUTE) &&
                    (valuesMap.count(SUMO_ATTR_DEPARTLANE) > 0) && GNEAttributeCarrier::canParse<double>(valuesMap[SUMO_ATTR_DEPARTLANE])) {
                double departLane = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_DEPARTLANE]);
                if (departLane >= objectsUnderCursor.getDemandElementFront()->getParentEdges().front()->getLanes().size()) {
                    myViewNet->setStatusBarText("Invalid " + toString(SUMO_ATTR_DEPARTLANE));
                    return false;
                }
            }
            // check if departSpeed is valid
            if ((valuesMap.count(SUMO_ATTR_DEPARTSPEED) > 0) && GNEAttributeCarrier::canParse<double>(valuesMap[SUMO_ATTR_DEPARTSPEED])) {
                double departSpeed = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_DEPARTSPEED]);
                if (departSpeed >= myVTypeSelector->getCurrentDemandElement()->getAttributeDouble(SUMO_ATTR_MAXSPEED)) {
                    myViewNet->setStatusBarText("Invalid " + toString(SUMO_ATTR_DEPARTSPEED));
                    return false;
                }
            }
            // check if we're creating a vehicle or a flow
            if (vehicleTag == SUMO_TAG_VEHICLE) {
                // Add parameter departure
                if (valuesMap[SUMO_ATTR_DEPART].empty()) {
                    valuesMap[SUMO_ATTR_DEPART] = "0";
                }
                // declare SUMOSAXAttributesImpl_Cached to convert valuesMap into SUMOSAXAttributes
                SUMOSAXAttributesImpl_Cached SUMOSAXAttrs(valuesMap, getPredefinedTagsMML(), toString(vehicleTag));
                // obtain vehicle parameters in vehicleParameters
                SUMOVehicleParameter* vehicleParameters = SUMOVehicleParserHelper::parseVehicleAttributes(vehicleTag, SUMOSAXAttrs, false);
                // check if vehicle was sucesfully created)
                if (vehicleParameters) {
                    // check if we're creating a vehicle over a existent route or over a embedded route
                    if (objectsUnderCursor.getDemandElementFront()->getTagProperty().getTag() == SUMO_TAG_ROUTE) {
                        GNERouteHandler::buildVehicleOverRoute(myViewNet->getNet(), true, *vehicleParameters);
                    }
                    // delete vehicleParameters
                    delete vehicleParameters;
                }
            } else {
                // set begin and end attributes
                if (valuesMap[SUMO_ATTR_BEGIN].empty()) {
                    valuesMap[SUMO_ATTR_BEGIN] = "0";
                }
                if (valuesMap[SUMO_ATTR_END].empty()) {
                    valuesMap[SUMO_ATTR_END] = "3600";
                }
                // declare SUMOSAXAttributesImpl_Cached to convert valuesMap into SUMOSAXAttributes
                SUMOSAXAttributesImpl_Cached SUMOSAXAttrs(valuesMap, getPredefinedTagsMML(), toString(vehicleTag));
                // obtain routeFlow parameters in routeFlowParameters
                SUMOVehicleParameter* routeFlowParameters = SUMOVehicleParserHelper::parseFlowAttributes(vehicleTag, SUMOSAXAttrs, false, 0, SUMOTime_MAX);
                // check if flow was sucesfully created)
                if (routeFlowParameters) {
                    // check if we're creating a vehicle over a existent route or over a embedded route
                    if (objectsUnderCursor.getDemandElementFront()->getTagProperty().getTag() == SUMO_TAG_ROUTE) {
                        GNERouteHandler::buildFlowOverRoute(myViewNet->getNet(), true, *routeFlowParameters);
                    }
                    // delete routeFlowParameters
                    delete routeFlowParameters;
                }
            }
            // refresh myVehicleAttributes
            myVehicleAttributes->refreshRows();
            // all ok, then return true;
            return true;
        } else {
            myViewNet->setStatusBarText(toString(vehicleTag) + " has to be placed within a route.");
            return false;
        }
    } else if (addEdge && objectsUnderCursor.getEdgeFront()) {
        // add clicked edge in PathCreator
        return myPathCreator->addEdge(objectsUnderCursor.getEdgeFront(), keyPressed.shiftKeyPressed(), keyPressed.controlKeyPressed());
    } else {
        return false;
    }
}


GNEFrameModuls::PathCreator*
GNEVehicleFrame::getPathCreator() const {
    return myPathCreator;
}

// ===========================================================================
// protected
// ===========================================================================

void
GNEVehicleFrame::tagSelected() {
    if (myVehicleTagSelector->getCurrentTagProperties().getTag() != SUMO_TAG_NOTHING) {
        // show vehicle type selector modul
        myVTypeSelector->showDemandElementSelector();
        // show path creator modul
        myPathCreator->showPathCreatorModul(myVehicleTagSelector->getCurrentTagProperties().getTag(), false, false);
    } else {
        // hide all moduls if vehicle isn't valid
        myVTypeSelector->hideDemandElementSelector();
        myVehicleAttributes->hideAttributesCreatorModul();
        myHelpCreation->hideHelpCreation();
    }
}


void
GNEVehicleFrame::demandElementSelected() {
    if (myVTypeSelector->getCurrentDemandElement()) {
        // show vehicle attributes modul
        myVehicleAttributes->showAttributesCreatorModul(myVehicleTagSelector->getCurrentTagProperties(), {});
        // set current VTypeClass in TripCreator
        myPathCreator->setVClass(myVTypeSelector->getCurrentDemandElement()->getVClass());
        // show help creation
        myHelpCreation->showHelpCreation();
    } else {
        // hide all moduls if selected item isn't valid
        myVehicleAttributes->hideAttributesCreatorModul();
        // hide help creation
        myHelpCreation->hideHelpCreation();

    }
}


void
GNEVehicleFrame::createPath() {
    // obtain tag (only for improve code legibility)
    SumoXMLTag vehicleTag = myVehicleTagSelector->getCurrentTagProperties().getTag();
    const bool embebbedRoute = ((vehicleTag == GNE_TAG_VEHICLE_WITHROUTE) || (vehicleTag == GNE_TAG_FLOW_WITHROUTE));
    // check number of edges
    if ((myPathCreator->getSelectedEdges().size() > 1) || (myPathCreator->getSelectedEdges().size() > 0 && embebbedRoute)) {
        // Declare map to keep attributes from Frames from Frame
        std::map<SumoXMLAttr, std::string> valuesMap = myVehicleAttributes->getAttributesAndValues(false);
        // Check if ID has to be generated
        if (valuesMap.count(SUMO_ATTR_ID) == 0) {
            valuesMap[SUMO_ATTR_ID] = myViewNet->getNet()->generateDemandElementID(vehicleTag);
        }
        // add VType
        valuesMap[SUMO_ATTR_TYPE] = myVTypeSelector->getCurrentDemandElement()->getID();
        // extract via attribute
        std::vector<GNEEdge*> viaEdges;
        for (int i = 1; i < ((int)myPathCreator->getSelectedEdges().size() - 1); i++) {
            viaEdges.push_back(myPathCreator->getSelectedEdges().at(i));
        }
        // continue depending of tag
        if (vehicleTag == SUMO_TAG_TRIP) {
            // Add parameter departure
            if (valuesMap[SUMO_ATTR_DEPART].empty()) {
                valuesMap[SUMO_ATTR_DEPART] = "0";
            }
            // declare SUMOSAXAttributesImpl_Cached to convert valuesMap into SUMOSAXAttributes
            SUMOSAXAttributesImpl_Cached SUMOSAXAttrs(valuesMap, getPredefinedTagsMML(), toString(vehicleTag));
            // obtain trip parameters
            SUMOVehicleParameter* tripParameters = SUMOVehicleParserHelper::parseVehicleAttributes(vehicleTag, SUMOSAXAttrs, false);
            // build trip in GNERouteHandler
            GNERouteHandler::buildTrip(myViewNet->getNet(), true, *tripParameters, myPathCreator->getSelectedEdges().front(), myPathCreator->getSelectedEdges().back(), viaEdges);
            // delete tripParameters
            delete tripParameters;
        } else if (vehicleTag == GNE_TAG_VEHICLE_WITHROUTE) {
            // Add parameter departure
            if (valuesMap[SUMO_ATTR_DEPART].empty()) {
                valuesMap[SUMO_ATTR_DEPART] = "0";
            }
            // get route edges
            std::vector<GNEEdge*> routeEdges;
            for (const auto& subPath : myPathCreator->getPath()) {
                for (const auto& edge : subPath.getSubPath()) {
                    routeEdges.push_back(edge);
                }
            }
            // declare SUMOSAXAttributesImpl_Cached to convert valuesMap into SUMOSAXAttributes
            SUMOSAXAttributesImpl_Cached SUMOSAXAttrs(valuesMap, getPredefinedTagsMML(), toString(vehicleTag));
            // obtain vehicle parameters
            SUMOVehicleParameter* vehicleParameters = SUMOVehicleParserHelper::parseVehicleAttributes(vehicleTag, SUMOSAXAttrs, false);
            // build vehicle with embebbed route in GNERouteHandler
            GNERouteHandler::buildVehicleEmbeddedRoute(myViewNet->getNet(), true, *vehicleParameters, routeEdges);
            // delete vehicleParamters
            delete vehicleParameters;
        } else if (vehicleTag == SUMO_TAG_FLOW) {
            // Add parameter departure
            if (valuesMap[SUMO_ATTR_DEPART].empty()) {
                valuesMap[SUMO_ATTR_DEPART] = "0";
            }
            // declare SUMOSAXAttributesImpl_Cached to convert valuesMap into SUMOSAXAttributes
            SUMOSAXAttributesImpl_Cached SUMOSAXAttrs(valuesMap, getPredefinedTagsMML(), toString(vehicleTag));
            // obtain flow parameters
            SUMOVehicleParameter* flowParameters = SUMOVehicleParserHelper::parseFlowAttributes(vehicleTag, SUMOSAXAttrs, false, 0, SUMOTime_MAX);
            // build trip in GNERouteHandler
            GNERouteHandler::buildFlow(myViewNet->getNet(), true, *flowParameters, myPathCreator->getSelectedEdges().front(), myPathCreator->getSelectedEdges().back(), viaEdges);
            // delete tripParameters
            delete flowParameters;
        } else if (vehicleTag == GNE_TAG_FLOW_WITHROUTE) {
            // set begin and end attributes
            if (valuesMap[SUMO_ATTR_BEGIN].empty()) {
                valuesMap[SUMO_ATTR_BEGIN] = "0";
            }
            // get route edges
            std::vector<GNEEdge*> routeEdges;
            for (const auto& subPath : myPathCreator->getPath()) {
                for (const auto& edge : subPath.getSubPath()) {
                    routeEdges.push_back(edge);
                }
            }
            // declare SUMOSAXAttributesImpl_Cached to convert valuesMap into SUMOSAXAttributes
            SUMOSAXAttributesImpl_Cached SUMOSAXAttrs(valuesMap, getPredefinedTagsMML(), toString(vehicleTag));
            // obtain flow parameters
            SUMOVehicleParameter* flowParameters = SUMOVehicleParserHelper::parseFlowAttributes(vehicleTag, SUMOSAXAttrs, false, 0, SUMOTime_MAX);
            // build flow with embebbed route in GNERouteHandler
            GNERouteHandler::buildFlowEmbeddedRoute(myViewNet->getNet(), true, *flowParameters, routeEdges);
            // delete flowParameters
            delete flowParameters;
        }
        // abort path creation
        myPathCreator->abortPathCreation();
        // refresh myVehicleAttributes
        myVehicleAttributes->refreshRows();
    }
}


/****************************************************************************/
