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
/// @file    ROMAEdge.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christian Roessel
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A basic edge for routing applications
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <utils/common/ValueTimeLine.h>
#include <utils/common/SUMOVehicleClass.h>
#include <router/ROEdge.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROLane;
class ROVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROMAEdge
 * @brief A basic edge for routing applications
 *
 * The edge contains two time lines, one for the travel time and one for a second
 *  measure which may be used for computing the costs of a route. After loading
 *  the weights, it is needed to call "buildTimeLines" in order to initialise
 *  these time lines.
 */
class ROMAEdge : public ROEdge {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge begins at
     * @param[in] to The node the edge ends at
     * @param[in] index The numeric id of the edge
     */
    ROMAEdge(const std::string& id, RONode* from, RONode* to, int index, const int priority);


    /// Destructor
    virtual ~ROMAEdge();

    /** @brief Adds information about a connected edge.
     *
     * In addition to ROEdge::addSuccessor it keeps track of left turns.
     *
     * @param[in] s The edge to add
     * @todo What about vehicle-type aware connections?
     */
    virtual void addSuccessor(ROEdge* s, ROEdge* via = nullptr, std::string dir = "");

    void setFlow(const double begin, const double end, const double flow) {
        myFlow.add(begin, end, flow);
    }

    double getFlow(const double time) const {
        return myFlow.getValue(time);
    }

    void setHelpFlow(const double begin, const double end, const double flow) {
        myHelpFlow.add(begin, end, flow);
    }

    double getHelpFlow(const double time) const {
        return myHelpFlow.getValue(time);
    }

private:
    std::set<ROMAEdge*> myLeftTurns;
    ValueTimeLine<double> myFlow;
    ValueTimeLine<double> myHelpFlow;

private:
    /// @brief Invalidated copy constructor
    ROMAEdge(const ROMAEdge& src);

    /// @brief Invalidated assignment operator
    ROMAEdge& operator=(const ROMAEdge& src);

};
