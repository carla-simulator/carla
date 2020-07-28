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
/// @file    RailEdge.h
/// @author  Jakob Erdmann
/// @date    26.02.2020
///
// The RailEdge is a wrapper around a ROEdge or a MSEdge used for railway routing
/****************************************************************************/
#pragma once
#include <config.h>
#include <cassert>

//#define RailEdge_DEBUG_TURNS
//#define RailEdge_DEBUG_INIT
//#define RailEdge_DEBUG_SUCCESSORS
#define RailEdge_DEBUGID ""
//#define RailEdge_DEBUG_COND(obj) ((obj != 0 && (obj)->getID() == RailEdge_DEBUGID))
#define RailEdge_DEBUG_COND(obj) (true)

// ===========================================================================
// class definitions
// ===========================================================================
/// @brief the edge type representing backward edges
template<class E, class V>
class RailEdge {
public:
    typedef RailEdge<E, V> _RailEdge;
    typedef std::vector<std::pair<const _RailEdge*, const _RailEdge*> > ConstEdgePairVector;

    RailEdge(const E* orig) :
        myNumericalID(orig->getNumericalID()),
        myOriginal(orig),
        myTurnaround(nullptr),
        myIsVirtual(true)
    { }

    RailEdge(const E* turnStart, const E* turnEnd, int numericalID) :
        myNumericalID(numericalID),
        myID("TrainReversal!" + turnStart->getID() + "->" + turnEnd->getID()),
        myOriginal(nullptr),
        myTurnaround(nullptr),
        myIsVirtual(true),
        myMaxLength(turnStart->getLength()),
        myStartLength(turnStart->getLength()) {
        myViaSuccessors.push_back(std::make_pair(turnEnd->getRailwayRoutingEdge(), nullptr));
    }

    void update(double maxTrainLength, const std::vector<const E*>& replacementEdges) {
        if (maxTrainLength > myMaxLength) {
            myMaxLength = maxTrainLength;
            myReplacementEdges = replacementEdges;
#ifdef RailEdge_DEBUG_INIT
            std::cout << "    update RailEdge " << getID() << " myMaxLength=" << myMaxLength << " repl=" << toString(myReplacementEdges) << "\n";
#endif
        }
    }

    void addVirtualTurns(const E* forward, const E* backward,
                         std::vector<_RailEdge*>& railEdges, int& numericalID, double dist,
                         double maxTrainLength, const std::vector<const E*>& replacementEdges) {
        // search backwards until dist and add virtual turnaround edges with
        // replacement edges up to the real turnaround
#ifdef RailEdge_DEBUG_INIT
        std::cout << "addVirtualTurns forward=" << forward->getID() << " backward=" << backward->getID() << " dist=" << dist
                  << " maxLength=" << maxTrainLength << " repl=" << toString(replacementEdges) << "\n";
#endif
        if (dist <= 0) {
            return;
        }
        for (const E* prev : forward->getPredecessors()) {
            if (prev == backward) {
                continue;
            }
            const E* bidi = prev->getBidiEdge();
            if (backward->isConnectedTo(*bidi, SVC_IGNORING)) {
                _RailEdge* prevRailEdge = prev->getRailwayRoutingEdge();
                if (prevRailEdge->myTurnaround == nullptr) {
                    prevRailEdge->myTurnaround = new _RailEdge(prev, bidi, numericalID++);
                    prevRailEdge->myViaSuccessors.push_back(std::make_pair(prevRailEdge->myTurnaround, nullptr));
                    railEdges.push_back(prevRailEdge->myTurnaround);
#ifdef RailEdge_DEBUG_INIT
                    std::cout << "  RailEdge " << prevRailEdge->getID() << " virtual turnaround " << prevRailEdge->myTurnaround->getID() << "\n";
#endif
                }
                prevRailEdge->myTurnaround->update(prev->getLength() + maxTrainLength, replacementEdges);
                std::vector<const E*> replacementEdges2;
                replacementEdges2.push_back(prev);
                replacementEdges2.insert(replacementEdges2.end(), replacementEdges.begin(), replacementEdges.end());
                addVirtualTurns(prev, bidi, railEdges, numericalID, dist - prev->getLength(),
                                maxTrainLength + prev->getLength(), replacementEdges2);
            }
        }
    }

    void init(std::vector<_RailEdge*>& railEdges, int& numericalID, double maxTrainLength) {
        // replace turnaround-via with an explicit RailEdge that checks length
        for (const auto& viaPair : myOriginal->getViaSuccessors()) {
            if (viaPair.first == myOriginal->getBidiEdge()) {
                // direction reversal
                if (myTurnaround == nullptr) {
                    myTurnaround = new _RailEdge(myOriginal, viaPair.first, numericalID++);
                    myViaSuccessors.push_back(std::make_pair(myTurnaround, nullptr));
                    railEdges.push_back(myTurnaround);
#ifdef RailEdge_DEBUG_INIT
                    std::cout << "RailEdge " << getID() << " actual turnaround " << myTurnaround->getID() << "\n";
#endif
                }
                myTurnaround->myIsVirtual = false;
                addVirtualTurns(myOriginal, viaPair.first, railEdges, numericalID,
                                maxTrainLength - getLength(), getLength(), std::vector<const E*> {myOriginal});
            } else {
                myViaSuccessors.push_back(std::make_pair(viaPair.first->getRailwayRoutingEdge(),
                                          viaPair.second == nullptr ? nullptr : viaPair.second->getRailwayRoutingEdge()));
            }
        }
#ifdef RailEdge_DEBUG_SUCCESSORS
        std::cout << "RailEdge " << getID() << " successors=" << myViaSuccessors.size() << " orig=" << myOriginal->getViaSuccessors().size() << "\n";
        for (const auto& viaPair : myViaSuccessors) {
            std::cout << "    " << viaPair.first->getID() << "\n";
        }
#endif
    }

    /// @brief Returns the index (numeric id) of the edge
    inline int getNumericalID() const {
        return myNumericalID;
    }

    /// @brief Returns the original edge
    const E* getOriginal() const {
        return myOriginal;
    }

    /** @brief Returns the id of the edge
     * @return The original edge's id
     */
    const std::string& getID() const {
        return myOriginal != nullptr ? myOriginal->getID() : myID;
    }

    void insertOriginalEdges(double length, std::vector<const E*>& into) const {
        if (myOriginal != nullptr) {
            into.push_back(myOriginal);
        } else {
            double seen = myStartLength;
            int nPushed = 0;
            if (seen >= length && !myIsVirtual) {
                return;
            }
            // we need to find a replacement edge that has a real turn
            for (const E* edge : myReplacementEdges) {
                into.push_back(edge);
                nPushed++;
                seen += edge->getLength();
                if (seen >= length && edge->isConnectedTo(*edge->getBidiEdge(), SVC_IGNORING)) {
                    break;
                }
                //std::cout << "insertOriginalEdges length=" << length << " seen=" << seen << " into=" << toString(into) << "\n";
            }
            const int last = (int)into.size() - 1;
            for (int i = 0; i < nPushed; i++) {
                into.push_back(into[last - i]->getBidiEdge());
            }
        }
    }

    /** @brief Returns the length of the edge
     * @return The original edge's length
     */
    double getLength() const {
        return myOriginal == nullptr ? 0 : myOriginal->getLength();
    }

    //const RailEdge* getBidiEdge() const {
    //    return myOriginal->getBidiEdge()->getRailwayRoutingEdge();
    //}

    bool isInternal() const {
        return myOriginal->isInternal();
    }

    inline bool prohibits(const V* const vehicle) const {
#ifdef RailEdge_DEBUG_TURNS
        if (myOriginal == nullptr && RailEdge_DEBUG_COND(vehicle)) {
            std::cout << getID() << " maxLength=" << myMaxLength << " veh=" << vehicle->getID() << " length=" << vehicle->getLength() << "\n";
        }
#endif
        return vehicle->getLength() > myMaxLength || (myOriginal != nullptr && myOriginal->prohibits(vehicle));
    }

    inline bool restricts(const V* const vehicle) const {
        return myOriginal != nullptr && myOriginal->restricts(vehicle);
    }

    const ConstEdgePairVector& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const {
        if (vClass == SVC_IGNORING || myOriginal == nullptr || myOriginal->isTazConnector()) { // || !MSNet::getInstance()->hasPermissions()) {
            return myViaSuccessors;
        }
#ifdef HAVE_FOX
        FXMutexLock lock(mySuccessorMutex);
#endif
        auto i = myClassesViaSuccessorMap.find(vClass);
        if (i != myClassesViaSuccessorMap.end()) {
            // can use cached value
            return i->second;
        }
        // instantiate vector
        ConstEdgePairVector& result = myClassesViaSuccessorMap[vClass];
        // this vClass is requested for the first time. rebuild all successors
        for (const auto& viaPair : myViaSuccessors) {
            if (viaPair.first->myOriginal == nullptr
                    || viaPair.first->myOriginal->isTazConnector()
                    || myOriginal->isConnectedTo(*viaPair.first->myOriginal, vClass)) {
                result.push_back(viaPair);
            }
        }
        return result;
    }

    bool isVirtual() const {
        return myIsVirtual;
    }

private:
    const int myNumericalID;
    const std::string myID;
    const E* myOriginal;
    _RailEdge* myTurnaround;
    bool myIsVirtual;

    /// @brief actual edges to return when passing this (turnaround) edge - only forward
    std::vector<const E*> myReplacementEdges;

    /// @brief maximum train length for passing this (turnaround) edge
    double myMaxLength = std::numeric_limits<double>::max();
    /// @brief length of the edge where this turn starts
    double myStartLength = 0;

    /// @brief The successors available for a given vClass
    mutable std::map<SUMOVehicleClass, ConstEdgePairVector> myClassesViaSuccessorMap;

    mutable ConstEdgePairVector myViaSuccessors;

#ifdef HAVE_FOX
    /// @brief Mutex for accessing successor edges
    mutable FXMutex mySuccessorMutex;
#endif

};
