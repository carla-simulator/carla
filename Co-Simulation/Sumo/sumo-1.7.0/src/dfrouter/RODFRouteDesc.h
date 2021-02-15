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
/// @file    RODFRouteDesc.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A route within the DFROUTER
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class RODFDetector;

typedef std::vector<ROEdge*> ROEdgeVector;

// ===========================================================================
// struct definitions
// ===========================================================================
/**
 * @struct RODFRouteDesc
 * @brief A route within the DFROUTER
 */
struct RODFRouteDesc {
    /// @brief The edges the route is made of
    ROEdgeVector edges2Pass;
    /// @brief The name of the route
    std::string routename;
    double duration_2;
    double distance;
    int passedNo;
    const ROEdge* endDetectorEdge;
    const ROEdge* lastDetectorEdge;
    double distance2Last;
    SUMOTime duration2Last;

    double overallProb;
    double factor;

};
