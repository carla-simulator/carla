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
/// @file    NBCont.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @date    Mon, 17 Dec 2001
///
// Some typedef definitions fir containers
/****************************************************************************/
#pragma once
#include <vector>
#include <set>
#include <utils/common/Named.h>

#include <config.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class NBNode;
class NBRouterEdge;


// ===========================================================================
// container definitions
// ===========================================================================
/// @brief container for (sorted) edges
typedef std::vector<NBEdge*> EdgeVector;
typedef std::vector<NBRouterEdge*> RouterEdgeVector;
typedef std::vector<const NBRouterEdge*> ConstRouterEdgeVector;

typedef std::vector<std::pair<const NBRouterEdge*, const NBRouterEdge*> > ConstRouterEdgePairVector;
typedef std::vector<std::pair<const NBRouterEdge*, const NBRouterEdge*> > NBViaSuccessor;

/// @brief container for unique edges
typedef std::set<NBEdge*> EdgeSet;

typedef std::set<NBNode*, ComparatorIdLess> NodeSet;

/// @brief container for (sorted) lanes. The lanes are sorted from rightmost (id=0) to leftmost (id=nolanes-1)
typedef std::vector<int> LaneVector;
