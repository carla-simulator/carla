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
/// @file    ReversedEdge.h
/// @author  Michael Behrisch
/// @date    29.01.2020
///
// The ReversedEdge is a wrapper around a ROEdge or a MSEdge used for
// backward search
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class definitions
// ===========================================================================
/// @brief the edge type representing backward edges
template<class E, class V>
class ReversedEdge {
public:
    typedef std::vector<std::pair<const ReversedEdge<E, V>*, const ReversedEdge<E, V>*> > ConstEdgePairVector;

    ReversedEdge(const E* orig) : myOriginal(orig) {
    }

    void init() {
        if (!myOriginal->isInternal()) {
            for (const auto& viaPair : myOriginal->getViaSuccessors()) {
                const ReversedEdge<E, V>* revSource = viaPair.first->getReversedRoutingEdge();
                const E* via = viaPair.second;
                const ReversedEdge<E, V>* preVia = nullptr;
                while (via != nullptr && via->isInternal()) {
                    via->getReversedRoutingEdge()->myViaSuccessors.push_back(std::make_pair(this, preVia));
                    preVia = via->getReversedRoutingEdge();
                    via = via->getViaSuccessors().front().second;
                }
                revSource->myViaSuccessors.push_back(std::make_pair(this, preVia));
            }
        }
    }

    /** @brief Returns the index (numeric id) of the edge
     * @return The original edge's numerical id
     */
    int getNumericalID() const {
        return myOriginal->getNumericalID();
    }

    /** @brief Returns the id of the edge
     * @return The original edge's id
     */
    const std::string& getID() const {
        return myOriginal->getID();
    }

    /** @brief Returns the length of the edge
     * @return The original edge's length
     */
    double getLength() const {
        return myOriginal->getLength();
    }

    const ReversedEdge* getBidiEdge() const {
        return myOriginal->getBidiEdge()->getReversedRoutingEdge();
    }

    bool isInternal() const {
        return myOriginal->isInternal();
    }

    inline bool prohibits(const V* const vehicle) const {
        return myOriginal->prohibits(vehicle);
    }

    inline bool restricts(const V* const vehicle) const {
        return myOriginal->restricts(vehicle);
    }

    static inline double getTravelTimeStatic(const ReversedEdge<E, V>* const edge, const V* const veh, double time) {
        return edge->myOriginal->getTravelTime(veh, time);
    }

    const ConstEdgePairVector& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const {
        if (vClass == SVC_IGNORING || myOriginal->isTazConnector()) { // || !MSNet::getInstance()->hasPermissions()) {
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
            if (viaPair.first->myOriginal->isTazConnector() || viaPair.first->myOriginal->isConnectedTo(*myOriginal, vClass)) {
                result.push_back(viaPair);
            }
        }
        return result;
    }

private:
    const E* myOriginal;
    /// @brief The successors available for a given vClass
    mutable std::map<SUMOVehicleClass, ConstEdgePairVector> myClassesViaSuccessorMap;

    mutable ConstEdgePairVector myViaSuccessors;

#ifdef HAVE_FOX
    /// @brief Mutex for accessing successor edges
    mutable FXMutex mySuccessorMutex;
#endif

};
