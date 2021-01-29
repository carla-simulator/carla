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
/// @file    GNERerouterDialog.h
/// @author  Pablo Alvarez Lopez
/// @date    April 2016
///
// Dialog for edit rerouters
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEAdditionalDialog.h"


// ===========================================================================
// class declarations
// ===========================================================================

class GNERerouter;
class GNERerouterInterval;
class GNERerouterIntervalDialog;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNERerouterDialog
 * @brief Dialog for edit rerouters
 */
class GNERerouterDialog : public GNEAdditionalDialog {
    /// @brief FOX-declaration
    FXDECLARE(GNERerouterDialog)

public:
    /// @brief Constructor
    GNERerouterDialog(GNERerouter* rerouterParent);

    /// @brief destructor
    ~GNERerouterDialog();

    /// @name FOX-callbacks
    /// @{
    /// @brief event after press accept button
    long onCmdAccept(FXObject*, FXSelector, void*);

    /// @brief event after press cancel button
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// @brief event after press reset button
    long onCmdReset(FXObject*, FXSelector, void*);

    /// @brief add new interval
    long onCmdAddInterval(FXObject*, FXSelector, void*);

    /// @brief remove or edit interval
    long onCmdClickedInterval(FXObject*, FXSelector, void*);
    /// @}

protected:
    FOX_CONSTRUCTOR(GNERerouterDialog)

    /// @brief button for add new interval
    FXButton* myAddInterval;

    /// @brief button for sort interval
    FXButton* mySortIntervals;

    /// @brief list with intervals
    FXTable* myIntervalTable;

private:
    /// @brief update data table
    void updateIntervalTable();

    /// @brief Invalidated copy constructor.
    GNERerouterDialog(const GNERerouterDialog&) = delete;

    /// @brief Invalidated assignment operator.
    GNERerouterDialog& operator=(const GNERerouterDialog&) = delete;
};

