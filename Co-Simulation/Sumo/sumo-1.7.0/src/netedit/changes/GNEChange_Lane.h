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
/// @file    GNEChange_Lane.h
/// @author  Jakob Erdmann
/// @date    April 2011
///
// A network change in which a single lane is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_Lane
 * A network change in which a single lane is created or deleted
 */
class GNEChange_Lane : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_Lane)

public:
    /**@brief Constructor for creating a lane
     * @param[in] edge The edge on which to apply changes
     * @param[in] laneAttrs The attributes of the lane to be created/
     */
    GNEChange_Lane(GNEEdge* edge, const NBEdge::Lane& laneAttrs);

    /**@brief Constructor for deleting a lane
     * @param[in] edge The edge on which to apply changes
     * @param[in] lane The lane to be deleted
     * @param[in] laneAttrs The attributes of the lane to be deleted
     * @param[in] forward Whether to delete (true/false)
     * @param[in] recomputeConnections Whether to recompute all connections for the affected edge
     */
    GNEChange_Lane(GNEEdge* edge, GNELane* lane, const NBEdge::Lane& laneAttrs, bool forward, bool recomputeConnections = true);

    /// @brief Destructor
    ~GNEChange_Lane();

    /// @name inherited from GNEChange
    /// @{
    /// @brief get undo Name
    FXString undoName() const;

    /// @brief get Redo name
    FXString redoName() const;

    /// @brief undo action
    void undo();

    /// @brief redo action
    void redo();
    /// @}

private:
    /// @brief we need the edge because it is the target of our change commands
    GNEEdge* myEdge;

    /// @brief we need to preserve the lane because it maybe the target of GNEChange_Attribute commands
    GNELane* myLane;

    /// @brief we need to preserve the attributes explicitly because they are not contained withing GNELane itself
    const NBEdge::Lane myLaneAttrs;

    /// @bried whether to recompute connection when adding a new lane
    bool myRecomputeConnections;
};
