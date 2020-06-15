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
/// @file    CHRouter.h
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    February 2012
///
// Shortest Path search using a Contraction Hierarchy
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <functional>
#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <iterator>
#include <deque>
#include <utils/common/SysUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include <utils/router/SUMOAbstractRouter.h>
#include "CHBuilder.h"

//#define CHRouter_DEBUG_QUERY
//#define CHRouter_DEBUG_QUERY_PERF

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class CHRouter
 * @brief Computes the shortest path through a contracted network
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
class CHRouter: public SUMOAbstractRouter<E, V> {

public:
    /// A meeting point of the two search scopes
    typedef std::pair<const typename SUMOAbstractRouter<E, V>::EdgeInfo*, const typename SUMOAbstractRouter<E, V>::EdgeInfo*> Meeting;

    /**
     * @class Unidirectional
     * class for searching in one direction
     */
    class Unidirectional {
    public:
        /// @brief Constructor
        Unidirectional(const std::vector<E*>& edges, bool forward):
            myAmForward(forward),
            myVehicle(0) {
            for (const E* const e : edges) {
                myEdgeInfos.push_back(typename SUMOAbstractRouter<E, V>::EdgeInfo(e));
            }
        }

        inline bool found(const E* const edge) const {
            return myFound.count(edge) > 0;
        }

        inline typename SUMOAbstractRouter<E, V>::EdgeInfo* getEdgeInfo(const E* const edge) {
            return &(myEdgeInfos[edge->getNumericalID()]);
        }

        inline const typename SUMOAbstractRouter<E, V>::EdgeInfo* getEdgeInfo(const E* const edge) const {
            return &(myEdgeInfos[edge->getNumericalID()]);
        }

        /**
         * @class EdgeInfoByEffortComparator
         * Class to compare (and so sort) nodes by their effort
         */
        class EdgeInfoByTTComparator {
        public:
            /// Comparing method
            bool operator()(const typename SUMOAbstractRouter<E, V>::EdgeInfo* nod1, const typename SUMOAbstractRouter<E, V>::EdgeInfo* nod2) const {
                if (nod1->effort == nod2->effort) {
                    return nod1->edge->getNumericalID() > nod2->edge->getNumericalID();
                }
                return nod1->effort > nod2->effort;
            }
        };


        void init(const E* const start, const V* const vehicle) {
            assert(vehicle != 0);
            // all EdgeInfos touched in the previous query are either in myFrontier or myFound: clean those up
            for (auto ei : myFrontier) {
                ei->reset();
            }
            myFrontier.clear();
            for (auto edge : myFound) {
                getEdgeInfo(edge)->reset();
            }
            myFound.clear();
            myVehicle = vehicle;
            auto startInfo = getEdgeInfo(start);
            startInfo->effort = 0.;
            startInfo->prev = nullptr;
            myFrontier.push_back(startInfo);
        }


        typedef std::vector<typename CHBuilder<E, V>::Connection> ConnectionVector;
        /** @brief explore on element from the frontier,update minTTSeen and meeting
         * if an EdgeInfo found by the otherSearch is encountered
         * returns whether stepping should continue
         */
        bool step(const std::vector<ConnectionVector>& uplinks, const Unidirectional& otherSearch, double& minTTSeen, Meeting& meeting) {
            // pop the node with the minimal length
            auto* const minimumInfo = myFrontier.front();
            std::pop_heap(myFrontier.begin(), myFrontier.end(), myComparator);
            myFrontier.pop_back();
            // check for a meeting with the other search
            const E* const minEdge = minimumInfo->edge;
#ifdef CHRouter_DEBUG_QUERY
            std::cout << "DEBUG: " << (myAmForward ? "Forward" : "Backward") << " hit '" << minEdge->getID() << "' Q: ";
            for (typename std::vector<EdgeInfo*>::iterator it = myFrontier.begin(); it != myFrontier.end(); it++) {
                std::cout << (*it)->traveltime << "," << (*it)->edge->getID() << " ";
            }
            std::cout << "\n";
#endif
            if (otherSearch.found(minEdge)) {
                const auto* const otherInfo = otherSearch.getEdgeInfo(minEdge);
                const double ttSeen = minimumInfo->effort + otherInfo->effort;
#ifdef CHRouter_DEBUG_QUERY
                std::cout << "DEBUG: " << (myAmForward ? "Forward" : "Backward") << "-Search hit other search at '" << minEdge->getID() << "', tt: " << ttSeen << " \n";
#endif
                if (ttSeen < minTTSeen) {
                    minTTSeen = ttSeen;
                    if (myAmForward) {
                        meeting.first = minimumInfo;
                        meeting.second = otherInfo;
                    } else {
                        meeting.first = otherInfo;
                        meeting.second = minimumInfo;
                    }
                }
            }
            // prepare next steps
            minimumInfo->visited = true;
            // XXX we only need to keep found elements if they have a higher rank than the lowest rank in the other search queue
            myFound.insert(minimumInfo->edge);
            for (const auto& uplink : uplinks[minEdge->getNumericalID()]) {
                const auto upwardInfo = &myEdgeInfos[uplink.target];
                const double effort = minimumInfo->effort + uplink.cost;
                const SUMOVehicleClass svc = myVehicle->getVClass();
                // check whether it can be used
                if ((uplink.permissions & svc) != svc) {
                    continue;
                }
                const double oldEffort = upwardInfo->effort;
                if (!upwardInfo->visited && effort < oldEffort) {
                    upwardInfo->effort = effort;
                    upwardInfo->prev = minimumInfo;
                    if (oldEffort == std::numeric_limits<double>::max()) {
                        myFrontier.push_back(upwardInfo);
                        std::push_heap(myFrontier.begin(), myFrontier.end(), myComparator);
                    } else {
                        std::push_heap(myFrontier.begin(),
                                       std::find(myFrontier.begin(), myFrontier.end(), upwardInfo) + 1,
                                       myComparator);
                    }
                }
            }
            // @note: this effectively does a full dijkstra search.
            // the effort compared to the naive stopping criterion is thus
            // quadrupled. We could implement a better stopping criterion (Holte)
            // However since the search shall take place in a contracted graph
            // it probably does not matter
            return !myFrontier.empty() && myFrontier.front()->effort < minTTSeen;
        }

    private:
        /// @brief the role of this search
        bool myAmForward;
        /// @brief the min edge heap
        std::vector<typename SUMOAbstractRouter<E, V>::EdgeInfo*> myFrontier;
        /// @brief the set of visited (settled) Edges
        std::set<const E*> myFound;
        /// @brief The container of edge information
        std::vector<typename SUMOAbstractRouter<E, V>::EdgeInfo> myEdgeInfos;

        EdgeInfoByTTComparator myComparator;

        const V* myVehicle;

    };

    /** @brief Constructor
     * @param[in] validatePermissions Whether a multi-permission hierarchy shall be built
     *            If set to false, the net is pruned in synchronize() and the
     *            hierarchy is tailored to the svc
     */
    CHRouter(const std::vector<E*>& edges, bool unbuildIsWarning, typename SUMOAbstractRouter<E, V>::Operation operation,
             const SUMOVehicleClass svc,
             SUMOTime weightPeriod,
             const bool havePermissions, const bool haveRestrictions):
        SUMOAbstractRouter<E, V>("CHRouter", unbuildIsWarning, operation, nullptr, havePermissions, haveRestrictions),
        myEdges(edges),
        myForwardSearch(edges, true),
        myBackwardSearch(edges, false),
        myHierarchyBuilder(new CHBuilder<E, V>(edges, unbuildIsWarning, svc, havePermissions)),
        myHierarchy(nullptr),
        myWeightPeriod(weightPeriod),
        myValidUntil(0),
        mySVC(svc) {
    }

    /** @brief Cloning constructor, should be used only for time independent instances which build a hierarchy only once
     */
    CHRouter(const std::vector<E*>& edges, bool unbuildIsWarning, typename SUMOAbstractRouter<E, V>::Operation operation,
             const SUMOVehicleClass svc,
             const typename CHBuilder<E, V>::Hierarchy* hierarchy,
             const bool havePermissions, const bool haveRestrictions) :
        SUMOAbstractRouter<E, V>("CHRouterClone", unbuildIsWarning, operation, nullptr, havePermissions, haveRestrictions),
        myEdges(edges),
        myForwardSearch(edges, true),
        myBackwardSearch(edges, false),
        myHierarchyBuilder(nullptr),
        myHierarchy(hierarchy),
        myWeightPeriod(SUMOTime_MAX),
        myValidUntil(SUMOTime_MAX),
        mySVC(svc) {
    }

    /// Destructor
    virtual ~CHRouter() {
        if (myHierarchyBuilder != nullptr) {
            delete myHierarchy;
            delete myHierarchyBuilder;
        }
    }


    virtual SUMOAbstractRouter<E, V>* clone() {
        if (myWeightPeriod == SUMOTime_MAX) {
            // we only need one hierarchy
            return new CHRouter<E, V>(myEdges, this->myErrorMsgHandler == MsgHandler::getWarningInstance(), this->myOperation,
                                      mySVC, myHierarchy, this->myHavePermissions, this->myHaveRestrictions);
        }
        return new CHRouter<E, V>(myEdges, this->myErrorMsgHandler == MsgHandler::getWarningInstance(), this->myOperation,
                                  mySVC, myWeightPeriod, this->myHavePermissions, this->myHaveRestrictions);
    }

    /** @brief Builds the route between the given edges using the minimum traveltime in the contracted graph
     * @note: since the contracted graph is static (weights averaged over time)
     * the computed routes only approximated shortest paths in the real graph
     * */
    virtual bool compute(const E* from, const E* to, const V* const vehicle,
                         SUMOTime msTime, std::vector<const E*>& into, bool silent = false) {
        assert(from != nullptr && to != nullptr);
        // assert(myHierarchyBuilder.mySPTree->validatePermissions() || vehicle->getVClass() == mySVC || mySVC == SVC_IGNORING);
        // do we need to rebuild the hierarchy?
        if (msTime >= myValidUntil) {
            assert(myHierarchyBuilder != nullptr); // only time independent clones do not have a builder
            while (msTime >= myValidUntil) {
                myValidUntil += myWeightPeriod;
            }
            delete myHierarchy;
            myHierarchy = myHierarchyBuilder->buildContractionHierarchy(myValidUntil - myWeightPeriod, vehicle, this);
        }
        // ready for routing
        this->startQuery();
        myForwardSearch.init(from, vehicle);
        myBackwardSearch.init(to, vehicle);
        double minTTSeen = std::numeric_limits<double>::max();
        Meeting meeting(nullptr, nullptr);
        bool continueForward = true;
        bool continueBackward = true;
        int num_visited_fw = 0;
        int num_visited_bw = 0;
        bool result = true;
        while (continueForward || continueBackward) {
            if (continueForward) {
                continueForward = myForwardSearch.step(myHierarchy->forwardUplinks, myBackwardSearch, minTTSeen, meeting);
                num_visited_fw += 1;
            }
            if (continueBackward) {
                continueBackward = myBackwardSearch.step(myHierarchy->backwardUplinks, myForwardSearch, minTTSeen, meeting);
                num_visited_bw += 1;
            }
        }
        if (minTTSeen < std::numeric_limits<double>::max()) {
            buildPathFromMeeting(meeting, into);
        } else {
            if (!silent) {
                this->myErrorMsgHandler->informf("No connection between edge '%' and edge '%' found.", from->getID(), to->getID());
            }
            result = false;
        }
#ifdef CHRouter_DEBUG_QUERY_PERF
        std::cout << "visited " << num_visited_fw + num_visited_bw << " edges (" << num_visited_fw << "," << num_visited_bw << ") ,final path length: " + toString(into.size()) + ")\n";
#endif
        this->endQuery(num_visited_bw + num_visited_fw);
        return result;
    }

    /// normal routing methods

    /// Builds the path from marked edges
    void buildPathFromMeeting(Meeting meeting, std::vector<const E*>& into) const {
        std::deque<const E*> tmp;
        const auto* backtrack = meeting.first;
        while (backtrack != 0) {
            tmp.push_front((E*) backtrack->edge);  // !!!
            backtrack = backtrack->prev;
        }
        backtrack = meeting.second->prev; // don't use central edge twice
        while (backtrack != 0) {
            tmp.push_back((E*) backtrack->edge);  // !!!
            backtrack = backtrack->prev;
        }
        // expand shortcuts
        const E* prev = 0;
        while (!tmp.empty()) {
            const E* cur = tmp.front();
            tmp.pop_front();
            if (prev == 0) {
                into.push_back(cur);
                prev = cur;
            } else {
                const E* via = getVia(prev, cur);
                if (via == 0) {
                    into.push_back(cur);
                    prev = cur;
                } else {
                    tmp.push_front(cur);
                    tmp.push_front(via);
                }
            }
        }
    }

private:
    // retrieve the via edge for a shortcut
    const E* getVia(const E* forwardFrom, const E* forwardTo) const {
        typename CHBuilder<E, V>::ConstEdgePair forward(forwardFrom, forwardTo);
        typename CHBuilder<E, V>::ShortcutVia::const_iterator it = myHierarchy->shortcuts.find(forward);
        if (it != myHierarchy->shortcuts.end()) {
            return it->second;
        } else {
            return 0;
        }
    }


private:
    /// @brief all edges with numerical ids
    const std::vector<E*>& myEdges;

    /// @brief the unidirectional search queues
    Unidirectional myForwardSearch;
    Unidirectional myBackwardSearch;

    CHBuilder<E, V>* myHierarchyBuilder;
    const typename CHBuilder<E, V>::Hierarchy* myHierarchy;

    /// @brief the validity duration of one weight interval
    const SUMOTime myWeightPeriod;

    /// @brief the validity duration of the current hierarchy (exclusive)
    SUMOTime myValidUntil;

    /// @brief the permissions for which the hierarchy was constructed
    const SUMOVehicleClass mySVC;
};
