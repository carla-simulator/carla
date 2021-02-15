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
/// @file    GNEFixDemandElements.h
/// @author  Pablo Alvarez Lopez
/// @date    March 2019
///
// Dialog used to fix demand elements during saving
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>

// ===========================================================================
// class declarations
// ===========================================================================
class GNEDemandElement;
class GNEViewNet;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEFixDemandElements
 * @brief Dialog for edit rerouters
 */
class GNEFixDemandElements : public FXDialogBox {
    /// @brief FOX-declaration
    FXDECLARE(GNEFixDemandElements)

public:
    /// @brief Constructor
    GNEFixDemandElements(GNEViewNet* viewNet, const std::vector<GNEDemandElement*>& invalidDemandElements);

    /// @brief destructor
    ~GNEFixDemandElements();

    /// @name FOX-callbacks
    /// @{
    /// @brief event when user select a option
    long onCmdSelectOption(FXObject* obj, FXSelector, void*);

    /// @brief event after press accept button
    long onCmdAccept(FXObject*, FXSelector, void*);

    /// @brief event after press cancel button
    long onCmdCancel(FXObject*, FXSelector, void*);
    /// @}

protected:
    /// @brief groupbox for list
    class DemandList : protected FXGroupBox {

    public:
        /// @brief constructor
        DemandList(GNEFixDemandElements* fixDemandElementsDialogParents, const std::vector<GNEDemandElement*>& invalidDemandElements);

        /// @brief vector with the invalid routes
        std::vector<GNEDemandElement*> myInvalidRoutes;

        /// @brief vector with the invalid vehicles
        std::vector<GNEDemandElement*> myInvalidVehicles;

        /// @brief vector with the invalid stops
        std::vector<GNEDemandElement*> myInvalidStops;

        /// @brief vector with the invalid person plans
        std::vector<GNEDemandElement*> myInvalidPersonPlans;

        /// @brief list with the demand elements
        FXTable* myTable;
    };

    /// @brief groupbox for all radio buttons related with fix route options
    class FixRouteOptions : protected FXGroupBox {

    public:
        /// @brief constructor
        FixRouteOptions(GNEFixDemandElements* fixDemandElementsDialogParents);

        /// @brief select option
        void selectOption(FXObject* option);

        /// @brief enable position options
        void enableFixRouteOptions();

        /// @brief disable position options
        void disableFixRouteOptions();

        /// @brief Option "Remove invalid routes"
        FXRadioButton* removeInvalidRoutes;

        /// @brief Option "Save invalid routes"
        FXRadioButton* saveInvalidRoutes;

        /// @brief Option "Select invalid routes and cancel"
        FXRadioButton* selectInvalidRoutesAndCancel;
    };

    /// @brief groupbox for all radio buttons related with fix vehicle options
    class FixVehicleOptions : protected FXGroupBox {

    public:
        /// @brief constructor
        FixVehicleOptions(GNEFixDemandElements* fixDemandElementsDialogParents);

        /// @brief select option
        void selectOption(FXObject* option);

        /// @brief enable consecutive lane options
        void enableFixVehicleOptions();

        /// @brief disable consecutive lane options
        void disableFixVehicleOptions();

        /// @brief Option "remove invalid elements"
        FXRadioButton* removeInvalidVehicles;

        /// @brief Option "save invalid vehicles"
        FXRadioButton* saveInvalidVehicles;

        /// @brief Option "Select invalid vehicles and cancel"
        FXRadioButton* selectInvalidVehiclesAndCancel;
    };

    /// @brief groupbox for all radio buttons related with fix stop options
    class FixStopOptions : public FXGroupBox {

    public:
        /// @brief build Position Options
        FixStopOptions(GNEFixDemandElements* fixDemandElementsDialogParents);

        /// @brief select option
        void selectOption(FXObject* option);

        /// @brief enable position options
        void enableFixStopOptions();

        /// @brief disable position options
        void disableFixStopOptions();

        /// @brief Option "Activate friendlyPos and save"
        FXRadioButton* activateFriendlyPositionAndSave;

        /// @brief Option "Fix Positions and save"
        FXRadioButton* fixPositionsAndSave;

        /// @brief Option "Save invalid"
        FXRadioButton* saveInvalid;

        /// @brief Option "Select invalid stops and cancel"
        FXRadioButton* selectInvalidStopsAndCancel;
    };

    /// @brief groupbox for all radio buttons related with fix person plan options
    class FixPersonPlanOptions : public FXGroupBox {

    public:
        /// @brief build Position Options
        FixPersonPlanOptions(GNEFixDemandElements* fixDemandElementsDialogParents);

        /// @brief select option
        void selectOption(FXObject* option);

        /// @brief enable position options
        void enableFixPersonPlanOptions();

        /// @brief disable position options
        void disableFixPersonPlanOptions();

        /// @brief Option "delete person plan"
        FXRadioButton* deletePersonPlan;

        /// @brief Option "Save invalid"
        FXRadioButton* saveInvalid;

        /// @brief Option "Select invalid person plans and cancel"
        FXRadioButton* selectInvalidPersonPlansAndCancel;
    };

    FOX_CONSTRUCTOR(GNEFixDemandElements)

    /// @brief view net
    GNEViewNet* myViewNet;

    /// @brief main frame
    FXVerticalFrame* myMainFrame;

    /// @brief list with the demand elements
    DemandList* myDemandList;

    /// @brief fix route options
    FixRouteOptions* myFixRouteOptions;

    /// @brief fix vehicle options
    FixVehicleOptions* myFixVehicleOptions;

    /// @brief fix stop options
    FixStopOptions* myFixStopOptions;

    /// @brief fix person plan options
    FixPersonPlanOptions* myFixPersonPlanOptions;

    /// @brief accept button
    FXButton* myAcceptButton;

    /// @brief cancel button
    FXButton* myCancelButton;

private:
    /// @brief Invalidated copy constructor.
    GNEFixDemandElements(const GNEFixDemandElements&) = delete;

    /// @brief Invalidated assignment operator.
    GNEFixDemandElements& operator=(const GNEFixDemandElements&) = delete;
};

