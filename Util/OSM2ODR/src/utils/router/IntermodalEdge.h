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
/// @file    IntermodalEdge.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Robert Hilbrich
/// @date    Mon, 03 March 2014
///
// The Edge definition for the Intermodal Router
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <utils/common/ValueTimeLine.h>
#include <utils/common/RandHelper.h>
#include "IntermodalTrip.h"


// ===========================================================================
// class definitions
// ===========================================================================
/// @brief the base edge type that is given to the internal router (SUMOAbstractRouter)
template<class E, class L, class N, class V>
class IntermodalEdge : public Named {
public:
    IntermodalEdge(const std::string id, int numericalID, const E* edge, const std::string& line, const double length = 0.) :
        Named(id),
        myNumericalID(numericalID),
        myEdge(edge),
        myLine(line),
        myLength(edge == nullptr || length > 0. ? length : edge->getLength()),
        myEfforts(nullptr) { }

    virtual ~IntermodalEdge() {}

    virtual bool includeInRoute(bool /* allEdges */) const {
        return false;
    }

    inline const std::string& getLine() const {
        return myLine;
    }

    inline const E* getEdge() const {
        return myEdge;
    }

    int getNumericalID() const {
        return myNumericalID;
    }

    void addSuccessor(IntermodalEdge* const s, IntermodalEdge* const via = nullptr) {
        myFollowingEdges.push_back(s);
        myFollowingViaEdges.push_back(std::make_pair(s, via));
    }

    void transferSuccessors(IntermodalEdge* to) {
        to->myFollowingEdges = myFollowingEdges;
        to->myFollowingViaEdges = myFollowingViaEdges;
        myFollowingEdges.clear();
        myFollowingViaEdges.clear();
    }

    void removeSuccessor(const IntermodalEdge* const edge) {
        myFollowingEdges.erase(std::find(myFollowingEdges.begin(), myFollowingEdges.end(), edge));
        for (auto it = myFollowingViaEdges.begin(); it != myFollowingViaEdges.end();) {
            if (it->first == edge) {
                it = myFollowingViaEdges.erase(it);
            } else {
                ++it;
            }
        }
    }

    virtual const std::vector<IntermodalEdge*>& getSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const {
        UNUSED_PARAMETER(vClass);
        // the network is already tailored. No need to check for permissions here
        return myFollowingEdges;
    }

    virtual const std::vector<std::pair<const IntermodalEdge*, const IntermodalEdge*> >& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const {
        UNUSED_PARAMETER(vClass);
        // the network is already tailored. No need to check for permissions here
        return myFollowingViaEdges;
    }

    virtual bool prohibits(const IntermodalTrip<E, N, V>* const /* trip */) const {
        return false;
    }

    virtual bool restricts(const IntermodalTrip<E, N, V>* const /* trip */) const {
        return false;
    }

    virtual double getTravelTime(const IntermodalTrip<E, N, V>* const /* trip */, double /* time */) const {
        return 0.;
    }

    virtual double getTravelTimeAggregated(const IntermodalTrip<E, N, V>* const trip, double time) const {
        return getTravelTime(trip, time);
    }

    /// @brief get intended vehicle id and departure time of next public transport ride
    virtual double getIntended(const double /* time */, std::string& /* intended */) const {
        return 0.;
    }

    static inline double getTravelTimeStatic(const IntermodalEdge* const edge, const IntermodalTrip<E, N, V>* const trip, double time) {
        return edge == nullptr ? 0. : edge->getTravelTime(trip, time);
    }

    static inline double getTravelTimeStaticRandomized(const IntermodalEdge* const edge, const IntermodalTrip<E, N, V>* const trip, double time) {
        return edge == nullptr ? 0. : edge->getTravelTime(trip, time) * RandHelper::rand(1., gWeightsRandomFactor);
    }

    static inline double getTravelTimeAggregated(const IntermodalEdge* const edge, const IntermodalTrip<E, N, V>* const trip, double time) {
        return edge == nullptr ? 0. : edge->getTravelTimeAggregated(trip, time);
    }


    virtual double getEffort(const IntermodalTrip<E, N, V>* const /* trip */, double /* time */) const {
        return 0.;
    }

    static inline double getEffortStatic(const IntermodalEdge* const edge, const IntermodalTrip<E, N, V>* const trip, double time) {
        return edge == nullptr || !edge->hasEffort() ? 0. : edge->getEffort(trip, time);
    }

    inline double getLength() const {
        return myLength;
    }

    inline void setLength(const double length) {
        myLength = length;
    }

    inline bool isInternal() const {
        return myEdge != nullptr && myEdge->isInternal();
    }

    virtual bool hasEffort() const {
        return myEfforts != nullptr;
    }

    virtual double getStartPos() const {
        return 0.;
    }

    virtual double getEndPos() const {
        return myLength;
    }

    // only used by AStar
    inline double getSpeedLimit() const {
        return myEdge != nullptr ? myEdge->getSpeedLimit() : 200. / 3.6;
    }

    // only used by AStar
    inline double getLengthGeometryFactor() const {
        return myEdge != nullptr ? myEdge->getLengthGeometryFactor() : 1;
    }

    // only used by AStar
    inline double getDistanceTo(const IntermodalEdge* other) const {
        return myEdge != nullptr && other->myEdge != nullptr && myEdge != other->myEdge ? myEdge->getDistanceTo(other->myEdge, true) : 0.;
    }

    // only used by AStar
    inline double getMinimumTravelTime(const IntermodalTrip<E, N, V>* const trip) const {
        return myLength / trip->getMaxSpeed();
    }

    /// @brief only used by mono-modal routing
    IntermodalEdge* getBidiEdge() const {
        return nullptr;
    }

protected:
    /// @brief List of edges that may be approached from this edge
    std::vector<IntermodalEdge*> myFollowingEdges;

    /// @brief List of edges that may be approached from this edge with optional internal vias
    std::vector<std::pair<const IntermodalEdge*, const IntermodalEdge*> > myFollowingViaEdges;

private:
    /// @brief the index in myEdges
    const int myNumericalID;

    /// @brief  the original edge
    const E* const myEdge;

    /// @brief public transport line or ped vs car
    const std::string myLine;

    /// @brief adaptable length (for splitted edges)
    double myLength;

    /// @brief Container for passing effort varying over time for the edge
    ValueTimeLine<double>* myEfforts;

private:
    /// @brief Invalidated copy constructor
    IntermodalEdge(const IntermodalEdge& src);

    /// @brief Invalidated assignment operator
    IntermodalEdge& operator=(const IntermodalEdge& src);

};
