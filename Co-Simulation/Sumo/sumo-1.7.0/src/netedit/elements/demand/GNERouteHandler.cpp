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
/// @file    GNERouteHandler.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
///
// Builds demand objects for netedit
/****************************************************************************/
#include <config.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_DemandElement.h>

#include "GNEPerson.h"
#include "GNEPersonTrip.h"
#include "GNERide.h"
#include "GNERoute.h"
#include "GNERouteHandler.h"
#include "GNEStop.h"
#include "GNEPersonStop.h"
#include "GNEVehicle.h"
#include "GNEVehicleType.h"
#include "GNEWalk.h"


// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNERouteHandler::RouteParameter - methods
// ---------------------------------------------------------------------------

GNERouteHandler::RouteParameter::RouteParameter() :
    loadedID(false),
    vClass(SVC_PASSENGER),
    color(RGBColor::BLACK) {
}


GNERouteHandler::RouteParameter::RouteParameter(GNEDemandElement* originalDemandElement) :
    routeID(originalDemandElement->getTagProperty().isRoute() ?
            originalDemandElement->getID() :
            originalDemandElement->getNet()->generateDemandElementID(SUMO_TAG_ROUTE)),
    loadedID(false),
    edges(originalDemandElement->getParentEdges()),
    vClass(originalDemandElement->getVClass()),
    color(originalDemandElement->getColor()) {
}


void
GNERouteHandler::RouteParameter::setEdges(GNENet* net, const std::string& edgeIDs) {
    // clear edges
    edges.clear();
    // obtain edges (And show warnings if isn't valid)
    if (GNEAttributeCarrier::canParse<std::vector<GNEEdge*> >(net, edgeIDs, true)) {
        edges = GNEAttributeCarrier::parse<std::vector<GNEEdge*> >(net, edgeIDs);
    }
}


void
GNERouteHandler::RouteParameter::setEdges(GNENet* net, const std::string& vehicleID, const std::string& fromID, const std::string& toID, const std::string& viaIDs) {
    // clear edges
    edges.clear();
    // only continue if at least one of the edges is defined
    if (fromID.size() + toID.size() > 0) {
        // obtain from and to edges
        GNEEdge* from = net->retrieveEdge(fromID, false);
        GNEEdge* to = net->retrieveEdge(toID, false);
        // check if edges are valid
        if (from == nullptr) {
            WRITE_ERROR("Invalid from-edge '" + fromID + "' used in trip '" + vehicleID + "'.");
        } else if (to == nullptr) {
            WRITE_ERROR("Invalid to-edge '" + toID + "' used in trip '" + vehicleID + "'.");
        } else if (!GNEAttributeCarrier::canParse<std::vector<GNEEdge*> >(net, viaIDs, false)) {
            WRITE_ERROR("Invalid 'via' edges used in trip '" + vehicleID + "'.");
        } else {
            // obtain via
            std::vector<GNEEdge*> viaEdges = GNEAttributeCarrier::parse<std::vector<GNEEdge*> >(net, viaIDs);
            // build edges (from - via - to)
            edges.push_back(from);
            for (const auto& i : viaEdges) {
                edges.push_back(i);
            }
            // check that from and to edge are different
            if (from != to) {
                edges.push_back(to);
            }
        }
    }
}


void
GNERouteHandler::RouteParameter::clearEdges() {
    edges.clear();
}

// ---------------------------------------------------------------------------
// GNERouteHandler - methods
// ---------------------------------------------------------------------------

GNERouteHandler::GNERouteHandler(const std::string& file, GNENet* net, bool undoDemandElements) :
    SUMORouteHandler(file, "", false),
    myNet(net),
    myUndoDemandElements(undoDemandElements),
    myLoadedVehicleWithEmbebbedRoute(nullptr),
    myAbort(false) {
}


GNERouteHandler::~GNERouteHandler() {}


bool
GNERouteHandler::isVehicleIdDuplicated(GNENet* net, const std::string& id) {
    // declare vehicle tags vector
    std::vector<SumoXMLTag> vehicleTags = {SUMO_TAG_VEHICLE, GNE_TAG_VEHICLE_WITHROUTE, SUMO_TAG_TRIP, GNE_TAG_FLOW_ROUTE, GNE_TAG_FLOW_WITHROUTE, SUMO_TAG_FLOW};
    for (const auto& vehicleTag : vehicleTags) {
        if (net->retrieveDemandElement(vehicleTag, id, false) != nullptr) {
            WRITE_ERROR("There is another " + toString(vehicleTag) + " with the same ID='" + id + "'.");
            return true;
        }
    }
    return false;
}


bool
GNERouteHandler::isPersonIdDuplicated(GNENet* net, const std::string& id) {
    for (SumoXMLTag personTag : std::vector<SumoXMLTag>({SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW})) {
        if (net->retrieveDemandElement(personTag, id, false) != nullptr) {
            WRITE_ERROR("There is another " + toString(personTag) + " with the same ID='" + id + "'.");
            return true;
        }
    }
    return false;
}


void
GNERouteHandler::buildRoute(GNENet* net, bool undoDemandElements, const RouteParameter& routeParameters, const std::vector<SUMOVehicleParameter::Stop>& activeStops) {
    // create GNERoute
    GNEDemandElement* route = new GNERoute(net, routeParameters);
    if (undoDemandElements) {
        net->getViewNet()->getUndoList()->p_begin("add " + route->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(route, true), true);
        // iterate over stops of myActiveRouteStops and create stops associated with it
        for (const auto& i : activeStops) {
            buildStop(net, true, i, route);
        }
        net->getViewNet()->getUndoList()->p_end();
    } else {
        net->getAttributeCarriers()->insertDemandElement(route);
        for (const auto& i : routeParameters.edges) {
            i->addChildElement(route);
        }
        route->incRef("buildRoute");
        // iterate over stops of myActiveRouteStops and create stops associated with it
        for (const auto& i : activeStops) {
            buildStop(net, false, i, route);
        }
    }
}

void
GNERouteHandler::buildVehicleOverRoute(GNENet* net, bool undoDemandElements, const SUMOVehicleParameter& vehicleParameters) {
    // first check if ID is duplicated
    if (!isVehicleIdDuplicated(net, vehicleParameters.id)) {
        // obtain routes and vtypes
        GNEDemandElement* vType = net->retrieveDemandElement(SUMO_TAG_VTYPE, vehicleParameters.vtypeid, false);
        GNEDemandElement* route = net->retrieveDemandElement(SUMO_TAG_ROUTE, vehicleParameters.routeid, false);
        if (vType == nullptr) {
            WRITE_ERROR("Invalid vehicle type '" + vehicleParameters.vtypeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (route == nullptr) {
            WRITE_ERROR("Invalid route '" + vehicleParameters.routeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTLANE_SET) && (vehicleParameters.departLaneProcedure == DepartLaneDefinition::GIVEN) && ((int)route->getParentEdges().front()->getLanes().size() < vehicleParameters.departLane)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTLANE) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departLane) + " is greater than number of lanes");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTSPEED_SET) && (vehicleParameters.departSpeedProcedure == DepartSpeedDefinition::GIVEN) && (vType->getAttributeDouble(SUMO_ATTR_MAXSPEED) < vehicleParameters.departSpeed)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTSPEED) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departSpeed) + " is greater than vType" + toString(SUMO_ATTR_MAXSPEED));
        } else {
            // create vehicle using vehicleParameters
            GNEDemandElement* vehicle = new GNEVehicle(net, vType, route, vehicleParameters);
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + vehicle->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(vehicle, true), true);
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, true, i, vehicle);
                }
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(vehicle);
                // set vehicle as child of vType and Route
                vType->addChildElement(vehicle);
                route->addChildElement(vehicle);
                vehicle->incRef("buildVehicleOverRoute");
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, false, i, vehicle);
                }
            }
        }
    }
}


void
GNERouteHandler::buildFlowOverRoute(GNENet* net, bool undoDemandElements, const SUMOVehicleParameter& vehicleParameters) {
    // first check if ID is duplicated
    if (!isVehicleIdDuplicated(net, vehicleParameters.id)) {
        // obtain routes and vtypes
        GNEDemandElement* vType = net->retrieveDemandElement(SUMO_TAG_VTYPE, vehicleParameters.vtypeid, false);
        GNEDemandElement* route = net->retrieveDemandElement(SUMO_TAG_ROUTE, vehicleParameters.routeid, false);
        if (vType == nullptr) {
            WRITE_ERROR("Invalid vehicle type '" + vehicleParameters.vtypeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (route == nullptr) {
            WRITE_ERROR("Invalid route '" + vehicleParameters.routeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTLANE_SET) && (vehicleParameters.departLaneProcedure == DepartLaneDefinition::GIVEN) && ((int)route->getParentEdges().front()->getLanes().size() < vehicleParameters.departLane)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTLANE) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departLane) + " is greater than number of lanes");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTSPEED_SET) && (vehicleParameters.departSpeedProcedure == DepartSpeedDefinition::GIVEN) && (vType->getAttributeDouble(SUMO_ATTR_MAXSPEED) < vehicleParameters.departSpeed)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTSPEED) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departSpeed) + " is greater than vType" + toString(SUMO_ATTR_MAXSPEED));
        } else {
            // create flow or trips using vehicleParameters
            GNEDemandElement* flow = new GNEVehicle(net, vType, route, vehicleParameters);
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + flow->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(flow, true), true);
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, true, i, flow);
                }
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(flow);
                // set flow as child of vType and Route
                vType->addChildElement(flow);
                route->addChildElement(flow);
                flow->incRef("buildFlowOverRoute");
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, false, i, flow);
                }
            }
        }
    }
}


void
GNERouteHandler::buildVehicleEmbeddedRoute(GNENet* net, bool undoDemandElements, SUMOVehicleParameter vehicleParameters, const std::vector<GNEEdge*>& edges) {
    // first check if ID is duplicated
    if (!isVehicleIdDuplicated(net, vehicleParameters.id) && (edges.size() > 0)) {
        // obtain vType
        GNEDemandElement* vType = net->retrieveDemandElement(SUMO_TAG_VTYPE, vehicleParameters.vtypeid, false);
        if (vType == nullptr) {
            WRITE_ERROR("Invalid vehicle type '" + vehicleParameters.vtypeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTLANE_SET) && (vehicleParameters.departLaneProcedure == DepartLaneDefinition::GIVEN) && ((int)edges.front()->getLanes().size() < vehicleParameters.departLane)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTLANE) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departLane) + " is greater than number of lanes");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTSPEED_SET) && (vehicleParameters.departSpeedProcedure == DepartSpeedDefinition::GIVEN) && (vType->getAttributeDouble(SUMO_ATTR_MAXSPEED) < vehicleParameters.departSpeed)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTSPEED) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departSpeed) + " is greater than vType" + toString(SUMO_ATTR_MAXSPEED));
        } else {
            // declare route parameters and fill it
            GNERouteHandler::RouteParameter embeddedRouteParameters;
            // use the same vehicle ID
            embeddedRouteParameters.routeID = vehicleParameters.id;
            // set edges
            embeddedRouteParameters.edges = edges;
            // set color
            embeddedRouteParameters.color = RGBColor::CYAN;
            // due vehicle was loaded without a route, change tag
            vehicleParameters.tag = GNE_TAG_VEHICLE_WITHROUTE;
            // create vehicle or trips using myTemporalVehicleParameter without a route
            GNEDemandElement* vehicle = new GNEVehicle(net, vType, vehicleParameters);
            // creaste embedded route
            GNEDemandElement* embeddedRoute = new GNERoute(net, vehicle, embeddedRouteParameters);
            // add both to net depending of myUndoDemandElements
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add vehicle and " + embeddedRoute->getTagStr());
                // add both in net using undoList
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(vehicle, true), true);
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(embeddedRoute, true), true);
                net->getViewNet()->getUndoList()->p_end();
            } else {
                // add vehicleOrRouteFlow in net and in their vehicle type parent
                net->getAttributeCarriers()->insertDemandElement(vehicle);
                // set vehicle as child of vType
                vType->addChildElement(vehicle);
                vehicle->incRef("buildVehicleWithEmbeddedRoute");
                // add route manually in net, and in all of their edges and in vehicleOrRouteFlow
                net->getAttributeCarriers()->insertDemandElement(embeddedRoute);
                for (const auto& edge : edges) {
                    edge->addChildElement(vehicle);
                }
                // set route as child of vehicle
                vehicle->addChildElement(embeddedRoute);
                embeddedRoute->incRef("buildVehicleWithEmbeddedRoute");
            }
        }
    }
}


void
GNERouteHandler::buildFlowEmbeddedRoute(GNENet* net, bool undoDemandElements, SUMOVehicleParameter vehicleParameters, const std::vector<GNEEdge*>& edges) {
    // first check if ID is duplicated
    if (!isVehicleIdDuplicated(net, vehicleParameters.id) && edges.size() > 0) {
        // obtain vType
        GNEDemandElement* vType = net->retrieveDemandElement(SUMO_TAG_VTYPE, vehicleParameters.vtypeid, false);
        if (vType == nullptr) {
            WRITE_ERROR("Invalid vehicle type '" + vehicleParameters.vtypeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTLANE_SET) && (vehicleParameters.departLaneProcedure == DepartLaneDefinition::GIVEN) && ((int)edges.front()->getLanes().size() < vehicleParameters.departLane)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTLANE) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departLane) + " is greater than number of lanes");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTSPEED_SET) && (vehicleParameters.departSpeedProcedure == DepartSpeedDefinition::GIVEN) && (vType->getAttributeDouble(SUMO_ATTR_MAXSPEED) < vehicleParameters.departSpeed)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTSPEED) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departSpeed) + " is greater than vType" + toString(SUMO_ATTR_MAXSPEED));
        } else {
            // declare route parameters and fill it
            GNERouteHandler::RouteParameter embeddedRouteParameters;
            // use the same vehicle ID
            embeddedRouteParameters.routeID = vehicleParameters.id;
            // set edges
            embeddedRouteParameters.edges = edges;
            // set color
            embeddedRouteParameters.color = RGBColor::CYAN;
            // due vehicle was loaded without a route, change tag
            vehicleParameters.tag = GNE_TAG_FLOW_WITHROUTE;
            // create vehicle or trips using myTemporalVehicleParameter without a route
            GNEDemandElement* flow = new GNEVehicle(net, vType, vehicleParameters);
            // creaste embedded route
            GNEDemandElement* embeddedRoute = new GNERoute(net, flow, embeddedRouteParameters);
            // add both to net depending of myUndoDemandElements
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add vehicle and " + embeddedRoute->getTagStr());
                // add both in net using undoList
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(flow, true), true);
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(embeddedRoute, true), true);
                net->getViewNet()->getUndoList()->p_end();
            } else {
                // add vehicleOrRouteFlow in net and in their vehicle type parent
                net->getAttributeCarriers()->insertDemandElement(flow);
                // set vehicle as child of vType
                vType->addChildElement(flow);
                flow->incRef("buildFlowWithEmbeddedRoute");
                // add route manually in net, and in all of their edges and in vehicleOrRouteFlow
                net->getAttributeCarriers()->insertDemandElement(embeddedRoute);
                for (const auto& edge : edges) {
                    edge->addChildElement(flow);
                }
                // set route as child of flow
                flow->addChildElement(embeddedRoute);
                embeddedRoute->incRef("buildFlowWithEmbeddedRoute");
            }
        }
    }
}


void
GNERouteHandler::buildTrip(GNENet* net, bool undoDemandElements, const SUMOVehicleParameter& vehicleParameters, GNEEdge* fromEdge, GNEEdge* toEdge, const std::vector<GNEEdge*>& via) {
    // check if exist another vehicle with the same ID (note: Vehicles, Flows and Trips share namespace)
    if (!isVehicleIdDuplicated(net, vehicleParameters.id)) {
        // obtain  vtypes
        GNEDemandElement* vType = net->retrieveDemandElement(SUMO_TAG_VTYPE, vehicleParameters.vtypeid, false);
        if (vType == nullptr) {
            WRITE_ERROR("Invalid vehicle type '" + vehicleParameters.vtypeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTLANE_SET) && ((vehicleParameters.departLaneProcedure == DepartLaneDefinition::GIVEN)) && ((int)fromEdge->getLanes().size() < vehicleParameters.departLane)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTLANE) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departLane) + " is greater than number of lanes");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTSPEED_SET) && (vehicleParameters.departSpeedProcedure == DepartSpeedDefinition::GIVEN) && (vType->getAttributeDouble(SUMO_ATTR_MAXSPEED) < vehicleParameters.departSpeed)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTSPEED) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departSpeed) + " is greater than vType" + toString(SUMO_ATTR_MAXSPEED));
        } else {
            // add "via" edges in vehicleParameters
            for (const auto& viaEdge : via) {
                vehicleParameters.via.push_back(viaEdge->getID());
            }
            // create trip or flow using tripParameters
            GNEDemandElement* trip = new GNEVehicle(net, vType, fromEdge, toEdge, via, vehicleParameters);
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + trip->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(trip, true), true);
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, true, i, trip);
                }
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(trip);
                // set vehicle as child of vType
                vType->addChildElement(trip);
                trip->incRef("buildTrip");
                // add reference in all edges
                fromEdge->addChildElement(trip);
                toEdge->addChildElement(trip);
                for (const auto& viaEdge : via) {
                    viaEdge->addChildElement(trip);
                }
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, false, i, trip);
                }
            }
        }
    }
}


void
GNERouteHandler::buildFlow(GNENet* net, bool undoDemandElements, const SUMOVehicleParameter& vehicleParameters, GNEEdge* fromEdge, GNEEdge* toEdge, const std::vector<GNEEdge*>& via) {
    // Check tag
    assert(vehicleParameters.tag == SUMO_TAG_FLOW);
    // check if exist another vehicle with the same ID (note: Vehicles, Flows and Trips share namespace)
    if (!isVehicleIdDuplicated(net, vehicleParameters.id)) {
        // obtain  vtypes
        GNEDemandElement* vType = net->retrieveDemandElement(SUMO_TAG_VTYPE, vehicleParameters.vtypeid, false);
        if (vType == nullptr) {
            WRITE_ERROR("Invalid vehicle type '" + vehicleParameters.vtypeid + "' used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'.");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTLANE_SET) && (vehicleParameters.departLaneProcedure == DepartLaneDefinition::GIVEN) && ((int)fromEdge->getLanes().size() < vehicleParameters.departLane)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTLANE) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departLane) + " is greater than number of lanes");
        } else if (vehicleParameters.wasSet(VEHPARS_DEPARTSPEED_SET) && (vehicleParameters.departSpeedProcedure == DepartSpeedDefinition::GIVEN) && (vType->getAttributeDouble(SUMO_ATTR_MAXSPEED) < vehicleParameters.departSpeed)) {
            WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTSPEED) + " used in " + toString(vehicleParameters.tag) + " '" + vehicleParameters.id + "'. " + toString(vehicleParameters.departSpeed) + " is greater than vType" + toString(SUMO_ATTR_MAXSPEED));
        } else {
            // add "via" edges in vehicleParameters
            for (const auto& viaEdge : via) {
                vehicleParameters.via.push_back(viaEdge->getID());
            }
            // create trip or flow using tripParameters
            GNEDemandElement* flow = new GNEVehicle(net, vType, fromEdge, toEdge, via, vehicleParameters);
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + flow->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(flow, true), true);
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, true, i, flow);
                }
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(flow);
                // set vehicle as child of vType
                vType->addChildElement(flow);
                flow->incRef("buildFlow");
                // add reference in all edges
                fromEdge->addChildElement(flow);
                toEdge->addChildElement(flow);
                for (const auto& viaEdge : via) {
                    viaEdge->addChildElement(flow);
                }
                // iterate over stops of vehicleParameters and create stops associated with it
                for (const auto& i : vehicleParameters.stops) {
                    buildStop(net, false, i, flow);
                }
            }
        }
    }
}


void
GNERouteHandler::buildStop(GNENet* net, bool undoDemandElements, const SUMOVehicleParameter::Stop& stopParameters, GNEDemandElement* stopParent) {
    // declare pointers to parent elements
    GNEAdditional* stoppingPlace = nullptr;
    GNELane* lane = nullptr;
    // GNEEdge* edge = nullptr;
    SumoXMLTag stopTagType = SUMO_TAG_NOTHING;
    bool validParentDemandElement = true;
    if (stopParameters.busstop.size() > 0) {
        stoppingPlace = net->retrieveAdditional(SUMO_TAG_BUS_STOP, stopParameters.busstop, false);
        // distinguish between stop for vehicles and stops for persons
        if (stopParent->getTagProperty().isPerson()) {
            stopTagType = GNE_TAG_PERSONSTOP_BUSSTOP;
        } else {
            stopTagType = SUMO_TAG_STOP_BUSSTOP;
        }
    } else if (stopParameters.containerstop.size() > 0) {
        stoppingPlace = net->retrieveAdditional(SUMO_TAG_CONTAINER_STOP, stopParameters.containerstop, false);
        // distinguish between stop for vehicles and stops for persons
        if (stopParent->getTagProperty().isPerson()) {
            WRITE_ERROR("Persons don't support stops at container stops");
            validParentDemandElement = false;
        } else {
            stopTagType = SUMO_TAG_STOP_CONTAINERSTOP;
        }
    } else if (stopParameters.chargingStation.size() > 0) {
        stoppingPlace = net->retrieveAdditional(SUMO_TAG_CHARGING_STATION, stopParameters.chargingStation, false);
        // distinguish between stop for vehicles and stops for persons
        if (stopParent->getTagProperty().isPerson()) {
            WRITE_ERROR("Persons don't support stops at charging stations");
            validParentDemandElement = false;
        } else {
            stopTagType = SUMO_TAG_STOP_CHARGINGSTATION;
        }
    } else if (stopParameters.parkingarea.size() > 0) {
        stoppingPlace = net->retrieveAdditional(SUMO_TAG_PARKING_AREA, stopParameters.parkingarea, false);
        // distinguish between stop for vehicles and stops for persons
        if (stopParent->getTagProperty().isPerson()) {
            WRITE_ERROR("Persons don't support stops at parking areas");
            validParentDemandElement = false;
        } else {
            stopTagType = SUMO_TAG_STOP_PARKINGAREA;
        }
    } else if (stopParameters.lane.size() > 0) {
        lane = net->retrieveLane(stopParameters.lane, false);
        stopTagType = SUMO_TAG_STOP_LANE;
    } else if (stopParameters.edge.size() > 0) {
        // edge = net->retrieveEdge(stopParameters.lane, false);
        stopTagType = GNE_TAG_PERSONSTOP_EDGE;
    }
    // first check that parent is valid
    if (validParentDemandElement) {
        // check if values are correct
        if (stoppingPlace && lane) {
            WRITE_ERROR("A stop must be defined either over a stoppingPlace or over a lane");
        } else if (!stoppingPlace && !lane) {
            WRITE_ERROR("A stop requires a stoppingPlace or a lane");
        } else if (stoppingPlace) {
            // create stop using stopParameters and stoppingPlace
            GNEDemandElement* stop = new GNEStop(stopTagType, net, stopParameters, stoppingPlace, stopParent);
            // add it depending of undoDemandElements
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + stop->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(stop, true), true);
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(stop);
                stoppingPlace->addChildElement(stop);
                stopParent->addChildElement(stop);
                stop->incRef("buildStoppingPlaceStop");
            }
        } else {
            // create stop using stopParameters and lane
            GNEDemandElement* stop = new GNEStop(net, stopParameters, lane, stopParent);
            // add it depending of undoDemandElements
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + stop->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(stop, true), true);
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(stop);
                lane->addChildElement(stop);
                stopParent->addChildElement(stop);
                stop->incRef("buildLaneStop");
            }
        }
    }
}


void
GNERouteHandler::buildPerson(GNENet* net, bool undoDemandElements, const SUMOVehicleParameter& personParameters) {
    // first check if ID is duplicated
    if (!isPersonIdDuplicated(net, personParameters.id)) {
        // obtain routes and vtypes
        GNEDemandElement* pType = net->retrieveDemandElement(SUMO_TAG_PTYPE, personParameters.vtypeid, false);
        if (pType == nullptr) {
            WRITE_ERROR("Invalid person type '" + personParameters.vtypeid + "' used in " + toString(personParameters.tag) + " '" + personParameters.id + "'.");
        } else {
            // create person using personParameters
            GNEDemandElement* person = new GNEPerson(SUMO_TAG_PERSON, net, pType, personParameters);
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + person->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(person, true), true);
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(person);
                // set person as child of pType and Route
                pType->addChildElement(person);
                person->incRef("buildPerson");
            }
        }
    }
}


void
GNERouteHandler::buildPersonFlow(GNENet* net, bool undoDemandElements, const SUMOVehicleParameter& personFlowParameters) {
    // first check if ID is duplicated
    if (!isPersonIdDuplicated(net, personFlowParameters.id)) {
        // obtain routes and vtypes
        GNEDemandElement* pType = net->retrieveDemandElement(SUMO_TAG_PTYPE, personFlowParameters.vtypeid, false);
        if (pType == nullptr) {
            WRITE_ERROR("Invalid personFlow type '" + personFlowParameters.vtypeid + "' used in " + toString(personFlowParameters.tag) + " '" + personFlowParameters.id + "'.");
        } else {
            // create personFlow using personFlowParameters
            GNEDemandElement* personFlow = new GNEPerson(SUMO_TAG_PERSONFLOW, net, pType, personFlowParameters);
            if (undoDemandElements) {
                net->getViewNet()->getUndoList()->p_begin("add " + personFlow->getTagStr());
                net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(personFlow, true), true);
                net->getViewNet()->getUndoList()->p_end();
            } else {
                net->getAttributeCarriers()->insertDemandElement(personFlow);
                // set personFlow as child of pType and Route
                pType->addChildElement(personFlow);
                personFlow->incRef("buildPersonFlow");
            }
        }
    }
}


bool
GNERouteHandler::buildPersonPlan(SumoXMLTag tag, GNEDemandElement* personParent, GNEFrameAttributesModuls::AttributesCreator* personPlanAttributes, GNEFrameModuls::PathCreator* pathCreator) {
    // get view net
    GNEViewNet* viewNet = personParent->getNet()->getViewNet();
    // Declare map to keep attributes from myPersonPlanAttributes
    std::map<SumoXMLAttr, std::string> valuesMap = personPlanAttributes->getAttributesAndValues(true);
    // get attributes
    const std::vector<std::string> types = GNEAttributeCarrier::parse<std::vector<std::string> >(valuesMap[SUMO_ATTR_VTYPES]);
    const std::vector<std::string> modes = GNEAttributeCarrier::parse<std::vector<std::string> >(valuesMap[SUMO_ATTR_MODES]);
    const std::vector<std::string> lines = GNEAttributeCarrier::parse<std::vector<std::string> >(valuesMap[SUMO_ATTR_LINES]);
    const double arrivalPos = (valuesMap.count(SUMO_ATTR_ARRIVALPOS) > 0) ? GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_ARRIVALPOS]) : 0;
    // get stop parameters
    SUMOVehicleParameter::Stop stopParameters;
    // get edges
    GNEEdge* fromEdge = (pathCreator->getSelectedEdges().size() > 0) ? pathCreator->getSelectedEdges().front() : nullptr;
    GNEEdge* toEdge = (pathCreator->getSelectedEdges().size() > 0) ? pathCreator->getSelectedEdges().back() : nullptr;
    // get busStops
    GNEAdditional* fromBusStop = pathCreator->getFromStoppingPlace(SUMO_TAG_BUS_STOP);
    GNEAdditional* toBusStop = pathCreator->getToStoppingPlace(SUMO_TAG_BUS_STOP);
    // get path edges
    std::vector<GNEEdge*> edges;
    for (const auto& path : pathCreator->getPath()) {
        for (const auto& edge : path.getSubPath()) {
            edges.push_back(edge);
        }
    }
    // get route
    GNEDemandElement* route = pathCreator->getRoute();
    // check what PersonPlan we're creating
    switch (tag) {
        // Person Trips
        case GNE_TAG_PERSONTRIP_EDGE_EDGE: {
            // check if person trip edge->edge can be created
            if (fromEdge && toEdge) {
                buildPersonTrip(viewNet->getNet(), true, personParent, fromEdge, toEdge, nullptr, nullptr, arrivalPos, types, modes);
                return true;
            } else {
                viewNet->setStatusBarText("A person trip from edge to edge needs two edges");
            }
            break;
        }
        case GNE_TAG_PERSONTRIP_EDGE_BUSSTOP: {
            // check if person trip edge->busStop can be created
            if (fromEdge && toBusStop) {
                buildPersonTrip(viewNet->getNet(), true, personParent, fromEdge, nullptr, nullptr, toBusStop, arrivalPos, types, modes);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from edge to bussTop needs an edge and a busSTop");
            }
            break;
        }
        case GNE_TAG_PERSONTRIP_BUSSTOP_EDGE: {
            // check if person trip busStop->edge can be created
            if (fromBusStop && toEdge) {
                buildPersonTrip(viewNet->getNet(), true, personParent, nullptr, toEdge, fromBusStop, nullptr, arrivalPos, types, modes);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from busStop to edge needs a busStop and an edge");
            }
            break;
        }
        case GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP: {
            // check if person trip busStop->busStop can be created
            if (fromBusStop && toBusStop) {
                buildPersonTrip(viewNet->getNet(), true, personParent, nullptr, nullptr, fromBusStop, toBusStop, arrivalPos, types, modes);
                return true;
            } else {
                viewNet->setStatusBarText("A person trip from busStop to busStop needs two busStops");
            }
            break;
        }
        // Walks
        case GNE_TAG_WALK_EDGE_EDGE: {
            // check if walk edge->edge can be created
            if (fromEdge && toEdge) {
                buildWalk(viewNet->getNet(), true, personParent, fromEdge, toEdge, nullptr, nullptr, {}, nullptr, arrivalPos);
                return true;
            } else {
                viewNet->setStatusBarText("A walk from edge to edge needs two edges");
            }
            break;
        }
        case GNE_TAG_WALK_EDGE_BUSSTOP: {
            // check if walk edge->busStop can be created
            if (fromEdge && toBusStop) {
                buildWalk(viewNet->getNet(), true, personParent, fromEdge, nullptr, nullptr, toBusStop, {}, nullptr, arrivalPos);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from edge to bussTop needs an edge and a busSTop");
            }
            break;
        }
        case GNE_TAG_WALK_BUSSTOP_EDGE: {
            // check if walk busStop->edge can be created
            if (fromBusStop && toEdge) {
                buildWalk(viewNet->getNet(), true, personParent, nullptr, toEdge, fromBusStop, nullptr, {}, nullptr, arrivalPos);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from busStop to edge needs a busStop and an edge");
            }
            break;
        }
        case GNE_TAG_WALK_BUSSTOP_BUSSTOP: {
            // check if walk busStop->busStop can be created
            if (fromBusStop && toBusStop) {
                buildWalk(viewNet->getNet(), true, personParent, nullptr, nullptr, fromBusStop, toBusStop, {}, nullptr, arrivalPos);
                return true;
            } else {
                viewNet->setStatusBarText("A walk from busStop to busStop needs two busStops");
            }
            break;
        }
        case GNE_TAG_WALK_EDGES: {
            // check if walk edges can be created
            if (edges.size() > 0) {
                buildWalk(viewNet->getNet(), true, personParent, nullptr, nullptr, nullptr, nullptr, edges, nullptr, arrivalPos);
                return true;
            } else {
                viewNet->setStatusBarText("A walk with edges attribute needs a list of edges");
            }
            break;
        }
        case GNE_TAG_WALK_ROUTE: {
            // check if walk edges can be created
            if (route) {
                buildWalk(viewNet->getNet(), true, personParent, nullptr, nullptr, nullptr, nullptr, {}, route, arrivalPos);
                return true;
            } else {
                viewNet->setStatusBarText("A route walk needs a route");
            }
            break;
        }
        // Rides
        case GNE_TAG_RIDE_EDGE_EDGE: {
            // check if ride edge->edge can be created
            if (fromEdge && toEdge) {
                buildRide(viewNet->getNet(), true, personParent, fromEdge, toEdge, nullptr, nullptr, arrivalPos, lines);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from edge to edge needs two edges");
            }
            break;
        }
        case GNE_TAG_RIDE_EDGE_BUSSTOP: {
            // check if ride edge->busStop can be created
            if (fromEdge && toBusStop) {
                buildRide(viewNet->getNet(), true, personParent, fromEdge, nullptr, nullptr, toBusStop, arrivalPos, lines);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from edge to busStop needs an edge and a busStop");
            }
            break;
        }
        case GNE_TAG_RIDE_BUSSTOP_EDGE: {
            // check if ride busStop->edge can be created
            if (fromBusStop && toEdge) {
                buildRide(viewNet->getNet(), true, personParent, nullptr, toEdge, fromBusStop, nullptr, arrivalPos, lines);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from busStop to edge needs a busStop and an edge");
            }
            break;
        }
        case GNE_TAG_RIDE_BUSSTOP_BUSSTOP: {
            // check if ride busStop->busStop can be created
            if (fromBusStop && toBusStop) {
                buildRide(viewNet->getNet(), true, personParent, nullptr, nullptr, fromBusStop, toBusStop, arrivalPos, lines);
                return true;
            } else {
                viewNet->setStatusBarText("A ride from busStop to busStop needs two busStops");
            }
            break;
        }
        // stops
        case GNE_TAG_PERSONSTOP_EDGE: {
            // check if ride busStop->busStop can be created
            if (fromEdge) {
                buildPersonStop(viewNet->getNet(), true, personParent, fromEdge, nullptr, stopParameters);
                return true;
            } else {
                viewNet->setStatusBarText("A stop has to be placed over an edge");
            }
            break;
        }
        case GNE_TAG_PERSONSTOP_BUSSTOP: {
            // check if ride busStop->busStop can be created
            if (fromBusStop) {
                buildPersonStop(viewNet->getNet(), true, personParent, nullptr, fromBusStop, stopParameters);
                return true;
            } else {
                viewNet->setStatusBarText("A stop has to be placed over a busStop");
            }
            break;
        }
        default:
            throw InvalidArgument("Invalid person plan tag");
    }
    // person plan wasn't created, then return false
    return false;
}


void
GNERouteHandler::buildPersonTrip(GNENet* net, bool undoDemandElements, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge,
                                 GNEAdditional* busStopFrom, GNEAdditional* busStopTo, double arrivalPos, const std::vector<std::string>& types, const std::vector<std::string>& modes) {
    // declare person trip
    GNEDemandElement* personTrip = nullptr;
    // create person trip depending of parameters
    if (fromEdge && toEdge) {
        // create person trip edge->edge
        personTrip = new GNEPersonTrip(net, personParent, fromEdge, toEdge, arrivalPos, types, modes);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_PERSONTRIP_EDGE_EDGE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(personTrip, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert person trip
            net->getAttributeCarriers()->insertDemandElement(personTrip);
            // set references in children
            personParent->addChildElement(personTrip);
            fromEdge->addChildElement(personTrip);
            toEdge->addChildElement(personTrip);
            // include reference
            personTrip->incRef("buildPersonTrip");
        }
    } else if (fromEdge && busStopTo) {
        // create person trip edge->busStop
        personTrip = new GNEPersonTrip(net, personParent, fromEdge, busStopTo, arrivalPos, types, modes);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_PERSONTRIP_EDGE_BUSSTOP) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(personTrip, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert person trip
            net->getAttributeCarriers()->insertDemandElement(personTrip);
            // set references in children
            personParent->addChildElement(personTrip);
            fromEdge->addChildElement(personTrip);
            busStopTo->addChildElement(personTrip);
            // include reference
            personTrip->incRef("buildPersonTrip");
        }
    } else if (busStopFrom && toEdge) {
        // create person trip busStop->edge
        personTrip = new GNEPersonTrip(net, personParent, busStopFrom, toEdge, arrivalPos, types, modes);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_PERSONTRIP_BUSSTOP_EDGE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(personTrip, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert person trip
            net->getAttributeCarriers()->insertDemandElement(personTrip);
            // set references in children
            personParent->addChildElement(personTrip);
            busStopFrom->addChildElement(personTrip);
            toEdge->addChildElement(personTrip);
            // include reference
            personTrip->incRef("buildPersonTrip");
        }
    } else if (busStopFrom && busStopTo) {
        // create person trip busStop->busStop
        personTrip = new GNEPersonTrip(net, personParent, busStopFrom, busStopTo, arrivalPos, types, modes);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(personTrip, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert person trip
            net->getAttributeCarriers()->insertDemandElement(personTrip);
            // set references in children
            personParent->addChildElement(personTrip);
            busStopFrom->addChildElement(personTrip);
            busStopTo->addChildElement(personTrip);
            // include reference
            personTrip->incRef("buildPersonTrip");
        }
    }
    // update geometry
    personParent->updateGeometry();
}


void
GNERouteHandler::buildWalk(GNENet* net, bool undoDemandElements, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge,
                           GNEAdditional* busStopFrom, GNEAdditional* busStopTo, const std::vector<GNEEdge*>& edges, GNEDemandElement* route, double arrivalPos) {
    // declare walk
    GNEDemandElement* walk = nullptr;
    // create walk depending of parameters
    if (fromEdge && toEdge) {
        // create walk edge->edge
        walk = new GNEWalk(net, personParent, fromEdge, toEdge, arrivalPos);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_WALK_EDGE_EDGE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(walk, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert walk
            net->getAttributeCarriers()->insertDemandElement(walk);
            // set references in children
            personParent->addChildElement(walk);
            fromEdge->addChildElement(walk);
            toEdge->addChildElement(walk);
            // include reference
            walk->incRef("buildWalk");
        }
    } else if (fromEdge && busStopTo) {
        // create walk edge->busStop
        walk = new GNEWalk(net, personParent, fromEdge, busStopTo, arrivalPos);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_WALK_EDGE_BUSSTOP) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(walk, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert walk
            net->getAttributeCarriers()->insertDemandElement(walk);
            // set references in children
            personParent->addChildElement(walk);
            fromEdge->addChildElement(walk);
            busStopTo->addChildElement(walk);
            // include reference
            walk->incRef("buildWalk");
        }
    } else if (busStopFrom && toEdge) {
        // create walk busStop->edge
        walk = new GNEWalk(net, personParent, busStopFrom, toEdge, arrivalPos);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_WALK_BUSSTOP_EDGE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(walk, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert walk
            net->getAttributeCarriers()->insertDemandElement(walk);
            // set references in children
            personParent->addChildElement(walk);
            busStopFrom->addChildElement(walk);
            toEdge->addChildElement(walk);
            // include reference
            walk->incRef("buildWalk");
        }
    } else if (busStopFrom && busStopTo) {
        // create walk busStop->busStop
        walk = new GNEWalk(net, personParent, busStopFrom, busStopTo, arrivalPos);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_WALK_BUSSTOP_BUSSTOP) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(walk, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert walk
            net->getAttributeCarriers()->insertDemandElement(walk);
            // set references in children
            personParent->addChildElement(walk);
            busStopFrom->addChildElement(walk);
            busStopTo->addChildElement(walk);
            // include reference
            walk->incRef("buildWalk");
        }
    } else if (edges.size() > 0) {
        // create walk edges
        walk = new GNEWalk(net, personParent, edges, arrivalPos);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_WALK_EDGES) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(walk, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert walk
            net->getAttributeCarriers()->insertDemandElement(walk);
            // set references in children
            personParent->addChildElement(walk);
            for (const auto& edge : edges) {
                edge->addChildElement(walk);
            }
            // include reference
            walk->incRef("buildWalk");
        }
    } else if (route) {
        // create walk edges
        walk = new GNEWalk(net, personParent, route, arrivalPos);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_WALK_ROUTE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(walk, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert walk
            net->getAttributeCarriers()->insertDemandElement(walk);
            // set references in children
            personParent->addChildElement(walk);
            route->addChildElement(walk);
            // include reference
            walk->incRef("buildWalk");
        }
    }
    // update geometry
    personParent->updateGeometry();
}


void
GNERouteHandler::buildRide(GNENet* net, bool undoDemandElements, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge,
                           GNEAdditional* busStopFrom, GNEAdditional* busStopTo, double arrivalPos, const std::vector<std::string>& lines) {
    // declare ride
    GNEDemandElement* ride = nullptr;
    // create ride depending of parameters
    if (fromEdge && toEdge) {
        // create ride edge->edge
        ride = new GNERide(net, personParent, fromEdge, toEdge, arrivalPos, lines);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_RIDE_EDGE_EDGE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(ride, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert ride
            net->getAttributeCarriers()->insertDemandElement(ride);
            // set references in children
            personParent->addChildElement(ride);
            fromEdge->addChildElement(ride);
            toEdge->addChildElement(ride);
            // include reference
            ride->incRef("buildRide");
        }
    } else if (fromEdge && busStopTo) {
        // create ride edge->busStop
        ride = new GNERide(net, personParent, fromEdge, busStopTo, arrivalPos, lines);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_RIDE_EDGE_BUSSTOP) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(ride, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert ride
            net->getAttributeCarriers()->insertDemandElement(ride);
            // set references in children
            personParent->addChildElement(ride);
            fromEdge->addChildElement(ride);
            busStopTo->addChildElement(ride);
            // include reference
            ride->incRef("buildRide");
        }
    } else if (busStopFrom && toEdge) {
        // create ride busStop->edge
        ride = new GNERide(net, personParent, busStopFrom, toEdge, arrivalPos, lines);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_RIDE_BUSSTOP_EDGE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(ride, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert ride
            net->getAttributeCarriers()->insertDemandElement(ride);
            // set references in children
            personParent->addChildElement(ride);
            busStopFrom->addChildElement(ride);
            toEdge->addChildElement(ride);
            // include reference
            ride->incRef("buildRide");
        }
    } else if (busStopFrom && busStopTo) {
        // create ride busStop->busStop
        ride = new GNERide(net, personParent, busStopFrom, busStopTo, arrivalPos, lines);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_RIDE_BUSSTOP_BUSSTOP) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(ride, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert ride
            net->getAttributeCarriers()->insertDemandElement(ride);
            // set references in children
            personParent->addChildElement(ride);
            busStopFrom->addChildElement(ride);
            busStopTo->addChildElement(ride);
            // include reference
            ride->incRef("buildRide");
        }
    }
    // update geometry
    personParent->updateGeometry();
}


void
GNERouteHandler::buildPersonStop(GNENet* net, bool undoDemandElements, GNEDemandElement* personParent, GNEEdge* edge, GNEAdditional* busStop, const SUMOVehicleParameter::Stop& stopParameters) {
    // declare personStop
    GNEDemandElement* personStop = nullptr;
    // create personStop depending of parameters
    if (edge) {
        // create personStop over edge
        personStop = new GNEPersonStop(net, personParent, edge, stopParameters);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_PERSONSTOP_EDGE) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(personStop, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert personStop
            net->getAttributeCarriers()->insertDemandElement(personStop);
            // set references in children
            personParent->addChildElement(personStop);
            edge->addChildElement(personStop);
            // include reference
            personStop->incRef("buildPersonStop");
        }
    } else if (busStop) {
        // create personStop over busStop
        personStop = new GNEPersonStop(net, personParent, busStop, stopParameters);
        // add element using undo list or directly, depending of undoDemandElements flag
        if (undoDemandElements) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(GNE_TAG_PERSONSTOP_BUSSTOP) + " within person '" + personParent->getID() + "'");
            net->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(personStop, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            // insert personStop
            net->getAttributeCarriers()->insertDemandElement(personStop);
            // set references in children
            personParent->addChildElement(personStop);
            busStop->addChildElement(personStop);
            // include reference
            personStop->incRef("buildPersonStop");
        }
    }
    // update geometry
    personParent->updateGeometry();
}


void
GNERouteHandler::transformToVehicle(GNEVehicle* originalVehicle, bool createEmbeddedRoute) {
    // get original vehicle tag
    SumoXMLTag tag = originalVehicle->getTagProperty().getTag();
    // get pointer to net
    GNENet* net = originalVehicle->getNet();
    // obtain vehicle parameters
    SUMOVehicleParameter vehicleParameters = *originalVehicle;
    // declare route parameters
    RouteParameter routeParameters;
    // set "yellow" as original route color
    routeParameters.color = RGBColor::YELLOW;
    // obtain edges depending of tag
    if ((tag == GNE_TAG_FLOW_ROUTE) || (tag == SUMO_TAG_VEHICLE)) {
        // get route edges
        routeParameters.edges = originalVehicle->getParentDemandElements().back()->getParentEdges();
        // get original route color
        routeParameters.color = originalVehicle->getParentDemandElements().back()->getColor();
    } else if ((tag == GNE_TAG_VEHICLE_WITHROUTE) || (tag == GNE_TAG_FLOW_WITHROUTE)) {
        // get embedded route edges
        routeParameters.edges = originalVehicle->getChildDemandElements().front()->getParentEdges();
    } else if ((tag == SUMO_TAG_TRIP) || (tag == SUMO_TAG_FLOW)) {
        // calculate path using from-via-to edges
        routeParameters.edges = originalVehicle->getNet()->getPathCalculator()->calculatePath(originalVehicle->getVClass(), originalVehicle->getParentEdges());
    }
    // only continue if edges are valid
    if (routeParameters.edges.empty()) {
        // declare header
        const std::string header = "Problem transforming to vehicle";
        // declare message
        const std::string message = "Vehicle cannot be transformed. Invalid number of edges";
        // write warning
        WRITE_DEBUG("Opened FXMessageBox " + header);
        // open message box
        FXMessageBox::warning(originalVehicle->getNet()->getViewNet()->getApp(), MBOX_OK, header.c_str(), "%s", message.c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox " + header);
    } else {
        // begin undo-redo operation
        net->getViewNet()->getUndoList()->p_begin("transform " + originalVehicle->getTagStr() + " to " + toString(SUMO_TAG_VEHICLE));
        // first delete vehicle
        net->deleteDemandElement(originalVehicle, net->getViewNet()->getUndoList());
        // check if new vehicle must have an embedded route
        if (createEmbeddedRoute) {
            // change tag in vehicle parameters
            vehicleParameters.tag = GNE_TAG_VEHICLE_WITHROUTE;
            // create a vehicle with embebbed routes
            buildVehicleEmbeddedRoute(net, true, vehicleParameters, routeParameters.edges);
        } else {
            // change tag in vehicle parameters
            vehicleParameters.tag = SUMO_TAG_VEHICLE;
            // generate a new route id
            routeParameters.routeID = net->generateDemandElementID(SUMO_TAG_ROUTE);
            // build route
            buildRoute(net, true, routeParameters, {});
            // set route ID in vehicle parameters
            vehicleParameters.routeid = routeParameters.routeID;
            // create vehicle
            buildVehicleOverRoute(net, true, vehicleParameters);
        }
        // end undo-redo operation
        net->getViewNet()->getUndoList()->p_end();
    }
}


void
GNERouteHandler::transformToRouteFlow(GNEVehicle* originalVehicle, bool createEmbeddedRoute) {
    // get original vehicle tag
    SumoXMLTag tag = originalVehicle->getTagProperty().getTag();
    // get pointer to net
    GNENet* net = originalVehicle->getNet();
    // obtain vehicle parameters
    SUMOVehicleParameter vehicleParameters = *originalVehicle;
    // declare route parameters
    RouteParameter routeParameters;
    // set "yellow" as original route color
    routeParameters.color = RGBColor::YELLOW;
    // obtain edges depending of tag
    if ((tag == GNE_TAG_FLOW_ROUTE) || (tag == SUMO_TAG_VEHICLE)) {
        // get route edges
        routeParameters.edges = originalVehicle->getParentDemandElements().back()->getParentEdges();
        // get original route color
        routeParameters.color = originalVehicle->getParentDemandElements().back()->getColor();
    } else if ((tag == GNE_TAG_VEHICLE_WITHROUTE) || (tag == GNE_TAG_FLOW_WITHROUTE)) {
        // get embedded route edges
        routeParameters.edges = originalVehicle->getChildDemandElements().front()->getParentEdges();
    } else if ((tag == SUMO_TAG_TRIP) || (tag == SUMO_TAG_FLOW)) {
        // calculate path using from-via-to edges
        routeParameters.edges = originalVehicle->getNet()->getPathCalculator()->calculatePath(originalVehicle->getVClass(), originalVehicle->getParentEdges());
    }
    // only continue if edges are valid
    if (routeParameters.edges.empty()) {
        // declare header
        const std::string header = "Problem transforming to vehicle";
        // declare message
        const std::string message = "Vehicle cannot be transformed. Invalid number of edges";
        // write warning
        WRITE_DEBUG("Opened FXMessageBox " + header);
        // open message box
        FXMessageBox::warning(originalVehicle->getNet()->getViewNet()->getApp(), MBOX_OK, header.c_str(), "%s", message.c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox " + header);
    } else {
        // begin undo-redo operation
        net->getViewNet()->getUndoList()->p_begin("transform " + originalVehicle->getTagStr() + " to " + toString(SUMO_TAG_VEHICLE));
        // first delete vehicle
        net->deleteDemandElement(originalVehicle, net->getViewNet()->getUndoList());
        // change depart
        if ((vehicleParameters.tag == SUMO_TAG_TRIP) || (vehicleParameters.tag == SUMO_TAG_VEHICLE) || (vehicleParameters.tag == GNE_TAG_VEHICLE_WITHROUTE)) {
            // set end
            vehicleParameters.repetitionEnd = vehicleParameters.depart + 3600;
            // set number
            vehicleParameters.repetitionNumber = 1800;
            vehicleParameters.parametersSet |= VEHPARS_NUMBER_SET;
            // unset parameters
            vehicleParameters.parametersSet &= ~VEHPARS_END_SET;
            vehicleParameters.parametersSet &= ~VEHPARS_VPH_SET;
            vehicleParameters.parametersSet &= ~VEHPARS_PERIOD_SET;
            vehicleParameters.parametersSet &= ~VEHPARS_PROB_SET;
        }
        // check if new vehicle must have an embedded route
        if (createEmbeddedRoute) {
            // change tag in vehicle parameters
            vehicleParameters.tag = GNE_TAG_FLOW_WITHROUTE;
            // create a flow with embebbed routes
            buildFlowEmbeddedRoute(net, true, vehicleParameters, routeParameters.edges);
        } else {
            // change tag in vehicle parameters
            vehicleParameters.tag = GNE_TAG_FLOW_ROUTE;
            // generate a new route id
            routeParameters.routeID = net->generateDemandElementID(SUMO_TAG_ROUTE);
            // build route
            buildRoute(net, true, routeParameters, {});
            // set route ID in vehicle parameters
            vehicleParameters.routeid = routeParameters.routeID;
            // create flow
            buildFlowOverRoute(net, true, vehicleParameters);
        }
        // end undo-redo operation
        net->getViewNet()->getUndoList()->p_end();
    }
}


void
GNERouteHandler::transformToTrip(GNEVehicle* originalVehicle) {
    // get original vehicle tag
    SumoXMLTag tag = originalVehicle->getTagProperty().getTag();
    // get pointer to net
    GNENet* net = originalVehicle->getNet();
    // obtain vehicle parameters
    SUMOVehicleParameter vehicleParameters = *originalVehicle;
    // get route
    GNEDemandElement* route = nullptr;
    // declare edges
    std::vector<GNEEdge*> edges;
    // obtain edges depending of tag
    if ((tag == SUMO_TAG_VEHICLE) || (tag == GNE_TAG_FLOW_ROUTE)) {
        // set route
        route = originalVehicle->getParentDemandElements().back();
        // get route edges
        edges = route->getParentEdges();
    } else if ((tag == GNE_TAG_VEHICLE_WITHROUTE) || (tag == GNE_TAG_FLOW_WITHROUTE)) {
        // get embedded route edges
        edges = originalVehicle->getChildDemandElements().front()->getParentEdges();
    } else if ((tag == SUMO_TAG_TRIP) || (tag == SUMO_TAG_FLOW)) {
        // just take parent edges (from and to)
        edges = originalVehicle->getParentEdges();
    }
    // only continue if edges are valid
    if (edges.size() < 2) {
        // declare header
        const std::string header = "Problem transforming to vehicle";
        // declare message
        const std::string message = "Vehicle cannot be transformed. Invalid number of edges";
        // write warning
        WRITE_DEBUG("Opened FXMessageBox " + header);
        // open message box
        FXMessageBox::warning(originalVehicle->getNet()->getViewNet()->getApp(), MBOX_OK, header.c_str(), "%s", message.c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox " + header);
    } else {
        // begin undo-redo operation
        net->getViewNet()->getUndoList()->p_begin("transform " + originalVehicle->getTagStr() + " to " + toString(SUMO_TAG_TRIP));
        // first delete vehicle
        net->deleteDemandElement(originalVehicle, net->getViewNet()->getUndoList());
        // check if route has to be deleted
        if (route && route->getChildDemandElements().empty()) {
            net->deleteDemandElement(route, net->getViewNet()->getUndoList());
        }
        // change tag in vehicle parameters
        vehicleParameters.tag = SUMO_TAG_TRIP;
        // create trip
        buildTrip(net, true, vehicleParameters, edges.front(), edges.back(), {});
        // end undo-redo operation
        net->getViewNet()->getUndoList()->p_end();
    }
}


void
GNERouteHandler::transformToFlow(GNEVehicle* originalVehicle) {
    // get original vehicle tag
    SumoXMLTag tag = originalVehicle->getTagProperty().getTag();
    // get pointer to net
    GNENet* net = originalVehicle->getNet();
    // obtain vehicle parameters
    SUMOVehicleParameter vehicleParameters = *originalVehicle;
    // declare route
    GNEDemandElement* route = nullptr;
    // declare edges
    std::vector<GNEEdge*> edges;
    // obtain edges depending of tag
    if ((tag == SUMO_TAG_VEHICLE) || (tag == GNE_TAG_FLOW_ROUTE)) {
        // set route
        route = originalVehicle->getParentDemandElements().back();
        // get route edges
        edges = route->getParentEdges();
    } else if ((tag == GNE_TAG_VEHICLE_WITHROUTE) || (tag == GNE_TAG_FLOW_WITHROUTE)) {
        // get embedded route edges
        edges = originalVehicle->getChildDemandElements().front()->getParentEdges();
    } else if ((tag == SUMO_TAG_TRIP) || (tag == SUMO_TAG_FLOW)) {
        // just take parent edges (from and to)
        edges = originalVehicle->getParentEdges();
    }
    // only continue if edges are valid
    if (edges.empty()) {
        // declare header
        const std::string header = "Problem transforming to vehicle";
        // declare message
        const std::string message = "Vehicle cannot be transformed. Invalid number of edges";
        // write warning
        WRITE_DEBUG("Opened FXMessageBox " + header);
        // open message box
        FXMessageBox::warning(originalVehicle->getNet()->getViewNet()->getApp(), MBOX_OK, header.c_str(), "%s", message.c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox " + header);
    } else {
        // begin undo-redo operation
        net->getViewNet()->getUndoList()->p_begin("transform " + originalVehicle->getTagStr() + " to " + toString(SUMO_TAG_VEHICLE));
        // first delete vehicle
        net->deleteDemandElement(originalVehicle, net->getViewNet()->getUndoList());
        // check if route has to be deleted
        if (route && route->getChildDemandElements().empty()) {
            net->deleteDemandElement(route, net->getViewNet()->getUndoList());
        }
        // change depart
        if ((vehicleParameters.tag == SUMO_TAG_TRIP) || (vehicleParameters.tag == SUMO_TAG_VEHICLE) || (vehicleParameters.tag == GNE_TAG_VEHICLE_WITHROUTE)) {
            // set end
            vehicleParameters.repetitionEnd = vehicleParameters.depart + 3600;
            // set number
            vehicleParameters.repetitionNumber = 1800;
            vehicleParameters.parametersSet |= VEHPARS_NUMBER_SET;
            // unset parameters
            vehicleParameters.parametersSet &= ~VEHPARS_END_SET;
            vehicleParameters.parametersSet &= ~VEHPARS_VPH_SET;
            vehicleParameters.parametersSet &= ~VEHPARS_PERIOD_SET;
            vehicleParameters.parametersSet &= ~VEHPARS_PROB_SET;
        }
        // change tag in vehicle parameters
        vehicleParameters.tag = SUMO_TAG_FLOW;
        // create flow
        buildFlow(net, true, vehicleParameters, edges.front(), edges.back(), {});
        // end undo-redo operation
        net->getViewNet()->getUndoList()->p_end();
    }
}


void
GNERouteHandler::transformToPerson(GNEPerson* /*originalPerson*/) {
    //
}


void
GNERouteHandler::transformToPersonFlow(GNEPerson* /*originalPerson*/) {
    //
}


void
GNERouteHandler::setFlowParameters(const SumoXMLAttr attribute, int& parameters) {
    // modify parametersSetCopy depending of given Flow attribute
    switch (attribute) {
        case SUMO_ATTR_END: {
            // give more priority to end
            parameters = VEHPARS_END_SET | VEHPARS_NUMBER_SET;
            break;
        }
        case SUMO_ATTR_NUMBER:
            parameters ^= VEHPARS_END_SET;
            parameters |= VEHPARS_NUMBER_SET;
            break;
        case SUMO_ATTR_VEHSPERHOUR:
        case SUMO_ATTR_PERSONSPERHOUR: {
            // give more priority to end
            if ((parameters & VEHPARS_END_SET) && (parameters & VEHPARS_NUMBER_SET)) {
                parameters = VEHPARS_END_SET;
            } else if (parameters & VEHPARS_END_SET) {
                parameters = VEHPARS_END_SET;
            } else if (parameters & VEHPARS_NUMBER_SET) {
                parameters = VEHPARS_NUMBER_SET;
            }
            // set VehsPerHour
            parameters |= VEHPARS_VPH_SET;
            break;
        }
        case SUMO_ATTR_PERIOD: {
            // give more priority to end
            if ((parameters & VEHPARS_END_SET) && (parameters & VEHPARS_NUMBER_SET)) {
                parameters = VEHPARS_END_SET;
            } else if (parameters & VEHPARS_END_SET) {
                parameters = VEHPARS_END_SET;
            } else if (parameters & VEHPARS_NUMBER_SET) {
                parameters = VEHPARS_NUMBER_SET;
            }
            // set period
            parameters |= VEHPARS_PERIOD_SET;
            break;
        }
        case SUMO_ATTR_PROB: {
            // give more priority to end
            if ((parameters & VEHPARS_END_SET) && (parameters & VEHPARS_NUMBER_SET)) {
                parameters = VEHPARS_END_SET;
            } else if (parameters & VEHPARS_END_SET) {
                parameters = VEHPARS_END_SET;
            } else if (parameters & VEHPARS_NUMBER_SET) {
                parameters = VEHPARS_NUMBER_SET;
            }
            // set probability
            parameters |= VEHPARS_PROB_SET;
            break;
        }
        default:
            break;
    }
}

// ===========================================================================
// protected
// ===========================================================================

void
GNERouteHandler::openVehicleTypeDistribution(const SUMOSAXAttributes& /*attrs*/) {
    // currently unused
}


void
GNERouteHandler::closeVehicleTypeDistribution() {
    // currently unused
}


void
GNERouteHandler::openRoute(const SUMOSAXAttributes& attrs) {
    // change abort flag
    myAbort = false;
    // check if route has ID (due embebbed routes)
    if (attrs.hasAttribute(SUMO_ATTR_ID)) {
        myRouteParameter.routeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_ROUTE, SUMO_ATTR_ID, myAbort);
        myRouteParameter.loadedID = true;
    } else {
        myRouteParameter.routeID.clear();
        myRouteParameter.loadedID = false;
    }
    // parse other attributes
    myRouteParameter.setEdges(myNet, GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, myRouteParameter.routeID, SUMO_TAG_ROUTE, SUMO_ATTR_EDGES, myAbort));
    myRouteParameter.color = GNEAttributeCarrier::parseAttributeFromXML<RGBColor>(attrs, myRouteParameter.routeID, SUMO_TAG_ROUTE, SUMO_ATTR_COLOR, myAbort);
}


void
GNERouteHandler::openFlow(const SUMOSAXAttributes& attrs) {
    // change abort flag
    myAbort = false;
    // parse flow attributes
    myRouteParameter.setEdges(myNet, myVehicleParameter->id,
                              GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, myVehicleParameter->id, SUMO_TAG_FLOW, SUMO_ATTR_FROM, myAbort),
                              GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, myVehicleParameter->id, SUMO_TAG_FLOW, SUMO_ATTR_TO, myAbort),
                              GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, myVehicleParameter->id, SUMO_TAG_FLOW, SUMO_ATTR_VIA, myAbort));
}


void
GNERouteHandler::openRouteFlow(const SUMOSAXAttributes& /*attrs*/) {
    // clear edges
    myRouteParameter.clearEdges();
}


void
GNERouteHandler::openTrip(const SUMOSAXAttributes& attrs) {
    // change abort flag
    myAbort = false;
    // parse trips attributes
    myRouteParameter.setEdges(myNet, myVehicleParameter->id,
                              GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, myVehicleParameter->id, SUMO_TAG_TRIP, SUMO_ATTR_FROM, myAbort),
                              GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, myVehicleParameter->id, SUMO_TAG_TRIP, SUMO_ATTR_TO, myAbort),
                              GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, myVehicleParameter->id, SUMO_TAG_TRIP, SUMO_ATTR_VIA, myAbort));
}


void
GNERouteHandler::closeRoute(const bool /* mayBeDisconnected */) {
    // first copy parameters from SUMORouteHanlder to myRouteParameter
    myRouteParameter.parameters = myLoadedParameterised;
    // clear loaded parameters after set
    myLoadedParameterised.clearParameter();
    // we have two possibilities: Either create a route with their own ID, or create a route within a vehicle
    if (myVehicleParameter) {
        // extra warning for embebbed routes
        if (myRouteParameter.loadedID) {
            WRITE_WARNING("Attribute ID ignored in embedded routes");
        }
        // check edges
        if (myRouteParameter.edges.size() == 0) {
            WRITE_ERROR("A route needs at least one edge. Vehicle with ID='" + myVehicleParameter->id + "' cannot be created");
        } else if (!isVehicleIdDuplicated(myNet, myVehicleParameter->id)) {
            // obtain vType
            GNEDemandElement* vType = myNet->retrieveDemandElement(SUMO_TAG_VTYPE, myVehicleParameter->vtypeid, false);
            if (vType == nullptr) {
                WRITE_ERROR("Invalid vehicle type '" + myVehicleParameter->vtypeid + "' used in " + toString(myVehicleParameter->tag) + " '" + myVehicleParameter->id + "'.");
            } else if (myVehicleParameter->wasSet(VEHPARS_DEPARTLANE_SET) && (myVehicleParameter->departLaneProcedure == DepartLaneDefinition::GIVEN) && (myRouteParameter.edges.front()->getLanes().size() < myVehicleParameter->departLane)) {
                WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTLANE) + " used in " + toString(myVehicleParameter->tag) + " '" + myVehicleParameter->id + "'. " + toString(myVehicleParameter->departLane) + " is greater than number of lanes");
            } else if (myVehicleParameter->wasSet(VEHPARS_DEPARTSPEED_SET) && (myVehicleParameter->departSpeedProcedure == DepartSpeedDefinition::GIVEN) && (vType->getAttributeDouble(SUMO_ATTR_MAXSPEED) < myVehicleParameter->departSpeed)) {
                WRITE_ERROR("Invalid " + toString(SUMO_ATTR_DEPARTSPEED) + " used in " + toString(myVehicleParameter->tag) + " '" + myVehicleParameter->id + "'. " + toString(myVehicleParameter->departSpeed) + " is greater than vType" + toString(SUMO_ATTR_MAXSPEED));
            } else {
                // due vehicle was loaded without a route, change tag
                myVehicleParameter->tag = (myVehicleParameter->tag == SUMO_TAG_VEHICLE) ? GNE_TAG_VEHICLE_WITHROUTE : GNE_TAG_FLOW_WITHROUTE;
                // create vehicle or trips using myTemporalVehicleParameter without a route
                myLoadedVehicleWithEmbebbedRoute = new GNEVehicle(myNet, vType, *myVehicleParameter);
                // creaste embedded route
                GNEDemandElement* embeddedRoute = new GNERoute(myNet, myLoadedVehicleWithEmbebbedRoute, myRouteParameter);
                // add both to net depending of myUndoDemandElements
                if (myUndoDemandElements) {
                    myNet->getViewNet()->getUndoList()->p_begin("add vehicle and " + embeddedRoute->getTagStr());
                    // add both in net using undoList
                    myNet->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(myLoadedVehicleWithEmbebbedRoute, true), true);
                    myNet->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(embeddedRoute, true), true);
                    // iterate over stops of myActiveRouteStops and create stops associated with it
                    for (const auto& i : myActiveRouteStops) {
                        buildStop(myNet, true, i, myLoadedVehicleWithEmbebbedRoute);
                    }
                } else {
                    // add vehicleOrRouteFlow in net and in their vehicle type parent
                    myNet->getAttributeCarriers()->insertDemandElement(myLoadedVehicleWithEmbebbedRoute);
                    vType->addChildElement(myLoadedVehicleWithEmbebbedRoute);
                    myLoadedVehicleWithEmbebbedRoute->incRef("buildVehicleAndRoute");
                    // add route manually in net, and in all of their edges and in vehicleOrRouteFlow
                    myNet->getAttributeCarriers()->insertDemandElement(embeddedRoute);
                    for (const auto& i : myRouteParameter.edges) {
                        i->addChildElement(myLoadedVehicleWithEmbebbedRoute);
                    }
                    myLoadedVehicleWithEmbebbedRoute->addChildElement(embeddedRoute);
                    embeddedRoute->incRef("buildVehicleAndRoute");
                    // iterate over stops of myActiveRouteStops and create stops associated with it
                    for (const auto& i : myActiveRouteStops) {
                        buildStop(myNet, false, i, myLoadedVehicleWithEmbebbedRoute);
                    }
                }
            }
        }
    } else if (myRouteParameter.loadedID == false) {
        WRITE_ERROR(toString(SUMO_TAG_ROUTE) + " needs a valid ID.");
    } else if (!SUMOXMLDefinitions::isValidVehicleID(myRouteParameter.routeID)) {
        WRITE_ERROR(toString(SUMO_TAG_ROUTE) + " ID='" + myRouteParameter.routeID + "' contains invalid characters.");
    } else if (myNet->retrieveDemandElement(SUMO_TAG_ROUTE, myRouteParameter.routeID, false) != nullptr) {
        WRITE_ERROR("There is another " + toString(SUMO_TAG_ROUTE) + " with the same ID='" + myRouteParameter.routeID + "'.");
    } else if (myRouteParameter.edges.size() == 0) {
        WRITE_ERROR("A route needs at least one edge.");
    } else {
        // build route
        buildRoute(myNet, myUndoDemandElements, myRouteParameter, myActiveRouteStops);
    }
}


void
GNERouteHandler::openRouteDistribution(const SUMOSAXAttributes& /*attrs*/) {
    // currently unused
}


void
GNERouteHandler::closeRouteDistribution() {
    // currently unused
}


void
GNERouteHandler::closeVehicle() {
    // first check if we're closing a vehicle with embebbed routes and stops
    if (myLoadedVehicleWithEmbebbedRoute) {
        myLoadedVehicleWithEmbebbedRoute = nullptr;
        // check if current command group size has to be ended
        if (myNet->getViewNet()->getUndoList()->currentCommandGroupSize() > 0) {
            myNet->getViewNet()->getUndoList()->p_end();
        }
    } else {
        // now check if myVehicleParameter was sucesfully created
        if (myVehicleParameter) {
            // build vehicle over route
            buildVehicleOverRoute(myNet, myUndoDemandElements, *myVehicleParameter);
        }
    }
}


void
GNERouteHandler::closeVType() {
    // first check that VType was sucesfully created
    if (myCurrentVType) {
        // first check if we're creating a vType or a pType
        SumoXMLTag vTypeTag = (myCurrentVType->vehicleClass == SVC_PEDESTRIAN) ? SUMO_TAG_PTYPE : SUMO_TAG_VTYPE;
        // check if loaded vType/pType is a default vtype
        if ((myCurrentVType->id == DEFAULT_VTYPE_ID) || (myCurrentVType->id == DEFAULT_PEDTYPE_ID) || (myCurrentVType->id == DEFAULT_BIKETYPE_ID)) {
            // overwrite default vehicle type
            GNEVehicleType::overwriteVType(myNet->retrieveDemandElement(vTypeTag, myCurrentVType->id, false), myCurrentVType, myNet->getViewNet()->getUndoList());
        } else if (myNet->retrieveDemandElement(vTypeTag, myCurrentVType->id, false) != nullptr) {
            WRITE_ERROR("There is another " + toString(vTypeTag) + " with the same ID='" + myCurrentVType->id + "'.");
        } else {
            // create vType/pType using myCurrentVType
            GNEDemandElement* vType = new GNEVehicleType(myNet, *myCurrentVType, vTypeTag);
            if (myUndoDemandElements) {
                myNet->getViewNet()->getUndoList()->p_begin("add " + vType->getTagStr());
                myNet->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(vType, true), true);
                myNet->getViewNet()->getUndoList()->p_end();
            } else {
                myNet->getAttributeCarriers()->insertDemandElement(vType);
                vType->incRef("buildVType");
            }
        }
    }
}


void
GNERouteHandler::closePerson() {
    // first check if myVehicleParameter was sucesfully created
    if (myVehicleParameter) {
        // first check if ID is duplicated
        if (!isPersonIdDuplicated(myNet, myVehicleParameter->id)) {
            // obtain ptype
            GNEDemandElement* pType = myNet->retrieveDemandElement(SUMO_TAG_PTYPE, myVehicleParameter->vtypeid, false);
            if (pType == nullptr) {
                WRITE_ERROR("Invalid person type '" + myVehicleParameter->vtypeid + "' used in " + toString(myVehicleParameter->tag) + " '" + myVehicleParameter->vtypeid + "'.");
            } else if (myPersonValues.checkPersonPlanValues()) {
                // create person using personParameters
                GNEPerson* person = new GNEPerson(SUMO_TAG_PERSON, myNet, pType, *myVehicleParameter);
                // begin undo-list creation
                myNet->getViewNet()->getUndoList()->p_begin("add " + person->getTagStr());
                // add person
                myNet->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(person, true), true);
                // iterate over all personplan children and add it
                for (const auto& personPlanValue : myPersonValues.myPersonPlanValues) {
                    switch (personPlanValue.tag) {
                        // Person Trips
                        case GNE_TAG_PERSONTRIP_EDGE_EDGE: {
                            buildPersonTrip(myNet, true, person, personPlanValue.fromEdge, personPlanValue.toEdge, nullptr, nullptr,
                                            personPlanValue.arrivalPos, personPlanValue.vTypes, personPlanValue.modes);
                            break;
                        }
                        case GNE_TAG_PERSONTRIP_EDGE_BUSSTOP: {
                            buildPersonTrip(myNet, true, person, personPlanValue.fromEdge, nullptr, nullptr, personPlanValue.toBusStop,
                                            personPlanValue.arrivalPos, personPlanValue.vTypes, personPlanValue.modes);
                            break;
                        }
                        case GNE_TAG_PERSONTRIP_BUSSTOP_EDGE: {
                            buildPersonTrip(myNet, true, person, nullptr, personPlanValue.toEdge, personPlanValue.fromBusStop, nullptr,
                                            personPlanValue.arrivalPos, personPlanValue.vTypes, personPlanValue.modes);
                            break;
                        }
                        case GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP: {
                            buildPersonTrip(myNet, true, person, nullptr, nullptr, personPlanValue.fromBusStop, personPlanValue.toBusStop,
                                            personPlanValue.arrivalPos, personPlanValue.vTypes, personPlanValue.modes);
                            break;
                        }
                        // Walks
                        case GNE_TAG_WALK_EDGE_EDGE: {
                            buildWalk(myNet, true, person, personPlanValue.fromEdge, personPlanValue.toEdge, nullptr, nullptr, {}, nullptr,
                                      personPlanValue.arrivalPos);
                            break;
                        }
                        case GNE_TAG_WALK_EDGE_BUSSTOP: {
                            buildWalk(myNet, true, person, personPlanValue.fromEdge, nullptr, nullptr, personPlanValue.toBusStop, {}, nullptr,
                                      personPlanValue.arrivalPos);
                            break;
                        }
                        case GNE_TAG_WALK_BUSSTOP_EDGE: {
                            buildWalk(myNet, true, person, nullptr, personPlanValue.toEdge, personPlanValue.fromBusStop, nullptr, {}, nullptr,
                                      personPlanValue.arrivalPos);
                            break;
                        }
                        case GNE_TAG_WALK_BUSSTOP_BUSSTOP: {
                            buildWalk(myNet, true, person, nullptr, nullptr, personPlanValue.fromBusStop, personPlanValue.toBusStop, {}, nullptr,
                                      personPlanValue.arrivalPos);
                            break;
                        }
                        case GNE_TAG_WALK_EDGES: {
                            buildWalk(myNet, true, person, nullptr, nullptr, nullptr, nullptr, personPlanValue.edges, nullptr,
                                      personPlanValue.arrivalPos);
                            break;
                        }
                        case GNE_TAG_WALK_ROUTE: {
                            buildWalk(myNet, true, person, nullptr, nullptr, nullptr, nullptr, {}, personPlanValue.route,
                                      personPlanValue.arrivalPos);
                            break;
                        }
                        // Rides
                        case GNE_TAG_RIDE_EDGE_EDGE: {
                            buildRide(myNet, true, person, personPlanValue.fromEdge, personPlanValue.toEdge, nullptr, nullptr,
                                      personPlanValue.arrivalPos, personPlanValue.lines);
                            break;
                        }
                        case GNE_TAG_RIDE_EDGE_BUSSTOP: {
                            buildRide(myNet, true, person, personPlanValue.fromEdge, nullptr, nullptr, personPlanValue.toBusStop,
                                      personPlanValue.arrivalPos, personPlanValue.lines);
                            break;
                        }
                        case GNE_TAG_RIDE_BUSSTOP_EDGE: {
                            buildRide(myNet, true, person, nullptr, personPlanValue.toEdge, personPlanValue.fromBusStop, nullptr,
                                      personPlanValue.arrivalPos, personPlanValue.lines);
                            break;
                        }
                        case GNE_TAG_RIDE_BUSSTOP_BUSSTOP: {
                            buildRide(myNet, true, person, nullptr, nullptr, personPlanValue.fromBusStop, personPlanValue.toBusStop,
                                      personPlanValue.arrivalPos, personPlanValue.lines);
                            break;
                        }
                        case GNE_TAG_PERSONSTOP_BUSSTOP:
                        case GNE_TAG_PERSONSTOP_EDGE: {
                            buildStop(myNet, true, personPlanValue.stopParameters, person);
                            break;
                        }
                        default:
                            throw InvalidArgument("Invalid person plan tag");
                    }
                }
                // finish creation
                myNet->getViewNet()->getUndoList()->p_end();
            }
        }
    }
    // clear person plan values
    myPersonValues.myPersonPlanValues.clear();
}

void
GNERouteHandler::closePersonFlow() {
    // first check if myVehicleParameter was sucesfully created
    if (myVehicleParameter) {
        // build person flow
        buildPersonFlow(myNet, myUndoDemandElements, *myVehicleParameter);
    }
}

void
GNERouteHandler::closeContainer() {
    // currently unused
}


void
GNERouteHandler::closeFlow() {
    // first check if we're closing a flow with embebbed routes and stops
    if (myLoadedVehicleWithEmbebbedRoute) {
        myLoadedVehicleWithEmbebbedRoute = nullptr;
        // check if current command group size has to be ended
        if (myNet->getViewNet()->getUndoList()->currentCommandGroupSize() > 0) {
            myNet->getViewNet()->getUndoList()->p_end();
        }
    } else if (myVehicleParameter) {
        // check if we're creating a flow or a routeFlow over route
        if (!myVehicleParameter->routeid.empty()) {
            // change tag
            myVehicleParameter->tag = GNE_TAG_FLOW_ROUTE;
            // build flow over route
            buildFlowOverRoute(myNet, myUndoDemandElements, *myVehicleParameter);
        } else if (myRouteParameter.edges.size() > 1) {
            // extract via edges
            std::vector<GNEEdge*> viaEdges;
            for (int i = 1; i < ((int)myRouteParameter.edges.size() - 1); i++) {
                viaEdges.push_back(myRouteParameter.edges.at(i));
            }
            // build flow from-via-to
            buildFlow(myNet, true, *myVehicleParameter, myRouteParameter.edges.front(), myRouteParameter.edges.back(), viaEdges);
        }
    }
}


void
GNERouteHandler::closeTrip() {
    // first check if myVehicleParameter was sucesfully created
    if (myVehicleParameter && (myRouteParameter.edges.size() > 1)) {
        // force reroute
        myVehicleParameter->parametersSet |= VEHPARS_FORCE_REROUTE;
        // extract via edges
        std::vector<GNEEdge*> viaEdges;
        for (int i = 1; i < ((int)myRouteParameter.edges.size() - 1); i++) {
            viaEdges.push_back(myRouteParameter.edges.at(i));
        }
        // build trip
        buildTrip(myNet, true, *myVehicleParameter, myRouteParameter.edges.front(), myRouteParameter.edges.back(), viaEdges);
    }
}


void
GNERouteHandler::addStop(const SUMOSAXAttributes& attrs) {
    // declare a personStop
    PersonPlansValues stop;
    std::string errorSuffix;
    if (myVehicleParameter != nullptr) {
        errorSuffix = " in " + toString(myVehicleParameter->tag) + " '" + myVehicleParameter->id + "'.";
    } else {
        errorSuffix = " in route '" + myActiveRouteID + "'.";
    }
    // try to parse stop
    myAbort = parseStop(stop.stopParameters, attrs, errorSuffix, MsgHandler::getErrorInstance());
    if (!myAbort) {
        return;
    }
    // try to parse the assigned bus stop
    if (stop.stopParameters.busstop != "") {
        // ok, we have a busStop
        GNEAdditional* bs = myNet->retrieveAdditional(SUMO_TAG_BUS_STOP, stop.stopParameters.busstop, false);
        if (bs == nullptr) {
            WRITE_ERROR(toString(SUMO_TAG_BUS_STOP) + " '" + stop.stopParameters.busstop + "' is not known" + errorSuffix);
            return;
        }
        // save lane
        stop.stopParameters.lane = bs->getAttribute(SUMO_ATTR_LANE);
        // save stoping place in stop
        stop.busStop = bs;
        // set tag
        stop.tag = SUMO_TAG_STOP_BUSSTOP;
        // special case for persons
        if ((myVehicleParameter != nullptr) && ((myVehicleParameter->tag == SUMO_TAG_PERSON) || (myVehicleParameter->tag == SUMO_TAG_PERSONFLOW))) {
            stop.tag = GNE_TAG_PERSONSTOP_BUSSTOP;
        }
    } else if (stop.stopParameters.containerstop != "") {
        // special case for persons
        if ((myVehicleParameter != nullptr) && ((myVehicleParameter->tag == SUMO_TAG_PERSON) || (myVehicleParameter->tag == SUMO_TAG_PERSONFLOW))) {
            WRITE_ERROR("Persons don't support " + toString(SUMO_TAG_CONTAINER_STOP) + "s");
            return;
        }
        // ok, we have a containerStop
        GNEAdditional* cs = myNet->retrieveAdditional(SUMO_TAG_CONTAINER_STOP, stop.stopParameters.containerstop, false);
        if (cs == nullptr) {
            WRITE_ERROR(toString(SUMO_TAG_CONTAINER_STOP) + " '" + stop.stopParameters.containerstop + "' is not known" + errorSuffix);
            return;
        }
        // save lane
        stop.stopParameters.lane = cs->getAttribute(SUMO_ATTR_LANE);
        // save stoping place in stop
        stop.containerStop = cs;
        // set tag
        stop.tag = SUMO_TAG_STOP_CONTAINERSTOP;

    } else if (stop.stopParameters.chargingStation != "") {
        // special case for persons
        if ((myVehicleParameter != nullptr) && ((myVehicleParameter->tag == SUMO_TAG_PERSON) || (myVehicleParameter->tag == SUMO_TAG_PERSONFLOW))) {
            WRITE_ERROR("Persons don't support " + toString(SUMO_TAG_CHARGING_STATION) + "s");
            return;
        }
        // ok, we have a chargingStation
        GNEAdditional* cs = myNet->retrieveAdditional(SUMO_TAG_CHARGING_STATION, stop.stopParameters.chargingStation, false);
        if (cs == nullptr) {
            WRITE_ERROR(toString(SUMO_TAG_CHARGING_STATION) + " '" + stop.stopParameters.chargingStation + "' is not known" + errorSuffix);
            return;
        }
        // save lane
        stop.stopParameters.lane = cs->getAttribute(SUMO_ATTR_LANE);
        // save stoping place in stop
        stop.chargingStation = cs;
        // set tag
        stop.tag = SUMO_TAG_STOP_CHARGINGSTATION;
    } else if (stop.stopParameters.parkingarea != "") {
        // special case for persons
        if ((myVehicleParameter != nullptr) && ((myVehicleParameter->tag == SUMO_TAG_PERSON) || (myVehicleParameter->tag == SUMO_TAG_PERSONFLOW))) {
            WRITE_ERROR("Persons don't support " + toString(SUMO_TAG_PARKING_AREA) + "s");
            return;
        }
        // ok, we have a parkingArea
        GNEAdditional* pa = myNet->retrieveAdditional(SUMO_TAG_PARKING_AREA, stop.stopParameters.parkingarea, false);
        if (pa == nullptr) {
            WRITE_ERROR(toString(SUMO_TAG_PARKING_AREA) + " '" + stop.stopParameters.parkingarea + "' is not known" + errorSuffix);
            return;
        }
        // save lane
        stop.stopParameters.lane = pa->getAttribute(SUMO_ATTR_LANE);
        // save stoping place in stop
        stop.parkingArea = pa;
        // set tag
        stop.tag = SUMO_TAG_STOP_PARKINGAREA;
    } else {
        // no, the lane and the position should be given
        // get the lane
        stop.stopParameters.lane = attrs.getOpt<std::string>(SUMO_ATTR_LANE, nullptr, myAbort, "");
        stop.lane = myNet->retrieveLane(stop.stopParameters.lane, false);
        // check if lane is valid
        if (myAbort && stop.stopParameters.lane != "") {
            if (stop.lane == nullptr) {
                WRITE_ERROR("The lane '" + stop.stopParameters.lane + "' for a stop is not known" + errorSuffix);
                return;
            }
        } else {
            WRITE_ERROR("A stop must be placed over a " + toString(SUMO_TAG_BUS_STOP) + ", a " + toString(SUMO_TAG_CONTAINER_STOP) +
                        ", a " + toString(SUMO_TAG_CHARGING_STATION) + ", a " + toString(SUMO_TAG_PARKING_AREA) + " or a " + toString(SUMO_TAG_LANE) + errorSuffix);
            return;
        }
        // calculate start and end position
        stop.stopParameters.endPos = attrs.getOpt<double>(SUMO_ATTR_ENDPOS, nullptr, myAbort, stop.lane->getLaneParametricLength());
        if (attrs.hasAttribute(SUMO_ATTR_POSITION)) {
            WRITE_ERROR("Deprecated attribute 'pos' in description of stop" + errorSuffix);
            stop.stopParameters.endPos = attrs.getOpt<double>(SUMO_ATTR_POSITION, nullptr, myAbort, stop.stopParameters.endPos);
        }
        stop.stopParameters.startPos = attrs.getOpt<double>(SUMO_ATTR_STARTPOS, nullptr, myAbort, MAX2(0., stop.stopParameters.endPos - 2 * POSITION_EPS));
        stop.stopParameters.friendlyPos = attrs.getOpt<bool>(SUMO_ATTR_FRIENDLY_POS, nullptr, myAbort, false);
        if (!myAbort || (checkStopPos(stop.stopParameters.startPos, stop.stopParameters.endPos, stop.lane->getLaneParametricLength(), POSITION_EPS, stop.stopParameters.friendlyPos) != STOPPOS_VALID)) {
            WRITE_ERROR("Invalid start or end position for stop on lane '" + stop.stopParameters.lane + "'" + errorSuffix);
            return;
        }
        // set tag
        stop.tag = SUMO_TAG_STOP_LANE;
        // special case for persons
        if ((myVehicleParameter != nullptr) && ((myVehicleParameter->tag == SUMO_TAG_PERSON) || (myVehicleParameter->tag == SUMO_TAG_PERSONFLOW))) {
            stop.tag = GNE_TAG_PERSONSTOP_EDGE;
        }
    }
    // now create or store stop
    if (myLoadedVehicleWithEmbebbedRoute) {
        buildStop(myNet, true, stop.stopParameters, myLoadedVehicleWithEmbebbedRoute);
    } else if (myVehicleParameter != nullptr) {
        if ((myVehicleParameter->tag == SUMO_TAG_PERSON) || (myVehicleParameter->tag == SUMO_TAG_PERSONFLOW)) {
            myPersonValues.myPersonPlanValues.push_back(stop);
        } else {
            myVehicleParameter->stops.push_back(stop.stopParameters);
        }
    } else {
        myActiveRouteStops.push_back(stop.stopParameters);
    }
}


void
GNERouteHandler::addPersonTrip(const SUMOSAXAttributes& attrs) {
    // add person trip
    myPersonValues.addPersonValue(myNet, SUMO_TAG_PERSONTRIP, attrs);
}


void
GNERouteHandler::addWalk(const SUMOSAXAttributes& attrs) {
    // add person walk
    myPersonValues.addPersonValue(myNet, SUMO_TAG_WALK, attrs);
}


void
GNERouteHandler::addRide(const SUMOSAXAttributes& attrs) {
    // add person ride
    myPersonValues.addPersonValue(myNet, SUMO_TAG_RIDE, attrs);
}


void
GNERouteHandler::addPerson(const SUMOSAXAttributes& /*attrs*/) {
    // currently unused
}


void
GNERouteHandler::addContainer(const SUMOSAXAttributes& /*attrs*/) {
    // currently unused
}


void
GNERouteHandler::addTransport(const SUMOSAXAttributes& /*attrs*/) {
    // currently unused
}


void
GNERouteHandler::addTranship(const SUMOSAXAttributes& /*attrs*/) {
    // currently unused
}

// ===========================================================================
// private members
// ===========================================================================

GNERouteHandler::PersonPlansValues::PersonPlansValues() :
    tag(SUMO_TAG_NOTHING),
    fromEdge(nullptr),
    toEdge(nullptr),
    fromBusStop(nullptr),
    toBusStop(nullptr),
    route(nullptr),
    arrivalPos(-1),
    edgeStop(nullptr),
    busStop(nullptr),
    containerStop(nullptr),
    chargingStation(nullptr),
    parkingArea(nullptr),
    lane(nullptr) {
}


void
GNERouteHandler::PersonPlansValues::updateGNETag() {
    // set GNE Tag depending of parameters
    if (fromEdge && toEdge) {
        // edge->edge
        if (tag == SUMO_TAG_PERSONTRIP) {
            tag = GNE_TAG_PERSONTRIP_EDGE_EDGE;
        } else if (tag == SUMO_TAG_WALK) {
            tag = GNE_TAG_WALK_EDGE_EDGE;
        } else if (tag == SUMO_TAG_RIDE) {
            tag = GNE_TAG_RIDE_EDGE_EDGE;
        }
    } else if (fromEdge && toBusStop) {
        // edge->busStop
        if (tag == SUMO_TAG_PERSONTRIP) {
            tag = GNE_TAG_PERSONTRIP_EDGE_BUSSTOP;
        } else if (tag == SUMO_TAG_WALK) {
            tag = GNE_TAG_WALK_EDGE_BUSSTOP;
        } else if (tag == SUMO_TAG_RIDE) {
            tag = GNE_TAG_RIDE_EDGE_BUSSTOP;
        }
    } else if (fromBusStop && toEdge) {
        // busStop->edge
        if (tag == SUMO_TAG_PERSONTRIP) {
            tag = GNE_TAG_PERSONTRIP_BUSSTOP_EDGE;
        } else if (tag == SUMO_TAG_WALK) {
            tag = GNE_TAG_WALK_BUSSTOP_EDGE;
        } else if (tag == SUMO_TAG_RIDE) {
            tag = GNE_TAG_RIDE_BUSSTOP_EDGE;
        }
    } else if (fromBusStop && toBusStop) {
        // busStop->busStop
        if (tag == SUMO_TAG_PERSONTRIP) {
            tag = GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP;
        } else if (tag == SUMO_TAG_WALK) {
            tag = GNE_TAG_WALK_BUSSTOP_BUSSTOP;
        } else if (tag == SUMO_TAG_RIDE) {
            tag = GNE_TAG_RIDE_BUSSTOP_BUSSTOP;
        }
    } else if (edges.size() > 0) {
        // walk edges
        tag = GNE_TAG_WALK_EDGES;
    } else if (route) {
        // walk route
        tag = GNE_TAG_WALK_EDGES;
    } else if (edgeStop || lane) {
        // person stop lane
        tag = GNE_TAG_PERSONSTOP_EDGE;
    } else {
        throw ProcessError("invalid combination");
    }
}


bool
GNERouteHandler::PersonPlansValues::checkIntegrity() const {
    bool correct = false;
    // edge->edge
    if (fromEdge && toEdge) {
        correct = true;
    }
    // edge->busStop
    if (fromEdge && toBusStop) {
        if (correct) {
            return false;
        } else {
            correct = true;
        }
    }
    // busStop->edge
    if (fromBusStop && toEdge) {
        if (correct) {
            return false;
        } else {
            correct = true;
        }
    }
    // busStop->busStop
    if (fromBusStop && toBusStop) {
        if (correct) {
            return false;
        } else {
            correct = true;
        }
    }
    // edges
    if (edges.size() > 0) {
        if (correct) {
            return false;
        } else {
            correct = true;
        }
    }
    // route
    if (route) {
        if (correct) {
            return false;
        } else {
            correct = true;
        }
    }
    // edge
    if (edgeStop) {
        if (correct) {
            return false;
        } else {
            correct = true;
        }
    }
    return correct;
}


bool
GNERouteHandler::PersonPlansValues::isFirstPersonPlan() const {
    // edge->edge
    if (fromEdge && toEdge) {
        return true;
    }
    // edge->busStop
    if (fromEdge && toBusStop) {
        return true;
    }
    // busStop->edge
    if (fromBusStop && toEdge) {
        return true;
    }
    // busStop->busStop
    if (fromBusStop && toBusStop) {
        return true;
    }
    // edges
    if (edges.size() > 0) {
        return true;
    }
    // route
    if (route) {
        return true;
    }
    // edge
    if (edgeStop) {
        return true;
    }
    return false;
}


GNEEdge*
GNERouteHandler::PersonPlansValues::getLastEdge() const {
    if (fromEdge) {
        return fromEdge;
    } else if (toEdge) {
        return toEdge;
    } else if (fromBusStop) {
        return fromBusStop->getParentLanes().front()->getParentEdge();
    } else if (toBusStop) {
        return toBusStop->getParentLanes().front()->getParentEdge();
    } else if (route) {
        return route->getParentEdges().back();
    } else if (edges.size() > 0) {
        return edges.back();
    } else if (edgeStop) {
        return edgeStop;
    } else {
        return nullptr;
    }
}


bool
GNERouteHandler::PersonValue::addPersonValue(GNENet* net, SumoXMLTag tag, const SUMOSAXAttributes& attrs) {
    // change abort flag
    bool abort = false;
    // declare person plan values
    PersonPlansValues personPlansValuesLoaded;
    // set tag
    personPlansValuesLoaded.tag = tag;
    // get from edge
    if (attrs.hasAttribute(SUMO_ATTR_FROM)) {
        // parse edge ID
        const std::string edgeStr = attrs.get<std::string>(SUMO_ATTR_FROM, "", abort, false);
        if (abort) {
            return false;
        } else {
            // retrieve edge
            personPlansValuesLoaded.fromEdge = net->retrieveEdge(edgeStr, false);
            // check if edge is valid
            if (personPlansValuesLoaded.fromEdge == nullptr) {
                return false;
            }
        }
    }
    // get to edge
    if (attrs.hasAttribute(SUMO_ATTR_TO)) {
        // parse edge ID
        const std::string edgeStr = attrs.get<std::string>(SUMO_ATTR_TO, "", abort, false);
        if (abort) {
            return false;
        } else {
            // retrieve edge
            personPlansValuesLoaded.toEdge = net->retrieveEdge(edgeStr, false);
            // check if edge is valid
            if (personPlansValuesLoaded.toEdge == nullptr) {
                return false;
            }
        }
    }
    // get to busStop
    if (attrs.hasAttribute(SUMO_ATTR_BUS_STOP)) {
        // parse busStop ID
        const std::string busStopStr = attrs.get<std::string>(SUMO_ATTR_BUS_STOP, "", abort, false);
        if (abort) {
            return false;
        } else {
            // retrieve busStop
            personPlansValuesLoaded.toBusStop = net->retrieveAdditional(SUMO_TAG_BUS_STOP, busStopStr, false);
            // check if busStop is valid
            if (personPlansValuesLoaded.toBusStop == nullptr) {
                return false;
            }
        }
    }
    // get edges
    if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
        // parse edges
        const std::string edgeIDs = attrs.get<std::string>(SUMO_ATTR_EDGES, "", abort, false);
        if (!abort && GNEAttributeCarrier::canParse<std::vector<GNEEdge*> >(net, edgeIDs, true)) {
            personPlansValuesLoaded.edges = GNEAttributeCarrier::parse<std::vector<GNEEdge*> >(net, edgeIDs);
            // check that edges aren't empty
            if (personPlansValuesLoaded.edges.empty()) {
                return false;
            }
        } else {
            return false;
        }
    }
    // get to route
    if (attrs.hasAttribute(SUMO_ATTR_ROUTE)) {
        // parse route ID
        const std::string routeStr = attrs.get<std::string>(SUMO_ATTR_ROUTE, "", abort, false);
        if (abort) {
            return false;
        } else {
            // retrieve route
            personPlansValuesLoaded.route = net->retrieveDemandElement(SUMO_TAG_ROUTE, routeStr, false);
            // check if route is valid
            if (personPlansValuesLoaded.route == nullptr) {
                return false;
            }
        }
    }
    // get vTypes
    if (attrs.hasAttribute(SUMO_ATTR_VTYPES)) {
        const std::string vTypes = attrs.get<std::string>(SUMO_ATTR_VTYPES, "", abort, false);
        if (!abort) {
            personPlansValuesLoaded.vTypes = GNEAttributeCarrier::parse<std::vector<std::string> >(vTypes);
        }
    }
    // get vTypes
    if (attrs.hasAttribute(SUMO_ATTR_VTYPES)) {
        const std::string vTypes = attrs.get<std::string>(SUMO_ATTR_VTYPES, "", abort, false);
        if (!abort) {
            personPlansValuesLoaded.vTypes = GNEAttributeCarrier::parse<std::vector<std::string> >(vTypes);
        }
    }
    // get modes
    if (attrs.hasAttribute(SUMO_ATTR_MODES)) {
        const std::string modes = attrs.get<std::string>(SUMO_ATTR_MODES, "", abort, false);
        if (!abort) {
            personPlansValuesLoaded.modes = GNEAttributeCarrier::parse<std::vector<std::string> >(modes);
            /* check modes */
        }
    }
    // get lines
    if (attrs.hasAttribute(SUMO_ATTR_LINES)) {
        const std::string lines = attrs.get<std::string>(SUMO_ATTR_LINES, "", abort, false);
        if (!abort) {
            personPlansValuesLoaded.lines = GNEAttributeCarrier::parse<std::vector<std::string> >(lines);
            /* check modes */
        }
    }
    // get arrival position
    if (attrs.hasAttribute(SUMO_ATTR_ARRIVALPOS)) {
        const std::string arrivalPosStr = attrs.get<std::string>(SUMO_ATTR_ARRIVALPOS, "", abort, false);
        if (!abort && GNEAttributeCarrier::canParse<double>(arrivalPosStr)) {
            personPlansValuesLoaded.arrivalPos = GNEAttributeCarrier::parse<double>(arrivalPosStr);
        }
    }
    // get lane stop
    if (attrs.hasAttribute(SUMO_ATTR_LANE)) {
        // parse lane ID
        const std::string laneStr = attrs.get<std::string>(SUMO_ATTR_LANE, "", abort, false);
        if (abort) {
            return false;
        } else {
            // retrieve lane
            const GNELane* lane = net->retrieveLane(laneStr, false);
            if (lane) {
                // get parent edge
                personPlansValuesLoaded.edgeStop = lane->getParentEdge();
            } else {
                return false;
            }
        }
    }
    // add personPlansValuesLoaded in myPersonPlanValues
    myPersonPlanValues.push_back(personPlansValuesLoaded);
    return true;
}


bool
GNERouteHandler::PersonValue::checkPersonPlanValues() {
    if (myPersonPlanValues.empty()) {
        return false;
    } else {
        // check first element
        if (!myPersonPlanValues.front().isFirstPersonPlan()) {
            return false;
        }
        // update tag of first element
        myPersonPlanValues.front().updateGNETag();
        // set tags and update person plan values
        for (int i = 1; i < (int)myPersonPlanValues.size(); i++) {
            // get current and previous plan
            PersonPlansValues& previousPlan = myPersonPlanValues.at(i - 1);
            PersonPlansValues& currentPlan = myPersonPlanValues.at(i);
            // set previous element in current plan
            if (previousPlan.toEdge) {
                currentPlan.fromEdge = previousPlan.toEdge;
            } else if (previousPlan.toBusStop) {
                currentPlan.fromBusStop = previousPlan.toBusStop;
            } else if (previousPlan.edges.size() > 0) {
                currentPlan.fromEdge = previousPlan.edges.back();
            } else if (previousPlan.route) {
                currentPlan.fromEdge = previousPlan.route->getParentEdges().back();
            } else if (previousPlan.edgeStop) {
                currentPlan.fromEdge = previousPlan.edgeStop;
            } else if (previousPlan.lane) {
                currentPlan.fromEdge = previousPlan.lane->getParentEdge();
            } else if (previousPlan.busStop) {
                currentPlan.fromBusStop = previousPlan.busStop;
            }
            // update GNETag
            currentPlan.updateGNETag();
        }
        return 1;
    }
}

/****************************************************************************/
