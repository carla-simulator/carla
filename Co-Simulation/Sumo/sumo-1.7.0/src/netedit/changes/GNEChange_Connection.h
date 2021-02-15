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
/// @file    GNEChange_Connection.h
/// @author  Jakob Erdmann
/// @date    May 2011
///
// A network change in which a single connection is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_Connection
 * A network change in which a single lane is created or deleted
 */
class GNEChange_Connection : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_Connection)

public:

    /**@brief Constructor for creating/deleting a connection
     * @param[in] edge The source edge of the connection
     * @param[in] nbCon The data of the connection
     * @param[in] check if in the moment of change connection was selected
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_Connection(GNEEdge* edge, NBEdge::Connection nbCon, bool selected, bool forward);

    /// @brief Destructor
    ~GNEChange_Connection();

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
    // @brief the connection object to be removed/re-added
    GNEEdge* myEdge;

    /// @brief the data which must be copied because the original reference does not persist
    NBEdge::Connection myNBEdgeConnection;
};
