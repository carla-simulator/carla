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
/// @file    GNEFixDemandElements.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2017
///
// Dialog used to fix demand elements during saving
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/elements/demand/GNEDemandElement.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>

#include "GNEFixDemandElements.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEFixDemandElements) GNEFixDemandElementsMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_OPERATION,  GNEFixDemandElements::onCmdSelectOption),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_ACCEPT,  GNEFixDemandElements::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_CANCEL,  GNEFixDemandElements::onCmdCancel),
};

// Object implementation
FXIMPLEMENT(GNEFixDemandElements, FXDialogBox, GNEFixDemandElementsMap, ARRAYNUMBER(GNEFixDemandElementsMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNEFixDemandElements::GNEFixDemandElements(GNEViewNet* viewNet, const std::vector<GNEDemandElement*>& invalidDemandElements) :
    FXDialogBox(viewNet->getApp(), "Fix demand elements problems", GUIDesignDialogBoxExplicit(500, 450)),
    myViewNet(viewNet) {
    // set busStop icon for this dialog
    setIcon(GUIIconSubSys::getIcon(GUIIcon::ROUTE));
    // create main frame
    myMainFrame = new FXVerticalFrame(this, GUIDesignAuxiliarFrame);
    // create demand list
    myDemandList = new DemandList(this, invalidDemandElements);
    // create fix route options
    myFixRouteOptions = new FixRouteOptions(this);
    // create fix vehicle  options
    myFixVehicleOptions = new FixVehicleOptions(this);
    // create fix stops options
    myFixStopOptions = new FixStopOptions(this);
    // create fix person plans options
    myFixPersonPlanOptions = new FixPersonPlanOptions(this);
    // check if fix route options has to be disabled
    if (myDemandList->myInvalidRoutes.empty()) {
        myFixRouteOptions->disableFixRouteOptions();
    }
    // check if fix vehicle options has to be disabled
    if (myDemandList->myInvalidVehicles.empty()) {
        myFixVehicleOptions->disableFixVehicleOptions();
    }
    // check if fix stop options has to be disabled
    if (myDemandList->myInvalidStops.empty()) {
        myFixStopOptions->disableFixStopOptions();
    }
    // check if fix person plan options has to be disabled
    if (myDemandList->myInvalidPersonPlans.empty()) {
        myFixPersonPlanOptions->disableFixPersonPlanOptions();
    }
    // create dialog buttons bot centered
    FXHorizontalFrame* buttonsFrame = new FXHorizontalFrame(myMainFrame, GUIDesignHorizontalFrame);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
    myAcceptButton = new FXButton(buttonsFrame, FXWindow::tr("&Accept"), GUIIconSubSys::getIcon(GUIIcon::ACCEPT), this, MID_GNE_BUTTON_ACCEPT, GUIDesignButtonAccept);
    myCancelButton = new FXButton(buttonsFrame, FXWindow::tr("&Cancel"), GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_GNE_BUTTON_CANCEL, GUIDesignButtonCancel);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
    // set focus in accept button
    myAcceptButton->setFocus();
}


GNEFixDemandElements::~GNEFixDemandElements() {
}


long
GNEFixDemandElements::onCmdSelectOption(FXObject* obj, FXSelector, void*) {
    myFixRouteOptions->selectOption(obj);
    myFixVehicleOptions->selectOption(obj);
    myFixStopOptions->selectOption(obj);
    myFixPersonPlanOptions->selectOption(obj);
    return 1;
}


long
GNEFixDemandElements::onCmdAccept(FXObject*, FXSelector, void*) {
    bool continueSaving = true;
    // check options for invalid routes
    if (myDemandList->myInvalidRoutes.size() > 0) {
        if (myFixRouteOptions->removeInvalidRoutes->getCheck() == TRUE) {
            // begin undo list
            myViewNet->getUndoList()->p_begin("delete invalid routes");
            // iterate over invalid routes to delete it
            for (auto i : myDemandList->myInvalidRoutes) {
                myViewNet->getNet()->deleteDemandElement(i, myViewNet->getUndoList());
            }
            // end undo list
            myViewNet->getUndoList()->p_end();
        } else if (myFixRouteOptions->selectInvalidRoutesAndCancel->getCheck() == TRUE) {
            // begin undo list
            myViewNet->getUndoList()->p_begin("select invalid routes");
            // iterate over invalid single lane elements to select all elements
            for (auto i : myDemandList->myInvalidRoutes) {
                i->setAttribute(GNE_ATTR_SELECTED, "true", myViewNet->getUndoList());
            }
            // end undo list
            myViewNet->getUndoList()->p_end();
            // abort saving
            continueSaving = false;
        }
    }
    // check options for invalid vehicles
    if (myDemandList->myInvalidVehicles.size() > 0) {
        if (myFixVehicleOptions->removeInvalidVehicles->getCheck() == TRUE) {
            // begin undo list
            myViewNet->getUndoList()->p_begin("delete invalid vehicles");
            // iterate over invalid stops to delete it
            for (auto i : myDemandList->myInvalidVehicles) {
                myViewNet->getNet()->deleteDemandElement(i, myViewNet->getUndoList());
            }
            // end undo list
            myViewNet->getUndoList()->p_end();
        } else if (myFixVehicleOptions->selectInvalidVehiclesAndCancel->getCheck() == TRUE) {
            // begin undo list
            myViewNet->getUndoList()->p_begin("select invalid vehicles");
            // iterate over invalid single lane elements to select all elements
            for (auto i : myDemandList->myInvalidVehicles) {
                i->setAttribute(GNE_ATTR_SELECTED, "true", myViewNet->getUndoList());
            }
            // end undo list
            myViewNet->getUndoList()->p_end();
            // abort saving
            continueSaving = false;
        }
    }
    // check options for stops
    if (myDemandList->myInvalidStops.size() > 0) {
        if (myFixStopOptions->activateFriendlyPositionAndSave->getCheck() == TRUE) {
            // begin undo list
            myViewNet->getUndoList()->p_begin("change " + toString(SUMO_ATTR_FRIENDLY_POS) + " of invalid stops");
            // iterate over invalid stops to enable friendly position
            for (auto i : myDemandList->myInvalidStops) {
                i->setAttribute(SUMO_ATTR_FRIENDLY_POS, "true", myViewNet->getUndoList());
            }
            myViewNet->getUndoList()->p_end();
        } else if (myFixStopOptions->fixPositionsAndSave->getCheck() == TRUE) {
            myViewNet->getUndoList()->p_begin("fix positions of invalid stops");
            // iterate over invalid stops to fix positions
            for (auto i : myDemandList->myInvalidStops) {
                i->fixDemandElementProblem();
            }
            myViewNet->getUndoList()->p_end();
        } else if (myFixStopOptions->selectInvalidStopsAndCancel->getCheck() == TRUE) {
            myViewNet->getUndoList()->p_begin("select invalid stops");
            // iterate over invalid stops to select all elements
            for (auto i : myDemandList->myInvalidStops) {
                i->setAttribute(GNE_ATTR_SELECTED, "true", myViewNet->getUndoList());
            }
            // end undo list
            myViewNet->getUndoList()->p_end();
            // abort saving
            continueSaving = false;
        }
    }
    // check options for person plans
    if (myDemandList->myInvalidPersonPlans.size() > 0) {
        if (myFixPersonPlanOptions->deletePersonPlan->getCheck() == TRUE) {
            /*
            // begin undo list
            myViewNet->getUndoList()->p_begin("change " + toString(SUMO_ATTR_FRIENDLY_POS) + " of invalid person plans");
            // iterate over invalid person plans to enable friendly position
            for (auto i : myDemandList->myInvalidPersonPlans) {
                i->setAttribute(SUMO_ATTR_FRIENDLY_POS, "true", myViewNet->getUndoList());
            }
            myViewNet->getUndoList()->p_end();
            */
        } else if (myFixPersonPlanOptions->selectInvalidPersonPlansAndCancel->getCheck() == TRUE) {
            myViewNet->getUndoList()->p_begin("select invalid person plans");
            // iterate over invalid person plans to select all elements
            for (auto i : myDemandList->myInvalidPersonPlans) {
                i->setAttribute(GNE_ATTR_SELECTED, "true", myViewNet->getUndoList());
            }
            // end undo list
            myViewNet->getUndoList()->p_end();
            // abort saving
            continueSaving = false;
        }
    }
    if (continueSaving) {
        // stop modal with TRUE (continue saving)
        getApp()->stopModal(this, TRUE);
    } else {
        // stop modal with TRUE (abort saving)
        getApp()->stopModal(this, FALSE);
    }
    return 1;
}


long
GNEFixDemandElements::onCmdCancel(FXObject*, FXSelector, void*) {
    // Stop Modal (abort saving)
    getApp()->stopModal(this, FALSE);
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFixDemandElements::DemandList - methods
// ---------------------------------------------------------------------------

GNEFixDemandElements::DemandList::DemandList(GNEFixDemandElements* fixDemandElementsDialogParents, const std::vector<GNEDemandElement*>& invalidDemandElements) :
    FXGroupBox(fixDemandElementsDialogParents->myMainFrame, "Routes and Vehicles with conflicts", GUIDesignGroupBoxFrameFill) {
    // Create table, copy intervals and update table
    myTable = new FXTable(this, this, MID_GNE_FIXSTOPPINGPLACES_CHANGE, GUIDesignTableAdditionals);
    myTable->setSelBackColor(FXRGBA(255, 255, 255, 255));
    myTable->setSelTextColor(FXRGBA(0, 0, 0, 255));
    myTable->setEditable(false);
    // separate demand elements in three groups
    for (const auto& i : invalidDemandElements) {
        if (i->getTagProperty().isVehicle()) {
            myInvalidVehicles.push_back(i);
        } else if (i->getTagProperty().isStop() || i->getTagProperty().isPersonStop()) {
            myInvalidStops.push_back(i);
        } else if (i->getTagProperty().isPersonPlan()) {
            myInvalidPersonPlans.push_back(i);
        } else {
            myInvalidRoutes.push_back(i);
        }
    }
    // clear table
    myTable->clearItems();
    // set number of rows
    myTable->setTableSize((int)(myInvalidRoutes.size() + myInvalidVehicles.size() + myInvalidStops.size() + myInvalidPersonPlans.size()), 3);
    // Configure list
    myTable->setVisibleColumns(4);
    myTable->setColumnWidth(0, GUIDesignTableIconCellWidth);
    myTable->setColumnWidth(1, 160);
    myTable->setColumnWidth(2, 280);
    myTable->setColumnText(0, "");
    myTable->setColumnText(1, toString(SUMO_ATTR_ID).c_str());
    myTable->setColumnText(2, "Conflict");
    myTable->getRowHeader()->setWidth(0);
    // Declare index for rows and pointer to FXTableItem
    int indexRow = 0;
    FXTableItem* item = nullptr;
    // iterate over invalid routes
    for (const auto& invalidRoute : myInvalidRoutes) {
        // Set icon
        item = new FXTableItem("", invalidRoute->getIcon());
        item->setIconPosition(FXTableItem::CENTER_X);
        myTable->setItem(indexRow, 0, item);
        // Set ID
        item = new FXTableItem(invalidRoute->getID().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 1, item);
        // Set conflict
        item = new FXTableItem(invalidRoute->getDemandElementProblem().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 2, item);
        // Update index
        indexRow++;
    }
    // iterate over invalid vehicles
    for (const auto& invalidVehicle : myInvalidVehicles) {
        // Set icon
        item = new FXTableItem("", invalidVehicle->getIcon());
        item->setIconPosition(FXTableItem::CENTER_X);
        myTable->setItem(indexRow, 0, item);
        // Set ID
        item = new FXTableItem(invalidVehicle->getID().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 1, item);
        // Set conflict
        item = new FXTableItem(invalidVehicle->getDemandElementProblem().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 2, item);
        // Update index
        indexRow++;
    }
    // iterate over invalid stops
    for (const auto& invalidStop : myInvalidStops) {
        // Set icon
        item = new FXTableItem("", invalidStop->getIcon());
        item->setIconPosition(FXTableItem::CENTER_X);
        myTable->setItem(indexRow, 0, item);
        // Set ID
        item = new FXTableItem(invalidStop->getID().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 1, item);
        // Set conflict
        item = new FXTableItem(invalidStop->getDemandElementProblem().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 2, item);
        // Update index
        indexRow++;
    }
    // iterate over invalid person plans
    for (const auto& invalidPersonPlan : myInvalidPersonPlans) {
        // Set icon
        item = new FXTableItem("", invalidPersonPlan->getIcon());
        item->setIconPosition(FXTableItem::CENTER_X);
        myTable->setItem(indexRow, 0, item);
        // Set ID
        item = new FXTableItem(invalidPersonPlan->getID().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 1, item);
        // Set conflict
        item = new FXTableItem(invalidPersonPlan->getDemandElementProblem().c_str());
        item->setJustify(FXTableItem::LEFT | FXTableItem::CENTER_Y);
        myTable->setItem(indexRow, 2, item);
        // Update index
        indexRow++;
    }
}


// ---------------------------------------------------------------------------
// GNEFixDemandElements::FixRouteOptions - methods
// ---------------------------------------------------------------------------

GNEFixDemandElements::FixRouteOptions::FixRouteOptions(GNEFixDemandElements* fixDemandElementsDialogParents) :
    FXGroupBox(fixDemandElementsDialogParents->myMainFrame, "Solution for routes", GUIDesignGroupBoxFrame) {
    // create horizontal frames for radio buttons
    FXHorizontalFrame* radioButtonsFrame = new FXHorizontalFrame(this, GUIDesignHorizontalFrame);
    // create radio button for remove invalid routes
    removeInvalidRoutes = new FXRadioButton(radioButtonsFrame, "Remove invalid routes",
                                            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create radio button for save invalid routes
    saveInvalidRoutes = new FXRadioButton(radioButtonsFrame, "Save invalid routes",
                                          fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create radio button for select invalid routes
    selectInvalidRoutesAndCancel = new FXRadioButton(radioButtonsFrame, "Select invalid routes",
            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // leave option "removeInvalidRoutes" as default
    removeInvalidRoutes->setCheck(true);
}


void
GNEFixDemandElements::FixRouteOptions::selectOption(FXObject* option) {
    if (option == removeInvalidRoutes) {
        removeInvalidRoutes->setCheck(true);
        saveInvalidRoutes->setCheck(false);
        selectInvalidRoutesAndCancel->setCheck(false);
    } else if (option == saveInvalidRoutes) {
        removeInvalidRoutes->setCheck(false);
        saveInvalidRoutes->setCheck(true);
        selectInvalidRoutesAndCancel->setCheck(false);
    } else if (option == selectInvalidRoutesAndCancel) {
        removeInvalidRoutes->setCheck(false);
        saveInvalidRoutes->setCheck(false);
        selectInvalidRoutesAndCancel->setCheck(true);
    }
}


void
GNEFixDemandElements::FixRouteOptions::enableFixRouteOptions() {
    removeInvalidRoutes->enable();
    saveInvalidRoutes->enable();
    selectInvalidRoutesAndCancel->enable();
}


void
GNEFixDemandElements::FixRouteOptions::disableFixRouteOptions() {
    removeInvalidRoutes->disable();
    saveInvalidRoutes->disable();
    selectInvalidRoutesAndCancel->disable();
}

// ---------------------------------------------------------------------------
// GNEFixDemandElements::FixVehicleOptions - methods
// ---------------------------------------------------------------------------

GNEFixDemandElements::FixVehicleOptions::FixVehicleOptions(GNEFixDemandElements* fixDemandElementsDialogParents) :
    FXGroupBox(fixDemandElementsDialogParents->myMainFrame, "Solution for vehicles", GUIDesignGroupBoxFrame) {
    // create horizontal frames for radio buttons
    FXHorizontalFrame* radioButtonsFrame = new FXHorizontalFrame(this, GUIDesignHorizontalFrame);
    // create radio button for remove invalid vehicles
    removeInvalidVehicles = new FXRadioButton(radioButtonsFrame, "Remove invalid vehicles",
            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create radio button for save invalid vehicles
    saveInvalidVehicles = new FXRadioButton(radioButtonsFrame, "Save invalid vehicles",
                                            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create radio button for select invalid vehicles
    selectInvalidVehiclesAndCancel = new FXRadioButton(radioButtonsFrame, "Select invalid vehicle",
            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // leave option "buildConnectionBetweenLanes" and "removeInvalidRoutes" as default
    removeInvalidVehicles->setCheck(true);
}


void
GNEFixDemandElements::FixVehicleOptions::selectOption(FXObject* option) {
    if (option == removeInvalidVehicles) {
        removeInvalidVehicles->setCheck(true);
        saveInvalidVehicles->setCheck(false);
        selectInvalidVehiclesAndCancel->setCheck(false);
    } else if (option == saveInvalidVehicles) {
        removeInvalidVehicles->setCheck(false);
        saveInvalidVehicles->setCheck(true);
        selectInvalidVehiclesAndCancel->setCheck(false);
    } else if (option == selectInvalidVehiclesAndCancel) {
        removeInvalidVehicles->setCheck(false);
        saveInvalidVehicles->setCheck(false);
        selectInvalidVehiclesAndCancel->setCheck(true);
    }
}


void
GNEFixDemandElements::FixVehicleOptions::enableFixVehicleOptions() {
    removeInvalidVehicles->enable();
    saveInvalidVehicles->enable();
    selectInvalidVehiclesAndCancel->enable();
}


void
GNEFixDemandElements::FixVehicleOptions::disableFixVehicleOptions() {
    removeInvalidVehicles->disable();
    saveInvalidVehicles->disable();
    selectInvalidVehiclesAndCancel->disable();
}

// ---------------------------------------------------------------------------
// GNEFixDemandElements::FixStopOptions - methods
// ---------------------------------------------------------------------------

GNEFixDemandElements::FixStopOptions::FixStopOptions(GNEFixDemandElements* fixDemandElementsDialogParents) :
    FXGroupBox(fixDemandElementsDialogParents->myMainFrame, "Select a solution for stops", GUIDesignGroupBoxFrame) {
    // create horizontal frames for radio buttons
    FXHorizontalFrame* RadioButtons = new FXHorizontalFrame(this, GUIDesignHorizontalFrame);
    // create Vertical Frame for left options
    FXVerticalFrame* RadioButtonsLeft = new FXVerticalFrame(RadioButtons, GUIDesignAuxiliarVerticalFrame);
    activateFriendlyPositionAndSave = new FXRadioButton(RadioButtonsLeft, "Activate friendlyPos and save",
            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    saveInvalid = new FXRadioButton(RadioButtonsLeft, "Save invalid positions",
                                    fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create Vertical Frame for right options
    FXVerticalFrame* RadioButtonsRight = new FXVerticalFrame(RadioButtons, GUIDesignAuxiliarVerticalFrame);
    selectInvalidStopsAndCancel = new FXRadioButton(RadioButtonsRight, "Select invalid Stops",
            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    fixPositionsAndSave = new FXRadioButton(RadioButtonsRight, "Fix positions and save",
                                            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // leave option "activateFriendlyPositionAndSave" as default
    activateFriendlyPositionAndSave->setCheck(true);
}


void
GNEFixDemandElements::FixStopOptions::selectOption(FXObject* option) {
    if (option == activateFriendlyPositionAndSave) {
        activateFriendlyPositionAndSave->setCheck(true);
        fixPositionsAndSave->setCheck(false);
        saveInvalid->setCheck(false);
        selectInvalidStopsAndCancel->setCheck(false);
    } else if (option == fixPositionsAndSave) {
        activateFriendlyPositionAndSave->setCheck(false);
        fixPositionsAndSave->setCheck(true);
        saveInvalid->setCheck(false);
        selectInvalidStopsAndCancel->setCheck(false);
    } else if (option == saveInvalid) {
        activateFriendlyPositionAndSave->setCheck(false);
        fixPositionsAndSave->setCheck(false);
        saveInvalid->setCheck(true);
        selectInvalidStopsAndCancel->setCheck(false);
    } else if (option == selectInvalidStopsAndCancel) {
        activateFriendlyPositionAndSave->setCheck(false);
        fixPositionsAndSave->setCheck(false);
        saveInvalid->setCheck(false);
        selectInvalidStopsAndCancel->setCheck(true);
    }
}


void
GNEFixDemandElements::FixStopOptions::enableFixStopOptions() {
    activateFriendlyPositionAndSave->enable();
    fixPositionsAndSave->enable();
    saveInvalid->enable();
    selectInvalidStopsAndCancel->enable();
}


void
GNEFixDemandElements::FixStopOptions::disableFixStopOptions() {
    activateFriendlyPositionAndSave->disable();
    fixPositionsAndSave->disable();
    saveInvalid->disable();
    selectInvalidStopsAndCancel->disable();
}

// ---------------------------------------------------------------------------
// GNEFixDemandElements::FixPersonPlanOptions - methods
// ---------------------------------------------------------------------------

GNEFixDemandElements::FixPersonPlanOptions::FixPersonPlanOptions(GNEFixDemandElements* fixDemandElementsDialogParents) :
    FXGroupBox(fixDemandElementsDialogParents->myMainFrame, "Select a solution for person plans", GUIDesignGroupBoxFrame) {
    // create horizontal frames for radio buttons
    FXHorizontalFrame* RadioButtons = new FXHorizontalFrame(this, GUIDesignHorizontalFrame);
    // create Vertical Frame for left options
    FXVerticalFrame* RadioButtonsLeft = new FXVerticalFrame(RadioButtons, GUIDesignAuxiliarVerticalFrame);
    deletePersonPlan = new FXRadioButton(RadioButtonsLeft, "Delete person plan",
                                         fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    saveInvalid = new FXRadioButton(RadioButtonsLeft, "Save invalid person plans",
                                    fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create Vertical Frame for right options
    FXVerticalFrame* RadioButtonsRight = new FXVerticalFrame(RadioButtons, GUIDesignAuxiliarVerticalFrame);
    selectInvalidPersonPlansAndCancel = new FXRadioButton(RadioButtonsRight, "Select invalid person plans",
            fixDemandElementsDialogParents, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // leave option "activateFriendlyPositionAndSave" as default
    deletePersonPlan->setCheck(true);
}


void
GNEFixDemandElements::FixPersonPlanOptions::selectOption(FXObject* option) {
    if (option == deletePersonPlan) {
        deletePersonPlan->setCheck(true);
        saveInvalid->setCheck(false);
        selectInvalidPersonPlansAndCancel->setCheck(false);
        selectInvalidPersonPlansAndCancel->setCheck(false);
    } else if (option == saveInvalid) {
        deletePersonPlan->setCheck(false);
        saveInvalid->setCheck(true);
        selectInvalidPersonPlansAndCancel->setCheck(false);
    } else if (option == selectInvalidPersonPlansAndCancel) {
        deletePersonPlan->setCheck(false);
        saveInvalid->setCheck(false);
        selectInvalidPersonPlansAndCancel->setCheck(true);
    }
}


void
GNEFixDemandElements::FixPersonPlanOptions::enableFixPersonPlanOptions() {
    deletePersonPlan->enable();
    saveInvalid->enable();
    selectInvalidPersonPlansAndCancel->enable();
}


void
GNEFixDemandElements::FixPersonPlanOptions::disableFixPersonPlanOptions() {
    deletePersonPlan->disable();
    saveInvalid->disable();
    selectInvalidPersonPlansAndCancel->disable();
}

/****************************************************************************/
