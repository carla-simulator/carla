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
/// @file    GNEVariableSpeedSignDialog.h
/// @author  Pablo Alvarez Lopez
/// @date    April 2016
///
// A class for edit phases of Variable Speed Signals
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <netedit/elements/additional/GNEVariableSpeedSignStep.h>

#include "GNEAdditionalDialog.h"

// ===========================================================================
// class declarations
// ===========================================================================

class GNEVariableSpeedSign;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEAdditionalDialog
 * @brief Dialog to edit sequences, parameters, etc.. of Additionals
 */
class GNEVariableSpeedSignDialog : public GNEAdditionalDialog {
    /// @brief FOX-declaration
    FXDECLARE(GNEVariableSpeedSignDialog)

public:
    /// @brief constructor
    GNEVariableSpeedSignDialog(GNEVariableSpeedSign* editedVariableSpeedSign);

    /// @brief destructor
    ~GNEVariableSpeedSignDialog();

    /// @name FOX-callbacks
    /// @{
    /// @brief event called after press add row button
    long onCmdAddStep(FXObject*, FXSelector, void*);

    /// @brief event called after edit row
    long onCmdEditStep(FXObject*, FXSelector, void*);

    /// @brief event called after clicked a row
    long onCmdClickedStep(FXObject*, FXSelector, void*);

    /// @brief event called after clicked over sort step button
    long onCmdSortSteps(FXObject*, FXSelector, void*);

    /// @brief event called after press accept button
    long onCmdAccept(FXObject*, FXSelector, void*);

    /// @brief event called after press cancel button
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// @brief event called after press cancel button
    long onCmdReset(FXObject*, FXSelector, void*);
    /// @}

protected:
    FOX_CONSTRUCTOR(GNEVariableSpeedSignDialog)

    /// @brief Table with the data
    FXTable* myStepsTable;

    /// @brief Button for insert row
    FXButton* myAddStepButton;

    /// @brief Button for sort row
    FXButton* mySortStepButton;

    // @brief Flag to check if current steps are valid
    bool myStepsValids;

private:
    /// @brief update data table
    void updateTableSteps();

    /// @brief Invalidated copy constructor.
    GNEVariableSpeedSignDialog(const GNEVariableSpeedSignDialog&) = delete;

    /// @brief Invalidated assignment operator.
    GNEVariableSpeedSignDialog& operator=(const GNEVariableSpeedSignDialog&) = delete;
};

