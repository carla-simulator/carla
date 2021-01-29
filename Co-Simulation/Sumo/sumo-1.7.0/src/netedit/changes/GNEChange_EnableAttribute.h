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
/// @file    GNEChange_EnableAttribute.h
/// @author  Pablo Alvarez Lopez
/// @date    Aug 2019
///
// A network change in which the attribute of some object is modified
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_EnableAttribute
 * @brief the function-object for an editing operation (abstract base)
 */
class GNEChange_EnableAttribute : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_EnableAttribute)

public:
    /**@brief Constructor
     * @param[in] ac The attribute-carrier to be modified
     * @param[in] originalAttributes original set of attributes
     * @param[in] newAttributes new set of attributes
     */
    GNEChange_EnableAttribute(GNEAttributeCarrier* ac, const int originalAttributes, const int newAttributes);

    /// @brief Destructor
    ~GNEChange_EnableAttribute();

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
    /**@brief the net to which all operations shall be applied
     * @note we are not responsible for the pointer
     */
    GNEAttributeCarrier* myAC;

    /// @brief original attributes
    const int myOriginalAttributes;

    /// @brief original attributes
    const int myNewAttributes;
};
