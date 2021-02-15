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
/// @file    GNEChange_Shape.h
/// @author  Pablo Alvarez Lopez
/// @date    Oct 2017
///
// A network change in which a single Shape is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
* @class GNEChange_Shape
* A network change in which a single poly is created or deleted
*/
class GNEChange_Shape : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_Shape)

public:
    /**@brief Constructor
    * @param[in] shape the shape to be changed
    * @param[in] forward Whether to create/delete (true/false)
    */
    GNEChange_Shape(GNEShape* shape, bool forward);

    /// @brief Destructor
    ~GNEChange_Shape();

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
    /// @brief pointer to shape
    GNEShape* myShape;
};
