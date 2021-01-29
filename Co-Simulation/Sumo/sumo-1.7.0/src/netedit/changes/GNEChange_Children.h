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
/// @file    GNEChange_Children.h
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2019
///
// A network change used to modify sorting of hierarchical element children
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_Children
 * A network change in which a additional element is created or deleted
 */
class GNEChange_Children : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_Children)

public:
    // @brief operation over child demand element
    enum Operation {
        MOVE_FRONT = 0, // Move element one position front
        MOVE_BACK = 1   // Move element one position back
    };

    /**@brief Constructor for creating/deleting an additional element
     * @param[in] additional The additional element to be created/deleted
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_Children(GNEDemandElement* demandElementParent, GNEDemandElement* demandElementChild, const Operation operation);

    /// @brief Destructor
    ~GNEChange_Children();

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
    /**@brief full information regarding the parent demand element element that will be modified
     * @note we assume shared responsibility for the pointer (via reference counting)
     */
    GNEDemandElement* myParentDemandElement;

    /// @brief demand element which position will be edited edited
    GNEDemandElement* myChildDemandElement;

    // @brief Operation to be apply
    const Operation myOperation;

    /// @brief copy of child demand elementss before apply operation
    const std::vector<GNEDemandElement*> myOriginalChildElements;

    /// @brief element children after apply operation
    std::vector<GNEDemandElement*> myEditedChildElements;
};
