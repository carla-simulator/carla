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
/// @file    SPTree.h
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    February 2012
///
// Shortest Path tree of limited depth using Dijkstras algorithm
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
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>


template<class E, class C>
class SPTree {

public:
    typedef std::vector<C> CHConnections;
    typedef std::pair<const C*, const C*> CHConnectionPair;
    typedef std::vector<CHConnectionPair> CHConnectionPairs;

    /**
     * @class EdgeInfoByEffortComparator
     * Class to compare (and so sort) nodes by their effort
     */
    class EdgeByTTComparator {
    public:
        /// Comparing method
        bool operator()(const E* a, const E* b) const {
            if (a->traveltime == b->traveltime) {
                return a->edge->getNumericalID() > b->edge->getNumericalID();
            }
            return a->traveltime > b->traveltime;
        }
    };


    /**
     * @brief Constructor
     */
    SPTree(int maxDepth, bool validatePermissions) :
        myMaxDepth(maxDepth),
        myValidatePermissions(validatePermissions) {
    }


    void init() {
        // all EdgeInfos touched in the previous query are either in myFrontier or myFound: clean those up
        for (typename std::vector<E*>::iterator i = myFrontier.begin(); i != myFrontier.end(); i++) {
            (*i)->reset();
        }
        myFrontier.clear();
        for (typename std::vector<E*>::iterator i = myFound.begin(); i != myFound.end(); i++) {
            (*i)->reset();
        }
        myFound.clear();
    }


    /**
     * @brief build a shortest path tree from start to a depth of myMaxdepth. The given
     * edge is excluded from this tree
     */
    void rebuildFrom(E* start, const E* excluded) {
        init();
        start->traveltime = 0;
        start->depth = 0;
        start->permissions = start->edge->getPermissions();
        myFrontier.push_back(start);
        // build SPT
        while (!myFrontier.empty()) {
            E* min = myFrontier.front();
            std::pop_heap(myFrontier.begin(), myFrontier.end(), myCmp);
            myFrontier.pop_back();
            myFound.push_back(min);
            min->visited = true;
            if (min->depth < myMaxDepth) {
                for (typename CHConnections::iterator it = min->followers.begin(); it != min->followers.end(); it++) {
                    C& con = *it;
                    E* follower = con.target;
                    if (follower == excluded) {
                        continue;
                    }
                    const double traveltime = min->traveltime + con.cost;
                    const double oldTraveltime = follower->traveltime;
                    if (!follower->visited && traveltime < oldTraveltime) {
                        follower->traveltime = traveltime;
                        follower->depth = min->depth + 1;
                        follower->permissions = (min->permissions & con.permissions);
                        if (oldTraveltime == std::numeric_limits<double>::max()) {
                            myFrontier.push_back(follower);
                            std::push_heap(myFrontier.begin(), myFrontier.end(), myCmp);
                        } else {
                            std::push_heap(myFrontier.begin(),
                                           std::find(myFrontier.begin(), myFrontier.end(), follower) + 1,
                                           myCmp);
                        }
                    }
                }
            }
        }
    }


    /// @brief whether permissions should be validated;
    inline bool validatePermissions() {
        return myValidatePermissions;
    }

    /// @brief save source/target pair for later validation
    void registerForValidation(const C* aInfo, const C* fInfo) {
        assert(myValidatePermissions);
        myShortcutsToValidate.push_back(CHConnectionPair(aInfo, fInfo));
    }


    /* @brief for each path source->excluded->target try to find a witness with a witness
     * with equal permissions */
    const CHConnectionPairs& getNeededShortcuts(const E* excluded) {
        assert(myValidatePermissions);
        myNeededShortcuts.clear();
        for (typename CHConnectionPairs::iterator it = myShortcutsToValidate.begin(); it != myShortcutsToValidate.end(); ++it) {
            const C* const aInfo = it->first;
            const C* const fInfo = it->second;
            const double bestWitness = dijkstraTT(
                                           aInfo->target, fInfo->target, excluded, (aInfo->permissions & fInfo->permissions));
            const double viaCost = aInfo->cost + fInfo->cost;
            if (viaCost < bestWitness) {
                myNeededShortcuts.push_back(*it);
            }
        }
        myShortcutsToValidate.clear();
        return myNeededShortcuts;
    }


private:
    // perform dijkstra search under permission constraints
    double dijkstraTT(E* start, E* dest, const E* excluded, SVCPermissions permissions) {
        init();
        start->traveltime = 0;
        start->depth = 0;
        myFrontier.push_back(start);
        // build SPT
        while (!myFrontier.empty()) {
            E* min = myFrontier.front();
            if (min == dest) {
                return dest->traveltime;
            }
            std::pop_heap(myFrontier.begin(), myFrontier.end(), myCmp);
            myFrontier.pop_back();
            myFound.push_back(min);
            min->visited = true;
            if (min->depth < myMaxDepth) {
                for (typename CHConnections::iterator it = min->followers.begin(); it != min->followers.end(); it++) {
                    C& con = *it;
                    E* follower = con.target;
                    if (follower == excluded) {
                        continue;
                    }
                    if ((con.permissions & permissions) != permissions) {
                        continue;
                    }
                    const double traveltime = min->traveltime + con.cost;
                    const double oldTraveltime = follower->traveltime;
                    if (!follower->visited && traveltime < oldTraveltime) {
                        follower->traveltime = traveltime;
                        follower->depth = min->depth + 1;
                        follower->permissions = (min->permissions & con.permissions);
                        if (oldTraveltime == std::numeric_limits<double>::max()) {
                            myFrontier.push_back(follower);
                            std::push_heap(myFrontier.begin(), myFrontier.end(), myCmp);
                        } else {
                            std::push_heap(myFrontier.begin(),
                                           std::find(myFrontier.begin(), myFrontier.end(), follower) + 1,
                                           myCmp);
                        }
                    }
                }
            }
        }
        return dest->traveltime;
    }


    // helper method for debugging
    void debugPrintVector(std::vector<E*>& vec, E* start, const E* excluded) {
        std::cout << "computed SPT from '" << start->edge->getID() << "' (excluding " << excluded->edge->getID() <<  ") with " << myFound.size() << " edges\n";
        for (typename std::vector<E*>::iterator it = vec.begin(); it != vec.end(); it++) {
            E* e = *it;
            std::cout << "(" << e->edge->getID() << "," << e->traveltime << ") ";
        }
        std::cout << "\n";
    }

    /// @brief the min edge heap
    std::vector<E*> myFrontier;
    /// @brief the list of visited edges (used when resetting)
    std::vector<E*> myFound;

    /// @brief comparator for search queue
    EdgeByTTComparator myCmp;

    /// @brief maximum search depth
    int myMaxDepth;

    /// @brief whether permissions should be validated
    bool myValidatePermissions;

    /// @brief vector of needed shortcuts after validation
    CHConnectionPairs myShortcutsToValidate;
    /// @brief vector of needed shortcuts after validation
    CHConnectionPairs myNeededShortcuts;
};
