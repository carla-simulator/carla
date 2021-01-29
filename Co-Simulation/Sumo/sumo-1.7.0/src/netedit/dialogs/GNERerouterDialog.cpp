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
/// @file    GNERerouterDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    April 2016
///
// Dialog for edit rerouters
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/changes/GNEChange_Additional.h>
#include <netedit/elements/additional/GNERerouter.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>

#include "GNERerouterDialog.h"
#include "GNERerouterIntervalDialog.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNERerouterDialog) GNERerouterDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_REROUTEDIALOG_ADD_INTERVAL,     GNERerouterDialog::onCmdAddInterval),
    FXMAPFUNC(SEL_DOUBLECLICKED,    MID_GNE_REROUTEDIALOG_TABLE_INTERVAL,   GNERerouterDialog::onCmdClickedInterval),
    FXMAPFUNC(SEL_TRIPLECLICKED,    MID_GNE_REROUTEDIALOG_TABLE_INTERVAL,   GNERerouterDialog::onCmdClickedInterval),
};

// Object implementation
FXIMPLEMENT(GNERerouterDialog, GNEAdditionalDialog, GNERerouterDialogMap, ARRAYNUMBER(GNERerouterDialogMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNERerouterDialog::GNERerouterDialog(GNERerouter* rerouterParent) :
    GNEAdditionalDialog(rerouterParent, false, 320, 240) {

    // create Horizontal frame for row elements
    FXHorizontalFrame* myAddIntervalFrame = new FXHorizontalFrame(myContentFrame, GUIDesignAuxiliarHorizontalFrame);
    // create Button and Label for adding new Wors
    myAddInterval = new FXButton(myAddIntervalFrame, "", GUIIconSubSys::getIcon(GUIIcon::ADD), this, MID_GNE_REROUTEDIALOG_ADD_INTERVAL, GUIDesignButtonIcon);
    new FXLabel(myAddIntervalFrame, ("Add new " + toString(SUMO_TAG_INTERVAL)).c_str(), nullptr, GUIDesignLabelThick);
    // create Button and Label for sort intervals
    mySortIntervals = new FXButton(myAddIntervalFrame, "", GUIIconSubSys::getIcon(GUIIcon::RELOAD), this, MID_GNE_REROUTEDIALOG_SORT_INTERVAL, GUIDesignButtonIcon);
    new FXLabel(myAddIntervalFrame, ("Sort " + toString(SUMO_TAG_INTERVAL) + "s").c_str(), nullptr, GUIDesignLabelThick);

    // Create table
    myIntervalTable = new FXTable(myContentFrame, this, MID_GNE_REROUTEDIALOG_TABLE_INTERVAL, GUIDesignTableAdditionals);
    myIntervalTable->setSelBackColor(FXRGBA(255, 255, 255, 255));
    myIntervalTable->setSelTextColor(FXRGBA(0, 0, 0, 255));
    myIntervalTable->setEditable(false);

    // update intervals
    updateIntervalTable();

    // start a undo list for editing local to this additional
    initChanges();

    // Open dialog as modal
    openAsModalDialog();
}


GNERerouterDialog::~GNERerouterDialog() {}


long
GNERerouterDialog::onCmdAccept(FXObject*, FXSelector, void*) {
    // Check if there is overlapping between Intervals
    if (!myEditedAdditional->checkChildAdditionalsOverlapping()) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
        // open warning Box
        FXMessageBox::warning(getApp(), MBOX_OK, "Overlapping detected", "%s", ("Values of '" + myEditedAdditional->getID() + "' cannot be saved. There are intervals overlapped.").c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
        return 0;
    } else {
        // accept changes before closing dialog
        acceptChanges();
        // Stop Modal
        getApp()->stopModal(this, TRUE);
        return 1;
    }
}


long
GNERerouterDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // cancel changes
    cancelChanges();
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNERerouterDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // reset changes
    resetChanges();
    // update interval table
    updateIntervalTable();
    return 1;
}


long
GNERerouterDialog::onCmdAddInterval(FXObject*, FXSelector, void*) {
    // create empty rerouter interval and configure it with GNERerouterIntervalDialog
    GNERerouterIntervalDialog(new GNERerouterInterval(this), false);
    // update interval table
    updateIntervalTable();
    return 1;
}


long
GNERerouterDialog::onCmdClickedInterval(FXObject*, FXSelector, void*) {
    // get rerouter children
    std::vector<GNEAdditional*> rerouterChildren;
    for (const auto& rerouterChild : myEditedAdditional->getChildAdditionals()) {
        if (!rerouterChild->getTagProperty().isSymbol()) {
            rerouterChildren.push_back(rerouterChild);
        }
    }
    // check if some delete button was pressed
    for (int i = 0; i < (int)rerouterChildren.size(); i++) {
        if (myIntervalTable->getItem(i, 2)->hasFocus()) {
            // remove interval
            myEditedAdditional->getNet()->getViewNet()->getUndoList()->add(new GNEChange_Additional(rerouterChildren.at(i), false), true);
            // update interval table after removing
            updateIntervalTable();
            return 1;
        }
    }
    // check if some begin or o end  button was pressed
    for (int i = 0; i < (int)rerouterChildren.size(); i++) {
        if (myIntervalTable->getItem(i, 0)->hasFocus() || myIntervalTable->getItem(i, 1)->hasFocus()) {
            // edit interval
            GNERerouterIntervalDialog(rerouterChildren.at(i), true);
            // update interval table after editing
            updateIntervalTable();
            return 1;
        }
    }
    // nothing to do
    return 0;
}


void
GNERerouterDialog::updateIntervalTable() {
    // get rerouter children
    std::vector<GNEAdditional*> rerouterChildren;
    for (const auto& rerouterChild : myEditedAdditional->getChildAdditionals()) {
        if (!rerouterChild->getTagProperty().isSymbol()) {
            rerouterChildren.push_back(rerouterChild);
        }
    }
    // clear table
    myIntervalTable->clearItems();
    // set number of rows
    myIntervalTable->setTableSize(int(rerouterChildren.size()), 3);
    // Configure list
    myIntervalTable->setVisibleColumns(4);
    myIntervalTable->setColumnWidth(0, 137);
    myIntervalTable->setColumnWidth(1, 136);
    myIntervalTable->setColumnWidth(2, GUIDesignTableIconCellWidth);
    myIntervalTable->setColumnText(0, toString(SUMO_ATTR_BEGIN).c_str());
    myIntervalTable->setColumnText(1, toString(SUMO_ATTR_END).c_str());
    myIntervalTable->setColumnText(2, "");
    myIntervalTable->getRowHeader()->setWidth(0);
    // Declare index for rows and pointer to FXTableItem
    int indexRow = 0;
    FXTableItem* item = nullptr;
    // iterate over values
    for (const auto& rerouterChild : rerouterChildren) {
        // Set time
        item = new FXTableItem(rerouterChild->getAttribute(SUMO_ATTR_BEGIN).c_str());
        myIntervalTable->setItem(indexRow, 0, item);
        // Set speed
        item = new FXTableItem(rerouterChild->getAttribute(SUMO_ATTR_END).c_str());
        myIntervalTable->setItem(indexRow, 1, item);
        // set remove
        item = new FXTableItem("", GUIIconSubSys::getIcon(GUIIcon::REMOVE));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myIntervalTable->setItem(indexRow, 2, item);
        // Update index
        indexRow++;
    }
}


/****************************************************************************/
