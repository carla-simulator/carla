/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSLinkCont.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// A vector of links
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include "MSLink.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLinkCont
 * A simple contanier of definitions about how a lane may be left
 */
typedef std::vector<MSLink*> MSLinkCont;


/**
 * @class MSLinkContHelper
 * Some helping functions for dealing with links.
 */
class MSLinkContHelper {
public:
    /** @brief Returns the internal lane that must be passed in order to get to the desired edge
        Returns 0 if no such edge exists */
    static const MSEdge* getInternalFollowingEdge(const MSLane* fromLane,
            const MSEdge* followerAfterInternal);

    static const MSLane* getInternalFollowingLane(const MSLane* fromLane,
            const MSLane* followerAfterInternal);

    /** @brief Returns the link connecting both lanes
        Both lanes have to be non-internal; 0 may be returned if no connection
        exists */
    static MSLink* getConnectingLink(const MSLane& from, const MSLane& to);
};
