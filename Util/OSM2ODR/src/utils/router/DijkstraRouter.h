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
/// @file    DijkstraRouter.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 25 July 2005
///
// Dijkstra shortest path algorithm using travel time or other values
/****************************************************************************/
#pragma once
#include <config.h>

#include <cassert>
#include <string>
#include <functional>
#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <iterator>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include "EffortCalculator.h"
#include "SUMOAbstractRouter.h"

//#define DijkstraRouter_DEBUG_QUERY
//#define DijkstraRouter_DEBUG_QUERY_PERF

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class DijkstraRouter
 * @brief Computes the shortest path through a network using the Dijkstra algorithm.
 *
 * The template parameters are:
 * @param E The edge class to use (MSEdge/ROEdge)
 * @param V The vehicle class to use (MSVehicle/ROVehicle)
 *
 * The router is edge-based. It must know the number of edges for internal reasons
 *  and whether a missing connection between two given edges (unbuild route) shall
 *  be reported as an error or as a warning.
 *
 */
template<class E, class V>
class DijkstraRouter : public SUMOAbstractRouter<E, V> {
public:
    /**
     * @class EdgeInfoByEffortComparator
     * Class to compare (and so sort) nodes by their effort
     */
    class EdgeInfoByEffortComparator {
    public:
        /// Comparing method
        bool operator()(const typename SUMOAbstractRouter<E, V>::EdgeInfo* nod1, const typename SUMOAbstractRouter<E, V>::EdgeInfo* nod2) const {
            if (nod1->effort == nod2->effort) {
                return nod1->edge->getNumericalID() > nod2->edge->getNumericalID();
            }
            return nod1->effort > nod2->effort;
        }
    };


    /// Constructor
    DijkstraRouter(const std::vector<E*>& edges, bool unbuildIsWarning, typename SUMOAbstractRouter<E, V>::Operation effortOperation,
                   typename SUMOAbstractRouter<E, V>::Operation ttOperation = nullptr, bool silent = false, EffortCalculator* calc = nullptr,
                   const bool havePermissions = false, const bool haveRestrictions = false) :
        SUMOAbstractRouter<E, V>("DijkstraRouter", unbuildIsWarning, effortOperation, ttOperation, havePermissions, haveRestrictions),
        mySilent(silent), myExternalEffort(calc) {
        for (typename std::vector<E*>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            myEdgeInfos.push_back(typename SUMOAbstractRouter<E, V>::EdgeInfo(*i));
        }
    }

    /// Destructor
    virtual ~DijkstraRouter() { }

    virtual SUMOAbstractRouter<E, V>* clone() {
        auto clone = new DijkstraRouter<E, V>(myEdgeInfos, this->myErrorMsgHandler == MsgHandler::getWarningInstance(),
                                              this->myOperation, this->myTTOperation, mySilent, myExternalEffort,
                                              this->myHavePermissions, this->myHaveRestrictions);
        clone->setAutoBulkMode(this->myAutoBulkMode);
        return clone;
    }

    void init() {
        // all EdgeInfos touched in the previous query are either in myFrontierList or myFound: clean those up
        for (auto& edgeInfo : myFrontierList) {
            edgeInfo->reset();
        }
        myFrontierList.clear();
        for (auto& edgeInfo : myFound) {
            edgeInfo->reset();
        }
        myFound.clear();
    }


    /** @brief Builds the route between the given edges using the minimum effort at the given time
        The definition of the effort depends on the wished routing scheme */
    bool compute(const E* from, const E* to, const V* const vehicle,
                 SUMOTime msTime, std::vector<const E*>& into, bool silent = false) {
        assert(from != nullptr && (vehicle == nullptr || to != nullptr));
        // check whether from and to can be used
        if (myEdgeInfos[from->getNumericalID()].prohibited || this->isProhibited(from, vehicle)) {
            if (!silent) {
                this->myErrorMsgHandler->inform("Vehicle '" + Named::getIDSecure(vehicle) + "' is not allowed on source edge '" + from->getID() + "'.");
            }
            return false;
        }
        if (to != nullptr && (myEdgeInfos[to->getNumericalID()].prohibited || this->isProhibited(to, vehicle))) {
            if (!silent) {
                this->myErrorMsgHandler->inform("Vehicle '" + Named::getIDSecure(vehicle) + "' is not allowed on destination edge '" + to->getID() + "'.");
            }
            return false;
        }
        double length = 0.; // dummy for the via edge cost update
        this->startQuery();
#ifdef DijkstraRouter_DEBUG_QUERY
        std::cout << "DEBUG: starting search for '" << Named::getIDSecure(vehicle) << "' time: " << STEPS2TIME(msTime) << "\n";
#endif
        const SUMOVehicleClass vClass = vehicle == nullptr ? SVC_IGNORING : vehicle->getVClass();
        std::tuple<const E*, const V*, SUMOTime> query = std::make_tuple(from, vehicle, msTime);
        if (this->myBulkMode || (this->myAutoBulkMode && query == myLastQuery)) {
            const auto& toInfo = myEdgeInfos[to->getNumericalID()];
            if (toInfo.visited) {
                buildPathFrom(&toInfo, into);
                this->endQuery(1);
                return true;
            }
        } else {
            init();
            // add begin node
            auto* const fromInfo = &(myEdgeInfos[from->getNumericalID()]);
            fromInfo->effort = 0.;
            fromInfo->prev = nullptr;
            fromInfo->leaveTime = STEPS2TIME(msTime);
            if (myExternalEffort != nullptr) {
                myExternalEffort->setInitialState(fromInfo->edge->getNumericalID());
            }
            myFrontierList.push_back(fromInfo);
        }
        myLastQuery = query;
        // loop
        int num_visited = 0;
        while (!myFrontierList.empty()) {
            num_visited += 1;
            // use the node with the minimal length
            auto* const minimumInfo = myFrontierList.front();
            const E* const minEdge = minimumInfo->edge;
#ifdef DijkstraRouter_DEBUG_QUERY
            std::cout << "DEBUG: hit '" << minEdge->getID() << "' Eff: " << minimumInfo->effort << ", Leave: " << minimumInfo->leaveTime << " Q: ";
            for (auto& it : myFrontierList) {
                std::cout << it->effort << "," << it->edge->getID() << " ";
            }
            std::cout << "\n";
#endif
            // check whether the destination node was already reached
            if (minEdge == to) {
                //propagate last external effort state to destination edge
                if (myExternalEffort != nullptr) {
                    myExternalEffort->update(minEdge->getNumericalID(), minimumInfo->prev->edge->getNumericalID(), minEdge->getLength());
                }
                buildPathFrom(minimumInfo, into);
                this->endQuery(num_visited);
#ifdef DijkstraRouter_DEBUG_QUERY_PERF
                std::cout << "visited " + toString(num_visited) + " edges (final path length=" + toString(into.size()) + " edges=" + toString(into) + ")\n";
#endif
                return true;
            }
            std::pop_heap(myFrontierList.begin(), myFrontierList.end(), myComparator);
            myFrontierList.pop_back();
            myFound.push_back(minimumInfo);
            minimumInfo->visited = true;
            const double effortDelta = this->getEffort(minEdge, vehicle, minimumInfo->leaveTime);
            const double leaveTime = minimumInfo->leaveTime + this->getTravelTime(minEdge, vehicle, minimumInfo->leaveTime, effortDelta);
            if (myExternalEffort != nullptr) {
                myExternalEffort->update(minEdge->getNumericalID(), minimumInfo->prev->edge->getNumericalID(), minEdge->getLength());
            }
            // check all ways from the node with the minimal length
            for (const std::pair<const E*, const E*>& follower : minEdge->getViaSuccessors(vClass)) {
                auto* const followerInfo = &(myEdgeInfos[follower.first->getNumericalID()]);
                // check whether it can be used
                if (followerInfo->prohibited || this->isProhibited(follower.first, vehicle)) {
                    continue;
                }
                double effort = minimumInfo->effort + effortDelta;
                double time = leaveTime;
                this->updateViaEdgeCost(follower.second, vehicle, time, effort, length);
                assert(effort >= minimumInfo->effort);
                assert(time >= minimumInfo->leaveTime);
                const double oldEffort = followerInfo->effort;
                if (!followerInfo->visited && effort < oldEffort) {
                    followerInfo->effort = effort;
                    followerInfo->leaveTime = time;
                    followerInfo->prev = minimumInfo;
                    if (oldEffort == std::numeric_limits<double>::max()) {
                        myFrontierList.push_back(followerInfo);
                        std::push_heap(myFrontierList.begin(), myFrontierList.end(), myComparator);
                    } else {
                        std::push_heap(myFrontierList.begin(),
                                       std::find(myFrontierList.begin(), myFrontierList.end(), followerInfo) + 1,
                                       myComparator);
                    }
                }
            }
        }
        this->endQuery(num_visited);
#ifdef DijkstraRouter_DEBUG_QUERY_PERF
        std::cout << "visited " + toString(num_visited) + " edges (unsuccessful path length: " + toString(into.size()) + ")\n";
#endif
        if (to != nullptr && !mySilent && !silent) {
            this->myErrorMsgHandler->informf("No connection between edge '%' and edge '%' found.", from->getID(), to->getID());
        }
        return false;
    }


    void prohibit(const std::vector<E*>& toProhibit) {
        for (E* const edge : this->myProhibited) {
            myEdgeInfos[edge->getNumericalID()].prohibited = false;
        }
        for (E* const edge : toProhibit) {
            myEdgeInfos[edge->getNumericalID()].prohibited = true;
        }
        this->myProhibited = toProhibit;
    }


    /// Builds the path from marked edges
    void buildPathFrom(const typename SUMOAbstractRouter<E, V>::EdgeInfo* rbegin, std::vector<const E*>& edges) {
        std::vector<const E*> tmp;
        while (rbegin != 0) {
            tmp.push_back(rbegin->edge);
            rbegin = rbegin->prev;
        }
        std::copy(tmp.rbegin(), tmp.rend(), std::back_inserter(edges));
    }

    const typename SUMOAbstractRouter<E, V>::EdgeInfo& getEdgeInfo(int index) const {
        return myEdgeInfos[index];
    }

private:
    DijkstraRouter(const std::vector<typename SUMOAbstractRouter<E, V>::EdgeInfo>& edgeInfos, bool unbuildIsWarning,
                   typename SUMOAbstractRouter<E, V>::Operation effortOperation, typename SUMOAbstractRouter<E, V>::Operation ttOperation, bool silent, EffortCalculator* calc,
                   const bool havePermissions, const bool haveRestrictions) :
        SUMOAbstractRouter<E, V>("DijkstraRouter", unbuildIsWarning, effortOperation, ttOperation, havePermissions, haveRestrictions),
        mySilent(silent),
        myExternalEffort(calc) {
        for (const auto& edgeInfo : edgeInfos) {
            myEdgeInfos.push_back(typename SUMOAbstractRouter<E, V>::EdgeInfo(edgeInfo.edge));
        }
    }

private:
    /// @brief whether to suppress warning/error if no route was found
    bool mySilent;

    /// cache of the last query to enable automated bulk routing
    std::tuple<const E*, const V*, SUMOTime> myLastQuery;

    EffortCalculator* const myExternalEffort;

    /// The container of edge information
    std::vector<typename SUMOAbstractRouter<E, V>::EdgeInfo> myEdgeInfos;

    /// A container for reusage of the min edge heap
    std::vector<typename SUMOAbstractRouter<E, V>::EdgeInfo*> myFrontierList;
    /// @brief list of visited Edges (for resetting)
    std::vector<typename SUMOAbstractRouter<E, V>::EdgeInfo*> myFound;

    EdgeInfoByEffortComparator myComparator;
};
