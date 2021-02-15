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
/// @file    RailwayRouter.h
/// @author  Jakob Erdmann
/// @date    Tue, 25 Feb 2020
///
// The RailwayRouter builds a special network for railway routing to handle train reversal restrictions (delegates to a SUMOAbstractRouter)
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>
#ifdef HAVE_FOX
#include <fx.h>
#endif
#include <utils/common/MsgHandler.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include "SUMOAbstractRouter.h"
#include "DijkstraRouter.h"
#include "RailEdge.h"


//#define RailwayRouter_DEBUG_ROUTES

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RailwayRouter
 * The router for rail vehicles for track networks where some sections may be used in both directions
 * and trains may reverse depending on location and train length length
 *
 * Example (assume each track section is 100m long) running from left to right and a negative sign indicates reverse direction
 *
 *       A     C      D
 *   .______.______.______.
 *   ._____/
 *       B
 *
 * Consider a train of 200m length that enters on B and must move to A (with a reversal):
 * The necessary route is B C D -D -C -A were D,-D are needed for the train to fully pass the switch
 *
 * We shadow the normal edge graph with a railEdge graph to include virtual
 * turnarounds that look at the train length.
 * The graph extension takes place via RailEdge::init()
 * For edge C we create a virtual turnaround (as successor of C) that connectes C with -C and is then expanded to C D -D -C for trains longer than 100m.
 * The expension takes place via RailEdge::insertOriginalEdges()
 *
 */
template<class E, class V>
class RailwayRouter : public SUMOAbstractRouter<E, V> {

private:


    typedef RailEdge<E, V> _RailEdge;
    typedef SUMOAbstractRouter<_RailEdge, V> _InternalRouter;
    typedef DijkstraRouter<_RailEdge, V> _InternalDijkstra;

public:

    /// Constructor
    RailwayRouter(const std::vector<E*>& edges, bool unbuildIsWarning, typename SUMOAbstractRouter<E, V>::Operation effortOperation,
                  typename SUMOAbstractRouter<E, V>::Operation ttOperation = nullptr, bool silent = false,
                  const bool havePermissions = false, const bool haveRestrictions = false, double maxTrainLength = 5000) :
        SUMOAbstractRouter<E, V>("RailwayRouter", unbuildIsWarning, effortOperation, ttOperation, havePermissions, haveRestrictions),
        myInternalRouter(nullptr), myOriginal(nullptr), mySilent(silent),
        myMaxTrainLength(maxTrainLength) {
        myStaticOperation = effortOperation;
        for (const E* const edge : edges) {
            myInitialEdges.push_back(edge->getRailwayRoutingEdge());
        }
    }

    /// Destructor
    virtual ~RailwayRouter() {
        delete myInternalRouter;
    }

    SUMOAbstractRouter<E, V>* clone() {
        return new RailwayRouter<E, V>(this);
    }

    /** @brief Builds the route between the given edges using the minimum effort at the given time
        The definition of the effort depends on the wished routing scheme */
    bool compute(const E* from, const E* to, const V* const vehicle, SUMOTime msTime, std::vector<const E*>& into, bool silent = false) {
        ensureInternalRouter();
        if (vehicle->getLength() > myMaxTrainLength) {
            WRITE_WARNINGF("Vehicle '%' with length % exceeds configured value of --railway.max-train-length %",
                           vehicle->getID(), toString(vehicle->getLength()), toString(myMaxTrainLength));
        }
        // make sure that the vehicle can turn-around when starting on a short edge (the virtual turn-around for this lies backwards along the route / track)
        std::vector<double> backLengths;
        double backDist = vehicle->getLength() - from->getLength();
        const E* start = from;
        while (backDist > 0) {
            const E* prev = getStraightPredecessor(start);
            if (prev == nullptr) {
                //WRITE_WARNING("Could not determine back edge for vehicle '" + vehicle->getID() + "' when routing from edge '" + from->getID() + "' at time " + time2string(msTime));
                break;
            }
            backDist -= prev->getLength();
            backLengths.push_back(prev->getLength() + (backLengths.empty() ? from->getLength() : backLengths.back()));
            start = prev;
        }

        std::vector<const _RailEdge*> intoTmp;
        bool success = myInternalRouter->compute(start->getRailwayRoutingEdge(), to->getRailwayRoutingEdge(), vehicle, msTime, intoTmp, silent);
#ifdef RailwayRouter_DEBUG_ROUTES
        std::cout << "RailRouter veh=" << vehicle->getID() << " from=" << from->getID() << " to=" << to->getID() << " t=" << time2string(msTime) << " success=" << success << "\n";
#endif
        if (success) {
            const size_t intoSize = into.size();
            const int backIndex = (int)backLengths.size() - 1;;
            for (const _RailEdge* railEdge : intoTmp) {
                // prevent premature reversal on back edge (extend train length)
                const double length = backIndex >= 0 ? backLengths[backIndex] : vehicle->getLength();
                railEdge->insertOriginalEdges(length, into);
            }
#ifdef RailwayRouter_DEBUG_ROUTES
            std::cout << "RailRouter: internal result=" << toString(intoTmp) << "\n";
            std::cout << "RailRouter: expanded result=" << toString(into) << "\n";
#endif
            if (backLengths.size() > 0) {
                // skip the virtual back-edges
                into.erase(into.begin() + intoSize, into.begin() + intoSize + backLengths.size());
#ifdef RailwayRouter_DEBUG_ROUTES
                std::cout << "RailRouter: backLengths=" << toString(backLengths) << " final result=" << toString(into) << "\n";
#endif
                if (*(into.begin() + intoSize) != from) {
                    WRITE_WARNING("Railway routing failure due to turn-around on short edge '" + from->getID()
                                  + "' for vehicle '" + vehicle->getID() + "' time=" + time2string(msTime) + ".");
                }
            }
        }
        return success;
    }

    void prohibit(const std::vector<E*>& toProhibit) {
        ensureInternalRouter();
        std::vector<_RailEdge*> railEdges;
        for (E* edge : toProhibit) {
            railEdges.push_back(edge->getRailwayRoutingEdge());
        }
        myInternalRouter->prohibit(railEdges);
    }


private:
    RailwayRouter(RailwayRouter* other) :
        SUMOAbstractRouter<E, V>(other),
        myInternalRouter(nullptr),
        myOriginal(other),
        mySilent(other->mySilent),
        myMaxTrainLength(other->myMaxTrainLength)
    {}

    void ensureInternalRouter() {
        if (myInternalRouter == nullptr) {
            myInternalRouter = new _InternalDijkstra(getRailEdges(), this->myErrorMsgHandler == MsgHandler::getWarningInstance(), &getTravelTimeStatic,
                    nullptr, mySilent, nullptr, this->myHavePermissions, this->myHaveRestrictions);
        }
    }

    const std::vector<_RailEdge*>& getRailEdges() {
        if (myOriginal != nullptr) {
            return myOriginal->getRailEdges();
        }
#ifdef HAVE_FOX
        FXMutexLock locker(myLock);
#endif
        if (myRailEdges.empty()) {
            myRailEdges = myInitialEdges;
            int numericalID = myInitialEdges.back()->getNumericalID() + 1;
            for (_RailEdge* railEdge : myInitialEdges) {
                railEdge->init(myRailEdges, numericalID, myMaxTrainLength);
            }
        }
        return myRailEdges;
    }

    static inline double getTravelTimeStatic(const RailEdge<E, V>* const edge, const V* const veh, double time) {
        if (edge->getOriginal() != nullptr) {
            return (*myStaticOperation)(edge->getOriginal(), veh, time);
        } else {
            // turnaround edge
            if (edge->isVirtual()) {
                // add up time for replacement edges
                std::vector<const E*> repl;
                edge->insertOriginalEdges(veh->getLength(), repl);
                assert(repl.size() > 0);
                double seenDist = 0;
                double result = 0;
                repl.pop_back(); // last edge must not be used fully
                for (const E* e : repl) {
                    result += (*myStaticOperation)(e, veh, time + result);
                    seenDist += e->getLength();
                }
                const double lengthOnLastEdge = MAX2(0.0, veh->getLength() - seenDist);
                return result + myReversalPenalty + lengthOnLastEdge * myReversalPenaltyFactor;
            } else {
                // XXX if the edge from which this turnaround starts is longer
                // than the vehicle, we could return a negative value here
                // because the turnaround may happen once the vehicle has driven onto the edge
                return myReversalPenalty;
            }
        }
    }


    static const E* getStraightPredecessor(const E* edge) {
        const E* result = nullptr;
        //std::cout << "  getStraightPredecessor edge=" << edge->getID() << "\n";
        for (const E* cand : edge->getPredecessors()) {
            if (!cand->isInternal() && cand->getBidiEdge() != edge) {
                //std::cout << "    cand=" << cand->getID() << "\n";
                if (result == nullptr) {
                    result = cand;
                } else {
                    // predecessor not unique. Better abort with a warning
                    return nullptr;
                }
            }
        }
        return result;
    }

private:
    _InternalRouter* myInternalRouter;
    RailwayRouter<E, V>* const myOriginal;
    /// @brief a RailEdge for every existing edge, filled on construction (but not in clones)
    std::vector<_RailEdge*> myInitialEdges;
    /// @brief complete rail network filled on demand (but not in clones)
    std::vector<_RailEdge*> myRailEdges;

    /// @brief whether to suppress warning/error if no route was found
    const bool mySilent;

    const double myMaxTrainLength;

#ifdef HAVE_FOX
    /// The mutex used to avoid concurrent updates of myRailEdges
    mutable FXMutex myLock;
#endif

    /// @brief The object's operation to perform. (hack)
    static typename SUMOAbstractRouter<E, V>::Operation myStaticOperation;

    static double myReversalPenalty;
    static double myReversalPenaltyFactor;

private:
    /// @brief Invalidated assignment operator
    RailwayRouter& operator=(const RailwayRouter& s);

};

template<class E, class V>
typename SUMOAbstractRouter<E, V>::Operation RailwayRouter<E, V>::myStaticOperation(nullptr);
template<class E, class V>
double RailwayRouter<E, V>::myReversalPenalty(60);
template<class E, class V>
double RailwayRouter<E, V>::myReversalPenaltyFactor(0.2); // 1/v


