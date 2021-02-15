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
/// @file    GNECalibratorFlowDialog.h
/// @author  Pablo Alvarez Lopez
/// @date    March 2017
///
// Dialog for edit calibrator flows
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEAdditionalDialog.h"

// ===========================================================================
// class declarations
// ===========================================================================

class GNECalibratorFlow;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNECalibratorFlowDialog
 * @brief Dialog for edit rerouter intervals
 */

class GNECalibratorFlowDialog : public GNEAdditionalDialog {
    /// @brief FOX-declaration
    FXDECLARE(GNECalibratorFlowDialog)

public:
    /// @brief constructor
    GNECalibratorFlowDialog(GNEAdditional* editedCalibratorFlow, bool updatingElement);

    /// @brief destructor
    ~GNECalibratorFlowDialog();

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
    FOX_CONSTRUCTOR(GNECalibratorFlowDialog)

    /// @brief flag to check if current calibrator flow is valid
    bool myCalibratorFlowValid;

    /// @brief current sumo attribute invalid
    SumoXMLAttr myInvalidAttr;

    /// @brief ComboBox for Type
    FXComboBox* myComboBoxVehicleType;

    /// @brief TextField for Route
    FXComboBox* myComboBoxRoute;

    /// @brief TextField for VehsPerHour
    FXTextField* myTextFieldVehsPerHour;

    /// @brief TextField for Speed
    FXTextField* myTextFieldSpeed;

    /// @brief TextField for Color
    FXTextField* myTextFieldColor;

    /// @brief TextField for Depart Lane
    FXTextField* myTextFieldDepartLane;

    /// @brief TextField for Depart Pos
    FXTextField* myTextFieldDepartPos;

    /// @brief TextField for Depart Speed
    FXTextField* myTextFieldDepartSpeed;

    /// @brief TextField for Arrival Lane
    FXTextField* myTextFieldArrivalLane;

    /// @brief TextField for Arrival Pos
    FXTextField* myTextFieldArrivalPos;

    /// @brief TextField for Arrival Speed
    FXTextField* myTextFieldArrivalSpeed;

    /// @brief TextField for Line
    FXTextField* myTextFieldLine;

    /// @brief TextField for PersonNumber
    FXTextField* myTextFieldPersonNumber;

    /// @brief TextField for Container Number
    FXTextField* myTextFieldContainerNumber;

    /// @brief checkButton for Reroute
    FXCheckButton* myRerouteCheckButton;

    /// @brief TextField for DepartPosLat
    FXTextField* myTextFieldDepartPosLat;

    /// @brief TextField for ArrivalPosLat
    FXTextField* myTextFieldArrivalPosLat;

    /// @brief TextField for Begin
    FXTextField* myTextFieldBegin;

    /// @brief TextField for End
    FXTextField* myTextFieldEnd;

    /// @brief update data fields
    void updateCalibratorFlowValues();

private:
    /// @brief Invalidated copy constructor.
    GNECalibratorFlowDialog(const GNECalibratorFlowDialog&) = delete;

    /// @brief Invalidated assignment operator.
    GNECalibratorFlowDialog& operator=(const GNECalibratorFlowDialog&) = delete;
};

