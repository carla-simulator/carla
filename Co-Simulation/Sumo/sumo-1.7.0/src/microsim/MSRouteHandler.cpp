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
/// @file    MSRouteHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Parser and container for routes during their loading
/****************************************************************************/
#include <config.h>

#include "MSRouteHandler.h"
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/transportables/MSStageDriving.h>
#include <microsim/transportables/MSStageTranship.h>
#include <microsim/MSEdge.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/MSVehicleControl.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>

#define JUNCTION_TAZ_MISSING_HELP "\nSet option '--junction-taz' or load a TAZ-file"


// ===========================================================================
// static members
// ===========================================================================
std::mt19937 MSRouteHandler::myParsingRNG;


// ===========================================================================
// method definitions
// ===========================================================================
MSRouteHandler::MSRouteHandler(const std::string& file, bool addVehiclesDirectly) :
    SUMORouteHandler(file, addVehiclesDirectly ? "" : "routes", true),
    myActiveRouteRepeat(0),
    myActiveRoutePeriod(0),
    myActivePlan(nullptr),
    myActiveContainerPlan(nullptr),
    myAddVehiclesDirectly(addVehiclesDirectly),
    myCurrentVTypeDistribution(nullptr),
    myCurrentRouteDistribution(nullptr),
    myAmLoadingState(false) {
    myActiveRoute.reserve(100);
}


MSRouteHandler::~MSRouteHandler() {}


void
MSRouteHandler::deleteActivePlans() {
    MSTransportable::MSTransportablePlan::iterator i;
    if (myActivePlan != nullptr) {
        for (i = myActivePlan->begin(); i != myActivePlan->end(); i++) {
            delete *i;
        }
        delete myActivePlan;
        myActivePlan = nullptr;
    }
    if (myActiveContainerPlan != nullptr) {
        for (i = myActiveContainerPlan->begin(); i != myActiveContainerPlan->end(); i++) {
            delete *i;
        }
        delete myActiveContainerPlan;
        myActivePlan = nullptr;
    }
}


void
MSRouteHandler::parseFromViaTo(SumoXMLTag tag, const SUMOSAXAttributes& attrs) {
    const std::string element = toString(tag);
    myActiveRoute.clear();
    bool useTaz = OptionsCont::getOptions().getBool("with-taz");
    if (useTaz && !myVehicleParameter->wasSet(VEHPARS_FROM_TAZ_SET) && !myVehicleParameter->wasSet(VEHPARS_TO_TAZ_SET)) {
        WRITE_WARNING("Taz usage was requested but no taz present in " + element + " '" + myVehicleParameter->id + "'!");
        useTaz = false;
    }
    bool ok = true;
    // from-attributes
    if ((useTaz || !attrs.hasAttribute(SUMO_ATTR_FROM)) &&
            (attrs.hasAttribute(SUMO_ATTR_FROM_TAZ) || attrs.hasAttribute(SUMO_ATTR_FROMJUNCTION))) {
        const bool useJunction = attrs.hasAttribute(SUMO_ATTR_FROMJUNCTION);
        const std::string tazType = useJunction ? "junction" : "taz";
        const std::string tazID = attrs.get<std::string>(useJunction ? SUMO_ATTR_FROMJUNCTION : SUMO_ATTR_FROM_TAZ, myVehicleParameter->id.c_str(), ok, true);
        const MSEdge* fromTaz = MSEdge::dictionary(tazID + "-source");
        if (fromTaz == nullptr) {
            throw ProcessError("Source " + tazType + " '" + tazID + "' not known for " + element + " '" + myVehicleParameter->id + "'!"
                               + (useJunction ? JUNCTION_TAZ_MISSING_HELP : ""));
        } else if (fromTaz->getNumSuccessors() == 0 && tag != SUMO_TAG_PERSON) {
            throw ProcessError("Source " + tazType + " '" + tazID + "' has no outgoing edges for " + element + " '" + myVehicleParameter->id + "'!");
        } else {
            myActiveRoute.push_back(fromTaz);
        }
    } else {
        MSEdge::parseEdgesList(attrs.getOpt<std::string>(SUMO_ATTR_FROM, myVehicleParameter->id.c_str(), ok, "", true),
                               myActiveRoute, "for " + element + " '" + myVehicleParameter->id + "'");
    }

    // via-attributes
    if (!attrs.hasAttribute(SUMO_ATTR_VIA) && !attrs.hasAttribute(SUMO_ATTR_ROUTE)) {
        myInsertStopEdgesAt = (int)myActiveRoute.size();
    }
    ConstMSEdgeVector viaEdges;
    if (attrs.hasAttribute(SUMO_ATTR_VIAJUNCTIONS)) {
        for (std::string junctionID : attrs.getStringVector(SUMO_ATTR_VIAJUNCTIONS)) {
            const MSEdge* viaSink = MSEdge::dictionary(junctionID + "-sink");
            if (viaSink == nullptr) {
                throw ProcessError("Junction-taz '" + junctionID + "' not found." + JUNCTION_TAZ_MISSING_HELP);
            } else {
                viaEdges.push_back(viaSink);
            }
        }
    } else {
        MSEdge::parseEdgesList(attrs.getOpt<std::string>(SUMO_ATTR_VIA, myVehicleParameter->id.c_str(), ok, "", true),
                               viaEdges, "for " + element + " '" + myVehicleParameter->id + "'");
    }
    for (const MSEdge* e : viaEdges) {
        myActiveRoute.push_back(e);
        myVehicleParameter->via.push_back(e->getID());
    }

    // to-attributes
    if ((useTaz || !attrs.hasAttribute(SUMO_ATTR_TO)) &&
            (attrs.hasAttribute(SUMO_ATTR_TO_TAZ) || attrs.hasAttribute(SUMO_ATTR_TOJUNCTION))) {
        const bool useJunction = attrs.hasAttribute(SUMO_ATTR_TOJUNCTION);
        const std::string tazType = useJunction ? "junction" : "taz";
        const std::string tazID = attrs.get<std::string>(useJunction ? SUMO_ATTR_TOJUNCTION : SUMO_ATTR_TO_TAZ, myVehicleParameter->id.c_str(), ok, true);
        const MSEdge* toTaz = MSEdge::dictionary(tazID + "-sink");
        if (toTaz == nullptr) {
            throw ProcessError("Sink " + tazType + " '" + tazID + "' not known for " + element + " '" + myVehicleParameter->id + "'!"
                               + (useJunction ? JUNCTION_TAZ_MISSING_HELP : ""));
        } else if (toTaz->getNumPredecessors() == 0 && tag != SUMO_TAG_PERSON) {
            throw ProcessError("Sink " + tazType + " '" + tazID + "' has no incoming edges for " + element + " '" + myVehicleParameter->id + "'!");
        } else {
            myActiveRoute.push_back(toTaz);
        }
    } else {
        MSEdge::parseEdgesList(attrs.getOpt<std::string>(SUMO_ATTR_TO, myVehicleParameter->id.c_str(), ok, "", true),
                               myActiveRoute, "for " + element + " '" + myVehicleParameter->id + "'");
    }
    myActiveRouteID = "!" + myVehicleParameter->id;
    if (myVehicleParameter->routeid == "") {
        myVehicleParameter->routeid = myActiveRouteID;
    }
}


void
MSRouteHandler::myStartElement(int element,
                               const SUMOSAXAttributes& attrs) {
    if (myActivePlan != nullptr && myActivePlan->empty() && myVehicleParameter->departProcedure == DEPART_TRIGGERED
            && (element == SUMO_TAG_WALK || element == SUMO_TAG_PERSONTRIP || element == SUMO_TAG_STOP)) {
        throw ProcessError("Triggered departure for person '" + myVehicleParameter->id + "' requires starting with a ride.");
    }
    SUMORouteHandler::myStartElement(element, attrs);
    try {
        switch (element) {
            case SUMO_TAG_PERSON:
            case SUMO_TAG_PERSONFLOW:
                if (!MSNet::getInstance()->getVehicleControl().hasVType(myVehicleParameter->vtypeid)) {
                    const std::string error = "The type '" + myVehicleParameter->vtypeid + "' for person '" + myVehicleParameter->id + "' is not known.";
                    delete myVehicleParameter;
                    myVehicleParameter = nullptr;
                    throw ProcessError(error);
                }
                myActivePlan = new MSTransportable::MSTransportablePlan();
                break;
            case SUMO_TAG_CONTAINER:
                myActiveContainerPlan = new MSTransportable::MSTransportablePlan();
                break;
            case SUMO_TAG_RIDE: {
                const std::string pid = myVehicleParameter->id;
                bool ok = true;
                MSEdge* from = nullptr;
                const std::string desc = attrs.get<std::string>(SUMO_ATTR_LINES, pid.c_str(), ok);
                StringTokenizer st(desc);
                std::string bsID = attrs.getOpt<std::string>(SUMO_ATTR_BUS_STOP, nullptr, ok, "");
                MSStoppingPlace* bs = nullptr;
                MSEdge* to = nullptr;
                if (bsID != "") {
                    bs = MSNet::getInstance()->getStoppingPlace(bsID, SUMO_TAG_BUS_STOP);
                    if (bs == nullptr) {
                        throw ProcessError("Unknown bus stop '" + bsID + "' for person '" + myVehicleParameter->id + "'.");
                    }
                    to = &bs->getLane().getEdge();
                }
                double arrivalPos = attrs.getOpt<double>(SUMO_ATTR_ARRIVALPOS, myVehicleParameter->id.c_str(), ok,
                                    bs == nullptr ? std::numeric_limits<double>::infinity() : bs->getEndLanePosition());

                SUMOVehicle* startVeh = nullptr;
                if (myActivePlan->empty() && myVehicleParameter->departProcedure == DEPART_TRIGGERED) {
                    if (st.size() != 1) {
                        throw ProcessError("Triggered departure for person '" + pid + "' requires a unique lines value.");
                    }
                    // person starts
                    MSVehicleControl& vehControl = MSNet::getInstance()->getVehicleControl();
                    const std::string vehID = st.front();
                    startVeh = vehControl.getVehicle(vehID);
                    if (startVeh == nullptr) {
                        throw ProcessError("Unknown vehicle '" + vehID + "' in triggered departure for person '" + pid + "'.");
                    }
                    if (startVeh->getParameter().departProcedure == DEPART_TRIGGERED) {
                        throw ProcessError("Cannot use triggered vehicle '" + vehID + "' in triggered departure for person '" + pid + "'.");
                    }
                    myVehicleParameter->depart = startVeh->getParameter().depart;
                }

                if (attrs.hasAttribute(SUMO_ATTR_FROM)) {
                    const std::string fromID = attrs.get<std::string>(SUMO_ATTR_FROM, pid.c_str(), ok);
                    from = MSEdge::dictionary(fromID);
                    if (from == nullptr) {
                        throw ProcessError("The from edge '" + fromID + "' within a ride of person '" + pid + "' is not known.");
                    }
                    if (!myActivePlan->empty() && myActivePlan->back()->getDestination() != from) {
                        if (myActivePlan->back()->getDestinationStop() == nullptr || &myActivePlan->back()->getDestinationStop()->getLane().getEdge() != from) {
                            throw ProcessError("Disconnected plan for person '" + myVehicleParameter->id +
                                               "' (edge '" + fromID + "' != edge '" + myActivePlan->back()->getDestination()->getID() + "').");
                        }
                    }
                    if (startVeh != nullptr && startVeh->getRoute().getEdges().front() != from) {
                        throw ProcessError("Disconnected plan for triggered person '" + pid +
                                           "' (edge '" + fromID + "' != edge '" + startVeh->getRoute().getEdges().front()->getID() + "').");
                    }
                    if (myActivePlan->empty()) {
                        myActivePlan->push_back(new MSStageWaiting(
                                                    from, nullptr, -1, myVehicleParameter->depart, myVehicleParameter->departPos, "start", true));
                    }
                } else if (myActivePlan->empty()) {
                    throw ProcessError("The start edge for person '" + pid + "' is not known.");
                }
                if (to == nullptr) {
                    const std::string toID = attrs.get<std::string>(SUMO_ATTR_TO, pid.c_str(), ok);
                    to = MSEdge::dictionary(toID);
                    if (to == nullptr) {
                        throw ProcessError("The to edge '" + toID + "' within a ride of person '" + pid + "' is not known.");
                    }
                }
                const std::string group = attrs.getOpt<std::string>(SUMO_ATTR_GROUP, pid.c_str(), ok, pid);
                const std::string intendedVeh = attrs.getOpt<std::string>(SUMO_ATTR_INTENDED, nullptr, ok, "");
                const SUMOTime intendedDepart = attrs.getOptSUMOTimeReporting(SUMO_ATTR_DEPART, nullptr, ok, -1);
                arrivalPos = SUMOVehicleParameter::interpretEdgePos(arrivalPos, to->getLength(), SUMO_ATTR_ARRIVALPOS, "person '" + pid + "' riding to edge '" + to->getID() + "'");
                myActivePlan->push_back(new MSStageDriving(to, bs, arrivalPos, st.getVector(), group, intendedVeh, intendedDepart));
                break;
            }
            case SUMO_TAG_TRANSPORT:
                try {
                    const std::string containerId = myVehicleParameter->id;
                    bool ok = true;
                    MSEdge* from = nullptr;
                    const std::string desc = attrs.get<std::string>(SUMO_ATTR_LINES, containerId.c_str(), ok);
                    StringTokenizer st(desc);
                    std::string csID = attrs.getOpt<std::string>(SUMO_ATTR_CONTAINER_STOP, nullptr, ok, "");
                    MSStoppingPlace* cs = nullptr;
                    if (csID != "") {
                        cs = MSNet::getInstance()->getStoppingPlace(csID, SUMO_TAG_CONTAINER_STOP);
                        if (cs == nullptr) {
                            throw ProcessError("Unknown container stop '" + csID + "' for container '" + myVehicleParameter->id + "'.");
                        }
                    }
                    double arrivalPos = attrs.getOpt<double>(SUMO_ATTR_ARRIVALPOS, myVehicleParameter->id.c_str(), ok,
                                        cs == nullptr ? std::numeric_limits<double>::infinity() : cs->getEndLanePosition());
                    if (attrs.hasAttribute(SUMO_ATTR_FROM)) {
                        const std::string fromID = attrs.get<std::string>(SUMO_ATTR_FROM, containerId.c_str(), ok);
                        from = MSEdge::dictionary(fromID);
                        if (from == nullptr) {
                            throw ProcessError("The from edge '" + fromID + "' within a transport of container '" + containerId + "' is not known.");
                        }
                        if (!myActiveContainerPlan->empty() && myActiveContainerPlan->back()->getDestination() != from) {
                            throw ProcessError("Disconnected plan for container '" + myVehicleParameter->id + "' (" + fromID + "!=" + myActiveContainerPlan->back()->getDestination()->getID() + ").");
                        }
                        if (myActiveContainerPlan->empty()) {
                            myActiveContainerPlan->push_back(new MSStageWaiting(
                                                                 from, nullptr, -1, myVehicleParameter->depart, myVehicleParameter->departPos, "start", true));
                        }
                    } else if (myActiveContainerPlan->empty()) {
                        throw ProcessError("The start edge within a transport of container '" + containerId + "' is not known.");
                    }
                    const std::string toID = attrs.get<std::string>(SUMO_ATTR_TO, containerId.c_str(), ok);
                    MSEdge* to = MSEdge::dictionary(toID);
                    if (to == nullptr) {
                        throw ProcessError("The to edge '" + toID + "' within a transport of container '" + containerId + "' is not known.");
                    }
                    arrivalPos = SUMOVehicleParameter::interpretEdgePos(arrivalPos, to->getLength(), SUMO_ATTR_ARRIVALPOS, "transport of container '" + containerId + "' to edge '" + to->getID() + "'");
                    myActiveContainerPlan->push_back(new MSStageDriving(to, cs, arrivalPos, st.getVector()));
                } catch (ProcessError&) {
                    deleteActivePlans();
                    throw;
                }
                break;
            case SUMO_TAG_TRANSHIP: {
                myActiveRoute.clear();
                bool ok = true;
                double departPos = attrs.getOpt<double>(SUMO_ATTR_DEPARTPOS, myVehicleParameter->id.c_str(), ok, 0);
                double arrivalPos = attrs.getOpt<double>(SUMO_ATTR_ARRIVALPOS, myVehicleParameter->id.c_str(), ok, -NUMERICAL_EPS);
                double speed = DEFAULT_CONTAINER_TRANSHIP_SPEED;
                const MSVehicleType* vtype = MSNet::getInstance()->getVehicleControl().getVType(myVehicleParameter->vtypeid);
                // need to check for explicitly set speed since we might have // DEFAULT_VEHTYPE
                if (vtype != nullptr && vtype->wasSet(VTYPEPARS_MAXSPEED_SET)) {
                    speed = vtype->getMaxSpeed();
                }
                speed = attrs.getOpt<double>(SUMO_ATTR_SPEED, nullptr, ok, speed);
                if (speed <= 0) {
                    throw ProcessError("Non-positive tranship speed for container  '" + myVehicleParameter->id + "'.");
                }
                std::string csID = attrs.getOpt<std::string>(SUMO_ATTR_CONTAINER_STOP, nullptr, ok, "");
                MSStoppingPlace* cs = nullptr;
                if (csID != "") {
                    cs = MSNet::getInstance()->getStoppingPlace(csID, SUMO_TAG_CONTAINER_STOP);
                    if (cs == nullptr) {
                        throw ProcessError("Unknown container stop '" + csID + "' for container '" + myVehicleParameter->id + "'.");
                    }
                    arrivalPos = cs->getEndLanePosition();
                }
                if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
                    MSEdge::parseEdgesList(attrs.get<std::string>(SUMO_ATTR_EDGES, myVehicleParameter->id.c_str(), ok), myActiveRoute, myActiveRouteID);
                } else {
                    if (attrs.hasAttribute(SUMO_ATTR_FROM) && attrs.hasAttribute(SUMO_ATTR_TO)) {
                        const std::string fromID = attrs.get<std::string>(SUMO_ATTR_FROM, myVehicleParameter->id.c_str(), ok);
                        MSEdge* from = MSEdge::dictionary(fromID);
                        if (from == nullptr) {
                            throw ProcessError("The from edge '" + fromID + "' within a tranship of container '" + myVehicleParameter->id + "' is not known.");
                        }
                        const std::string toID = attrs.get<std::string>(SUMO_ATTR_TO, myVehicleParameter->id.c_str(), ok);
                        MSEdge* to = MSEdge::dictionary(toID);
                        if (to == nullptr) {
                            throw ProcessError("The to edge '" + toID + "' within a tranship of container '" + myVehicleParameter->id + "' is not known.");
                        }
                        //the route of the container's tranship stage consists only of the 'from' and the 'to' edge
                        myActiveRoute.push_back(from);
                        myActiveRoute.push_back(to);
                        if (myActiveRoute.empty()) {
                            const std::string error = "No connection found between '" + from->getID() + "' and '" + to->getID() + "' for container '" + myVehicleParameter->id + "'.";
                            if (!MSGlobals::gCheckRoutes) {
                                myActiveRoute.push_back(from);
                            } else {
                                WRITE_ERROR(error);
                            }
                        }
                    }
                }
                if (myActiveRoute.empty()) {
                    throw ProcessError("No edges to tranship container '" + myVehicleParameter->id + "'.");
                }
                if (!myActiveContainerPlan->empty() && myActiveContainerPlan->back()->getDestination() != myActiveRoute.front()) {
                    throw ProcessError("Disconnected plan for container '" + myVehicleParameter->id + "' (" + myActiveRoute.front()->getID() + "!=" + myActiveContainerPlan->back()->getDestination()->getID() + ").");
                }
                if (myActiveContainerPlan->empty()) {
                    myActiveContainerPlan->push_back(new MSStageWaiting(
                                                         myActiveRoute.front(), nullptr, -1, myVehicleParameter->depart, departPos, "start", true));
                }
                myActiveContainerPlan->push_back(new MSStageTranship(myActiveRoute, cs, speed, departPos, arrivalPos));
                myActiveRoute.clear();
                break;
            }
            case SUMO_TAG_FLOW:
                parseFromViaTo((SumoXMLTag)element, attrs);
                break;
            case SUMO_TAG_TRIP:
                parseFromViaTo((SumoXMLTag)element, attrs);
                break;
            default:
                break;
        }
    } catch (ProcessError&) {
        delete myVehicleParameter;
        myVehicleParameter = nullptr;
        throw;
    }
}


void
MSRouteHandler::openVehicleTypeDistribution(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    myCurrentVTypeDistributionID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (ok) {
        myCurrentVTypeDistribution = new RandomDistributor<MSVehicleType*>();
        if (attrs.hasAttribute(SUMO_ATTR_VTYPES)) {
            const std::string vTypes = attrs.get<std::string>(SUMO_ATTR_VTYPES, myCurrentVTypeDistributionID.c_str(), ok);
            StringTokenizer st(vTypes);
            while (st.hasNext()) {
                std::string vtypeID = st.next();
                MSVehicleType* type = MSNet::getInstance()->getVehicleControl().getVType(vtypeID, &myParsingRNG);
                if (type == nullptr) {
                    throw ProcessError("Unknown vtype '" + vtypeID + "' in distribution '" + myCurrentVTypeDistributionID + "'.");
                }
                myCurrentVTypeDistribution->add(type, type->getDefaultProbability());
            }
        }
    }
}


void
MSRouteHandler::closeVehicleTypeDistribution() {
    if (myCurrentVTypeDistribution != nullptr) {
        if (MSGlobals::gStateLoaded && MSNet::getInstance()->getVehicleControl().hasVTypeDistribution(myCurrentVTypeDistributionID)) {
            delete myCurrentVTypeDistribution;
            return;
        }
        if (myCurrentVTypeDistribution->getOverallProb() == 0) {
            delete myCurrentVTypeDistribution;
            throw ProcessError("Vehicle type distribution '" + myCurrentVTypeDistributionID + "' is empty.");
        }
        if (!MSNet::getInstance()->getVehicleControl().addVTypeDistribution(myCurrentVTypeDistributionID, myCurrentVTypeDistribution)) {
            delete myCurrentVTypeDistribution;
            throw ProcessError("Another vehicle type (or distribution) with the id '" + myCurrentVTypeDistributionID + "' exists.");
        }
        myCurrentVTypeDistribution = nullptr;
    }
}


void
MSRouteHandler::openRoute(const SUMOSAXAttributes& attrs) {
    myActiveRoute.clear();
    myInsertStopEdgesAt = -1;
    // check whether the id is really necessary
    std::string rid;
    if (myCurrentRouteDistribution != nullptr) {
        myActiveRouteID = myCurrentRouteDistributionID + "#" + toString(myCurrentRouteDistribution->getProbs().size()); // !!! document this
        rid =  "distribution '" + myCurrentRouteDistributionID + "'";
    } else if (myVehicleParameter != nullptr) {
        // ok, a vehicle is wrapping the route,
        //  we may use this vehicle's id as default
        myActiveRouteID = "!" + myVehicleParameter->id; // !!! document this
        if (attrs.hasAttribute(SUMO_ATTR_ID)) {
            WRITE_WARNING("Ids of internal routes are ignored (vehicle '" + myVehicleParameter->id + "').");
        }
    } else {
        bool ok = true;
        myActiveRouteID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok, false);
        if (!ok) {
            return;
        }
        rid = "'" + myActiveRouteID + "'";
    }
    if (myVehicleParameter != nullptr) { // have to do this here for nested route distributions
        rid =  "for vehicle '" + myVehicleParameter->id + "'";
    }
    bool ok = true;
    if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
        MSEdge::parseEdgesList(attrs.get<std::string>(SUMO_ATTR_EDGES, myActiveRouteID.c_str(), ok), myActiveRoute, rid);
    }
    myActiveRouteRefID = attrs.getOpt<std::string>(SUMO_ATTR_REFID, myActiveRouteID.c_str(), ok, "");
    if (myActiveRouteRefID != "" && MSRoute::dictionary(myActiveRouteRefID, &myParsingRNG) == nullptr) {
        WRITE_ERROR("Invalid reference to route '" + myActiveRouteRefID + "' in route " + rid + ".");
    }
    myActiveRouteProbability = attrs.getOpt<double>(SUMO_ATTR_PROB, myActiveRouteID.c_str(), ok, DEFAULT_VEH_PROB);
    myActiveRouteColor = attrs.hasAttribute(SUMO_ATTR_COLOR) ? new RGBColor(attrs.get<RGBColor>(SUMO_ATTR_COLOR, myActiveRouteID.c_str(), ok)) : nullptr;
    myActiveRouteRepeat = attrs.getOpt<int>(SUMO_ATTR_REPEAT, myActiveRouteID.c_str(), ok, 0);
    myActiveRoutePeriod = attrs.getOptSUMOTimeReporting(SUMO_ATTR_CYCLETIME, myActiveRouteID.c_str(), ok,
                          // handle obsolete attribute name
                          attrs.getOptSUMOTimeReporting(SUMO_ATTR_PERIOD, myActiveRouteID.c_str(), ok, 0));
    if (attrs.hasAttribute(SUMO_ATTR_PERIOD)) {
        WRITE_WARNING("Attribute 'period' is deprecated for route. Use 'cycleTime' instead.");
    }
    if (myActiveRouteRepeat > 0) {
        if (MSGlobals::gCheckRoutes) {
            SUMOVehicleClass vClass = SVC_IGNORING;
            if (myVehicleParameter != nullptr) {
                MSVehicleControl& vehControl = MSNet::getInstance()->getVehicleControl();
                MSVehicleType* vtype = vehControl.getVType(myVehicleParameter->vtypeid, &myParsingRNG);
                if (vtype != nullptr) {
                    vClass = vtype->getVehicleClass();
                }
            }
            if (myActiveRoute.size() > 0 && !myActiveRoute.back()->isConnectedTo(*myActiveRoute.front(), vClass)) {
                WRITE_ERROR("Disconnected route " + rid + " when repeating.");
            }
        }
    }
    myCurrentCosts = attrs.getOpt<double>(SUMO_ATTR_COST, myActiveRouteID.c_str(), ok, -1);
    if (ok && myCurrentCosts != -1 && myCurrentCosts < 0) {
        WRITE_ERROR("Invalid cost for route '" + myActiveRouteID + "'.");
    }
}


void
MSRouteHandler::openFlow(const SUMOSAXAttributes& /*attrs*/) {
    // Currently unused
}


void
MSRouteHandler::openRouteFlow(const SUMOSAXAttributes& /*attrs*/) {
    // Currently unused
}


void
MSRouteHandler::openTrip(const SUMOSAXAttributes& /*attrs*/) {
    // Currently unsued
}


void
MSRouteHandler::closeRoute(const bool mayBeDisconnected) {
    std::string type = "vehicle";
    if (mayBeDisconnected) {
        if (myVehicleParameter->repetitionNumber >= 0) {
            type = "flow";
        } else {
            type = "trip";
        }
    }

    try {
        const bool mustReroute = myActiveRoute.size() == 0 && myActiveRouteStops.size() != 0;
        if (mustReroute) {
            // implicit route from stops
            for (const SUMOVehicleParameter::Stop& stop : myActiveRouteStops) {
                myActiveRoute.push_back(&MSLane::dictionary(stop.lane)->getEdge());
            }
        }
        if (myActiveRoute.size() == 0) {
            delete myActiveRouteColor;
            myActiveRouteColor = nullptr;
            if (myActiveRouteRefID != "" && myCurrentRouteDistribution != nullptr) {
                const MSRoute* route = MSRoute::dictionary(myActiveRouteRefID, &myParsingRNG);
                if (route != nullptr) {
                    if (myCurrentRouteDistribution->add(route, myActiveRouteProbability)) {
                        route->addReference();
                    }
                }
                myActiveRouteID = "";
                myActiveRouteRefID = "";
                return;
            }
            if (myVehicleParameter != nullptr) {
                throw ProcessError("The route for " + type + " '" + myVehicleParameter->id + "' has no edges.");
            } else {
                throw ProcessError("Route '" + myActiveRouteID + "' has no edges.");
            }
        }
        if (myActiveRoute.size() == 1 && myActiveRoute.front()->isTazConnector()) {
            throw ProcessError("The routing information for " + type + " '" + myVehicleParameter->id + "' is insufficient.");
        }
        if (myActiveRouteRepeat > 0) {
            // duplicate route
            ConstMSEdgeVector tmpEdges = myActiveRoute;
            auto tmpStops = myActiveRouteStops;
            for (int i = 0; i < myActiveRouteRepeat; i++) {
                myActiveRoute.insert(myActiveRoute.begin(), tmpEdges.begin(), tmpEdges.end());
                for (SUMOVehicleParameter::Stop stop : tmpStops) {
                    if (stop.until > 0) {
                        if (myActiveRoutePeriod <= 0) {
                            const std::string description = myVehicleParameter != nullptr
                                                            ?  "for " + type + " '" + myVehicleParameter->id + "'"
                                                            :  "'" + myActiveRouteID + "'";
                            throw ProcessError("Cannot repeat stops with 'until' in route " + description + " because no cycleTime is defined.");
                        }
                        stop.until += myActiveRoutePeriod * (i + 1);
                    }
                    if (stop.arrival > 0) {
                        if (myActiveRoutePeriod <= 0) {
                            const std::string description = myVehicleParameter != nullptr
                                                            ?  "for " + type + " '" + myVehicleParameter->id + "'"
                                                            :  "'" + myActiveRouteID + "'";
                            throw ProcessError("Cannot repeat stops with 'arrival' in route " + description + " because no cycleTime is defined.");
                        }
                        stop.arrival += myActiveRoutePeriod * (i + 1);
                    }
                    myActiveRouteStops.push_back(stop);
                }
            }
        }
        MSRoute* route = new MSRoute(myActiveRouteID, myActiveRoute,
                                     myVehicleParameter == nullptr || myVehicleParameter->repetitionNumber >= 1,
                                     myActiveRouteColor, myActiveRouteStops);
        route->setPeriod(myActiveRoutePeriod);
        route->setCosts(myCurrentCosts);
        route->setReroute(mustReroute);
        myActiveRoute.clear();
        if (!MSRoute::dictionary(myActiveRouteID, route)) {
            delete route;
            if (!MSGlobals::gStateLoaded) {
                if (myVehicleParameter != nullptr) {
                    if (MSNet::getInstance()->getVehicleControl().getVehicle(myVehicleParameter->id) == nullptr) {
                        throw ProcessError("Another route for " + type + " '" + myVehicleParameter->id + "' exists.");
                    } else {
                        throw ProcessError("A vehicle with id '" + myVehicleParameter->id + "' already exists.");
                    }
                } else {
                    throw ProcessError("Another route (or distribution) with the id '" + myActiveRouteID + "' exists.");
                }
            }
        } else {
            if (myCurrentRouteDistribution != nullptr) {
                if (myCurrentRouteDistribution->add(route, myActiveRouteProbability)) {
                    route->addReference();
                }
            }
        }
        myActiveRouteID = "";
        myActiveRouteColor = nullptr;
        myActiveRouteStops.clear();
    } catch (ProcessError&) {
        delete myVehicleParameter;
        throw;
    }
}


void
MSRouteHandler::openRouteDistribution(const SUMOSAXAttributes& attrs) {
    // check whether the id is really necessary
    if (myVehicleParameter != nullptr) {
        // ok, a vehicle is wrapping the route,
        //  we may use this vehicle's id as default
        myCurrentRouteDistributionID = "!" + myVehicleParameter->id; // !!! document this
    } else {
        bool ok = true;
        myCurrentRouteDistributionID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
        if (!ok) {
            return;
        }
    }
    myCurrentRouteDistribution = new RandomDistributor<const MSRoute*>();
    std::vector<double> probs;
    if (attrs.hasAttribute(SUMO_ATTR_PROBS)) {
        bool ok = true;
        StringTokenizer st(attrs.get<std::string>(SUMO_ATTR_PROBS, myCurrentRouteDistributionID.c_str(), ok));
        while (st.hasNext()) {
            probs.push_back(StringUtils::toDoubleSecure(st.next(), 1.0));
        }
    }
    if (attrs.hasAttribute(SUMO_ATTR_ROUTES)) {
        bool ok = true;
        StringTokenizer st(attrs.get<std::string>(SUMO_ATTR_ROUTES, myCurrentRouteDistributionID.c_str(), ok));
        int probIndex = 0;
        while (st.hasNext()) {
            std::string routeID = st.next();
            const MSRoute* route = MSRoute::dictionary(routeID, &myParsingRNG);
            if (route == nullptr) {
                throw ProcessError("Unknown route '" + routeID + "' in distribution '" + myCurrentRouteDistributionID + "'.");
            }
            const double prob = ((int)probs.size() > probIndex ? probs[probIndex] : 1.0);
            if (myCurrentRouteDistribution->add(route, prob, false)) {
                route->addReference();
            }
            probIndex++;
        }
        if (probs.size() > 0 && probIndex != (int)probs.size()) {
            WRITE_WARNING("Got " + toString(probs.size()) + " probabilities for " + toString(probIndex) +
                          " routes in routeDistribution '" + myCurrentRouteDistributionID + "'");
        }
    }
}


void
MSRouteHandler::closeRouteDistribution() {
    if (myCurrentRouteDistribution != nullptr) {
        const bool haveSameID = MSRoute::dictionary(myCurrentRouteDistributionID, &myParsingRNG) != nullptr;
        if (MSGlobals::gStateLoaded && haveSameID) {
            delete myCurrentRouteDistribution;
            return;
        }
        if (haveSameID) {
            delete myCurrentRouteDistribution;
            throw ProcessError("Another route (or distribution) with the id '" + myCurrentRouteDistributionID + "' exists.");
        }
        if (myCurrentRouteDistribution->getOverallProb() == 0) {
            delete myCurrentRouteDistribution;
            throw ProcessError("Route distribution '" + myCurrentRouteDistributionID + "' is empty.");
        }
        MSRoute::dictionary(myCurrentRouteDistributionID, myCurrentRouteDistribution, myVehicleParameter == nullptr);
        myCurrentRouteDistribution = nullptr;
    }
}


void
MSRouteHandler::closeVehicle() {
    // get nested route
    const MSRoute* route = MSRoute::dictionary("!" + myVehicleParameter->id, &myParsingRNG);
    MSVehicleControl& vehControl = MSNet::getInstance()->getVehicleControl();
    if (myVehicleParameter->departProcedure == DEPART_GIVEN) {
        // let's check whether this vehicle had to depart before the simulation starts
        if (!(myAddVehiclesDirectly || checkLastDepart()) || (myVehicleParameter->depart < string2time(OptionsCont::getOptions().getString("begin")) && !myAmLoadingState)) {
            if (route != nullptr) {
                route->addReference();
                route->release();
            }
            return;
        }
    }

    // get the vehicle's type
    MSVehicleType* vtype = nullptr;

    try {
        if (myVehicleParameter->vtypeid != "") {
            vtype = vehControl.getVType(myVehicleParameter->vtypeid, &myParsingRNG);
            if (vtype == nullptr) {
                throw ProcessError("The vehicle type '" + myVehicleParameter->vtypeid + "' for vehicle '" + myVehicleParameter->id + "' is not known.");
            }
            if (vtype->getVehicleClass() == SVC_PEDESTRIAN) {
                WRITE_WARNING("Vehicle type '" + vtype->getID() + "' with vClass=pedestrian should only be used for persons and not for vehicle '" + myVehicleParameter->id + "'.");
            }
        } else {
            // there should be one (at least the default one)
            vtype = vehControl.getVType(DEFAULT_VTYPE_ID, &myParsingRNG);
        }
        if (myVehicleParameter->wasSet(VEHPARS_ROUTE_SET)) {
            // if the route id was given, prefer that one
            if (route != nullptr && !myAmLoadingState) {
                WRITE_WARNING("Ignoring child element 'route' for vehicle '" + myVehicleParameter->id + "' because attribute 'route' is set.");
            }
            route = MSRoute::dictionary(myVehicleParameter->routeid, &myParsingRNG);
        }
        if (route == nullptr) {
            // nothing found? -> error
            if (myVehicleParameter->routeid != "") {
                throw ProcessError("The route '" + myVehicleParameter->routeid + "' for vehicle '" + myVehicleParameter->id + "' is not known.");
            } else {
                throw ProcessError("Vehicle '" + myVehicleParameter->id + "' has no route.");
            }
        }
        myActiveRouteID = "";

    } catch (ProcessError&) {
        delete myVehicleParameter;
        throw;
    }
    if (route->mustReroute()) {
        myVehicleParameter->parametersSet |= VEHPARS_FORCE_REROUTE;
        if (myVehicleParameter->stops.size() > 0) {
            route = addVehicleStopsToImplicitRoute(route, false);
        }
    }

    // try to build the vehicle
    SUMOVehicle* vehicle = nullptr;
    if (vehControl.getVehicle(myVehicleParameter->id) == nullptr) {
        try {
            vehicle = vehControl.buildVehicle(myVehicleParameter, route, vtype, !MSGlobals::gCheckRoutes);
        } catch (const ProcessError& e) {
            if (!MSGlobals::gCheckRoutes) {
                WRITE_WARNING(e.what());
                vehControl.fixVehicleCounts();
                myVehicleParameter = nullptr;
                vehicle = nullptr;
                return;
            } else {
                throw e;
            }
        }
        const SUMOTime origDepart = myVehicleParameter->depart;
        // maybe we do not want this vehicle to be inserted due to scaling
        int quota = myAmLoadingState ? 1 : vehControl.getQuota();
        if (quota > 0) {
            registerLastDepart();
            myVehicleParameter->depart += MSNet::getInstance()->getInsertionControl().computeRandomDepartOffset();
            vehControl.addVehicle(myVehicleParameter->id, vehicle);
            for (int i = 1; i < quota; i++) {
                if (vehicle->getParameter().departProcedure == DEPART_GIVEN) {
                    MSNet::getInstance()->getInsertionControl().add(vehicle);
                }
                SUMOVehicleParameter* newPars = new SUMOVehicleParameter(*myVehicleParameter);
                newPars->id = myVehicleParameter->id + "." + toString(i);
                newPars->depart = origDepart + MSNet::getInstance()->getInsertionControl().computeRandomDepartOffset();
                vehicle = vehControl.buildVehicle(newPars, route, vtype, !MSGlobals::gCheckRoutes);
                vehControl.addVehicle(newPars->id, vehicle);
            }
            myVehicleParameter = nullptr;
        } else {
            vehControl.deleteVehicle(vehicle, true);
            myVehicleParameter = nullptr;
            vehicle = nullptr;
        }
    } else {
        // strange: another vehicle with the same id already exists
        if (!MSGlobals::gStateLoaded) {
            // and was not loaded while loading a simulation state
            // -> error
            std::string veh_id = myVehicleParameter->id;
            delete myVehicleParameter;
            myVehicleParameter = nullptr;
            throw ProcessError("Another vehicle with the id '" + veh_id + "' exists.");
        } else {
            // ok, it seems to be loaded previously while loading a simulation state
            vehicle = nullptr;
        }
    }
    // check whether the vehicle shall be added directly to the network or
    //  shall stay in the internal buffer
    if (vehicle != nullptr) {
        if (vehicle->getParameter().departProcedure == DEPART_GIVEN) {
            MSNet::getInstance()->getInsertionControl().add(vehicle);
        }
    }
}


MSRoute*
MSRouteHandler::addVehicleStopsToImplicitRoute(const MSRoute* route, bool isPermanent) {
    // the route was defined without edges and its current edges were
    // derived from route-stops.
    // We may need to add additional edges for the vehicle-stops
    assert(myVehicleParameter->wasSet(VEHPARS_ROUTE_SET));
    assert(route->getStops().size() > 0);
    ConstMSEdgeVector edges = route->getEdges();
    for (SUMOVehicleParameter::Stop stop : myVehicleParameter->stops) {
        MSEdge* stopEdge = &MSLane::dictionary(stop.lane)->getEdge();
        if (stop.index == 0) {
            if (edges.front() != stopEdge ||
                    route->getStops().front().endPos < stop.endPos) {
                edges.insert(edges.begin(), stopEdge);
            }
        } else if (stop.index == STOP_INDEX_END) {
            if (edges.back() != stopEdge ||
                    route->getStops().back().endPos > stop.endPos) {
                edges.push_back(stopEdge);
            }
        } else {
            WRITE_WARNING("Could not merge vehicle stops for vehicle '" + myVehicleParameter->id + "' into implicitly defined route '" + route->getID() + "'");
        }
    }
    MSRoute* newRoute = new MSRoute("!" + myVehicleParameter->id, edges,
                                    isPermanent, new RGBColor(route->getColor()), route->getStops());
    if (!MSRoute::dictionary(newRoute->getID(), newRoute)) {
        delete newRoute;
        throw ProcessError("Could not adapt implicit route for " + std::string(isPermanent ? "flow" : "vehicle") + "  '" + myVehicleParameter->id + "'");
    }
    return newRoute;
}


void
MSRouteHandler::closePerson() {
    if (myActivePlan->size() == 0) {
        const std::string error = "Person '" + myVehicleParameter->id + "' has no plan.";
        delete myVehicleParameter;
        myVehicleParameter = nullptr;
        deleteActivePlans();
        throw ProcessError(error);
    }
    // let's check whether this person had to depart before the simulation starts
    if (!(myAddVehiclesDirectly || checkLastDepart())
            || (myVehicleParameter->depart < string2time(OptionsCont::getOptions().getString("begin")) && !myAmLoadingState)) {
        delete myVehicleParameter;
        myVehicleParameter = nullptr;
        deleteActivePlans();
        return;
    }
    // type existence has been checked on opening
    MSVehicleType* type = MSNet::getInstance()->getVehicleControl().getVType(myVehicleParameter->vtypeid, &myParsingRNG);
    addFlowPerson(myVehicleParameter->depart, type, myVehicleParameter->id, -1);
    registerLastDepart();
    myVehicleParameter = nullptr;
    myActivePlan = nullptr;
}


void
MSRouteHandler::closePersonFlow() {
    if (myActivePlan->size() == 0) {
        const std::string error = "personFlow '" + myVehicleParameter->id + "' has no plan.";
        delete myVehicleParameter;
        myVehicleParameter = nullptr;
        deleteActivePlans();
        throw ProcessError(error);
    }
    // let's check whether this person had to depart before the simulation starts
    if (!(myAddVehiclesDirectly || checkLastDepart())
            || (myVehicleParameter->depart < string2time(OptionsCont::getOptions().getString("begin")) && !myAmLoadingState)) {
        delete myVehicleParameter;
        myVehicleParameter = nullptr;
        deleteActivePlans();
        return;
    }
    // type existence has been checked on opening
    MSVehicleType* type = MSNet::getInstance()->getVehicleControl().getVType(myVehicleParameter->vtypeid, &myParsingRNG);
    // instantiate all persons of this flow
    int i = 0;
    registerLastDepart();
    std::string baseID = myVehicleParameter->id;
    if (myVehicleParameter->repetitionProbability > 0) {
        if (myVehicleParameter->repetitionEnd == SUMOTime_MAX) {
            throw ProcessError("probabilistic personFlow '" + myVehicleParameter->id + "' must specify end time");
        } else {
            for (SUMOTime t = myVehicleParameter->depart; t < myVehicleParameter->repetitionEnd; t += TIME2STEPS(1)) {
                if (RandHelper::rand(&myParsingRNG) < myVehicleParameter->repetitionProbability) {
                    addFlowPerson(t, type, baseID, i++);
                }
            }
        }
    } else {
        SUMOTime depart = myVehicleParameter->depart;
        for (; i < myVehicleParameter->repetitionNumber; i++) {
            addFlowPerson(depart, type, baseID, i);
            depart += myVehicleParameter->repetitionOffset;
        }
    }

    myVehicleParameter = nullptr;
    myActivePlan = nullptr;
}


void
MSRouteHandler::addFlowPerson(SUMOTime depart, MSVehicleType* type, const std::string& baseID, int i) {
    MSTransportableControl& pc = MSNet::getInstance()->getPersonControl();
    const int quota = MSNet::getInstance()->getVehicleControl().getQuota(-1, pc.getLoadedNumber());
    if (quota == 0) {
        pc.addDiscarded();
    }
    for (int j = 0; j < quota; j++) {
        if (i > 0 || j > 0) {
            // copy parameter and plan because the person takes over responsibility
            SUMOVehicleParameter* copyParam = new SUMOVehicleParameter();
            *copyParam = *myVehicleParameter;
            myVehicleParameter = copyParam;
            MSTransportable::MSTransportablePlan* copyPlan = new MSTransportable::MSTransportablePlan();
            for (MSStage* s : *myActivePlan) {
                copyPlan->push_back(s->clone());
            }
            myActivePlan = copyPlan;
            if (myVehicleParameter->departPosProcedure == DepartPosDefinition::RANDOM) {
                const double initialDepartPos = RandHelper::rand(myActivePlan->front()->getDestination()->getLength(), &myParsingRNG);
                myActivePlan->front()->setArrivalPos(initialDepartPos);
            }
        }
        myVehicleParameter->id = (baseID
                                  + (i >= 0 ? "." + toString(i) : "")
                                  + (j > 0 ?  "." + toString(j) : ""));
        myVehicleParameter->depart = depart += MSNet::getInstance()->getInsertionControl().computeRandomDepartOffset();
        MSTransportable* person = pc.buildPerson(myVehicleParameter, type, myActivePlan, &myParsingRNG);
        if (!pc.add(person)) {
            ProcessError error("Another person with the id '" + myVehicleParameter->id + "' exists.");
            delete person;
            if (!MSGlobals::gStateLoaded) {
                throw error;
            }
        }
    }
}


void
MSRouteHandler::closeVType() {
    MSVehicleType* vehType = MSVehicleType::build(*myCurrentVType);
    if (!MSNet::getInstance()->getVehicleControl().addVType(vehType)) {
        const std::string id = vehType->getID();
        delete vehType;
        if (!MSGlobals::gStateLoaded) {
            throw ProcessError("Another vehicle type (or distribution) with the id '" + id + "' exists.");
        }
    } else {
        if (myCurrentVTypeDistribution != nullptr) {
            myCurrentVTypeDistribution->add(vehType, vehType->getDefaultProbability());
        }
    }
}


void
MSRouteHandler::closeContainer() {
    if (myActiveContainerPlan->size() == 0) {
        throw ProcessError("Container '" + myVehicleParameter->id + "' has no plan.");
    }
    MSVehicleType* type = MSNet::getInstance()->getVehicleControl().getVType(myVehicleParameter->vtypeid);
    if (type == nullptr) {
        throw ProcessError("The type '" + myVehicleParameter->vtypeid + "' for container '" + myVehicleParameter->id + "' is not known.");
    }
    MSTransportable* container = MSNet::getInstance()->getContainerControl().buildContainer(myVehicleParameter, type, myActiveContainerPlan);
    // @todo: consider myScale?
    if (myAddVehiclesDirectly || checkLastDepart()) {
        if (MSNet::getInstance()->getContainerControl().add(container)) {
            registerLastDepart();
        } else {
            ProcessError error("Another container with the id '" + myVehicleParameter->id + "' exists.");
            delete container;
            throw error;
        }
    } else {
        // warning already given
        delete container;
    }
    myVehicleParameter = nullptr;
    myActiveContainerPlan = nullptr;
}


void
MSRouteHandler::closeFlow() {
    myInsertStopEdgesAt = -1;
    if (myVehicleParameter->repetitionNumber == 0) {
        delete myVehicleParameter;
        myVehicleParameter = nullptr;
        return;
    }
    // let's check whether vehicles had to depart before the simulation starts
    myVehicleParameter->repetitionsDone = 0;
    if (myVehicleParameter->repetitionProbability < 0) {
        const SUMOTime offsetToBegin = string2time(OptionsCont::getOptions().getString("begin")) - myVehicleParameter->depart;
        while (myVehicleParameter->repetitionsDone * myVehicleParameter->repetitionOffset < offsetToBegin) {
            myVehicleParameter->repetitionsDone++;
            if (myVehicleParameter->repetitionsDone == myVehicleParameter->repetitionNumber) {
                delete myVehicleParameter;
                myVehicleParameter = nullptr;
                return;
            }
        }
    }
    if (MSNet::getInstance()->getVehicleControl().getVType(myVehicleParameter->vtypeid, &myParsingRNG) == nullptr) {
        throw ProcessError("The vehicle type '" + myVehicleParameter->vtypeid + "' for flow '" + myVehicleParameter->id + "' is not known.");
    }
    if (myVehicleParameter->routeid[0] == '!' && MSRoute::dictionary(myVehicleParameter->routeid, &myParsingRNG) == nullptr) {
        myVehicleParameter->parametersSet |= VEHPARS_FORCE_REROUTE;
        closeRoute(true);
    }
    const MSRoute* route = MSRoute::dictionary(myVehicleParameter->routeid, &myParsingRNG);
    if (route == nullptr) {
        throw ProcessError("The route '" + myVehicleParameter->routeid + "' for flow '" + myVehicleParameter->id + "' is not known.");
    }
    if (route->mustReroute()) {
        myVehicleParameter->parametersSet |= VEHPARS_FORCE_REROUTE;
        if (myVehicleParameter->stops.size() > 0) {
            route = addVehicleStopsToImplicitRoute(route, true);
            myVehicleParameter->routeid = route->getID();
        }
    }
    myActiveRouteID = "";

    // check whether the vehicle shall be added directly to the network or
    //  shall stay in the internal buffer
    if (myAddVehiclesDirectly || checkLastDepart()) {
        if (MSNet::getInstance()->getInsertionControl().addFlow(myVehicleParameter)) {
            registerLastDepart();
        } else {
            throw ProcessError("Another flow with the id '" + myVehicleParameter->id + "' exists.");
        }
    }
    myVehicleParameter = nullptr;
}


void
MSRouteHandler::closeTrip() {
    myVehicleParameter->parametersSet |= VEHPARS_FORCE_REROUTE;
    closeRoute(true);
    closeVehicle();
}


void
MSRouteHandler::addStop(const SUMOSAXAttributes& attrs) {
    std::string errorSuffix;
    if (myActivePlan != nullptr) {
        errorSuffix = " in person '" + myVehicleParameter->id + "'.";
    } else if (myVehicleParameter != nullptr) {
        errorSuffix = " in vehicle '" + myVehicleParameter->id + "'.";
    } else if (myActiveContainerPlan != nullptr) {
        errorSuffix = " in container '" + myVehicleParameter->id + "'.";
    } else {
        errorSuffix = " in route '" + myActiveRouteID + "'.";
    }
    SUMOVehicleParameter::Stop stop;
    bool ok = parseStop(stop, attrs, errorSuffix, MsgHandler::getErrorInstance());
    if (!ok) {
        return;
    }
    const MSEdge* edge = nullptr;
    MSStoppingPlace* toStop = nullptr;
    // try to parse the assigned bus stop
    if (stop.busstop != "") {
        // ok, we have a bus stop
        MSStoppingPlace* bs = MSNet::getInstance()->getStoppingPlace(stop.busstop, SUMO_TAG_BUS_STOP);
        if (bs == nullptr) {
            WRITE_ERROR("The busStop '" + stop.busstop + "' is not known" + errorSuffix);
            return;
        }
        toStop = bs;
        const MSLane& l = bs->getLane();
        stop.lane = l.getID();
        stop.endPos = bs->getEndLanePosition();
        stop.startPos = bs->getBeginLanePosition();
        edge = &l.getEdge();
    } //try to parse the assigned container stop
    else if (stop.containerstop != "") {
        // ok, we have obviously a container stop
        MSStoppingPlace* cs = MSNet::getInstance()->getStoppingPlace(stop.containerstop, SUMO_TAG_CONTAINER_STOP);
        if (cs == nullptr) {
            WRITE_ERROR("The containerStop '" + stop.containerstop + "' is not known" + errorSuffix);
            return;
        }
        toStop = cs;
        const MSLane& l = cs->getLane();
        stop.lane = l.getID();
        stop.endPos = cs->getEndLanePosition();
        stop.startPos = cs->getBeginLanePosition();
        edge = &l.getEdge();
    } //try to parse the assigned parking area
    else if (stop.parkingarea != "") {
        // ok, we have obviously a parking area
        MSStoppingPlace* pa = MSNet::getInstance()->getStoppingPlace(stop.parkingarea, SUMO_TAG_PARKING_AREA);
        if (pa == nullptr) {
            WRITE_ERROR("The parkingArea '" + stop.parkingarea + "' is not known" + errorSuffix);
            return;
        }
        toStop = pa;
        const MSLane& l = pa->getLane();
        stop.lane = l.getID();
        stop.endPos = pa->getEndLanePosition();
        stop.startPos = pa->getBeginLanePosition();
        edge = &l.getEdge();
    } else if (stop.chargingStation != "") {
        // ok, we have a charging station
        MSStoppingPlace* cs = MSNet::getInstance()->getStoppingPlace(stop.chargingStation, SUMO_TAG_CHARGING_STATION);
        if (cs == nullptr) {
            WRITE_ERROR("The chargingStation '" + stop.chargingStation + "' is not known" + errorSuffix);
            return;
        }
        toStop = cs;
        const MSLane& l = cs->getLane();
        stop.lane = l.getID();
        stop.endPos = cs->getEndLanePosition();
        stop.startPos = cs->getBeginLanePosition();
        edge = &l.getEdge();
    } else if (stop.overheadWireSegment != "") {
        // ok, we have an overhead wire segment
        MSStoppingPlace* ows = MSNet::getInstance()->getStoppingPlace(stop.overheadWireSegment, SUMO_TAG_OVERHEAD_WIRE_SEGMENT);
        if (ows == nullptr) {
            WRITE_ERROR("The overhead wire segment '" + stop.overheadWireSegment + "' is not known" + errorSuffix);
            return;
        }
        toStop = ows;
        const MSLane& l = ows->getLane();
        stop.lane = l.getID();
        stop.endPos = ows->getEndLanePosition();
        stop.startPos = ows->getBeginLanePosition();
        edge = &l.getEdge();
    } else {
        // no, the lane and the position should be given
        // get the lane
        stop.lane = attrs.getOpt<std::string>(SUMO_ATTR_LANE, nullptr, ok, "");
        if (ok && stop.lane != "") {
            MSLane* stopLane = MSLane::dictionary(stop.lane);
            if (stopLane == nullptr || (stopLane->isInternal() && !MSGlobals::gUsingInternalLanes)) {
                WRITE_ERROR("The lane '" + stop.lane + "' for a stop is not known" + errorSuffix);
                return;
            }
        } else {
            if (myActivePlan && !myActivePlan->empty()) {
                const MSStoppingPlace* bs = myActivePlan->back()->getDestinationStop();
                if (bs != nullptr) {
                    edge = &bs->getLane().getEdge();
                    stop.lane = bs->getLane().getID();
                    stop.endPos = bs->getEndLanePosition();
                    stop.startPos = bs->getBeginLanePosition();
                } else {
                    edge = myActivePlan->back()->getDestination();
                    stop.lane = edge->getLanes()[0]->getID();
                    stop.endPos = myActivePlan->back()->getArrivalPos();
                    stop.startPos = MAX2(0., stop.endPos - MIN_STOP_LENGTH);
                }
            } else {
                WRITE_ERROR("A stop must be placed on a busStop, a chargingStation, an overheadWireSegment, a containerStop, a parkingArea or a lane" + errorSuffix);
                return;
            }
        }
        edge = &MSLane::dictionary(stop.lane)->getEdge();
        stop.endPos = attrs.getOpt<double>(SUMO_ATTR_ENDPOS, nullptr, ok, MSLane::dictionary(stop.lane)->getLength());
        if (attrs.hasAttribute(SUMO_ATTR_POSITION)) {
            WRITE_WARNING("Deprecated attribute 'pos' in description of stop" + errorSuffix);
            stop.endPos = attrs.getOpt<double>(SUMO_ATTR_POSITION, nullptr, ok, stop.endPos);
        }
        stop.startPos = attrs.getOpt<double>(SUMO_ATTR_STARTPOS, nullptr, ok, MAX2(0., stop.endPos - MIN_STOP_LENGTH));
        const bool friendlyPos = attrs.getOpt<bool>(SUMO_ATTR_FRIENDLY_POS, nullptr, ok, false);
        if (!ok || (checkStopPos(stop.startPos, stop.endPos, MSLane::dictionary(stop.lane)->getLength(), POSITION_EPS, friendlyPos) != StopPos::STOPPOS_VALID)) {
            WRITE_ERROR("Invalid start or end position for stop on lane '" + stop.lane + "'" + errorSuffix);
            return;
        }
    }
    if (myActivePlan) {
        if (myActivePlan->empty()) {
            double departPos = toStop == nullptr || myVehicleParameter->wasSet(VEHPARS_DEPARTPOS_SET)
                               ? myVehicleParameter->departPos
                               : (toStop->getBeginLanePosition() + toStop->getEndLanePosition()) / 2;
            myActivePlan->push_back(new MSStageWaiting(
                                        edge, toStop, -1, myVehicleParameter->depart, departPos, "start", true));
        } else if (myActivePlan->back()->getDestination() != edge) {
            throw ProcessError("Disconnected plan for person '" + myVehicleParameter->id + "' (" + edge->getID() + "!=" + myActivePlan->back()->getDestination()->getID() + ").");
        }  else if (myActivePlan->back()->getStageType() == MSStageType::WAITING
                    && (attrs.hasAttribute(SUMO_ATTR_ENDPOS) || attrs.hasAttribute(SUMO_ATTR_STARTPOS))) {
            const double start = SUMOVehicleParameter::interpretEdgePos(stop.startPos, edge->getLength(), SUMO_ATTR_STARTPOS, "stopping at " + edge->getID());
            const double end = SUMOVehicleParameter::interpretEdgePos(stop.endPos, edge->getLength(), SUMO_ATTR_ENDPOS, "stopping at " + edge->getID());
            const double prevAr = myActivePlan->back()->getArrivalPos();
            if (start > prevAr + NUMERICAL_EPS || end < prevAr - NUMERICAL_EPS) {
                WRITE_WARNING("Disconnected plan for person '" + myVehicleParameter->id
                              + "' (stop range " + toString(start) + "-" + toString(end) + " does not cover previous arrival position " + toString(prevAr) +  + ").");
            }
        }
        std::string actType = attrs.getOpt<std::string>(SUMO_ATTR_ACTTYPE, nullptr, ok, "waiting");
        double pos = (stop.startPos + stop.endPos) / 2.;
        if (!myActivePlan->empty()) {
            pos = myActivePlan->back()->getArrivalPos();
        }
        myActivePlan->push_back(new MSStageWaiting(
                                    &MSLane::dictionary(stop.lane)->getEdge(), toStop, stop.duration, stop.until, pos, actType, false));

    } else if (myActiveContainerPlan) {
        if (myActiveContainerPlan->empty()) {
            double departPos = toStop == nullptr || myVehicleParameter->wasSet(VEHPARS_DEPARTPOS_SET)
                               ? myVehicleParameter->departPos
                               : (toStop->getBeginLanePosition() + toStop->getEndLanePosition()) / 2;
            myActiveContainerPlan->push_back(new MSStageWaiting(
                                                 &MSLane::dictionary(stop.lane)->getEdge(), toStop, -1, myVehicleParameter->depart, departPos, "start", true));
        } else if (myActiveContainerPlan->back()->getDestination() != &MSLane::dictionary(stop.lane)->getEdge()) {
            throw ProcessError("Disconnected plan for container '" + myVehicleParameter->id + "' (" + MSLane::dictionary(stop.lane)->getEdge().getID() + "!=" + myActiveContainerPlan->back()->getDestination()->getID() + ").");
        }
        std::string actType = attrs.getOpt<std::string>(SUMO_ATTR_ACTTYPE, nullptr, ok, "waiting");
        myActiveContainerPlan->push_back(new MSStageWaiting(
                                             &MSLane::dictionary(stop.lane)->getEdge(), toStop, stop.duration, stop.until, stop.startPos, actType, false));
    } else if (myVehicleParameter != nullptr) {
        myVehicleParameter->stops.push_back(stop);
    } else {
        myActiveRouteStops.push_back(stop);
    }
    if (myInsertStopEdgesAt >= 0) {
        //std::cout << " myInsertStopEdgesAt=" << myInsertStopEdgesAt << " edge=" << edge->getID() << " myRoute=" << toString(myActiveRoute) << "\n";
        myActiveRoute.insert(myActiveRoute.begin() + myInsertStopEdgesAt, edge);
        myInsertStopEdgesAt++;
    }
}


void
MSRouteHandler::parseWalkPositions(const SUMOSAXAttributes& attrs, const std::string& personID,
                                   const MSEdge* fromEdge, const MSEdge*& toEdge,
                                   double& departPos, double& arrivalPos, MSStoppingPlace*& bs,
                                   const MSStage* const lastStage, bool& ok) {
    const std::string description = "person '" + personID + "' walking from " + fromEdge->getID();

    if (attrs.hasAttribute(SUMO_ATTR_DEPARTPOS)) {
        WRITE_WARNING("The attribute departPos is no longer supported for walks, please use the person attribute, the arrivalPos of the previous step or explicit stops.");
    }
    departPos = 0.;
    if (lastStage != nullptr) {
        if (lastStage->getDestinationStop() != nullptr) {
            departPos = lastStage->getDestinationStop()->getAccessPos(fromEdge);
        } else if (lastStage->getDestination() == fromEdge) {
            departPos = lastStage->getArrivalPos();
        } else if (lastStage->getDestination()->getToJunction() == fromEdge->getToJunction()) {
            departPos = fromEdge->getLength();
        }
    }

    std::string bsID = attrs.getOpt<std::string>(SUMO_ATTR_BUS_STOP, nullptr, ok, "");
    if (bsID != "") {
        bs = MSNet::getInstance()->getStoppingPlace(bsID, SUMO_TAG_BUS_STOP);
        if (bs == nullptr) {
            throw ProcessError("Unknown bus stop '" + bsID + "' for " + description + ".");
        }
        arrivalPos = bs->getAccessPos(toEdge != nullptr ? toEdge : &bs->getLane().getEdge());
        if (arrivalPos < 0) {
            throw ProcessError("Bus stop '" + bsID + "' is not connected to arrival edge '" + toEdge->getID() + "' for " + description + ".");
        }
        if (attrs.hasAttribute(SUMO_ATTR_ARRIVALPOS)) {
            const double length = toEdge != nullptr ? toEdge->getLength() : bs->getLane().getLength();
            const double arrPos = SUMOVehicleParserHelper::parseWalkPos(SUMO_ATTR_ARRIVALPOS, myHardFail, description, length,
                                  attrs.get<std::string>(SUMO_ATTR_ARRIVALPOS, description.c_str(), ok), &myParsingRNG);
            if (arrPos >= bs->getBeginLanePosition() && arrPos < bs->getEndLanePosition()) {
                arrivalPos = arrPos;
            } else {
                WRITE_WARNING("Ignoring arrivalPos for " + description + " because it is outside the given stop '" + toString(SUMO_ATTR_BUS_STOP) + "'.");
                arrivalPos = bs->getAccessPos(&bs->getLane().getEdge());
            }
        }
    } else {
        if (toEdge == nullptr) {
            throw ProcessError("No destination edge for " + description + ".");
        }
        if (attrs.hasAttribute(SUMO_ATTR_ARRIVALPOS)) {
            arrivalPos = SUMOVehicleParserHelper::parseWalkPos(SUMO_ATTR_ARRIVALPOS, myHardFail, description, toEdge->getLength(),
                         attrs.get<std::string>(SUMO_ATTR_ARRIVALPOS, description.c_str(), ok), &myParsingRNG);
        } else {
            arrivalPos = toEdge->getLength() / 2.;
        }
    }
}


void
MSRouteHandler::addPersonTrip(const SUMOSAXAttributes& attrs) {
    myActiveRoute.clear();
    bool ok = true;
    const char* const id = myVehicleParameter->id.c_str();
    const MSEdge* from = nullptr;
    const MSEdge* to = nullptr;
    parseFromViaTo(SUMO_TAG_PERSON, attrs);
    myInsertStopEdgesAt = -1;
    if (attrs.hasAttribute(SUMO_ATTR_FROM) || attrs.hasAttribute(SUMO_ATTR_FROMJUNCTION) || attrs.hasAttribute(SUMO_ATTR_FROM_TAZ)) {
        from = myActiveRoute.front();
    } else if (myActivePlan->empty()) {
        throw ProcessError("Start edge not defined for person '" + myVehicleParameter->id + "'.");
    } else {
        from = myActivePlan->back()->getDestination();
    }
    if (attrs.hasAttribute(SUMO_ATTR_TO) || attrs.hasAttribute(SUMO_ATTR_TOJUNCTION) || attrs.hasAttribute(SUMO_ATTR_TO_TAZ)) {
        to = myActiveRoute.back();
    } // else, to may also be derived from stopping place

    const SUMOTime duration = attrs.getOptSUMOTimeReporting(SUMO_ATTR_DURATION, id, ok, -1);
    if (attrs.hasAttribute(SUMO_ATTR_DURATION) && duration <= 0) {
        throw ProcessError("Non-positive walking duration for  '" + myVehicleParameter->id + "'.");
    }

    double departPos = 0;
    double arrivalPos = 0;
    MSStoppingPlace* stoppingPlace = nullptr;
    parseWalkPositions(attrs, myVehicleParameter->id, from, to, departPos, arrivalPos, stoppingPlace, nullptr, ok);

    const std::string modes = attrs.getOpt<std::string>(SUMO_ATTR_MODES, id, ok, "");
    SVCPermissions modeSet = 0;
    std::string errorMsg;
    // try to parse person modes
    if (!SUMOVehicleParameter::parsePersonModes(modes, "person", id, modeSet, errorMsg)) {
        throw InvalidArgument(errorMsg);
    }
    MSVehicleControl& vehControl = MSNet::getInstance()->getVehicleControl();
    const std::string types = attrs.getOpt<std::string>(SUMO_ATTR_VTYPES, id, ok, "");
    for (StringTokenizer st(types); st.hasNext();) {
        const std::string vtypeid = st.next();
        if (vehControl.getVType(vtypeid) == nullptr) {
            throw InvalidArgument("The vehicle type '" + vtypeid + "' in a trip for person '" + myVehicleParameter->id + "' is not known.");
        }
        modeSet |= SVC_PASSENGER;
    }
    const double speed = attrs.getOpt<double>(SUMO_ATTR_SPEED, id, ok, -1.);
    if (attrs.hasAttribute(SUMO_ATTR_SPEED) && speed <= 0) {
        throw ProcessError("Non-positive walking speed for  '" + myVehicleParameter->id + "'.");
    }
    const double walkFactor = attrs.getOpt<double>(SUMO_ATTR_WALKFACTOR, id, ok, OptionsCont::getOptions().getFloat("persontrip.walkfactor"));
    const double departPosLat = attrs.getOpt<double>(SUMO_ATTR_DEPARTPOS_LAT, nullptr, ok, 0);
    if (ok) {
        if (myActivePlan->empty()) {
            double initialDepartPos = myVehicleParameter->departPos;
            if (myVehicleParameter->departPosProcedure == DepartPosDefinition::RANDOM) {
                initialDepartPos = RandHelper::rand(from->getLength(), &myParsingRNG);
            }
            myActivePlan->push_back(new MSStageWaiting(from, nullptr, -1, myVehicleParameter->depart, initialDepartPos, "start", true));
        }
        myVehicleParameter->parametersSet |= VEHPARS_FORCE_REROUTE;
        MSStoppingPlace* fromStop = myActivePlan->empty() ? nullptr : myActivePlan->back()->getDestinationStop();
        myActivePlan->push_back(new MSStageTrip(from, fromStop, to == nullptr ? &stoppingPlace->getLane().getEdge() : to, stoppingPlace, duration, modeSet, types, speed, walkFactor, departPosLat, attrs.hasAttribute(SUMO_ATTR_ARRIVALPOS), arrivalPos));
    }
    myActiveRoute.clear();
}


void
MSRouteHandler::addWalk(const SUMOSAXAttributes& attrs) {
    // parse walks from->to as person trips
    if (attrs.hasAttribute(SUMO_ATTR_EDGES) || attrs.hasAttribute(SUMO_ATTR_ROUTE)) {
        try {
            myActiveRoute.clear();
            bool ok = true;
            const SUMOTime duration = attrs.getOptSUMOTimeReporting(SUMO_ATTR_DURATION, nullptr, ok, -1);
            if (attrs.hasAttribute(SUMO_ATTR_DURATION) && duration <= 0) {
                throw ProcessError("Non-positive walking duration for  '" + myVehicleParameter->id + "'.");
            }
            double speed = -1; // default to vType speed
            if (attrs.hasAttribute(SUMO_ATTR_SPEED)) {
                speed = attrs.get<double>(SUMO_ATTR_SPEED, nullptr, ok);
                if (speed <= 0) {
                    throw ProcessError("Non-positive walking speed for  '" + myVehicleParameter->id + "'.");
                }
            }
            double departPos = 0;
            double arrivalPos = 0;
            MSStoppingPlace* bs = nullptr;
            if (attrs.hasAttribute(SUMO_ATTR_ROUTE)) {
                const std::string routeID = attrs.get<std::string>(SUMO_ATTR_ROUTE, myVehicleParameter->id.c_str(), ok);
                const MSRoute* route = MSRoute::dictionary(routeID, &myParsingRNG);
                if (route == nullptr) {
                    throw ProcessError("The route '" + routeID + "' for walk of person '" + myVehicleParameter->id + "' is not known.");
                }
                myActiveRoute = route->getEdges();
            } else {
                MSEdge::parseEdgesList(attrs.get<std::string>(SUMO_ATTR_EDGES, myVehicleParameter->id.c_str(), ok), myActiveRoute, myActiveRouteID);
            }
            if (myActivePlan->empty()) {
                double initialDepartPos = myVehicleParameter->departPos;
                if (myVehicleParameter->departPosProcedure == DepartPosDefinition::RANDOM) {
                    initialDepartPos = RandHelper::rand(myActiveRoute.front()->getLength(), &myParsingRNG);
                }
                myActivePlan->push_back(new MSStageWaiting(myActiveRoute.front(), nullptr, -1, myVehicleParameter->depart, initialDepartPos, "start", true));
            }
            parseWalkPositions(attrs, myVehicleParameter->id, myActiveRoute.front(), myActiveRoute.back(), departPos, arrivalPos, bs, myActivePlan->back(), ok);
            if (myActiveRoute.empty()) {
                throw ProcessError("No edges to walk for person '" + myVehicleParameter->id + "'.");
            }
            if (myActivePlan->back()->getDestination() != myActiveRoute.front() &&
                    myActivePlan->back()->getDestination()->getToJunction() != myActiveRoute.front()->getFromJunction() &&
                    myActivePlan->back()->getDestination()->getToJunction() != myActiveRoute.front()->getToJunction()) {
                if (myActivePlan->back()->getDestinationStop() == nullptr || myActivePlan->back()->getDestinationStop()->getAccessPos(myActiveRoute.front()) < 0.) {
                    throw ProcessError("Disconnected plan for person '" + myVehicleParameter->id + "' (" + myActiveRoute.front()->getID() + " not connected to " + myActivePlan->back()->getDestination()->getID() + ").");
                }
            }
            const double departPosLat = attrs.getOpt<double>(SUMO_ATTR_DEPARTPOS_LAT, nullptr, ok, 0);
            myActivePlan->push_back(new MSPerson::MSPersonStage_Walking(myVehicleParameter->id, myActiveRoute, bs, duration, speed, departPos, arrivalPos, departPosLat));
            myActiveRoute.clear();
        } catch (ProcessError&) {
            deleteActivePlans();
            throw;
        }
    } else {
        addPersonTrip(attrs);
    }
}


void
MSRouteHandler::addPerson(const SUMOSAXAttributes& /*attrs*/) {
}


void
MSRouteHandler::addContainer(const SUMOSAXAttributes& /*attrs*/) {
}


void
MSRouteHandler::addRide(const SUMOSAXAttributes& /*attrs*/) {
}


void
MSRouteHandler::addTransport(const SUMOSAXAttributes& /*attrs*/) {
}


void
MSRouteHandler::addTranship(const SUMOSAXAttributes& /*attrs*/) {
}


/****************************************************************************/
