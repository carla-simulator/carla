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
/// @file    GNERouteDialog.h
/// @author  Pablo Alvarez Lopez
/// @date    March 2017
///
// Dialog for edit calibrator routes
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEDemandElementDialog.h"

// ===========================================================================
// class declarations
// ===========================================================================

class GNERoute;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNERouteDialog
 * @brief Dialog for edit Calibrator Routes
 */

class GNERouteDialog : public GNEDemandElementDialog {
    /// @brief FOX-declaration
    FXDECLARE(GNERouteDialog)

public:
    /// @brief constructor
    GNERouteDialog(GNEDemandElement* editedCalibratorRoute, bool updatingElement);

    /// @brief destructor
    ~GNERouteDialog();

    /// @name FOX-callbacks
    /// @{
    /// @brief event after press accept button
    long onCmdAccept(FXObject*, FXSelector, void*);

    /// @brief event after press cancel button
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// @brief event after press reset button
    long onCmdReset(FXObject*, FXSelector, void*);

    /// @brief event after change value
    long onCmdSetVariable(FXObject*, FXSelector, void*);
    /// @}

protected:
    FOX_CONSTRUCTOR(GNERouteDialog)

    /// @brief flag to check if current calibrator vehicleType is valid
    bool myCalibratorRouteValid;

    /// @brief current sumo attribute invalid
    SumoXMLAttr myInvalidAttr;

    /// @brief route ID
    FXTextField* myTextFieldRouteID;

    /// @brief list of edges (string)
    FXTextField* myTextFieldEdges;

    /// @brief color of route
    FXTextField* myTextFieldColor;

    /// @brief update data fields
    void updateCalibratorRouteValues();

private:
    /// @brief Invalidated copy constructor.
    GNERouteDialog(const GNERouteDialog&) = delete;

    /// @brief Invalidated assignment operator.
    GNERouteDialog& operator=(const GNERouteDialog&) = delete;
};

