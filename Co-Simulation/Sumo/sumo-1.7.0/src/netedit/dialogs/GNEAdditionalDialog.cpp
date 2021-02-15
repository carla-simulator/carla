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
/// @file    GNEAdditionalDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    April 2016
///
// A abstract class for editing additional elements
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/elements/additional/GNEAdditional.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>

#include "GNEAdditionalDialog.h"

// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEAdditionalDialog) GNEAdditionalDialogMap[] = {
    FXMAPFUNC(SEL_KEYPRESS,     0,                      GNEAdditionalDialog::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE,   0,                      GNEAdditionalDialog::onKeyRelease),
    FXMAPFUNC(SEL_CLOSE,        0,                      GNEAdditionalDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND,      MID_GNE_BUTTON_ACCEPT,  GNEAdditionalDialog::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND,      MID_GNE_BUTTON_CANCEL,  GNEAdditionalDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND,      MID_GNE_BUTTON_RESET,   GNEAdditionalDialog::onCmdReset),
};

// Object abstract implementation
FXIMPLEMENT_ABSTRACT(GNEAdditionalDialog, FXTopWindow, GNEAdditionalDialogMap, ARRAYNUMBER(GNEAdditionalDialogMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNEAdditionalDialog::GNEAdditionalDialog(GNEAdditional* editedAdditional, bool updatingElement, int width, int height) :
    FXTopWindow(editedAdditional->getNet()->getViewNet(), ("Edit '" + editedAdditional->getID() + "' data").c_str(), editedAdditional->getIcon(), editedAdditional->getIcon(), GUIDesignDialogBoxExplicit(width, height)),
    myEditedAdditional(editedAdditional),
    myUpdatingElement(updatingElement),
    myChangesDescription("change " + editedAdditional->getTagStr() + " values"),
    myNumberOfChanges(0) {
    // create main frame
    FXVerticalFrame* mainFrame = new FXVerticalFrame(this, GUIDesignAuxiliarFrame);
    // Create frame for contents
    myContentFrame = new FXVerticalFrame(mainFrame, GUIDesignContentsFrame);
    // create buttons centered
    FXHorizontalFrame* buttonsFrame = new FXHorizontalFrame(mainFrame, GUIDesignHorizontalFrame);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
    myAcceptButton = new FXButton(buttonsFrame, "accept\t\tclose accepting changes",  GUIIconSubSys::getIcon(GUIIcon::ACCEPT), this, MID_GNE_BUTTON_ACCEPT, GUIDesignButtonAccept);
    myCancelButton = new FXButton(buttonsFrame, "cancel\t\tclose discarding changes", GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_GNE_BUTTON_CANCEL, GUIDesignButtonCancel);
    myResetButton = new FXButton(buttonsFrame,  "reset\t\treset to previous values",  GUIIconSubSys::getIcon(GUIIcon::RESET),  this, MID_GNE_BUTTON_RESET,  GUIDesignButtonReset);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
}


GNEAdditionalDialog::~GNEAdditionalDialog() {
    // return focus to GNEViewNet to avoid minimization
    getParent()->setFocus();
}


FXint
GNEAdditionalDialog::openAsModalDialog(FXuint placement) {
    // create Dialog
    create();
    // show in the given position
    show(placement);
    // refresh APP
    getApp()->refresh();
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    return getApp()->runModalFor(this);
}


GNEAdditional*
GNEAdditionalDialog::getEditedAdditional() const {
    return myEditedAdditional;
}


long
GNEAdditionalDialog::onKeyPress(FXObject* sender, FXSelector sel, void* ptr) {
    return FXTopWindow::onKeyPress(sender, sel, ptr);
}


long
GNEAdditionalDialog::onKeyRelease(FXObject* sender, FXSelector sel, void* ptr) {
    return FXTopWindow::onKeyRelease(sender, sel, ptr);
}


void
GNEAdditionalDialog::changeAdditionalDialogHeader(const std::string& newHeader) {
    // change FXDialogBox title
    setTitle(newHeader.c_str());
}


void
GNEAdditionalDialog::initChanges() {
    // init commandGroup
    myEditedAdditional->getNet()->getViewNet()->getUndoList()->p_begin(myChangesDescription);
    // save number of command group changes
    myNumberOfChanges = myEditedAdditional->getNet()->getViewNet()->getUndoList()->currentCommandGroupSize();
}


void
GNEAdditionalDialog::acceptChanges() {
    // commit changes or abort last command group depending of number of changes did
    if (myNumberOfChanges < myEditedAdditional->getNet()->getViewNet()->getUndoList()->currentCommandGroupSize()) {
        myEditedAdditional->getNet()->getViewNet()->getUndoList()->p_end();
    } else {
        myEditedAdditional->getNet()->getViewNet()->getUndoList()->p_abortLastCommandGroup();
    }
}


void
GNEAdditionalDialog::cancelChanges() {
    myEditedAdditional->getNet()->getViewNet()->getUndoList()->p_abortLastCommandGroup();
}


void
GNEAdditionalDialog::resetChanges() {
    // abort last command group an start editing again
    myEditedAdditional->getNet()->getViewNet()->getUndoList()->p_abortLastCommandGroup();
    myEditedAdditional->getNet()->getViewNet()->getUndoList()->p_begin(myChangesDescription);
}


/****************************************************************************/
