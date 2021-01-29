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
/// @file    GNERouteDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    March 2017
///
// Dialog for edit calibrator routes
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/elements/demand/GNERoute.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>

#include "GNERouteDialog.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNERouteDialog) GNERouteDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,  GNERouteDialog::onCmdSetVariable),
};

// Object implementation
FXIMPLEMENT(GNERouteDialog, GNEDemandElementDialog, GNERouteDialogMap, ARRAYNUMBER(GNERouteDialogMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNERouteDialog::GNERouteDialog(GNEDemandElement* editedCalibratorRoute, bool updatingElement) :
    GNEDemandElementDialog(editedCalibratorRoute, updatingElement, 400, 120),
    myCalibratorRouteValid(true) {
    // change default header
    std::string typeOfOperation =  + " for ";
    changeDemandElementDialogHeader(myUpdatingElement ? "Edit " + myEditedDemandElement->getTagStr() + " of " : "Create " + myEditedDemandElement->getTagStr());

    // Create auxiliar frames for data
    FXHorizontalFrame* columns = new FXHorizontalFrame(myContentFrame, GUIDesignUniformHorizontalFrame);
    FXVerticalFrame* columnLeft = new FXVerticalFrame(columns, GUIDesignAuxiliarFrame);
    FXVerticalFrame* columnRight = new FXVerticalFrame(columns, GUIDesignAuxiliarFrame);

    // create ID's elements
    new FXLabel(columnLeft, toString(SUMO_ATTR_ID).c_str(), nullptr, GUIDesignLabelLeftThick);
    myTextFieldRouteID = new FXTextField(columnRight, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);

    // create list of edge's elements
    new FXLabel(columnLeft, toString(SUMO_ATTR_EDGES).c_str(), nullptr, GUIDesignLabelLeftThick);
    myTextFieldEdges = new FXTextField(columnRight, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);

    // create color's elements
    new FXLabel(columnLeft, toString(SUMO_ATTR_COLOR).c_str(), nullptr, GUIDesignLabelLeftThick);
    myTextFieldColor = new FXTextField(columnRight, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);

    // update tables
    updateCalibratorRouteValues();

    // start a undo list for editing local to this demand element
    initChanges();

    // add element if we aren't updating an existent element
    if (myUpdatingElement == false) {
        myEditedDemandElement->getNet()->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(myEditedDemandElement, true), true);
        // Routes are created without edges
        myCalibratorRouteValid = false;
        myInvalidAttr = SUMO_ATTR_EDGES;
    }

    // open as modal dialog
    openAsModalDialog();
}


GNERouteDialog::~GNERouteDialog() {}


long
GNERouteDialog::onCmdAccept(FXObject*, FXSelector, void*) {
    if (myCalibratorRouteValid == false) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
        std::string operation1 = myUpdatingElement ? ("updating") : ("creating");
        std::string operation2 = myUpdatingElement ? ("updated") : ("created");
        std::string tagString = myEditedDemandElement->getTagStr();
        // open warning dialog box
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Error " + operation1 + " " + tagString).c_str(), "%s",
                              (tagString + " cannot be " + operation2 + " because parameter " + toString(myInvalidAttr) + " is invalid.").c_str());
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
GNERouteDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // cancel changes
    cancelChanges();
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNERouteDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // reset changes
    resetChanges();
    // update fields
    updateCalibratorRouteValues();
    return 1;
}


long
GNERouteDialog::onCmdSetVariable(FXObject*, FXSelector, void*) {
    // At start we assumed, that all values are valid
    myCalibratorRouteValid = true;
    myInvalidAttr = SUMO_ATTR_NOTHING;
    // set color of myTextFieldRouteID, depending if current value is valid or not
    if (myEditedDemandElement->getID() == myTextFieldRouteID->getText().text()) {
        myTextFieldRouteID->setTextColor(FXRGB(0, 0, 0));
        myEditedDemandElement->setAttribute(SUMO_ATTR_ID, myTextFieldRouteID->getText().text(), myEditedDemandElement->getNet()->getViewNet()->getUndoList());
    } else if (myEditedDemandElement->isValid(SUMO_ATTR_ID, myTextFieldRouteID->getText().text())) {
        myTextFieldRouteID->setTextColor(FXRGB(0, 0, 0));
        myEditedDemandElement->setAttribute(SUMO_ATTR_ID, myTextFieldRouteID->getText().text(), myEditedDemandElement->getNet()->getViewNet()->getUndoList());
    } else {
        myTextFieldRouteID->setTextColor(FXRGB(255, 0, 0));
        myCalibratorRouteValid = false;
        myInvalidAttr = SUMO_ATTR_ID;
    }
    // set color of myTextFieldRouteEdges, depending if current value is valEdges or not
    if (myEditedDemandElement->isValid(SUMO_ATTR_EDGES, myTextFieldEdges->getText().text())) {
        myTextFieldEdges->setTextColor(FXRGB(0, 0, 0));
        myEditedDemandElement->setAttribute(SUMO_ATTR_EDGES, myTextFieldEdges->getText().text(), myEditedDemandElement->getNet()->getViewNet()->getUndoList());
    } else {
        myTextFieldEdges->setTextColor(FXRGB(255, 0, 0));
        myCalibratorRouteValid = false;
        myInvalidAttr = SUMO_ATTR_EDGES;
    }
    // set color of myTextFieldColor, depending if current value is valid or not
    if (myEditedDemandElement->isValid(SUMO_ATTR_COLOR, myTextFieldColor->getText().text())) {
        myTextFieldColor->setTextColor(FXRGB(0, 0, 0));
        myEditedDemandElement->setAttribute(SUMO_ATTR_COLOR, myTextFieldColor->getText().text(), myEditedDemandElement->getNet()->getViewNet()->getUndoList());
    } else {
        myTextFieldColor->setTextColor(FXRGB(255, 0, 0));
        myCalibratorRouteValid = false;
        myInvalidAttr = SUMO_ATTR_COLOR;
    }
    return 1;
}


void
GNERouteDialog::updateCalibratorRouteValues() {
    myTextFieldRouteID->setText(myEditedDemandElement->getID().c_str());
    myTextFieldEdges->setText(myEditedDemandElement->getAttribute(SUMO_ATTR_EDGES).c_str());
    myTextFieldColor->setText(myEditedDemandElement->getAttribute(SUMO_ATTR_COLOR).c_str());
}


/****************************************************************************/
