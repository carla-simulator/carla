/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    ROJTREdge.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Yun-Pang Floetteroed
/// @date    Tue, 20 Jan 2004
///
// An edge the jtr-router may route through
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <utils/common/ValueTimeLine.h>
#include <router/ROEdge.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROJTREdge
 * @brief An edge the jtr-router may route through
 *
 * A router edge extended by the definition about the probability a
 *  vehicle chooses a certain following edge over time.
 */
class ROJTREdge : public ROEdge {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge begins at
     * @param[in] to The node the edge ends at
     * @param[in] index The numeric id of the edge
     */
    ROJTREdge(const std::string& id, RONode* from, RONode* to, int index, const int priority);


    /// @brief Destructor
    ~ROJTREdge();


    /** @brief Adds information about a connected edge
     *
     * Makes this edge know the given following edge. Calls ROEdge::addFollower.
     *
     * Additionally it generates the entry for the given following edge
     *  in myFollowingDefs.
     *
     * @param[in] s The following edge
     * @see ROEdge::addFollower
     */
    void addSuccessor(ROEdge* s, ROEdge* via = nullptr, std::string dir = "");


    /** @brief adds the information about the percentage of using a certain follower
     *
     * @param[in] follower The following edge
     * @param[in] begTime Time begin (in seconds) for which this probability is valid
     * @param[in] endTime Time end (in seconds) for which this probability is valid
     * @param[in] probability The probability to use the given follower
     */
    void addFollowerProbability(ROJTREdge* follower,
                                double begTime, double endTime, double probability);


    /** @brief Returns the next edge to use
     * @param[in] veh The vehicle to choose the next edge for
     * @param[in] time The time at which the next edge shall be entered (in seconds)
     * @param[in] avoid The set of edges to avoid
     * @return The chosen edge
     */
    ROJTREdge* chooseNext(const ROVehicle* const veh, double time, const std::set<const ROEdge*>& avoid) const;


    /** @brief Sets the turning definition defaults
     * @param[in] def The turning percentage defaults
     */
    void setTurnDefaults(const std::vector<double>& defs);

    /// @brief register source flow on this edge
    int getSourceFlow() const {
        return mySourceFlows;
    }

    /// @brief register flow on this edge
    void changeSourceFlow(int value) {
        mySourceFlows += value;
    }

private:
    /// @brief Definition of a map that stores the probabilities of using a certain follower over time
    typedef std::map<ROJTREdge*, ValueTimeLine<double>*, ComparatorIdLess> FollowerUsageCont;

    /// @brief Storage for the probabilities of using a certain follower over time
    FollowerUsageCont myFollowingDefs;

    /// @brief The defaults for turnings
    std::vector<double> myParsedTurnings;

    /// @brief the flows departing from this edge in the given time
    //ValueTimeLine<int> mySourceFlows;
    int mySourceFlows;

private:
    /// @brief invalidated copy constructor
    ROJTREdge(const ROJTREdge& src);

    /// @brief invalidated assignment operator
    ROJTREdge& operator=(const ROJTREdge& src);


};
