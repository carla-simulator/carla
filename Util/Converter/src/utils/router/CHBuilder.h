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
/// @file    CHBuilder.h
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    February 2012
///
// Contraction Hierarchy Builder for the shortest path search
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
#include <utils/common/SysUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include <utils/router/SUMOAbstractRouter.h>
#include "SPTree.h"

//#define CHRouter_DEBUG_CONTRACTION
//#define CHRouter_DEBUG_CONTRACTION_WITNESSES
//#define CHRouter_DEBUG_CONTRACTION_QUEUE
//#define CHRouter_DEBUG_CONTRACTION_DEGREE
//#define CHRouter_DEBUG_WEIGHTS

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
 * @param PF The prohibition function to use (prohibited_withPermissions/noProhibitions)
 *
 * The router is edge-based. It must know the number of edges for internal reasons
 *  and whether a missing connection between two given edges (unbuild route) shall
 *  be reported as an error or as a warning.
 *
 */
template<class E, class V>
class CHBuilder {

public:
    /// @brief Forward/backward connection with associated forward/backward cost
    // forward connections are used only in forward search
    // backward connections are used only in backwards search
    class Connection {
    public:
        Connection(int t, double c, SVCPermissions p): target(t), cost(c), permissions(p) {}
        int target;
        double cost;
        SVCPermissions permissions;
    };

    typedef std::pair<const E*, const E*> ConstEdgePair;
    typedef std::map<ConstEdgePair, const E*> ShortcutVia;
    struct Hierarchy {
        ShortcutVia shortcuts;
        std::vector<std::vector<Connection> > forwardUplinks;
        std::vector<std::vector<Connection> > backwardUplinks;
    };

    /** @brief Constructor
     * @param[in] validatePermissions Whether a multi-permission hierarchy shall be built
     *            If set to false, the net is pruned in synchronize() and the
     *            hierarchy is tailored to the svc
     */
    CHBuilder(const std::vector<E*>& edges, bool unbuildIsWarning,
              const SUMOVehicleClass svc,
              bool validatePermissions):
        myEdges(edges),
        myErrorMsgHandler(unbuildIsWarning ? MsgHandler::getWarningInstance() : MsgHandler::getErrorInstance()),
        mySPTree(new SPTree<CHInfo, CHConnection>(4, validatePermissions)),
        mySVC(svc),
        myUpdateCount(0) {
        for (typename std::vector<E*>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            myCHInfos.push_back(CHInfo(*i));
        }
    }

    /// Destructor
    virtual ~CHBuilder() {
        delete mySPTree;
    }


    const Hierarchy* buildContractionHierarchy(SUMOTime time, const V* const vehicle, const SUMOAbstractRouter<E, V>* effortProvider) {
        Hierarchy* result = new Hierarchy();
        const int numEdges = (int)myCHInfos.size();
        const std::string vClass = (mySPTree->validatePermissions() ?
                                    "all vehicle classes " : "vClass='" + SumoVehicleClassStrings.getString(mySVC) + "' ");
        PROGRESS_BEGIN_MESSAGE("Building Contraction Hierarchy for " + vClass
                               + "and time=" + time2string(time) + " (" + toString(numEdges) + " edges)\n");
        const long startMillis = SysUtils::getCurrentMillis();
        // init queue
        std::vector<CHInfo*> queue; // max heap: edge to be contracted is front
        // reset previous connections etc
        for (int i = 0; i < numEdges; i++) {
            myCHInfos[i].resetContractionState();
            result->forwardUplinks.push_back(std::vector<Connection>());
            result->backwardUplinks.push_back(std::vector<Connection>());
        }
        // copy connections from the original net
        const double time_seconds = STEPS2TIME(time); // timelines store seconds!
        for (int i = 0; i < numEdges; i++) {
            synchronize(myCHInfos[i], time_seconds, vehicle, effortProvider);
        }
        // synchronization is finished. now we can compute priorities for the first time
        for (int i = 0; i < numEdges; i++) {
            myCHInfos[i].updatePriority(mySPTree);
            queue.push_back(&(myCHInfos[i]));
        }
        std::make_heap(queue.begin(), queue.end(), myCmp);
        int contractionRank = 0;
        // contraction loop
        while (!queue.empty()) {
            while (tryUpdateFront(queue)) {}
            CHInfo* max = queue.front();
            max->rank = contractionRank;
#ifdef CHRouter_DEBUG_CONTRACTION
            std::cout << "contracting '" << max->edge->getID() << "' with prio: " << max->priority << " (rank " << contractionRank << ")\n";
#endif
            const E* const edge = max->edge;
            // add outgoing connections to the forward search
            const int edgeID = edge->getNumericalID();
            for (typename CHConnections::const_iterator it = max->followers.begin(); it != max->followers.end(); it++) {
                const CHConnection& con = *it;
                result->forwardUplinks[edgeID].push_back(Connection(con.target->edge->getNumericalID(), con.cost, con.permissions));
                disconnect(con.target->approaching, max);
                con.target->updatePriority(0);
            }
            // add incoming connections to the backward search
            for (typename CHConnections::const_iterator it = max->approaching.begin(); it != max->approaching.end(); it++) {
                const CHConnection& con = *it;
                result->backwardUplinks[edgeID].push_back(Connection(con.target->edge->getNumericalID(), con.cost, con.permissions));
                disconnect(con.target->followers, max);
                con.target->updatePriority(0);
            }
            // add shortcuts to the net
            for (typename std::vector<Shortcut>::const_iterator it = max->shortcuts.begin(); it != max->shortcuts.end(); it++) {
                const ConstEdgePair& edgePair = it->edgePair;
                result->shortcuts[edgePair] = edge;
                CHInfo* from = getCHInfo(edgePair.first);
                CHInfo* to = getCHInfo(edgePair.second);
                from->followers.push_back(CHConnection(to, it->cost, it->permissions, it->underlying));
                to->approaching.push_back(CHConnection(from, it->cost, it->permissions, it->underlying));
            }
            // if you need to debug the chrouter with MSVC uncomment the following line, hierarchy building will get slower and the hierarchy may change though
            //std::make_heap(queue.begin(), queue.end(), myCmp);
            // remove from queue
            std::pop_heap(queue.begin(), queue.end(), myCmp);
            queue.pop_back();
            /*
            if (contractionRank % 10000 == 0) {
                // update all and rebuild queue
                for (typename std::vector<CHInfo*>::iterator it = queue.begin(); it != queue.end(); ++it) {
                    (*it)->updatePriority(mySPTree);
                }
                std::make_heap(queue.begin(), queue.end(), myCmp);
            }
            */
            contractionRank++;
        }
        // reporting
        const long duration = SysUtils::getCurrentMillis() - startMillis;
        WRITE_MESSAGE("Created " + toString(result->shortcuts.size()) + " shortcuts.");
        WRITE_MESSAGE("Recomputed priority " + toString(myUpdateCount) + " times.");
        MsgHandler::getMessageInstance()->endProcessMsg("done (" + toString(duration) + "ms).");
        PROGRESS_DONE_MESSAGE();
        myUpdateCount = 0;
        return result;
    }

private:
    struct Shortcut {
        Shortcut(ConstEdgePair e, double c, int u, SVCPermissions p):
            edgePair(e), cost(c), underlying(u), permissions(p) {}
        ConstEdgePair edgePair;
        double cost;
        int underlying;
        SVCPermissions permissions;
    };


    class CHInfo;

    /// @brief Forward/backward connection with associated FORWARD cost
    class CHConnection {
    public:
        CHConnection(CHInfo* t, double c, SVCPermissions p, int u):
            target(t), cost(c), permissions(p), underlying(u) {}
        CHInfo* target;
        double cost;
        SVCPermissions permissions;
        /// the number of connections underlying this connection
        int underlying;
    };

    typedef std::vector<CHConnection> CHConnections;
    typedef std::pair<const CHConnection*, const CHConnection*> CHConnectionPair;
    typedef std::vector<CHConnectionPair> CHConnectionPairs;

    /* @brief container class to use when building the contraction hierarchy.
     * instances are reused every time the hierarchy is rebuilt (new time slice)
     * but they must be synchronized first */
    class CHInfo {
    public:
        /// @brief Constructor
        CHInfo(const E* e) :
            edge(e),
            priority(0.),
            contractedNeighbors(0),
            rank(-1),
            level(0),
            underlyingTotal(0),
            visited(false),
            traveltime(std::numeric_limits<double>::max()),
            depth(0),
            permissions(SVC_IGNORING) {
        }

        /// @brief recompute the contraction priority and report whether it changed
        bool updatePriority(SPTree<CHInfo, CHConnection>* spTree) {
            if (spTree != 0) {
                updateShortcuts(spTree);
                updateLevel();
            } else {
                contractedNeighbors += 1; // called when a connected edge was contracted
            }
            const double oldPriority = priority;
            // priority term as used by abraham []
            const int edge_difference = (int)followers.size() + (int)approaching.size() - 2 * (int)shortcuts.size();
            priority = (double)(2 * edge_difference - contractedNeighbors - underlyingTotal - 5 * level);
            return priority != oldPriority;
        }

        /// compute needed shortcuts when contracting this edge
        void updateShortcuts(SPTree<CHInfo, CHConnection>* spTree) {
            const bool validatePermissions = spTree->validatePermissions();
#ifdef CHRouter_DEBUG_CONTRACTION_DEGREE
            const int degree = (int)approaching.size() + (int)followers.size();
            std::cout << "computing shortcuts for '" + edge->getID() + "' with degree " + toString(degree) + "\n";
#endif
            shortcuts.clear();
            underlyingTotal = 0;
            for (typename CHConnections::iterator it_a = approaching.begin(); it_a != approaching.end(); it_a++) {
                CHConnection& aInfo = *it_a;
                // build shortest path tree in a fixed neighborhood
                spTree->rebuildFrom(aInfo.target, this);
                for (typename CHConnections::iterator it_f = followers.begin(); it_f != followers.end(); it_f++) {
                    CHConnection& fInfo = *it_f;
                    const double viaCost = aInfo.cost + fInfo.cost;
                    const SVCPermissions viaPermissions = (aInfo.permissions & fInfo.permissions);
                    if (fInfo.target->traveltime > viaCost) {
                        // found no faster path -> we need a shortcut via edge
#ifdef CHRouter_DEBUG_CONTRACTION_WITNESSES
                        debugNoWitness(aInfo, fInfo);
#endif
                        const int underlying = aInfo.underlying + fInfo.underlying;
                        underlyingTotal += underlying;
                        shortcuts.push_back(Shortcut(ConstEdgePair(aInfo.target->edge, fInfo.target->edge),
                                                     viaCost, underlying, viaPermissions));

                    } else if (validatePermissions) {
                        if ((fInfo.target->permissions & viaPermissions) != viaPermissions) {
                            // witness has weaker restrictions. try to find another witness
                            spTree->registerForValidation(&aInfo, &fInfo);
                        } else {
#ifdef CHRouter_DEBUG_CONTRACTION_WITNESSES
                            debugNoWitness(aInfo, fInfo);
#endif
                        }
                    } else {
#ifdef CHRouter_DEBUG_CONTRACTION_WITNESSES
                        debugNoWitness(aInfo, fInfo);
#endif
                    }
                }
            }
            // insert shortcuts needed due to unmet permissions
            if (validatePermissions) {
                const CHConnectionPairs& pairs = spTree->getNeededShortcuts(this);
                for (typename CHConnectionPairs::const_iterator it = pairs.begin(); it != pairs.end(); ++it) {
                    const CHConnection* aInfo = it->first;
                    const CHConnection* fInfo = it->second;
                    const double viaCost = aInfo->cost + fInfo->cost;
                    const SVCPermissions viaPermissions = (aInfo->permissions & fInfo->permissions);
                    const int underlying = aInfo->underlying + fInfo->underlying;
                    underlyingTotal += underlying;
                    shortcuts.push_back(Shortcut(ConstEdgePair(aInfo->target->edge, fInfo->target->edge),
                                                 viaCost, underlying, viaPermissions));
                }
            }
        }


        // update level as defined by Abraham
        void updateLevel() {
            int maxLower = std::numeric_limits<int>::min();
            int otherRank;
            for (typename CHConnections::iterator it = approaching.begin(); it != approaching.end(); it++) {
                otherRank = it->target->rank;
                if (otherRank < rank) {
                    maxLower = MAX2(rank, maxLower);
                }
            }
            for (typename CHConnections::iterator it = followers.begin(); it != followers.end(); it++) {
                otherRank = it->target->rank;
                if (otherRank < rank) {
                    maxLower = MAX2(rank, maxLower);
                }
            }
            if (maxLower == std::numeric_limits<int>::min()) {
                level = 0;
            } else {
                level = maxLower + 1;
            }
        }

        // resets state before rebuilding the hierarchy
        void resetContractionState() {
            contractedNeighbors = 0;
            rank = -1;
            level = 0;
            underlyingTotal = 0;
            shortcuts.clear();
            followers.clear();
            approaching.clear();
        }


        /// @brief The current edge - not const since it may receive shortcut edges
        const E* edge;
        /// @brief The contraction priority
        double priority;
        /// @brief The needed shortcuts
        std::vector<Shortcut> shortcuts;
        /// @brief priority subterms
        int contractedNeighbors;
        int rank;
        int level;
        int underlyingTotal;

        /// @brief connections (only valid after synchronization)
        CHConnections followers;
        CHConnections approaching;


        /// members used in SPTree
        bool visited;
        /// Effort to reach the edge
        double traveltime;
        /// number of edges from start
        int depth;
        /// the permissions when reaching this edge on the fastest path
        // @note: we may miss some witness paths by making traveltime the only
        // criteria durinng search
        SVCPermissions permissions;

        inline void reset() {
            traveltime = std::numeric_limits<double>::max();
            visited = false;
        }


        /// debugging methods
        inline void debugNoWitness(const CHConnection& aInfo, const CHConnection& fInfo) {
            std::cout << "adding shortcut between " << aInfo.target->edge->getID() << ", " << fInfo.target->edge->getID() << " via " << edge->getID() << "\n";
        }

        inline void debugWitness(const CHConnection& aInfo, const CHConnection& fInfo) {
            const double viaCost = aInfo.cost + fInfo.cost;
            std::cout << "found witness with length " << fInfo.target->traveltime << " against via " << edge->getID() << " (length " << viaCost << ") for " << aInfo.target->edge->getID() << ", " << fInfo.target->edge->getID() << "\n";
        }

    };


    /**
     * @class EdgeInfoByRankComparator
     * Class to compare (and so sort) nodes by their contraction priority
     */
    class CHInfoComparator {
    public:
        /// Comparing method
        bool operator()(const CHInfo* a, const CHInfo* b) const {
            if (a->priority == b->priority) {
                return a->edge->getNumericalID() > b->edge->getNumericalID();
            } else {
                return a->priority < b->priority;
            };
        }
    };


    inline CHInfo* getCHInfo(const E* const edge) {
        return &(myCHInfos[edge->getNumericalID()]);
    }


    /// @brief copy connections from the original net (modified destructively during contraction)
    void synchronize(CHInfo& info, double time, const V* const vehicle, const SUMOAbstractRouter<E, V>* effortProvider) {
        // forward and backward connections are used only in forward search,
        // thus approaching costs are those of the approaching edge and not of the edge itself
        const bool prune = !mySPTree->validatePermissions();
        const E* const edge = info.edge;
        if (prune && ((edge->getPermissions() & mySVC) != mySVC)) {
            return;
        }
        const double baseCost = effortProvider->getEffort(edge, vehicle, time);

        for (const std::pair<const E*, const E*>& successor : edge->getViaSuccessors(mySVC)) {
            const E* fEdge = successor.first;
            if (prune && ((fEdge->getPermissions() & mySVC) != mySVC)) {
                continue;
            }
            CHInfo* const follower = getCHInfo(fEdge);
            const SVCPermissions permissions = (edge->getPermissions() & fEdge->getPermissions());
            double cost = baseCost;
            const E* viaEdge = successor.second;
            while (viaEdge != nullptr && viaEdge->isInternal()) {
                cost += effortProvider->getEffort(viaEdge, vehicle, time);
                viaEdge = viaEdge->getViaSuccessors().front().first;
            }
            info.followers.push_back(CHConnection(follower, cost, permissions, 1));
            follower->approaching.push_back(CHConnection(&info, cost, permissions, 1));
        }
#ifdef CHRouter_DEBUG_WEIGHTS
        std::cout << time << ": " << edge->getID() << " cost: " << cost << "\n";
#endif
        // @todo: check whether we even need to save approaching in ROEdge;
    }


    /// @brief remove all connections to/from the given edge (assume it exists only once)
    void disconnect(CHConnections& connections, CHInfo* other) {
        for (typename CHConnections::iterator it = connections.begin(); it != connections.end(); it++) {
            if (it->target == other) {
                connections.erase(it);
                return;
            }
        }
        assert(false);
    }

    /** @brief tries to update the priority of the first edge
     * @return wether updating changed the first edge
     */
    bool tryUpdateFront(std::vector<CHInfo*>& queue) {
        myUpdateCount++;
        CHInfo* max = queue.front();
#ifdef CHRouter_DEBUG_CONTRACTION_QUEUE
        std::cout << "updating '" << max->edge->getID() << "'\n";
        debugPrintQueue(queue);
#endif
        if (max->updatePriority(mySPTree)) {
            std::pop_heap(queue.begin(), queue.end(), myCmp);
            std::push_heap(queue.begin(), queue.end(), myCmp);
            return true;
        } else {
            return false;
        }
    }

    // helper method for debugging
    void debugPrintQueue(std::vector<CHInfo*>& queue) {
        for (typename std::vector<CHInfo*>::iterator it = queue.begin(); it != queue.end(); it++) {
            CHInfo* chInfo = *it;
            std::cout << "(" << chInfo->edge->getID() << "," << chInfo->priority << ") ";
        }
        std::cout << "\n";
    }

private:
    /// @brief all edges with numerical ids
    const std::vector<E*>& myEdges;

    /// @brief the handler for routing errors
    MsgHandler* const myErrorMsgHandler;

    /// @brief static vector for lookup
    std::vector<CHInfo> myCHInfos;

    /// @brief Comparator for contraction priority
    CHInfoComparator myCmp;

    /// @brief the shortest path tree to use when searching for shortcuts
    SPTree<CHInfo, CHConnection>* mySPTree;

    /// @brief the permissions for which the hierarchy was constructed
    const SUMOVehicleClass mySVC;

    /// @brief counters for performance logging
    int myUpdateCount;

private:
    /// @brief Invalidated assignment operator
    CHBuilder& operator=(const CHBuilder& s);
};
