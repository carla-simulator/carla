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
/// @file    IntermodalRouter.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 03 March 2014
///
// The IntermodalRouter builds a special network and (delegates to a SUMOAbstractRouter)
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include "SUMOAbstractRouter.h"
#include "DijkstraRouter.h"
#include "AStarRouter.h"
#include "IntermodalNetwork.h"
#include "EffortCalculator.h"
#include "CarEdge.h"
#include "StopEdge.h"
#include "PedestrianRouter.h"

//#define IntermodalRouter_DEBUG_ROUTES


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class IntermodalRouter
 * The router for pedestrians (on a bidirectional network of sidewalks and crossings)
 */
template<class E, class L, class N, class V>
class IntermodalRouter : public SUMOAbstractRouter<E, IntermodalTrip<E, N, V> > {
public:
    typedef IntermodalNetwork<E, L, N, V> Network;

private:
    typedef void(*CreateNetCallback)(IntermodalRouter <E, L, N, V>&);
    typedef IntermodalEdge<E, L, N, V> _IntermodalEdge;
    typedef IntermodalTrip<E, N, V> _IntermodalTrip;
    typedef SUMOAbstractRouter<_IntermodalEdge, _IntermodalTrip> _InternalRouter;
    typedef DijkstraRouter<_IntermodalEdge, _IntermodalTrip> _InternalDijkstra;
    typedef AStarRouter<_IntermodalEdge, _IntermodalTrip> _InternalAStar;

public:
    struct TripItem {
        TripItem(const std::string& _line = "") :
            line(_line), intended(_line) {}
        std::string line;
        std::string vType = "";
        std::string destStop = "";
        std::string intended; // intended public transport vehicle id
        double depart = -1.; // intended public transport departure
        std::vector<const E*> edges;
        double traveltime = 0.;
        double cost = 0.;
        double length = 0.;
        double departPos = INVALID_DOUBLE;
        double arrivalPos = INVALID_DOUBLE;
        std::string description = "";
    };

    /// Constructor
    IntermodalRouter(CreateNetCallback callback, const int carWalkTransfer, const std::string& routingAlgorithm,
                     const int routingMode = 0, EffortCalculator* calc = nullptr) :
        SUMOAbstractRouter<E, _IntermodalTrip>("IntermodalRouter", true, nullptr, nullptr, false, false),
        myAmClone(false), myInternalRouter(nullptr), myIntermodalNet(nullptr),
        myCallback(callback), myCarWalkTransfer(carWalkTransfer), myRoutingAlgorithm(routingAlgorithm),
        myRoutingMode(routingMode), myExternalEffort(calc) {
    }

    /// Destructor
    virtual ~IntermodalRouter() {
        delete myInternalRouter;
        if (!myAmClone) {
            delete myIntermodalNet;
        }
    }

    SUMOAbstractRouter<E, _IntermodalTrip>* clone() {
        createNet();
        return new IntermodalRouter<E, L, N, V>(myIntermodalNet, myCarWalkTransfer, myRoutingAlgorithm, myRoutingMode, myExternalEffort);
    }

    int getCarWalkTransfer() const {
        return myCarWalkTransfer;
    }

    /** @brief Builds the route between the given edges using the minimum effort at the given time
        The definition of the effort depends on the wished routing scheme */
    bool compute(const E* from, const E* to, const double departPos, const double arrivalPos,
                 const std::string stopID, const double speed,
                 const V* const vehicle, const SVCPermissions modeSet, const SUMOTime msTime,
                 std::vector<TripItem>& into, const double externalFactor = 0.) {
        createNet();
        _IntermodalTrip trip(from, to, departPos, arrivalPos, speed, msTime, 0, vehicle, modeSet, myExternalEffort, externalFactor);
        std::vector<const _IntermodalEdge*> intoEdges;
        //std::cout << "compute from=" << from->getID() << " to=" << to->getID() << " dPos=" << departPos << " aPos=" << arrivalPos << " stopID=" << stopID << " speed=" << speed << " veh=" << Named::getIDSecure(vehicle) << " modeSet=" << modeSet << " t=" << msTime << " iFrom=" << myIntermodalNet->getDepartEdge(from, trip.departPos)->getID() << " iTo=" << (stopID != "" ? myIntermodalNet->getStopEdge(stopID) : myIntermodalNet->getArrivalEdge(to, trip.arrivalPos))->getID() << "\n";
        const bool success = myInternalRouter->compute(myIntermodalNet->getDepartEdge(from, trip.departPos),
                             stopID != "" ? myIntermodalNet->getStopEdge(stopID) : myIntermodalNet->getArrivalEdge(to, trip.arrivalPos),
                             &trip, msTime, intoEdges);
        if (success) {
            std::string lastLine = "";
            double time = STEPS2TIME(msTime);
            double effort = 0.;
            double length = 0.;
            const _IntermodalEdge* prev = nullptr;
            for (const _IntermodalEdge* iEdge : intoEdges) {
                if (iEdge->includeInRoute(false)) {
                    if (iEdge->getLine() == "!stop") {
                        if (into.size() > 0) {
                            // previous stage ends at stop
                            into.back().destStop = iEdge->getID();
                            if (myExternalEffort != nullptr) {
                                into.back().description = myExternalEffort->output(iEdge->getNumericalID());
                            }
                            if (lastLine == "!ped") {
                                lastLine = ""; // a stop always starts a new trip item
                            }
                        } else {
                            // trip starts at stop
                            lastLine = "";
                            into.push_back(TripItem("!stop"));
                            into.back().destStop = iEdge->getID();
                        }
                    } else {
                        if (iEdge->getLine() != lastLine) {
                            lastLine = iEdge->getLine();
                            if (lastLine == "!car") {
                                into.push_back(TripItem(vehicle->getID()));
                                into.back().vType = vehicle->getParameter().vtypeid;
                            } else if (lastLine == "!ped") {
                                into.push_back(TripItem());
                            } else {
                                into.push_back(TripItem(lastLine));
                                into.back().depart = iEdge->getIntended(time, into.back().intended);
                            }
                            into.back().departPos = iEdge->getStartPos();
                        }
                        if (into.back().edges.empty() || into.back().edges.back() != iEdge->getEdge()) {
                            into.back().edges.push_back(iEdge->getEdge());
                            into.back().arrivalPos = iEdge->getEndPos();
                        }
                    }
                }
                const double prevTime = time, prevEffort = effort, prevLength = length;
                myInternalRouter->updateViaCost(prev, iEdge, &trip, time, effort, length);
                prev = iEdge;
                if (!into.empty()) {
                    into.back().traveltime += time - prevTime;
                    into.back().cost += effort - prevEffort;
                    into.back().length += length - prevLength;
                    if (into.back().depart < 0) {
                        into.back().depart = prevTime;
                    }
                }
            }
        }
#ifdef IntermodalRouter_DEBUG_ROUTES
        double time = STEPS2TIME(msTime);
        for (const _IntermodalEdge* iEdge : intoEdges) {
            const double edgeEffort = myInternalRouter->getEffort(iEdge, &trip, time);
            time += edgeEffort;
            std::cout << iEdge->getID() << "(" << iEdge->getLine() << "): " << edgeEffort << std::endl;
        }
        std::cout << TIME2STEPS(msTime) << " trip from " << from->getID() << " to " << (to != nullptr ? to->getID() : stopID)
                  << " departPos=" << trip.departPos
                  << " arrivalPos=" << trip.arrivalPos
                  << " modes=" << getVehicleClassNames(modeSet)
                  << " edges=" << toString(intoEdges)
//                  << " resultEdges=" << toString(into)
                  << " time=" << time
                  << "\n";
#endif
        return success;
    }

    /** @brief Builds the route between the given edges using the minimum effort at the given time
        The definition of the effort depends on the wished routing scheme */
    bool compute(const E*, const E*, const _IntermodalTrip* const,
                 SUMOTime, std::vector<const E*>&, bool) {
        throw ProcessError("Do not use this method");
    }

    void prohibit(const std::vector<E*>& toProhibit) {
        createNet();
        std::vector<_IntermodalEdge*> toProhibitPE;
        for (typename std::vector<E*>::const_iterator it = toProhibit.begin(); it != toProhibit.end(); ++it) {
            toProhibitPE.push_back(myIntermodalNet->getBothDirections(*it).first);
            toProhibitPE.push_back(myIntermodalNet->getBothDirections(*it).second);
            toProhibitPE.push_back(myIntermodalNet->getCarEdge(*it));
        }
        myInternalRouter->prohibit(toProhibitPE);
    }

    void writeNetwork(OutputDevice& dev) {
        createNet();
        for (_IntermodalEdge* e : myIntermodalNet->getAllEdges()) {
            dev.openTag(SUMO_TAG_EDGE);
            dev.writeAttr(SUMO_ATTR_ID, e->getID());
            dev.writeAttr(SUMO_ATTR_LINE, e->getLine());
            dev.writeAttr(SUMO_ATTR_LENGTH, e->getLength());
            dev.writeAttr("successors", toString(e->getSuccessors(SVC_IGNORING)));
            dev.closeTag();
        }
    }

    void writeWeights(OutputDevice& dev) {
        createNet();
        _IntermodalTrip trip(nullptr, nullptr, 0., 0., DEFAULT_PEDESTRIAN_SPEED, 0, 0, nullptr, SVC_PASSENGER | SVC_BICYCLE | SVC_BUS);
        for (_IntermodalEdge* e : myIntermodalNet->getAllEdges()) {
            dev.openTag(SUMO_TAG_EDGE);
            dev.writeAttr(SUMO_ATTR_ID, e->getID());
            dev.writeAttr("traveltime", e->getTravelTime(&trip, 0.));
            dev.writeAttr("effort", e->getEffort(&trip, 0.));
            dev.closeTag();
        }
    }

    Network* getNetwork() const {
        return myIntermodalNet;
    }

    EffortCalculator* getExternalEffort() const {
        return myExternalEffort;
    }

private:
    IntermodalRouter(Network* net, const int carWalkTransfer, const std::string& routingAlgorithm,
                     const int routingMode, EffortCalculator* calc) :
        SUMOAbstractRouter<E, _IntermodalTrip>("IntermodalRouterClone", true, nullptr, nullptr, false, false), myAmClone(true),
        myInternalRouter(new _InternalDijkstra(net->getAllEdges(), true,
                                               gWeightsRandomFactor > 1 ? & _IntermodalEdge::getTravelTimeStaticRandomized : & _IntermodalEdge::getTravelTimeStatic)),
        myIntermodalNet(net), myCarWalkTransfer(carWalkTransfer), myRoutingAlgorithm(routingAlgorithm), myRoutingMode(routingMode), myExternalEffort(calc) {}

    static inline double getCombined(const _IntermodalEdge* const edge, const _IntermodalTrip* const trip, double time) {
        return edge->getTravelTime(trip, time) + trip->externalFactor * trip->calc->getEffort(edge->getNumericalID());
    }

    inline void createNet() {
        if (myIntermodalNet == nullptr) {
            myIntermodalNet = new Network(E::getAllEdges(), false, myCarWalkTransfer);
            myIntermodalNet->addCarEdges(E::getAllEdges());
            myCallback(*this);
            switch (myRoutingMode) {
                case 0:
                    if (myRoutingAlgorithm == "astar") {
                        myInternalRouter = new _InternalAStar(myIntermodalNet->getAllEdges(), true,
                                                              gWeightsRandomFactor > 1 ? &_IntermodalEdge::getTravelTimeStaticRandomized : &_IntermodalEdge::getTravelTimeStatic, nullptr, true);
                    } else {
                        myInternalRouter = new _InternalDijkstra(myIntermodalNet->getAllEdges(), true,
                                gWeightsRandomFactor > 1 ? &_IntermodalEdge::getTravelTimeStaticRandomized : &_IntermodalEdge::getTravelTimeStatic, nullptr, false, nullptr, true);
                    }
                    break;
                case 1:
                    myInternalRouter = new _InternalDijkstra(myIntermodalNet->getAllEdges(), true, &_IntermodalEdge::getTravelTimeAggregated, nullptr, false, nullptr, true);
                    break;
                case 2:
                    myInternalRouter = new _InternalDijkstra(myIntermodalNet->getAllEdges(), true, &_IntermodalEdge::getEffortStatic, &_IntermodalEdge::getTravelTimeStatic, false, nullptr, true);
                    break;
                case 3:
                    if (myExternalEffort != nullptr) {
                        std::vector<std::string> edgeLines;
                        for (const auto e : myIntermodalNet->getAllEdges()) {
                            edgeLines.push_back(e->getLine());
                        }
                        myExternalEffort->init(edgeLines);
                    }
                    myInternalRouter = new _InternalDijkstra(myIntermodalNet->getAllEdges(), true, &getCombined, &_IntermodalEdge::getTravelTimeStatic, false, myExternalEffort, true);
                    break;
            }
        }
    }

private:
    const bool myAmClone;
    _InternalRouter* myInternalRouter;
    Network* myIntermodalNet;
    CreateNetCallback myCallback;
    const int myCarWalkTransfer;
    const std::string myRoutingAlgorithm;
    const int myRoutingMode;
    EffortCalculator* const myExternalEffort;


private:
    /// @brief Invalidated assignment operator
    IntermodalRouter& operator=(const IntermodalRouter& s);

};
