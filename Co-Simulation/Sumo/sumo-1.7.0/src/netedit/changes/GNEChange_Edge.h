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
/// @file    GNEChange_Edge.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// A network change in which a single edge is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_Edge
 * A network change in which a single edge is created or deleted
 */
class GNEChange_Edge : public GNEChange {
    // @brief FOX Declaration
    FXDECLARE_ABSTRACT(GNEChange_Edge)

public:
    /**@brief Constructor for creating/deleting an edge
     * @param[in] edge The edge to be created/deleted
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_Edge(GNEEdge* edge, bool forward);

    /// @brief Destructor
    ~GNEChange_Edge();

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

protected:
    /// @brief add given lane into parents and children
    void addEdgeLanes();

    /// @brief remove given lane from parents and children
    void removeEdgeLanes();

    /// @brief vector of references to vector of parent shapes (used by edge lanes)
    std::vector<std::vector<GNEShape*> > myLaneParentShapes;

    /// @brief vector of references to vector of parent additionals (used by edge lanes)
    std::vector<std::vector<GNEAdditional*> > myLaneParentAdditionals;

    /// @brief vector of references to vector of parent demand elements (used by edge lanes)
    std::vector<std::vector<GNEDemandElement*> > myLaneParentDemandElements;

    /// @brief vector of references to vector of parent generic datas (used by edge lanes)
    std::vector<std::vector<GNEGenericData*> > myLaneParentGenericData;

    /// @brief vector of references to vector of child shapes (used by edge lanes)
    std::vector<std::vector<GNEShape*> > myChildLaneShapes;

    /// @brief vector of references to vector of child additional (used by edge lanes)
    std::vector<std::vector<GNEAdditional*> > myChildLaneAdditionals;

    /// @brief vector of references to vector of child demand elements (used by edge lanes)
    std::vector<std::vector<GNEDemandElement*> > myChildLaneDemandElements;

    /// @brief vector of references to vector of child generic datas (used by edge lanes)
    std::vector<std::vector<GNEGenericData*> > myChildLaneGenericData;

private:
    /**@brief full information regarding the edge that is to be created/deleted
     * @note we assume shared responsibility for the pointer (via reference counting)
     */
    GNEEdge* myEdge;
};
