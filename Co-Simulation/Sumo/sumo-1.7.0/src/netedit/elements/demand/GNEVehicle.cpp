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
/// @file    GNEVehicle.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
///
// Representation of vehicles in NETEDIT
/****************************************************************************/
#include <cmath>
#include <microsim/devices/MSDevice_BTreceiver.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_EnableAttribute.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIBaseVehicleHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEVehicle.h"
#include "GNERouteHandler.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GNEVehicle::GNESingleVehiclePopupMenu) GNESingleVehiclePopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_VEHICLE_TRANSFORM,   GNEVehicle::GNESingleVehiclePopupMenu::onCmdTransform),
};

FXDEFMAP(GNEVehicle::GNESelectedVehiclesPopupMenu) GNESelectedVehiclesPopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_VEHICLE_TRANSFORM,   GNEVehicle::GNESelectedVehiclesPopupMenu::onCmdTransform),
};

// Object implementation
FXIMPLEMENT(GNEVehicle::GNESingleVehiclePopupMenu,      GUIGLObjectPopupMenu,   GNESingleVehiclePopupMenuMap,       ARRAYNUMBER(GNESingleVehiclePopupMenuMap))
FXIMPLEMENT(GNEVehicle::GNESelectedVehiclesPopupMenu,   GUIGLObjectPopupMenu,   GNESelectedVehiclesPopupMenuMap,    ARRAYNUMBER(GNESelectedVehiclesPopupMenuMap))

// ===========================================================================
// GNEVehicle::GNESingleVehiclePopupMenu
// ===========================================================================

GNEVehicle::GNESingleVehiclePopupMenu::GNESingleVehiclePopupMenu(GNEVehicle* vehicle, GUIMainWindow& app, GUISUMOAbstractView& parent) :
    GUIGLObjectPopupMenu(app, parent, *vehicle),
    myVehicle(vehicle),
    myTransformToVehicle(nullptr),
    myTransformToVehicleWithEmbeddedRoute(nullptr),
    myTransformToRouteFlow(nullptr),
    myTransformToRouteFlowWithEmbeddedRoute(nullptr),
    myTransformToTrip(nullptr),
    myTransformToFlow(nullptr) {
    // build header
    myVehicle->buildPopupHeader(this, app);
    // build menu command for center button and copy cursor position to clipboard
    myVehicle->buildCenterPopupEntry(this);
    myVehicle->buildPositionCopyEntry(this, false);
    // buld menu commands for names
    new FXMenuCommand(this, ("Copy " + myVehicle->getTagStr() + " name to clipboard").c_str(), nullptr, this, MID_COPY_NAME);
    new FXMenuCommand(this, ("Copy " + myVehicle->getTagStr() + " typed name to clipboard").c_str(), nullptr, this, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(this);
    // build selection and show parameters menu
    myVehicle->getNet()->getViewNet()->buildSelectionACPopupEntry(this, myVehicle);
    myVehicle->buildShowParamsPopupEntry(this);
    // add transform functions only in demand mode
    if (myVehicle->getNet()->getViewNet()->getEditModes().isCurrentSupermodeDemand()) {
        // Get icons
        FXIcon* vehicleIcon = GUIIconSubSys::getIcon(GUIIcon::VEHICLE);
        FXIcon* tripIcon = GUIIconSubSys::getIcon(GUIIcon::TRIP);
        FXIcon* routeFlowIcon = GUIIconSubSys::getIcon(GUIIcon::ROUTEFLOW);
        FXIcon* flowIcon = GUIIconSubSys::getIcon(GUIIcon::FLOW);
        // create menu pane for transform operations
        FXMenuPane* transformOperation = new FXMenuPane(this);
        this->insertMenuPaneChild(transformOperation);
        new FXMenuCascade(this, "transform to", nullptr, transformOperation);
        // Create menu comands for all transform
        myTransformToVehicle = new FXMenuCommand(transformOperation, "Vehicle", vehicleIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToVehicleWithEmbeddedRoute = new FXMenuCommand(transformOperation, "Vehicle (embedded route)", vehicleIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToRouteFlow = new FXMenuCommand(transformOperation, "RouteFlow", routeFlowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToRouteFlowWithEmbeddedRoute = new FXMenuCommand(transformOperation, "RouteFlow (embedded route)", routeFlowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToTrip = new FXMenuCommand(transformOperation, "Trip", tripIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToFlow = new FXMenuCommand(transformOperation, "Flow", flowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        // check what menu command has to be disabled
        if (myVehicle->getTagProperty().getTag() == SUMO_TAG_VEHICLE) {
            myTransformToVehicle->disable();
        } else if (myVehicle->getTagProperty().getTag() == GNE_TAG_VEHICLE_WITHROUTE) {
            myTransformToVehicleWithEmbeddedRoute->disable();
        } else if (myVehicle->getTagProperty().getTag() == GNE_TAG_FLOW_ROUTE) {
            myTransformToRouteFlow->disable();
        } else if (myVehicle->getTagProperty().getTag() == GNE_TAG_FLOW_WITHROUTE) {
            myTransformToRouteFlowWithEmbeddedRoute->disable();
        } else if (myVehicle->getTagProperty().getTag() == SUMO_TAG_TRIP) {
            myTransformToTrip->disable();
        } else if (myVehicle->getTagProperty().getTag() == SUMO_TAG_FLOW) {
            myTransformToFlow->disable();
        }
    }
}


GNEVehicle::GNESingleVehiclePopupMenu::~GNESingleVehiclePopupMenu() {}


long
GNEVehicle::GNESingleVehiclePopupMenu::onCmdTransform(FXObject* obj, FXSelector, void*) {
    if (obj == myTransformToVehicle) {
        GNERouteHandler::transformToVehicle(myVehicle, false);
    } else if (obj == myTransformToVehicleWithEmbeddedRoute) {
        GNERouteHandler::transformToVehicle(myVehicle, true);
    } else if (obj == myTransformToRouteFlow) {
        GNERouteHandler::transformToRouteFlow(myVehicle, false);
    } else if (obj == myTransformToRouteFlowWithEmbeddedRoute) {
        GNERouteHandler::transformToRouteFlow(myVehicle, true);
    } else if (obj == myTransformToTrip) {
        GNERouteHandler::transformToTrip(myVehicle);
    } else if (obj == myTransformToFlow) {
        GNERouteHandler::transformToFlow(myVehicle);
    }
    return 1;
}

// ===========================================================================
// GNEVehicle::GNESelectedVehiclesPopupMenu
// ===========================================================================

GNEVehicle::GNESelectedVehiclesPopupMenu::GNESelectedVehiclesPopupMenu(GNEVehicle* vehicle, const std::vector<GNEVehicle*>& selectedVehicle, GUIMainWindow& app, GUISUMOAbstractView& parent) :
    GUIGLObjectPopupMenu(app, parent, *vehicle),
    mySelectedVehicles(selectedVehicle),
    myVehicleTag(vehicle->getTagProperty().getTag()),
    myTransformToVehicle(nullptr),
    myTransformToVehicleWithEmbeddedRoute(nullptr),
    myTransformToRouteFlow(nullptr),
    myTransformToRouteFlowWithEmbeddedRoute(nullptr),
    myTransformToTrip(nullptr),
    myTransformToFlow(nullptr),
    myTransformAllVehiclesToVehicle(nullptr),
    myTransformAllVehiclesToVehicleWithEmbeddedRoute(nullptr),
    myTransformAllVehiclesToRouteFlow(nullptr),
    myTransformAllVehiclesToRouteFlowWithEmbeddedRoute(nullptr),
    myTransformAllVehiclesToTrip(nullptr),
    myTransformAllVehiclesToFlow(nullptr) {
    // build header
    vehicle->buildPopupHeader(this, app);
    // build menu command for center button and copy cursor position to clipboard
    vehicle->buildCenterPopupEntry(this);
    vehicle->buildPositionCopyEntry(this, false);
    // buld menu commands for names
    new FXMenuCommand(this, ("Copy " + vehicle->getTagStr() + " name to clipboard").c_str(), nullptr, this, MID_COPY_NAME);
    new FXMenuCommand(this, ("Copy " + vehicle->getTagStr() + " typed name to clipboard").c_str(), nullptr, this, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(this);
    // build selection and show parameters menu
    vehicle->getNet()->getViewNet()->buildSelectionACPopupEntry(this, vehicle);
    vehicle->buildShowParamsPopupEntry(this);
    // add transform functions only in demand mode
    if (vehicle->getNet()->getViewNet()->getEditModes().isCurrentSupermodeDemand()) {
        // Get icons
        FXIcon* vehicleIcon = GUIIconSubSys::getIcon(GUIIcon::VEHICLE);
        FXIcon* tripIcon = GUIIconSubSys::getIcon(GUIIcon::TRIP);
        FXIcon* routeFlowIcon = GUIIconSubSys::getIcon(GUIIcon::ROUTEFLOW);
        FXIcon* flowIcon = GUIIconSubSys::getIcon(GUIIcon::FLOW);
        // create menu pane for transform operations
        FXMenuPane* transformOperation = new FXMenuPane(this);
        this->insertMenuPaneChild(transformOperation);
        new FXMenuCascade(this, "transform to", nullptr, transformOperation);
        // Create menu comands for all transform
        myTransformToVehicle = new FXMenuCommand(transformOperation,
                ("Vehicles (Only " + vehicle->getTagStr() + ")").c_str(), vehicleIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToVehicleWithEmbeddedRoute = new FXMenuCommand(transformOperation,
                ("Vehicles (embedded route, only " + vehicle->getTagStr() + ")").c_str(), vehicleIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToRouteFlow = new FXMenuCommand(transformOperation,
                ("RouteFlows (Only " + vehicle->getTagStr() + ")").c_str(), routeFlowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToRouteFlowWithEmbeddedRoute = new FXMenuCommand(transformOperation,
                ("RouteFlows (embedded route, only " + vehicle->getTagStr() + ")").c_str(), routeFlowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToTrip = new FXMenuCommand(transformOperation,
                                              ("Trips (Only " + vehicle->getTagStr() + ")").c_str(), tripIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformToFlow = new FXMenuCommand(transformOperation,
                                              ("Flows (Only " + vehicle->getTagStr() + ")").c_str(), flowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        // create separator
        new FXMenuSeparator(transformOperation);
        // Create menu comands for all transform all vehicles
        myTransformAllVehiclesToVehicle = new FXMenuCommand(transformOperation, "Vehicles", vehicleIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformAllVehiclesToVehicleWithEmbeddedRoute = new FXMenuCommand(transformOperation, "Vehicles (embedded route)", vehicleIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformAllVehiclesToRouteFlow = new FXMenuCommand(transformOperation, "RouteFlows", routeFlowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformAllVehiclesToRouteFlowWithEmbeddedRoute = new FXMenuCommand(transformOperation, "RouteFlows (embedded route)", routeFlowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformAllVehiclesToTrip = new FXMenuCommand(transformOperation, "Trips", tripIcon, this, MID_GNE_VEHICLE_TRANSFORM);
        myTransformAllVehiclesToFlow = new FXMenuCommand(transformOperation, "Flows", flowIcon, this, MID_GNE_VEHICLE_TRANSFORM);
    }
}


GNEVehicle::GNESelectedVehiclesPopupMenu::~GNESelectedVehiclesPopupMenu() {}


long
GNEVehicle::GNESelectedVehiclesPopupMenu::onCmdTransform(FXObject* obj, FXSelector, void*) {
    // iterate over all selected vehicles
    for (const auto& i : mySelectedVehicles) {
        if ((obj == myTransformToVehicle) &&
                (i->getTagProperty().getTag() == myVehicleTag)) {
            GNERouteHandler::transformToVehicle(i, false);
        } else if ((obj == myTransformToVehicleWithEmbeddedRoute) &&
                   (i->getTagProperty().getTag() == myVehicleTag)) {
            GNERouteHandler::transformToVehicle(i, true);
        } else if ((obj == myTransformToRouteFlow) &&
                   (i->getTagProperty().getTag() == myVehicleTag)) {
            GNERouteHandler::transformToRouteFlow(i, false);
        } else if ((obj == myTransformToRouteFlowWithEmbeddedRoute) &&
                   (i->getTagProperty().getTag() == myVehicleTag)) {
            GNERouteHandler::transformToRouteFlow(i, true);
        } else if ((obj == myTransformToTrip) &&
                   (i->getTagProperty().getTag() == myVehicleTag)) {
            GNERouteHandler::transformToTrip(i);
        } else if ((obj == myTransformToFlow) &&
                   (i->getTagProperty().getTag() == myVehicleTag)) {
            GNERouteHandler::transformToFlow(i);
        } else if (obj == myTransformAllVehiclesToVehicle) {
            GNERouteHandler::transformToVehicle(i, false);
        } else if (obj == myTransformAllVehiclesToVehicleWithEmbeddedRoute) {
            GNERouteHandler::transformToVehicle(i, true);
        } else if (obj == myTransformAllVehiclesToRouteFlow) {
            GNERouteHandler::transformToRouteFlow(i, false);
        } else if (obj == myTransformAllVehiclesToRouteFlowWithEmbeddedRoute) {
            GNERouteHandler::transformToRouteFlow(i, true);
        } else if (obj == myTransformAllVehiclesToTrip) {
            GNERouteHandler::transformToTrip(i);
        } else if (obj == myTransformAllVehiclesToFlow) {
            GNERouteHandler::transformToFlow(i);
        }
    }
    return 1;
}

// ===========================================================================
// member method definitions
// ===========================================================================

GNEVehicle::GNEVehicle(SumoXMLTag tag, GNENet* net, const std::string& vehicleID, GNEDemandElement* vehicleType, GNEDemandElement* route) :
    GNEDemandElement(vehicleID, net, (tag == GNE_TAG_FLOW_ROUTE) ? GLO_ROUTEFLOW : GLO_VEHICLE, tag,
{}, {}, {}, {}, {}, {}, {vehicleType, route}, {}),
SUMOVehicleParameter() {
    // SUMOVehicleParameter ID has to be set manually
    id = vehicleID;
    // set manually vtypeID (needed for saving)
    vtypeid = vehicleType->getID();
}


GNEVehicle::GNEVehicle(GNENet* net, GNEDemandElement* vehicleType, GNEDemandElement* route, const SUMOVehicleParameter& vehicleParameters) :
    GNEDemandElement(vehicleParameters.id, net, (vehicleParameters.tag == GNE_TAG_FLOW_ROUTE) ? GLO_ROUTEFLOW : GLO_VEHICLE, vehicleParameters.tag,
{}, {}, {}, {}, {}, {}, {vehicleType, route}, {}),
SUMOVehicleParameter(vehicleParameters) {
    // SUMOVehicleParameter ID has to be set manually
    id = vehicleParameters.id;
    // set manually vtypeID (needed for saving)
    vtypeid = vehicleType->getID();
}


GNEVehicle::GNEVehicle(GNENet* net, GNEDemandElement* vehicleType, const SUMOVehicleParameter& vehicleParameters) :
    GNEDemandElement(vehicleParameters.id, net, (vehicleParameters.tag == GNE_TAG_VEHICLE_WITHROUTE) ? GLO_VEHICLE : GLO_ROUTEFLOW, vehicleParameters.tag,
{}, {}, {}, {}, {}, {}, {vehicleType}, {}),
SUMOVehicleParameter(vehicleParameters) {
    // SUMOVehicleParameter ID has to be set manually
    id = vehicleParameters.id;
    // reset routeid
    routeid.clear();
    // set manually vtypeID (needed for saving)
    vtypeid = vehicleType->getID();
}


GNEVehicle::GNEVehicle(SumoXMLTag tag, GNENet* net, const std::string& vehicleID, GNEDemandElement* vehicleType, GNEEdge* fromEdge, GNEEdge* toEdge,
                       const std::vector<GNEEdge*>& via) :
    GNEDemandElement(vehicleID, net, (tag == SUMO_TAG_FLOW) ? GLO_FLOW : GLO_TRIP, tag,
{}, {fromEdge, toEdge}, {}, {}, {}, {}, {vehicleType}, {}),
SUMOVehicleParameter() {
    // set via parameter without updating references
    replaceMiddleParentEdges(toString(via), false);
    // compute vehicle
    computePath();
}


GNEVehicle::GNEVehicle(GNENet* net, GNEDemandElement* vehicleType, GNEEdge* fromEdge, GNEEdge* toEdge, const std::vector<GNEEdge*>& via,
                       const SUMOVehicleParameter& vehicleParameters) :
    GNEDemandElement(vehicleParameters.id, net, (vehicleParameters.tag == SUMO_TAG_FLOW) ? GLO_FLOW : GLO_TRIP, vehicleParameters.tag,
{}, {fromEdge, toEdge}, {}, {}, {}, {}, {vehicleType}, {}),
SUMOVehicleParameter(vehicleParameters) {
    // set via parameter without updating references
    replaceMiddleParentEdges(toString(via), false);
    // compute vehicle
    computePath();
}


GNEVehicle::~GNEVehicle() {}


std::string
GNEVehicle::getBegin() const {
    // obtain depart
    std::string departStr = time2string(depart);
    // we need to handle depart as a tuple of 20 numbers (format: 000000...00<departTime>)
    departStr.reserve(20 - departStr.size());
    // add 0s at the beginning of departStr until we have 20 numbers
    for (int i = (int)departStr.size(); i < 20; i++) {
        departStr.insert(departStr.begin(), '0');
    }
    return departStr;
}


void
GNEVehicle::writeDemandElement(OutputDevice& device) const {
    // obtain tag depending if tagProperty has a synonym
    SumoXMLTag synonymTag = myTagProperty.hasTagSynonym() ? myTagProperty.getTagSynonym() : myTagProperty.getTag();
    // attribute VType musn't be written if is DEFAULT_VTYPE_ID
    if (getParentDemandElements().at(0)->getID() == DEFAULT_VTYPE_ID) {
        // unset VType parameter
        parametersSet &= ~VEHPARS_VTYPE_SET;
        // write vehicle attributes (VType will not be written)
        write(device, OptionsCont::getOptions(), synonymTag);
        // set VType parameter again
        parametersSet |= VEHPARS_VTYPE_SET;
    } else {
        // write vehicle attributes, including VType
        write(device, OptionsCont::getOptions(), synonymTag, getParentDemandElements().at(0)->getID());
    }
    // write specific attribute depeding of tag property
    if (myTagProperty.getTag() == SUMO_TAG_VEHICLE || myTagProperty.getTag() == GNE_TAG_FLOW_ROUTE) {
        // write route
        device.writeAttr(SUMO_ATTR_ROUTE, getParentDemandElements().at(1)->getID());
    }
    // write from, to and edge vias
    if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
        // write manually from/to edges (it correspond to fron and back parent edges)
        device.writeAttr(SUMO_ATTR_FROM, getParentEdges().front()->getID());
        device.writeAttr(SUMO_ATTR_TO, getParentEdges().back()->getID());
        // only write via if there isn't empty
        if (via.size() > 0) {
            device.writeAttr(SUMO_ATTR_VIA, via);
        }
    }
    // write specific routeFlow/flow attributes
    if ((myTagProperty.getTag() == GNE_TAG_FLOW_ROUTE) || (myTagProperty.getTag() == GNE_TAG_FLOW_WITHROUTE) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
        // write routeFlow values depending if it was set
        if (isAttributeEnabled(SUMO_ATTR_END)) {
            device.writeAttr(SUMO_ATTR_END,  time2string(repetitionEnd));
        }
        if (isAttributeEnabled(SUMO_ATTR_NUMBER)) {
            device.writeAttr(SUMO_ATTR_NUMBER, repetitionNumber);
        }
        if (isAttributeEnabled(SUMO_ATTR_VEHSPERHOUR)) {
            device.writeAttr(SUMO_ATTR_VEHSPERHOUR, 3600. / STEPS2TIME(repetitionOffset));
        }
        if (isAttributeEnabled(SUMO_ATTR_PERIOD)) {
            device.writeAttr(SUMO_ATTR_PERIOD, time2string(repetitionOffset));
        }
        if (isAttributeEnabled(SUMO_ATTR_PROB)) {
            device.writeAttr(SUMO_ATTR_PROB, repetitionProbability);
        }
    }
    // write parameters
    writeParams(device);
    // write child demand elements associated to this vehicle
    for (const auto& i : getChildDemandElements()) {
        i->writeDemandElement(device);
    }
    // close vehicle tag
    device.closeTag();
}


bool
GNEVehicle::isDemandElementValid() const {
    // only trips or flows can have problems
    if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
        // check if from and to are the same edges
        if ((getParentEdges().size() == 2) && (getParentEdges().at(0) == getParentEdges().at(1))) {
            return true;
        } else if (getPath().size() > 0) {
            // if path edges isn't empty, then there is a valid route
            return true;
        } else {
            return false;
        }
    } else if (getParentDemandElements().size() == 2) {
        // check if exist a valid path using route parent edges
        if (myNet->getPathCalculator()->calculatePath(getParentDemandElements().at(0)->getVClass(), getParentDemandElements().at(1)->getParentEdges()).size() > 0) {
            return true;
        } else {
            return false;
        }
    } else if (getChildDemandElements().size() > 0 && (getChildDemandElements().front()->getTagProperty().getTag() == GNE_TAG_ROUTE_EMBEDDED)) {
        // check if exist a valid path using embebbed route edges
        if (myNet->getPathCalculator()->calculatePath(getParentDemandElements().at(0)->getVClass(), getChildDemandElements().front()->getParentEdges()).size() > 0) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


std::string
GNEVehicle::getDemandElementProblem() const {
    // only trips or flows can have problems
    if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
        // check if exist at least a connection between every edge
        for (int i = 1; i < (int)getParentEdges().size(); i++) {
            if (myNet->getPathCalculator()->consecutiveEdgesConnected(getParentDemandElements().at(0)->getVClass(), getParentEdges().at((int)i - 1), getParentEdges().at(i)) == false) {
                return ("There is no valid path between edges '" + getParentEdges().at((int)i - 1)->getID() + "' and '" + getParentEdges().at(i)->getID() + "'");
            }
        }
        // there is connections bewteen all edges, then all ok
        return "";
    } else if (getParentDemandElements().size() == 2) {
        // get route parent edges
        const std::vector<GNEEdge*>& routeEdges = getParentDemandElements().at(1)->getParentEdges();
        // check if exist at least a connection between every edge
        for (int i = 1; i < (int)routeEdges.size(); i++) {
            if (myNet->getPathCalculator()->consecutiveEdgesConnected(getParentDemandElements().at(0)->getVClass(), routeEdges.at((int)i - 1), routeEdges.at(i)) == false) {
                return ("There is no valid path between route edges '" + routeEdges.at((int)i - 1)->getID() + "' and '" + routeEdges.at(i)->getID() + "'");
            }
        }
        // there is connections bewteen all edges, then all ok
        return "";
    } else if (getChildDemandElements().size() > 0 && (getChildDemandElements().front()->getTagProperty().getTag() == GNE_TAG_ROUTE_EMBEDDED)) {
        // get embebbed route edges
        const std::vector<GNEEdge*>& routeEdges = getChildDemandElements().front()->getParentEdges();
        // check if exist at least a connection between every edge
        for (int i = 1; i < (int)routeEdges.size(); i++) {
            if (myNet->getPathCalculator()->consecutiveEdgesConnected(getParentDemandElements().at(0)->getVClass(), routeEdges.at((int)i - 1), routeEdges.at(i)) == false) {
                return ("There is no valid path between embebbed route edges '" + routeEdges.at((int)i - 1)->getID() + "' and '" + routeEdges.at(i)->getID() + "'");
            }
        }
        // there is connections bewteen all edges, then all ok
        return "";
    } else {
        return "";
    }
}


void
GNEVehicle::fixDemandElementProblem() {

}


SUMOVehicleClass
GNEVehicle::getVClass() const {
    return getParentDemandElements().front()->getVClass();
}


const RGBColor&
GNEVehicle::getColor() const {
    return color;
}


void
GNEVehicle::startGeometryMoving() {
    // Vehicles cannot be moved
}


void
GNEVehicle::endGeometryMoving() {
    // Vehicles cannot be moved
}


void
GNEVehicle::moveGeometry(const Position&) {
    // Vehicles cannot be moved
}


void
GNEVehicle::commitGeometryMoving(GNEUndoList*) {
    // Vehicles cannot be moved
}


void
GNEVehicle::updateGeometry() {
    // declare extreme geometry
    GNEGeometry::ExtremeGeometry extremeGeometry;
    // check if depart and arrival pos lanes are defined
    if (departPosProcedure == DepartPosDefinition::GIVEN) {
        extremeGeometry.laneStartPosition = departPos;
    }
    if (arrivalPosProcedure == ArrivalPosDefinition::GIVEN) {
        extremeGeometry.laneEndPosition = arrivalPos;
    }
    // get first allowed lane
    GNELane* firstLane = getFirstAllowedVehicleLane();
    // check if first lane wasn't sucesfully obtained
    if (!firstLane) {
        if ((myTagProperty.getTag() == SUMO_TAG_VEHICLE) || (myTagProperty.getTag() == GNE_TAG_FLOW_ROUTE)) {
            // use route edges
            firstLane = getParentDemandElements().at(1)->getParentEdges().front()->getLanes().front();
        } else if ((myTagProperty.getTag() == GNE_TAG_VEHICLE_WITHROUTE) || (myTagProperty.getTag() == GNE_TAG_FLOW_WITHROUTE)) {
            // use embebbed route
            if (getChildDemandElements().size() > 0) {
                firstLane = getChildDemandElements().front()->getParentEdges().front()->getLanes().front();
            } else {
                firstLane = nullptr;
            }
        } else if ((getPath().size() > 0) && getPath().front().getLane()) {
            // use path edges
            firstLane = getPath().front().getLane();
        } else if (getParentEdges().size() > 0) {
            // use first
            firstLane = getParentEdges().front()->getLanes().front();
        } else {
            firstLane = nullptr;
        }
    }
    // continue only if lane was sucesfully found
    if (firstLane) {
        // check if depart pos has to be adjusted
        if ((departPosProcedure == DepartPosDefinition::GIVEN) && (extremeGeometry.laneStartPosition < 0)) {
            extremeGeometry.laneStartPosition += firstLane->getLaneShape().length();
        }
        // continue depending of tag
        if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
            // calculate edge geometry path using path
            GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getPath(), extremeGeometry);
        } else if ((myTagProperty.getTag() == SUMO_TAG_VEHICLE) || (myTagProperty.getTag() == GNE_TAG_FLOW_ROUTE)) {
            // calculate edge geometry path using route edges
            GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getParentDemandElements().at(1)->getPath(), extremeGeometry);
        } else if ((myTagProperty.getTag() == GNE_TAG_VEHICLE_WITHROUTE) || (myTagProperty.getTag() == GNE_TAG_FLOW_WITHROUTE)) {
            // calculate edge geometry path using embedded route edges
            GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getChildDemandElements().front()->getPath(), extremeGeometry);
        }
        // update start pos geometry
        myDemandElementGeometry.updateGeometry(firstLane, extremeGeometry.laneStartPosition);
        firstLane->getParentEdge()->updateVehicleStackLabels();
    }
    // update child demand elementss
    for (const auto& i : getChildDemandElements()) {
        i->updateGeometry();
    }
}


void
GNEVehicle::computePath() {
    // calculate path (only for flows and trips)
    if ((myTagProperty.getTag() == SUMO_TAG_FLOW) || (myTagProperty.getTag() == SUMO_TAG_TRIP)) {
        calculatePathLanes(getVClass(), true,
                           getFirstAllowedVehicleLane(),
                           getLastAllowedVehicleLane(),
                           getViaEdges());
    }
    // update geometry
    updateGeometry();
}


void
GNEVehicle::invalidatePath() {
    // reset path (only for flows and trips)
    if ((myTagProperty.getTag() == SUMO_TAG_FLOW) || (myTagProperty.getTag() == SUMO_TAG_TRIP)) {
        resetPathLanes(getVClass(), true,
                       getFirstAllowedVehicleLane(),
                       getLastAllowedVehicleLane(),
                       getViaEdges());
    }
    // update geometry
    updateGeometry();
}


Position
GNEVehicle::getPositionInView() const {
    return myDemandElementGeometry.getPosition();
}


GUIGLObjectPopupMenu*
GNEVehicle::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    if (isAttributeCarrierSelected()) {
        // obtain all selected vehicles
        std::vector<GNEDemandElement*> selectedDemandElements = myNet->retrieveDemandElements(true);
        std::vector<GNEVehicle*> selectedVehicles;
        selectedVehicles.reserve(selectedDemandElements.size());
        for (const auto& i : selectedDemandElements) {
            if (i->getTagProperty().isVehicle()) {
                selectedVehicles.push_back(dynamic_cast<GNEVehicle*>(i));
            }
        }
        // return a GNESelectedVehiclesPopupMenu
        return new GNESelectedVehiclesPopupMenu(this, selectedVehicles, app, parent);
    } else {
        // return a GNESingleVehiclePopupMenu
        return new GNESingleVehiclePopupMenu(this, app, parent);
    }
}


std::string
GNEVehicle::getParentName() const {
    if ((myTagProperty.getTag() == SUMO_TAG_VEHICLE) || (myTagProperty.getTag() == GNE_TAG_FLOW_ROUTE)) {
        return getParentDemandElements().at(1)->getID();
    } else if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
        return getParentEdges().front()->getID();
    } else {
        throw ProcessError("Invalid vehicle tag");
    }
}


Boundary
GNEVehicle::getCenteringBoundary() const {
    Boundary vehicleBoundary;
    vehicleBoundary.add(myDemandElementGeometry.getPosition());
    vehicleBoundary.grow(20);
    return vehicleBoundary;
}


void
GNEVehicle::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEVehicle::drawGL(const GUIVisualizationSettings& s) const {
    // only drawn in super mode demand
    if (myNet->getViewNet()->getNetworkViewOptions().showDemandElements() &&
            myNet->getViewNet()->getDataViewOptions().showDemandElements() &&
            myNet->getViewNet()->getDemandViewOptions().showNonInspectedDemandElements(this)) {
        // declare common attributes
        const bool drawSpreadVehicles = (myNet->getViewNet()->getNetworkViewOptions().drawSpreadVehicles() || myNet->getViewNet()->getDemandViewOptions().drawSpreadVehicles());
        const double exaggeration = s.vehicleSize.getExaggeration(s, this);
        const double width = getParentDemandElements().at(0)->getAttributeDouble(SUMO_ATTR_WIDTH);
        const double length = getParentDemandElements().at(0)->getAttributeDouble(SUMO_ATTR_LENGTH);
        const double vehicleSizeSquared = (width * width) * (length * length) * (exaggeration * exaggeration);
        // obtain Position an rotation (depending of draw spread vehicles)
        const Position vehiclePosition = drawSpreadVehicles ? mySpreadGeometry.getPosition() : myDemandElementGeometry.getPosition();
        const double vehicleRotation = drawSpreadVehicles ? mySpreadGeometry.getRotation() : myDemandElementGeometry.getRotation();
        // check that position is valid
        if (vehiclePosition != Position::INVALID) {
            // first push name
            glPushName(getGlID());
            // first check if if mouse is enought near to this vehicle to draw it
            if (s.drawForRectangleSelection && (myNet->getViewNet()->getPositionInformation().distanceSquaredTo2D(vehiclePosition) >= (vehicleSizeSquared + 2))) {
                // push draw matrix
                glPushMatrix();
                // Start with the drawing of the area traslating matrix to origin
                myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
                // translate to drawing position
                glTranslated(vehiclePosition.x(), vehiclePosition.y(), 0);
                glRotated(vehicleRotation, 0, 0, -1);
                // extra translation needed to draw vehicle over edge (to avoid selecting problems)
                glTranslated(0, (-1) * length, 0);
                GLHelper::drawBoxLine(Position(0, 1), 0, 2, 1);
                // Pop last matrix
                glPopMatrix();
            } else {
                SUMOVehicleShape shape = getVehicleShapeID(getParentDemandElements().at(0)->getAttribute(SUMO_ATTR_GUISHAPE));
                // push draw matrix
                glPushMatrix();
                // Start with the drawing of the area traslating matrix to origin
                myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
                // translate to drawing position
                glTranslated(vehiclePosition.x(), vehiclePosition.y(), 0);
                glRotated(vehicleRotation, 0, 0, -1);
                // extra translation needed to draw vehicle over edge (to avoid selecting problems)
                glTranslated(0, (-1) * length, 0);
                // set lane color
                setColor(s);
                double upscaleLength = exaggeration;
                if ((exaggeration > 1) && (length > 5)) {
                    // reduce the length/width ratio because this is not usefull at high zoom
                    upscaleLength = MAX2(1.0, upscaleLength * (5 + sqrt(length - 5)) / length);
                }
                glScaled(exaggeration, upscaleLength, 1);
                // check if we're drawing in selecting mode
                if (s.drawForRectangleSelection) {
                    // draw vehicle as a box and don't draw the rest of details
                    GUIBaseVehicleHelper::drawAction_drawVehicleAsBoxPlus(width, length);
                } else {
                    // draw the vehicle depending of detail level
                    if (s.drawDetail(s.detailSettings.vehicleShapes, exaggeration)) {
                        GUIBaseVehicleHelper::drawAction_drawVehicleAsPoly(s, shape, width, length);
                    } else if (s.drawDetail(s.detailSettings.vehicleBoxes, exaggeration)) {
                        GUIBaseVehicleHelper::drawAction_drawVehicleAsBoxPlus(width, length);
                    } else if (s.drawDetail(s.detailSettings.vehicleTriangles, exaggeration)) {
                        GUIBaseVehicleHelper::drawAction_drawVehicleAsTrianglePlus(width, length);
                    }

                    /*
                    switch (s.vehicleQuality) {
                        case 0:
                            GUIBaseVehicleHelper::drawAction_drawVehicleAsTrianglePlus(width, length);
                            break;
                        case 1:
                            GUIBaseVehicleHelper::drawAction_drawVehicleAsBoxPlus(width, length);
                            break;
                        default:
                            GUIBaseVehicleHelper::drawAction_drawVehicleAsPoly(s, shape, width, length);
                            break;
                    }
                    */
                    // check if min gap has to be drawn
                    if (s.drawMinGap) {
                        const double minGap = -1 * getParentDemandElements().at(0)->getAttributeDouble(SUMO_ATTR_MINGAP);
                        glColor3d(0., 1., 0.);
                        glBegin(GL_LINES);
                        glVertex2d(0., 0);
                        glVertex2d(0., minGap);
                        glVertex2d(-.5, minGap);
                        glVertex2d(.5, minGap);
                        glEnd();
                    }
                    // drawing name at GLO_MAX fails unless translating z
                    glTranslated(0, MIN2(length / 2, double(5)), -getType());
                    glScaled(1 / exaggeration, 1 / upscaleLength, 1);
                    glRotated(vehicleRotation, 0, 0, -1);
                    drawName(Position(0, 0), s.scale, getParentDemandElements().at(0)->getAttribute(SUMO_ATTR_GUISHAPE) == "pedestrian" ? s.personName : s.vehicleName, s.angle);
                    // draw line
                    if (s.vehicleName.show && line != "") {
                        glTranslated(0, 0.6 * s.vehicleName.scaledSize(s.scale), 0);
                        GLHelper::drawTextSettings(s.vehicleName, "line:" + line, Position(0, 0), s.scale, s.angle);
                    }
                }
                // pop draw matrix
                glPopMatrix();
                // draw stack label
                if ((myStackedLabelNumber > 0) && !drawSpreadVehicles) {
                    drawStackLabel(s, vehiclePosition, vehicleRotation, width, length);
                }
                // draw flow label
                if ((myTagProperty.getTag() == SUMO_TAG_FLOW) || (myTagProperty.getTag() == GNE_TAG_FLOW_ROUTE) || (myTagProperty.getTag() == GNE_TAG_FLOW_WITHROUTE)) {
                    drawFlowLabel(s, vehiclePosition, vehicleRotation, width, length);
                }
                // check if dotted contours has to be drawn
                if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
                    // draw using drawDottedContourClosedShape
                    GNEGeometry::drawDottedSquaredShape(true, s, vehiclePosition, length * 0.5, width * 0.5, length * -0.5, 0, vehicleRotation, exaggeration);
                }
                if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
                    // draw using drawDottedContourClosedShape
                    GNEGeometry::drawDottedSquaredShape(false, s, vehiclePosition, length * 0.5, width * 0.5, length * -0.5, 0, vehicleRotation, exaggeration);
                }
            }
            // pop name
            glPopName();
        }
    }
}


void
GNEVehicle::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const {
    if (!s.drawForRectangleSelection && ((s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) || isAttributeCarrierSelected())) {
        // declare flag to draw spread vehicles
        const bool drawSpreadVehicles = (myNet->getViewNet()->getNetworkViewOptions().drawSpreadVehicles() || myNet->getViewNet()->getDemandViewOptions().drawSpreadVehicles());
        // calculate width
        const double width = s.vehicleSize.getExaggeration(s, lane) * s.widthSettings.trip;
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, getType() + offsetFront);
        // Set color of the base
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedVehicleColor);
        } else {
            GLHelper::setColor(s.colorSettings.vehicleTrips);
        }
        // iterate over segments
        if (drawSpreadVehicles) {
            for (const auto& segment : myDemandElementSegmentGeometry) {
                // draw partial segment
                if (segment.isLaneSegment() && (segment.getLane() == lane)) {
                    GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, width);
                }
            }
        } else {
            for (const auto& segment : myDemandElementSegmentGeometry) {
                // draw partial segment
                if (segment.isLaneSegment() && (segment.getLane() == lane)) {
                    GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, width);
                }
            }
        }
        // Pop last matrix
        glPopMatrix();
        // Draw name if isn't being drawn for selecting
        if (!s.drawForRectangleSelection) {
            drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
        }
        // check if shape dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // get first and last allowed lanes
            const GNELane* firstLane = getFirstAllowedVehicleLane();
            const GNELane* lastLane = getLastAllowedVehicleLane();
            // iterate over segments
            for (const auto& segment : myDemandElementSegmentGeometry) {
                if (segment.isLaneSegment() && (segment.getLane() == lane)) {
                    // draw partial segment
                    if (firstLane == lane) {
                        // draw front dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), width, true, false);
                    } else if (lastLane == lane) {
                        // draw back dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), width, false, true);
                    } else {
                        // draw dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, lane->getDottedLaneGeometry(), width, false, false);
                    }
                }
            }
        }
        // Pop name
        glPopName();
    }
}


void
GNEVehicle::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const {
    if (!s.drawForRectangleSelection && fromLane->getLane2laneConnections().exist(toLane) &&
            ((s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) || isAttributeCarrierSelected())) {
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // obtain lane2lane geometry
        const GNEGeometry::Geometry& lane2laneGeometry = fromLane->getLane2laneConnections().getLane2laneGeometry(toLane);
        // calculate width
        const double width = s.vehicleSize.getExaggeration(s, fromLane) * s.widthSettings.trip;
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, getType() + offsetFront);
        // Set color of the base
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedVehicleColor);
        } else {
            GLHelper::setColor(s.colorSettings.vehicleTrips);
        }
        // draw lane2lane
        GNEGeometry::drawGeometry(myNet->getViewNet(), lane2laneGeometry, width);
        // Pop last matrix
        glPopMatrix();
        // check if shape dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // draw lane2lane dotted geometry
            if (fromLane->getLane2laneConnections().exist(toLane)) {
                GNEGeometry::drawDottedContourLane(true, s, fromLane->getLane2laneConnections().getLane2laneDottedGeometry(toLane), width, false, false);
            }
        }
        // Pop name
        glPopName();
    }
}


std::string
GNEVehicle::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_TYPE:
            return getParentDemandElements().at(0)->getID();
        case SUMO_ATTR_COLOR:
            if (wasSet(VEHPARS_COLOR_SET)) {
                return toString(color);
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_COLOR);
            }
        case SUMO_ATTR_DEPARTLANE:
            if (wasSet(VEHPARS_DEPARTLANE_SET)) {
                return getDepartLane();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_DEPARTLANE);
            }
        case SUMO_ATTR_DEPARTPOS:
            if (wasSet(VEHPARS_DEPARTPOS_SET)) {
                return getDepartPos();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_DEPARTPOS);
            }
        case SUMO_ATTR_DEPARTSPEED:
            if (wasSet(VEHPARS_DEPARTSPEED_SET)) {
                return getDepartSpeed();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_DEPARTSPEED);
            }
        case SUMO_ATTR_ARRIVALLANE:
            if (wasSet(VEHPARS_ARRIVALLANE_SET)) {
                return getArrivalLane();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_ARRIVALLANE);
            }
        case SUMO_ATTR_ARRIVALPOS:
            if (wasSet(VEHPARS_ARRIVALPOS_SET)) {
                return getArrivalPos();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_ARRIVALPOS);
            }
        case SUMO_ATTR_ARRIVALSPEED:
            if (wasSet(VEHPARS_ARRIVALSPEED_SET)) {
                return getArrivalSpeed();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_ARRIVALSPEED);
            }
        case SUMO_ATTR_LINE:
            if (wasSet(VEHPARS_LINE_SET)) {
                return line;
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_LINE);
            }
        case SUMO_ATTR_PERSON_NUMBER:
            if (wasSet(VEHPARS_PERSON_NUMBER_SET)) {
                return toString(personNumber);
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_PERSON_NUMBER);
            }
        case SUMO_ATTR_CONTAINER_NUMBER:
            if (wasSet(VEHPARS_CONTAINER_NUMBER_SET)) {
                return toString(containerNumber);
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_CONTAINER_NUMBER);
            }
        case SUMO_ATTR_REROUTE:
            if (wasSet(VEHPARS_CONTAINER_NUMBER_SET)) {
                return "true";
            } else {
                return "false";
            }
        case SUMO_ATTR_DEPARTPOS_LAT:
            if (wasSet(VEHPARS_DEPARTPOSLAT_SET)) {
                return getDepartPosLat();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_DEPARTPOS_LAT);
            }
        case SUMO_ATTR_ARRIVALPOS_LAT:
            if (wasSet(VEHPARS_ARRIVALPOSLAT_SET)) {
                return getArrivalPosLat();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_ARRIVALPOS_LAT);
            }
        // Specific of vehicles
        case SUMO_ATTR_DEPART:
            return time2string(depart);
        case SUMO_ATTR_ROUTE:
            if (getParentDemandElements().size() == 2) {
                return getParentDemandElements().at(1)->getID();
            } else {
                return "";
            }
        // Specific of Trips
        case SUMO_ATTR_FROM:
            return getParentEdges().front()->getID();
        case SUMO_ATTR_TO:
            return getParentEdges().back()->getID();
        case SUMO_ATTR_VIA:
            return toString(via);
        // Specific of routeFlows
        case SUMO_ATTR_BEGIN:
            return time2string(depart);
        case SUMO_ATTR_END:
            return time2string(repetitionEnd);
        case SUMO_ATTR_VEHSPERHOUR:
            return toString(3600 / STEPS2TIME(repetitionOffset));
        case SUMO_ATTR_PERIOD:
            return time2string(repetitionOffset);
        case SUMO_ATTR_PROB:
            return toString(repetitionProbability);
        case SUMO_ATTR_NUMBER:
            return toString(repetitionNumber);
        //
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEVehicle::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_DEPART:
        case SUMO_ATTR_BEGIN:
            return STEPS2TIME(depart);
        case SUMO_ATTR_DEPARTPOS:
            // only return departPos it if is given
            if (departPosProcedure == DepartPosDefinition::GIVEN) {
                return departPos;
            } else {
                return 0;
            }
        case SUMO_ATTR_ARRIVALPOS:
            // only return departPos it if is given
            if (arrivalPosProcedure == ArrivalPosDefinition::GIVEN) {
                return arrivalPos;
            } else {
                return -1;
            }
        case SUMO_ATTR_WIDTH:
        case SUMO_ATTR_LENGTH:
        case SUMO_ATTR_MINGAP:
            return getParentDemandElements().at(0)->getAttributeDouble(key);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNEVehicle::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_TYPE:
        case SUMO_ATTR_COLOR:
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
        // Specific of vehicles
        case SUMO_ATTR_DEPART:
        case SUMO_ATTR_ROUTE:
        // Specific of Trips
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
        case SUMO_ATTR_VIA:
        //
        // Specific of routeFlows
        case SUMO_ATTR_BEGIN:
        case SUMO_ATTR_END:
        case SUMO_ATTR_NUMBER:
        case SUMO_ATTR_VEHSPERHOUR:
        case SUMO_ATTR_PERIOD:
        case SUMO_ATTR_PROB:
        //
        case GNE_ATTR_PARAMETERS:
        case GNE_ATTR_SELECTED:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEVehicle::isValid(SumoXMLAttr key, const std::string& value) {
    // declare string error
    std::string error;
    switch (key) {
        case SUMO_ATTR_ID:
            // Vehicles, Trips and Flows share namespace
            if (SUMOXMLDefinitions::isValidVehicleID(value) &&
                    (myNet->retrieveDemandElement(SUMO_TAG_VEHICLE, value, false) == nullptr) &&
                    (myNet->retrieveDemandElement(SUMO_TAG_TRIP, value, false) == nullptr) &&
                    (myNet->retrieveDemandElement(GNE_TAG_FLOW_ROUTE, value, false) == nullptr) &&
                    (myNet->retrieveDemandElement(SUMO_TAG_FLOW, value, false) == nullptr)) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_TYPE:
            return SUMOXMLDefinitions::isValidTypeID(value) && (myNet->retrieveDemandElement(SUMO_TAG_VTYPE, value, false) != nullptr);
        case SUMO_ATTR_COLOR:
            return canParse<RGBColor>(value);
        case SUMO_ATTR_DEPARTLANE: {
            int dummyDepartLane;
            DepartLaneDefinition dummyDepartLaneProcedure;
            parseDepartLane(value, toString(SUMO_TAG_VEHICLE), id, dummyDepartLane, dummyDepartLaneProcedure, error);
            // if error is empty, check if depart lane is correct
            if (error.empty()) {
                const GNELane* lane = getFirstAllowedVehicleLane();
                if (lane == nullptr) {
                    return false;
                } else if (dummyDepartLaneProcedure != DepartLaneDefinition::GIVEN) {
                    return true;
                } else {
                    return dummyDepartLane < (int)lane->getParentEdge()->getLanes().size();
                }
            } else {
                return false;
            }
        }
        case SUMO_ATTR_DEPARTPOS: {
            double dummyDepartPos;
            DepartPosDefinition dummyDepartPosProcedure;
            parseDepartPos(value, toString(SUMO_TAG_VEHICLE), id, dummyDepartPos, dummyDepartPosProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        case SUMO_ATTR_DEPARTSPEED: {
            double dummyDepartSpeed;
            DepartSpeedDefinition dummyDepartSpeedProcedure;
            parseDepartSpeed(value, toString(SUMO_TAG_VEHICLE), id, dummyDepartSpeed, dummyDepartSpeedProcedure, error);
            // if error is empty, check if depart speed is correct
            if (error.empty()) {
                if (dummyDepartSpeedProcedure != DepartSpeedDefinition::GIVEN) {
                    return true;
                } else {
                    return (dummyDepartSpeed <= getParentDemandElements().at(0)->getAttributeDouble(SUMO_ATTR_MAXSPEED));
                }
            } else {
                return false;
            }
        }
        case SUMO_ATTR_ARRIVALLANE: {
            int dummyArrivalLane;
            ArrivalLaneDefinition dummyArrivalLaneProcedure;
            parseArrivalLane(value, toString(SUMO_TAG_VEHICLE), id, dummyArrivalLane, dummyArrivalLaneProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        case SUMO_ATTR_ARRIVALPOS: {
            double dummyArrivalPos;
            ArrivalPosDefinition dummyArrivalPosProcedure;
            parseArrivalPos(value, toString(SUMO_TAG_VEHICLE), id, dummyArrivalPos, dummyArrivalPosProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        case SUMO_ATTR_ARRIVALSPEED: {
            double dummyArrivalSpeed;
            ArrivalSpeedDefinition dummyArrivalSpeedProcedure;
            parseArrivalSpeed(value, toString(SUMO_TAG_VEHICLE), id, dummyArrivalSpeed, dummyArrivalSpeedProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        case SUMO_ATTR_LINE:
            return true;
        case SUMO_ATTR_PERSON_NUMBER:
            return canParse<int>(value) && parse<int>(value) >= 0;
        case SUMO_ATTR_CONTAINER_NUMBER:
            return canParse<int>(value) && parse<int>(value) >= 0;
        case SUMO_ATTR_REROUTE:
            return true;    // check
        case SUMO_ATTR_DEPARTPOS_LAT: {
            double dummyDepartPosLat;
            DepartPosLatDefinition dummyDepartPosLatProcedure;
            parseDepartPosLat(value, toString(SUMO_TAG_VEHICLE), id, dummyDepartPosLat, dummyDepartPosLatProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        case SUMO_ATTR_ARRIVALPOS_LAT: {
            double dummyArrivalPosLat;
            ArrivalPosLatDefinition dummyArrivalPosLatProcedure;
            parseArrivalPosLat(value, toString(SUMO_TAG_VEHICLE), id, dummyArrivalPosLat, dummyArrivalPosLatProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        // Specific of vehicles
        case SUMO_ATTR_DEPART: {
            SUMOTime dummyDepart;
            DepartDefinition dummyDepartProcedure;
            parseDepart(value, toString(SUMO_TAG_VEHICLE), id, dummyDepart, dummyDepartProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        case SUMO_ATTR_ROUTE:
            if (getParentDemandElements().size() == 2) {
                return SUMOXMLDefinitions::isValidVehicleID(value) && (myNet->retrieveDemandElement(SUMO_TAG_ROUTE, value, false) != nullptr);
            } else {
                return true;
            }
        // Specific of Trips
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveEdge(value, false) != nullptr);
        case SUMO_ATTR_VIA:
            if (value.empty()) {
                return true;
            } else {
                return canParse<std::vector<GNEEdge*> >(myNet, value, false);
            }
        // Specific of routeFlows
        case SUMO_ATTR_BEGIN:
            if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_END:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_VEHSPERHOUR:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) > 0);
            } else {
                return false;
            }
        case SUMO_ATTR_PERIOD:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) > 0);
            } else {
                return false;
            }
        case SUMO_ATTR_PROB:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_NUMBER:
            if (canParse<int>(value)) {
                return (parse<int>(value) >= 0);
            } else {
                return false;
            }
        //
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEVehicle::enableAttribute(SumoXMLAttr key, GNEUndoList* undoList) {
    // obtain a copy of parameter sets
    int newParametersSet = parametersSet;
    // modify newParametersSet
    GNERouteHandler::setFlowParameters(key, newParametersSet);
    // add GNEChange_EnableAttribute
    undoList->add(new GNEChange_EnableAttribute(this, parametersSet, newParametersSet), true);
}


void
GNEVehicle::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // nothing to disable
}


bool
GNEVehicle::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_END:
            return (parametersSet & VEHPARS_END_SET) != 0;
        case SUMO_ATTR_NUMBER:
            return (parametersSet & VEHPARS_NUMBER_SET) != 0;
        case SUMO_ATTR_VEHSPERHOUR:
            return (parametersSet & VEHPARS_VPH_SET) != 0;
        case SUMO_ATTR_PERIOD:
            return (parametersSet & VEHPARS_PERIOD_SET) != 0;
        case SUMO_ATTR_PROB:
            return (parametersSet & VEHPARS_PROB_SET) != 0;
        default:
            return true;
    }
}


std::string
GNEVehicle::getPopUpID() const {
    return getTagStr();
}


std::string
GNEVehicle::getHierarchyName() const {
    // special case for Trips and flow
    if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
        // check if we're inspecting a Edge
        if (myNet->getViewNet()->getInspectedAttributeCarrier() &&
                myNet->getViewNet()->getInspectedAttributeCarrier()->getTagProperty().getTag() == SUMO_TAG_EDGE) {
            // check if edge correspond to a "from", "to" or "via" edge
            if (getParentEdges().front() == myNet->getViewNet()->getInspectedAttributeCarrier()) {
                return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID) + " (from)";
            } else if (getParentEdges().front() == myNet->getViewNet()->getInspectedAttributeCarrier()) {
                return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID) + " (to)";
            } else {
                // iterate over via
                for (const auto& i : via) {
                    if (i == myNet->getViewNet()->getInspectedAttributeCarrier()->getID()) {
                        return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID) + " (via)";
                    }
                }
            }
        }
    }
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID);
}

// ===========================================================================
// protected
// ===========================================================================

void
GNEVehicle::setColor(const GUIVisualizationSettings& s) const {
    // change color
    if (drawUsingSelectColor()) {
        GLHelper::setColor(s.colorSettings.selectedVehicleColor);
    } else {
        // obtain vehicle color
        const GUIColorer& c = s.vehicleColorer;
        // set color depending of vehicle color active
        switch (c.getActive()) {
            case 0: {
                // test for emergency vehicle
                if (getParentDemandElements().at(0)->getAttribute(SUMO_ATTR_GUISHAPE) == "emergency") {
                    GLHelper::setColor(RGBColor::WHITE);
                    break;
                }
                // test for firebrigade
                if (getParentDemandElements().at(0)->getAttribute(SUMO_ATTR_GUISHAPE) == "firebrigade") {
                    GLHelper::setColor(RGBColor::RED);
                    break;
                }
                // test for police car
                if (getParentDemandElements().at(0)->getAttribute(SUMO_ATTR_GUISHAPE) == "police") {
                    GLHelper::setColor(RGBColor::BLUE);
                    break;
                }
                // check if color was set
                if (wasSet(VEHPARS_COLOR_SET)) {
                    GLHelper::setColor(color);
                    break;
                } else {
                    // take their parent's color)
                    GLHelper::setColor(getParentDemandElements().at(0)->getColor());
                    break;
                }
                break;
            }
            case 2: {
                if (wasSet(VEHPARS_COLOR_SET)) {
                    GLHelper::setColor(color);
                } else {
                    GLHelper::setColor(c.getScheme().getColor(0));
                }
                break;
            }
            case 3: {
                if (getParentDemandElements().at(0)->isAttributeEnabled(SUMO_ATTR_COLOR)) {
                    GLHelper::setColor(getParentDemandElements().at(0)->getColor());
                } else {
                    GLHelper::setColor(c.getScheme().getColor(0));
                }
                break;
            }
            case 4: {
                if (getParentDemandElements().at(1)->getColor() != RGBColor::DEFAULT_COLOR) {
                    GLHelper::setColor(getParentDemandElements().at(1)->getColor());
                } else {
                    GLHelper::setColor(c.getScheme().getColor(0));
                }
                break;
            }
            case 5: {
                Position p = getParentDemandElements().at(1)->getParentEdges().at(0)->getLanes().at(0)->getLaneShape()[0];
                const Boundary& b = myNet->getBoundary();
                Position center = b.getCenter();
                double hue = 180. + atan2(center.x() - p.x(), center.y() - p.y()) * 180. / M_PI;
                double sat = p.distanceTo(center) / center.distanceTo(Position(b.xmin(), b.ymin()));
                GLHelper::setColor(RGBColor::fromHSV(hue, sat, 1.));
                break;
            }
            case 6: {
                Position p = getParentDemandElements().at(1)->getParentEdges().back()->getLanes().at(0)->getLaneShape()[-1];
                const Boundary& b = myNet->getBoundary();
                Position center = b.getCenter();
                double hue = 180. + atan2(center.x() - p.x(), center.y() - p.y()) * 180. / M_PI;
                double sat = p.distanceTo(center) / center.distanceTo(Position(b.xmin(), b.ymin()));
                GLHelper::setColor(RGBColor::fromHSV(hue, sat, 1.));
                break;
            }
            case 7: {
                Position pb = getParentDemandElements().at(1)->getParentEdges().at(0)->getLanes().at(0)->getLaneShape()[0];
                Position pe = getParentDemandElements().at(1)->getParentEdges().back()->getLanes().at(0)->getLaneShape()[-1];
                const Boundary& b = myNet->getBoundary();
                double hue = 180. + atan2(pb.x() - pe.x(), pb.y() - pe.y()) * 180. / M_PI;
                Position minp(b.xmin(), b.ymin());
                Position maxp(b.xmax(), b.ymax());
                double sat = pb.distanceTo(pe) / minp.distanceTo(maxp);
                GLHelper::setColor(RGBColor::fromHSV(hue, sat, 1.));
                break;
            }
            case 29: { // color randomly (by pointer hash)
                std::hash<const GNEVehicle*> ptr_hash;
                const double hue = (double)(ptr_hash(this) % 360); // [0-360]
                const double sat = ((ptr_hash(this) / 360) % 67) / 100.0 + 0.33; // [0.33-1]
                GLHelper::setColor(RGBColor::fromHSV(hue, sat, 1.));
                break;
            }
            default: {
                GLHelper::setColor(c.getScheme().getColor(0));
            }
        }
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNEVehicle::setAttribute(SumoXMLAttr key, const std::string& value) {
    // declare string error
    std::string error;
    // flag to upate stack label
    bool updateSpreadStackGeometry = false;
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            // set manually vehicle ID (needed for saving)
            id = value;
            // Change IDs of all person plans children (stops, embedded routes...)
            for (const auto& childDemandElement : getChildDemandElements()) {
                childDemandElement->setMicrosimID(getID());
            }
            break;
        case SUMO_ATTR_TYPE:
            replaceDemandElementParent(SUMO_TAG_VTYPE, value, 0);
            // set manually vtypeID (needed for saving)
            vtypeid = value;
            break;
        case SUMO_ATTR_COLOR:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                color = parse<RGBColor>(value);
                // mark parameter as set
                parametersSet |= VEHPARS_COLOR_SET;
            } else {
                // set default value
                color = parse<RGBColor>(myTagProperty.getDefaultValue(key));
                // unset parameter
                parametersSet &= ~VEHPARS_COLOR_SET;
            }
            break;
        case SUMO_ATTR_DEPARTLANE:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseDepartLane(value, toString(SUMO_TAG_VEHICLE), id, departLane, departLaneProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_DEPARTLANE_SET;
            } else {
                // set default value
                parseDepartLane(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, departLane, departLaneProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_DEPARTLANE_SET;
            }
            break;
        case SUMO_ATTR_DEPARTPOS:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseDepartPos(value, toString(SUMO_TAG_VEHICLE), id, departPos, departPosProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_DEPARTPOS_SET;
            } else {
                // set default value
                parseDepartPos(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, departPos, departPosProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_DEPARTPOS_SET;
            }
            updateGeometry();
            updateSpreadStackGeometry = true;
            break;
        case SUMO_ATTR_DEPARTSPEED:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseDepartSpeed(value, toString(SUMO_TAG_VEHICLE), id, departSpeed, departSpeedProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_DEPARTSPEED_SET;
            } else {
                // set default value
                parseDepartSpeed(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, departSpeed, departSpeedProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_DEPARTSPEED_SET;
            }
            break;
        case SUMO_ATTR_ARRIVALLANE:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseArrivalLane(value, toString(SUMO_TAG_VEHICLE), id, arrivalLane, arrivalLaneProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_ARRIVALLANE_SET;
            } else {
                // set default value
                parseArrivalLane(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, arrivalLane, arrivalLaneProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_ARRIVALLANE_SET;
            }
            break;
        case SUMO_ATTR_ARRIVALPOS:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseArrivalPos(value, toString(SUMO_TAG_VEHICLE), id, arrivalPos, arrivalPosProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_ARRIVALPOS_SET;
            } else {
                // set default value
                parseArrivalPos(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, arrivalPos, arrivalPosProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_ARRIVALPOS_SET;
            }
            updateGeometry();
            updateSpreadStackGeometry = true;
            break;
        case SUMO_ATTR_ARRIVALSPEED:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseArrivalSpeed(value, toString(SUMO_TAG_VEHICLE), id, arrivalSpeed, arrivalSpeedProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_ARRIVALSPEED_SET;
            } else {
                // set default value
                parseArrivalSpeed(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, arrivalSpeed, arrivalSpeedProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_ARRIVALSPEED_SET;
            }
            break;
        case SUMO_ATTR_LINE:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                line = value;
                // mark parameter as set
                parametersSet |= VEHPARS_LINE_SET;
            } else {
                // set default value
                line = myTagProperty.getDefaultValue(key);
                // unset parameter
                parametersSet &= ~VEHPARS_LINE_SET;
            }
            break;
        case SUMO_ATTR_PERSON_NUMBER:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                personNumber = parse<int>(value);
                // mark parameter as set
                parametersSet |= VEHPARS_PERSON_NUMBER_SET;
            } else {
                // set default value
                personNumber = parse<int>(myTagProperty.getDefaultValue(key));
                // unset parameter
                parametersSet &= ~VEHPARS_PERSON_NUMBER_SET;
            }
            break;
        case SUMO_ATTR_CONTAINER_NUMBER:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                containerNumber = parse<int>(value);
                // mark parameter as set
                parametersSet |= VEHPARS_CONTAINER_NUMBER_SET;
            } else {
                // set default value
                containerNumber = parse<int>(myTagProperty.getDefaultValue(key));
                // unset parameter
                parametersSet &= ~VEHPARS_CONTAINER_NUMBER_SET;
            }
            break;
        case SUMO_ATTR_REROUTE:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                // mark parameter as set
                parametersSet |= VEHPARS_ROUTE_SET;
            } else {
                // unset parameter
                parametersSet &= ~VEHPARS_ROUTE_SET;
            }
            break;
        case SUMO_ATTR_DEPARTPOS_LAT:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseDepartPosLat(value, toString(SUMO_TAG_VEHICLE), id, departPosLat, departPosLatProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_DEPARTPOSLAT_SET;
            } else {
                // set default value
                parseDepartPosLat(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, departPosLat, departPosLatProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_DEPARTPOSLAT_SET;
            }
            break;
        case SUMO_ATTR_ARRIVALPOS_LAT:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseArrivalPosLat(value, toString(SUMO_TAG_VEHICLE), id, arrivalPosLat, arrivalPosLatProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_ARRIVALPOSLAT_SET;
            } else {
                // set default value
                parseArrivalPosLat(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, arrivalPosLat, arrivalPosLatProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_ARRIVALPOSLAT_SET;
            }
            parseArrivalPosLat(value, toString(SUMO_TAG_VEHICLE), id, arrivalPosLat, arrivalPosLatProcedure, error);
            break;
        // Specific of vehicles
        case SUMO_ATTR_DEPART: {
            parseDepart(value, toString(SUMO_TAG_VEHICLE), id, depart, departProcedure, error);
            break;
        }
        case SUMO_ATTR_ROUTE:
            if (getParentDemandElements().size() == 2) {
                replaceDemandElementParent(SUMO_TAG_ROUTE, value, 1);
            }
            updateGeometry();
            updateSpreadStackGeometry = true;
            break;
        // Specific of Trips and flow
        case SUMO_ATTR_FROM: {
            // change first edge
            replaceFirstParentEdge(value);
            // compute vehicle
            computePath();
            updateSpreadStackGeometry = true;
            break;
        }
        case SUMO_ATTR_TO: {
            // change last edge
            replaceLastParentEdge(value);
            // compute vehicle
            computePath();
            updateSpreadStackGeometry = true;
            break;
        }
        case SUMO_ATTR_VIA: {
            if (!value.empty()) {
                // set new via edges
                via = parse< std::vector<std::string> >(value);
                // mark parameter as set
                parametersSet |= VEHPARS_VIA_SET;
            } else {
                // clear via
                via.clear();
                // unset parameter
                parametersSet &= ~VEHPARS_VIA_SET;
            }
            // update via
            replaceMiddleParentEdges(value, true);
            // compute vehicle
            computePath();
            updateSpreadStackGeometry = true;
            break;
        }
        // Specific of routeFlows
        case SUMO_ATTR_BEGIN: {
            depart = string2time(value);
            break;
        }
        case SUMO_ATTR_END:
            repetitionEnd = string2time(value);
            break;
        case SUMO_ATTR_VEHSPERHOUR:
            repetitionOffset = TIME2STEPS(3600 / parse<double>(value));
            break;
        case SUMO_ATTR_PERIOD:
            repetitionOffset = string2time(value);
            break;
        case SUMO_ATTR_PROB:
            repetitionProbability = parse<double>(value);
            break;
        case SUMO_ATTR_NUMBER:
            repetitionNumber = parse<int>(value);
            break;
        //
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
    // check if stack label has to be updated
    if (updateSpreadStackGeometry) {
        if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
            getParentEdges().front()->updateVehicleStackLabels();
            getParentEdges().front()->updateVehicleSpreadGeometries();
        } else if (getParentDemandElements().size() == 2) {
            getParentDemandElements().at(1)->getParentEdges().front()->updateVehicleStackLabels();
            getParentDemandElements().at(1)->getParentEdges().front()->updateVehicleSpreadGeometries();
        } else if (getChildDemandElements().size() > 0) {
            getChildDemandElements().front()->getParentEdges().front()->updateVehicleStackLabels();
            getChildDemandElements().front()->getParentEdges().front()->updateVehicleSpreadGeometries();
        }
    }
}


void
GNEVehicle::setEnabledAttribute(const int enabledAttributes) {
    parametersSet = enabledAttributes;
}


void
GNEVehicle::drawStackLabel(const GUIVisualizationSettings& /*s*/, const Position& vehiclePosition, const double vehicleRotation, const double width, const double length) const {
    // declare contour width
    const double contourWidth = 0.05;
    // Push matrix
    glPushMatrix();
    // Traslate to vehicle top
    glTranslated(vehiclePosition.x(), vehiclePosition.y(), GLO_ROUTE + getType() + 0.1 + GLO_PERSONFLOW);
    glRotated(vehicleRotation, 0, 0, -1);
    glTranslated((width / 2.0) + 0.35, 0, 0);
    // draw external box
    GLHelper::setColor(RGBColor::GREY);
    GLHelper::drawBoxLine(Position(), Position(), 0, length, 0.3);
    // draw internal box
    glTranslated(0, 0, 0.1);
    GLHelper::setColor(RGBColor(0, 128, 0));
    GLHelper::drawBoxLine(Position(0, -contourWidth), Position(0, -contourWidth), 0, length - (contourWidth * 2), 0.3 - contourWidth);
    // draw stack label
    GLHelper::drawText("vehicles stacked: " + toString(myStackedLabelNumber), Position(0, length / -2.0), .1, 0.6, RGBColor::WHITE, 90, 0, -1);
    // pop draw matrix
    glPopMatrix();
}


void
GNEVehicle::drawFlowLabel(const GUIVisualizationSettings& /*s*/, const Position& vehiclePosition, const double vehicleRotation, const double width, const double length) const {
    // declare contour width
    const double contourWidth = 0.05;
    // Push matrix
    glPushMatrix();
    // Traslate to vehicle bot
    glTranslated(vehiclePosition.x(), vehiclePosition.y(), GLO_ROUTE + getType() + 0.1 + GLO_PERSONFLOW);
    glRotated(vehicleRotation, 0, 0, -1);
    glTranslated(-1 * ((width / 2.0) + 0.35), 0, 0);
    // draw external box
    GLHelper::setColor(RGBColor::GREY);
    GLHelper::drawBoxLine(Position(), Position(), 0, length, 0.3);
    // draw internal box
    glTranslated(0, 0, 0.1);
    GLHelper::setColor(RGBColor::CYAN);
    GLHelper::drawBoxLine(Position(0, -contourWidth), Position(0, -contourWidth), 0, length - (contourWidth * 2), 0.3 - contourWidth);
    // draw stack label
    GLHelper::drawText("Flow", Position(0, length / -2.0), .1, 0.6, RGBColor::BLACK, 90, 0, -1);
    // pop draw matrix
    glPopMatrix();
}


/****************************************************************************/
