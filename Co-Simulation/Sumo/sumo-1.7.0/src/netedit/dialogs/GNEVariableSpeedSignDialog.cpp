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
/// @file    GNEVariableSpeedSignDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    April 2016
///
// A class for edit phases of Variable Speed Signals
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/changes/GNEChange_Additional.h>
#include <netedit/elements/additional/GNEVariableSpeedSign.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>

#include "GNEVariableSpeedSignDialog.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEVariableSpeedSignDialog) GNERerouterDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_VARIABLESPEEDSIGN_ADDROW,   GNEVariableSpeedSignDialog::onCmdAddStep),
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_VARIABLESPEEDSIGN_SORT,     GNEVariableSpeedSignDialog::onCmdSortSteps),
    FXMAPFUNC(SEL_CLICKED,          MID_GNE_VARIABLESPEEDSIGN_TABLE,    GNEVariableSpeedSignDialog::onCmdClickedStep),
    FXMAPFUNC(SEL_DOUBLECLICKED,    MID_GNE_VARIABLESPEEDSIGN_TABLE,    GNEVariableSpeedSignDialog::onCmdClickedStep),
    FXMAPFUNC(SEL_TRIPLECLICKED,    MID_GNE_VARIABLESPEEDSIGN_TABLE,    GNEVariableSpeedSignDialog::onCmdClickedStep),
    FXMAPFUNC(SEL_UPDATE,           MID_GNE_VARIABLESPEEDSIGN_TABLE,    GNEVariableSpeedSignDialog::onCmdEditStep),
};

// Object implementation
FXIMPLEMENT(GNEVariableSpeedSignDialog, GNEAdditionalDialog, GNERerouterDialogMap, ARRAYNUMBER(GNERerouterDialogMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNEVariableSpeedSignDialog::GNEVariableSpeedSignDialog(GNEVariableSpeedSign* editedVariableSpeedSign) :
    GNEAdditionalDialog(editedVariableSpeedSign, false, 300, 400),
    myStepsValids(false) {

    // create Horizontal frame for row elements
    FXHorizontalFrame* myAddStepFrame = new FXHorizontalFrame(myContentFrame, GUIDesignAuxiliarHorizontalFrame);
    // create Button and Label for adding new Wors
    myAddStepButton = new FXButton(myAddStepFrame, "", GUIIconSubSys::getIcon(GUIIcon::ADD), this, MID_GNE_VARIABLESPEEDSIGN_ADDROW, GUIDesignButtonIcon);
    new FXLabel(myAddStepFrame, ("Add new " + toString(SUMO_TAG_STEP)).c_str(), nullptr, GUIDesignLabelThick);
    // create Button and Label for sort intervals
    mySortStepButton = new FXButton(myAddStepFrame, "", GUIIconSubSys::getIcon(GUIIcon::RELOAD), this, MID_GNE_VARIABLESPEEDSIGN_SORT, GUIDesignButtonIcon);
    new FXLabel(myAddStepFrame, ("Sort " + toString(SUMO_TAG_STEP) + "s").c_str(), nullptr, GUIDesignLabelThick);

    // create List with the data
    myStepsTable = new FXTable(myContentFrame, this, MID_GNE_VARIABLESPEEDSIGN_TABLE, GUIDesignTableAdditionals);
    myStepsTable->setSelBackColor(FXRGBA(255, 255, 255, 255));
    myStepsTable->setSelTextColor(FXRGBA(0, 0, 0, 255));

    // update table
    updateTableSteps();

    // start a undo list for editing local to this additional
    initChanges();

    // Open dialog as modal
    openAsModalDialog();
}


GNEVariableSpeedSignDialog::~GNEVariableSpeedSignDialog() {}


long
GNEVariableSpeedSignDialog::onCmdAddStep(FXObject*, FXSelector, void*) {
    // create step
    GNEVariableSpeedSignStep* step = new GNEVariableSpeedSignStep(myEditedAdditional, 0, 30);
    // add it using GNEChange_additional
    myEditedAdditional->getNet()->getViewNet()->getUndoList()->add(new GNEChange_Additional(step, true), true);
    // Update table
    updateTableSteps();
    return 1;
}


long
GNEVariableSpeedSignDialog::onCmdEditStep(FXObject*, FXSelector, void*) {
    // get VSS children
    std::vector<GNEAdditional*> VSSChildren;
    for (const auto& VSSChild : myEditedAdditional->getChildAdditionals()) {
        if (!VSSChild->getTagProperty().isSymbol()) {
            VSSChildren.push_back(VSSChild);
        }
    }
    myStepsValids = true;
    // iterate over table and check that all parameters are correct
    for (int i = 0; i < myStepsTable->getNumRows(); i++) {
        GNEAdditional* step = VSSChildren.at(i);
        if (step->isValid(SUMO_ATTR_TIME, myStepsTable->getItem(i, 0)->getText().text()) == false) {
            myStepsValids = false;
            myStepsTable->getItem(i, 2)->setIcon(GUIIconSubSys::getIcon(GUIIcon::INCORRECT));
        } else if (step->isValid(SUMO_ATTR_SPEED, myStepsTable->getItem(i, 1)->getText().text()) == false) {
            myStepsValids = false;
            myStepsTable->getItem(i, 2)->setIcon(GUIIconSubSys::getIcon(GUIIcon::INCORRECT));
        } else {
            // we need filter attribute (to avoid problemes as 1 != 1.00)
            double time = GNEAttributeCarrier::parse<double>(myStepsTable->getItem(i, 0)->getText().text());
            double speed = GNEAttributeCarrier::parse<double>(myStepsTable->getItem(i, 1)->getText().text());
            // set new values in Closing  reroute
            step->setAttribute(SUMO_ATTR_TIME, toString(time), myEditedAdditional->getNet()->getViewNet()->getUndoList());
            step->setAttribute(SUMO_ATTR_SPEED, toString(speed), myEditedAdditional->getNet()->getViewNet()->getUndoList());
            // set Correct label
            myStepsTable->getItem(i, 2)->setIcon(GUIIconSubSys::getIcon(GUIIcon::CORRECT));
        }
    }
    // update list
    myStepsTable->update();
    return 1;
}


long
GNEVariableSpeedSignDialog::onCmdClickedStep(FXObject*, FXSelector, void*) {
    // get VSS children
    std::vector<GNEAdditional*> VSSChildren;
    for (const auto& VSSChild : myEditedAdditional->getChildAdditionals()) {
        if (!VSSChild->getTagProperty().isSymbol()) {
            VSSChildren.push_back(VSSChild);
        }
    }
    // check if some delete button was pressed
    for (int i = 0; i < (int)VSSChildren.size(); i++) {
        if (myStepsTable->getItem(i, 3)->hasFocus()) {
            myStepsTable->removeRows(i);
            myEditedAdditional->getNet()->getViewNet()->getUndoList()->add(new GNEChange_Additional(VSSChildren.at(i), false), true);
            // Update table
            updateTableSteps();
            return 1;
        }
    }
    return 0;
}


long
GNEVariableSpeedSignDialog::onCmdSortSteps(FXObject*, FXSelector, void*) {
    // update table
    updateTableSteps();
    return 1;
}


long
GNEVariableSpeedSignDialog::onCmdAccept(FXObject*, FXSelector, void*) {
    if (myStepsValids == false) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
        // open warning Box
        FXMessageBox::warning(getApp(), MBOX_OK, ("Error updating " + toString(SUMO_TAG_VSS) + " " + toString(SUMO_TAG_STEP)).c_str(), "%s",
                              (toString(SUMO_TAG_VSS) + " " + toString(SUMO_TAG_STEP) + "cannot be updated because there are invalid values").c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
        return 0;
    } else {
        // accept changes before closing dialog
        acceptChanges();
        // stop dialgo sucesfully
        getApp()->stopModal(this, TRUE);
        return 1;
    }
}


long
GNEVariableSpeedSignDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // cancel changes
    cancelChanges();
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNEVariableSpeedSignDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // reset changes
    resetChanges();
    // update steps tables
    updateTableSteps();
    return 1;
}


void
GNEVariableSpeedSignDialog::updateTableSteps() {
    // get VSS children
    std::vector<GNEAdditional*> VSSChildren;
    for (const auto& VSSChild : myEditedAdditional->getChildAdditionals()) {
        if (!VSSChild->getTagProperty().isSymbol()) {
            VSSChildren.push_back(VSSChild);
        }
    }
    // clear table
    myStepsTable->clearItems();
    // set number of rows
    myStepsTable->setTableSize(int(VSSChildren.size()), 4);
    // Configure list
    myStepsTable->setVisibleColumns(4);
    myStepsTable->setColumnWidth(0, 115);
    myStepsTable->setColumnWidth(1, 114);
    myStepsTable->setColumnWidth(2, GUIDesignTableIconCellWidth);
    myStepsTable->setColumnWidth(3, GUIDesignTableIconCellWidth);
    myStepsTable->setColumnText(0, "timeStep");
    myStepsTable->setColumnText(1, "speed (m/s)");
    myStepsTable->setColumnText(2, "");
    myStepsTable->setColumnText(3, "");
    myStepsTable->getRowHeader()->setWidth(0);
    // Declare index for rows and pointer to FXTableItem
    FXTableItem* item = nullptr;
    // iterate over values
    for (int i = 0; i < (int)VSSChildren.size(); i++) {
        // Set time
        item = new FXTableItem(VSSChildren.at(i)->getAttribute(SUMO_ATTR_TIME).c_str());
        myStepsTable->setItem(i, 0, item);
        // Set speed
        item = new FXTableItem(VSSChildren.at(i)->getAttribute(SUMO_ATTR_SPEED).c_str());
        myStepsTable->setItem(i, 1, item);
        // set valid icon
        item = new FXTableItem("");
        item->setIcon(GUIIconSubSys::getIcon(GUIIcon::CORRECT));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myStepsTable->setItem(i, 2, item);
        // set remove Icon
        item = new FXTableItem("", GUIIconSubSys::getIcon(GUIIcon::REMOVE));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myStepsTable->setItem(i, 3, item);
    }
}


/****************************************************************************/
