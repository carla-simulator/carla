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
/// @file    MSEdgeWeightsStorage.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    02.11.2009
///
// A storage for edge travel times and efforts
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/ValueTimeLine.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class SUMOVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSEdgeWeightsStorage
 * @brief A storage for edge travel times and efforts
 */
class MSEdgeWeightsStorage {
public:
    /// @brief Constructor
    MSEdgeWeightsStorage();


    /// @brief Destructor
    ~MSEdgeWeightsStorage();


    /** @brief Returns a travel time for an edge and time if stored
     * @param[in] e The edge for which the travel time shall be retrieved
     * @param[in] t The time for which the travel time shall be retrieved
     * @param[in] value The value if the requested edge/time is described
     * @return Whether the requested edge/time is described
     */
    bool retrieveExistingTravelTime(const MSEdge* const e, const double t, double& value) const;


    /** @brief Returns an effort for an edge and time if stored
     * @param[in] e The edge for which the effort shall be retrieved
     * @param[in] t The time for which the effort shall be retrieved
     * @param[in] value The value if the requested edge/time is described
     * @return Whether the requested edge/time is described
     */
    bool retrieveExistingEffort(const MSEdge* const e, const double t, double& value) const;


    /** @brief Adds a travel time information for an edge and a time span
     * @param[in] e The described edge
     * @param[in] begin The begin of the described time span
     * @param[in] end The end of the described time span
     * @param[in] value The travel time value for this edge and time span
     */
    void addTravelTime(const MSEdge* const e, double begin, double end, double value);


    /** @brief Adds an effort information for an edge and a time span
     * @param[in] e The described edge
     * @param[in] begin The begin of the described time span
     * @param[in] end The end of the described time span
     * @param[in] value Theeffort value for this edge and time span
     */
    void addEffort(const MSEdge* const e, double begin, double end, double value);


    /** @brief Removes the travel time information for an edge
     * @param[in] e The described edge
     */
    void removeTravelTime(const MSEdge* const e);


    /** @brief Removes the effort information for an edge
     * @param[in] e The described edge
     */
    void removeEffort(const MSEdge* const e);


    /** @brief Returns the information whether any travel time is known for the given edge
     * @param[in] e The investigated edge
     * @return Whether any travel time information about this edge is stored
     */
    bool knowsTravelTime(const MSEdge* const e) const;


    /** @brief Returns the information whether any effort is known for the given edge
     * @param[in] e The investigated edge
     * @return Whether any travel time information about this edge is stored
     */
    bool knowsEffort(const MSEdge* const e) const;


private:
    /// @brief A map of edge->time->travel time
    std::map<const MSEdge*, ValueTimeLine<double> > myTravelTimes;

    /// @brief A map of edge->time->effort
    std::map<const MSEdge*, ValueTimeLine<double> > myEfforts;


private:
    /// @brief Invalidated copy constructor.
    MSEdgeWeightsStorage(const MSEdgeWeightsStorage&);

    /// @brief Invalidated assignment operator.
    MSEdgeWeightsStorage& operator=(const MSEdgeWeightsStorage&);


};
