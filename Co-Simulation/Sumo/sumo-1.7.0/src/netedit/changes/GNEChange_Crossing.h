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
/// @file    GNEChange_Crossing.h
/// @author  Pablo Alvarez Lopez
/// @date    Oct 2016
///
// A network change in which a single crossing is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

class GNEJunction;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_Crossing
 * A network change in which a single crossing is created or deleted
 */
class GNEChange_Crossing : public GNEChange {
    // @brief FOX Declaration
    FXDECLARE_ABSTRACT(GNEChange_Crossing)

public:
    /**@brief Constructor for creating/deleting an crossing
     * @param[in] JunctionParent GNEJunction in which the crossing will be created/deleted
     * @param[in] edges vector of edges of crossing
     * @param[in] width value with the width of crossing
     * @param[in] priority boolean with the priority of crossing
     * @param[in] check if in the moment of change connection was selected
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_Crossing(GNEJunction* junctionParent, const std::vector<NBEdge*>& edges, double width,
                       bool priority, int customTLIndex, int customTLIndex2, const PositionVector& customShape, bool selected, bool forward);

    /**@brief Constructor for creating/deleting an crossing based on an existing object
     */
    GNEChange_Crossing(GNEJunction* junctionParent, const NBNode::Crossing& crossing, bool forward);

    /// @brief Destructor
    ~GNEChange_Crossing();

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
    /// @brief full information regarding the Junction in which GNECRossing is created
    GNEJunction* myJunctionParent;

    /// @brief vector to save all edges of GNECrossing
    std::vector<NBEdge*> myEdges;

    /// @brief width of GNECrossing
    double myWidth;

    /// @brief priority of GNECrossing
    bool myPriority;

    /// @brief custom index of GNECrossing
    int myCustomTLIndex;

    /// @brief custom index of GNECrossing (reverse direction)
    int myCustomTLIndex2;

    /// @brief priority of GNECrossing
    PositionVector myCustomShape;
};
