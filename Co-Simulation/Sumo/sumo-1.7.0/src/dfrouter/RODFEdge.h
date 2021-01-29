/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2006-2020 German Aerospace Center (DLR) and others.
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
/// @file    RODFEdge.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Yun-Pang Floetteroed
/// @date    Thu, 16.03.2006
///
// An edge within the DFROUTER
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <router/ROEdge.h>
#include <utils/geom/Position.h>
#include "RODFDetectorFlow.h"


// ===========================================================================
// class declarations
// ===========================================================================
class ROLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFEdge
 */
class RODFEdge : public ROEdge {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge begins at
     * @param[in] to The node the edge ends at
     * @param[in] index The numeric id of the edge
     */
    RODFEdge(const std::string& id, RONode* from, RONode* to, int index, const int priority);


    /// @brief Destructor
    ~RODFEdge();

    void setFlows(const std::vector<FlowDef>& flows);

    const std::vector<FlowDef>& getFlows() const;


private:
    std::vector<FlowDef> myFlows;

private:
    /// @brief Invalidated copy constructor
    RODFEdge(const RODFEdge& src);

    /// @brief Invalidated assignment operator
    RODFEdge& operator=(const RODFEdge& src);

};
