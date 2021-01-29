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
/// @file    GNERerouterIntervalDialog.h
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2017
///
// Dialog for edit rerouter intervals
/****************************************************************************/
#pragma once
#include <config.h>
#include <netedit/elements/additional/GNERerouterInterval.h>
#include <netedit/elements/additional/GNEClosingLaneReroute.h>
#include <netedit/elements/additional/GNEParkingAreaReroute.h>
#include <netedit/elements/additional/GNEClosingReroute.h>
#include <netedit/elements/additional/GNEDestProbReroute.h>
#include <netedit/elements/additional/GNERouteProbReroute.h>

#include "GNEAdditionalDialog.h"


// ===========================================================================
// class declarations
// ===========================================================================

class GNERerouterInterval;


// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNERerouterIntervalDialog
 * @brief Dialog for edit rerouter intervals
 */

class GNERerouterIntervalDialog : public GNEAdditionalDialog {
    /// @brief FOX-declaration
    FXDECLARE(GNERerouterIntervalDialog)

public:
    /// @brief constructor
    GNERerouterIntervalDialog(GNEAdditional* rerouterInterval, bool updatingElement);

    /// @brief destructor
    ~GNERerouterIntervalDialog();

    /// @name FOX-callbacks
    /// @{
    /// @brief event after press accept button
    long onCmdAccept(FXObject*, FXSelector, void*);

    /// @brief event after press cancel button
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// @brief event after press reset button
    long onCmdReset(FXObject*, FXSelector, void*);

    /// @brief add closing lane reroute
    long onCmdAddClosingLaneReroute(FXObject*, FXSelector, void*);

    /// @brief add closing reroute
    long onCmdAddClosingReroute(FXObject*, FXSelector, void*);

    /// @brief add dest prob reroute
    long onCmdAddDestProbReroute(FXObject*, FXSelector, void*);

    /// @brief add route prob reroute
    long onCmdAddRouteProbReroute(FXObject*, FXSelector, void*);

    /// @brief add parkingAreaReroute
    long onCmdAddParkingAreaReroute(FXObject*, FXSelector, void*);

    /// @brief remove closing lane reroute
    long onCmdClickedClosingLaneReroute(FXObject*, FXSelector, void*);

    /// @brief remove closing reroute
    long onCmdClickedClosingReroute(FXObject*, FXSelector, void*);

    /// @brief remove dest prob reroute
    long onCmdClickedDestProbReroute(FXObject*, FXSelector, void*);

    /// @brief remove route prob reroute
    long onCmdClickedRouteProbReroute(FXObject*, FXSelector, void*);

    /// @brief remove parkingAreaReroute
    long onCmdClickedParkingAreaReroute(FXObject*, FXSelector, void*);

    /// @brief /edit closing lane reroute
    long onCmdEditClosingLaneReroute(FXObject*, FXSelector, void*);

    /// @brief /edit closing reroute
    long onCmdEditClosingReroute(FXObject*, FXSelector, void*);

    /// @brief /edit dest prob reroute
    long onCmdEditDestProbReroute(FXObject*, FXSelector, void*);

    /// @brief /edit route prob reroute
    long onCmdEditRouteProbReroute(FXObject*, FXSelector, void*);

    /// @brief /edit parkingAreaReroute
    long onCmdEditParkingAreaReroute(FXObject*, FXSelector, void*);

    /// @brief called when begin or endis changed
    long onCmdChangeBeginEnd(FXObject*, FXSelector, void*);

    /// @}

protected:
    FOX_CONSTRUCTOR(GNERerouterIntervalDialog)

    /// @brief begin time text field
    FXTextField* myBeginTextField;

    /// @brief end time text field
    FXTextField* myEndTextField;

    /// @brief begin/end time check label
    FXLabel* myCheckLabel;

    /// @brief button for add new closing lane reroutes
    FXButton* myAddClosingLaneReroutes;

    /// @brief button for add new closing reroutes
    FXButton* myAddClosingReroutes;

    /// @brief button for add new destiny probability reroutes
    FXButton* myAddDestProbReroutes;

    /// @brief button for add new route probability reroutes
    FXButton* myAddRouteProbReroute;

    /// @brief button for add new parkingAreaReroute
    FXButton* myAddParkingAreaReroute;

    /// @brief list with closing lane reroutes
    FXTable* myClosingLaneRerouteTable;

    /// @brief list with closing reroutes
    FXTable* myClosingRerouteTable;

    /// @brief list with destiny probability reroutes
    FXTable* myDestProbRerouteTable;

    /// @brief list with route probability reroute
    FXTable* myRouteProbRerouteTable;

    /// @brief list with parkingAreaReroutes
    FXTable* myParkingAreaRerouteTable;

    /// @brief closing Reroutes edited
    std::vector<GNEAdditional*> myClosingReroutesEdited;

    /// @brief closing Lane Reroutes edited
    std::vector<GNEAdditional*> myClosingLaneReroutesEdited;

    /// @brief Dest Prob Reroutes edited
    std::vector<GNEAdditional*> myDestProbReroutesEdited;

    /// @brief Route Prob Reroutes edited
    std::vector<GNEAdditional*> myRouteProbReroutesEdited;

    /// @brief Parking Area reroute edited
    std::vector<GNEAdditional*> myParkingAreaRerouteEdited;

    /// @brief flag to check if begin an end are valid
    bool myBeginEndValid;

    /// @brief flag to check if closing lane reroutes are valid
    bool myClosingLaneReroutesValid;

    /// @brief flag to check if closing reroutes are valid
    bool myClosingReroutesValid;

    /// @brief flag to check if Destiny probability reroutes are valid
    bool myDestProbReroutesValid;

    /// @brief flag to check if parkingAreaReroutes are valid
    bool myParkingAreaReroutesValid;

    /// @brief flag to check if route prob reroutes are valid
    bool myRouteProbReroutesValid;

private:
    /// @brief update data of closing lane reroute table
    void updateClosingLaneReroutesTable();

    /// @brief update data of closing reroute table
    void updateClosingReroutesTable();

    /// @brief update data of destiny probabilitry reroute table
    void updateDestProbReroutesTable();

    /// @brief update data of probabilitry reroutes table
    void updateRouteProbReroutesTable();

    /// @brief update data of parkingAreaReroute table
    void updateParkingAreaReroutesTable();

    /// @brief Invalidated copy constructor.
    GNERerouterIntervalDialog(const GNERerouterIntervalDialog&) = delete;

    /// @brief Invalidated assignment operator.
    GNERerouterIntervalDialog& operator=(const GNERerouterIntervalDialog&) = delete;
};

