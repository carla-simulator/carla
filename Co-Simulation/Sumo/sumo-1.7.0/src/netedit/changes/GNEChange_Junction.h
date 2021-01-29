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
/// @file    GNEChange_Junction.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// A network change in which a single junction is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_Junction
 * A network change in which a single junction is created or deleted
 */
class GNEChange_Junction : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_Junction)

public:
    /**@brief Constructor for creating/deleting a junction
     * @param[in] junction The junction to be created/deleted
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_Junction(GNEJunction* junction, bool forward);

    /// @brief Destructor
    ~GNEChange_Junction();

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
    /**@brief full information regarding the junction that is to be created/deleted
     * @note we assume shared responsibility for the pointer (via reference counting)
     */
    GNEJunction* myJunction;
};
