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
/// @file    GNEUndoList.cpp
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// FXUndoList is pretty dandy but some features are missing:
//   - we cannot find out wether we have currently begun an undo-group and
//     thus abort() is hard to use.
//   - onUpd-methods do not disable undo/redo while in an undo-group
//
// GNEUndoList inherits from FXUndoList and patches some methods. these are
// prefixed with p_
/****************************************************************************/
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNEViewNet.h>

#include "GNEApplicationWindow.h"
#include "GNEUndoList.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GNEUndoList) GNEUndoListMap[] = {
    //FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REVERT,     FXUndoList::onCmdRevert),
    //FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_UNDO,       FXUndoList::onCmdUndo),
    //FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REDO,       FXUndoList::onCmdRedo),
    //FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_UNDO_ALL,   FXUndoList::onCmdUndoAll),
    //FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REDO_ALL,   FXUndoList::onCmdRedoAll),
    //
    //FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO_COUNT, FXUndoList::onUpdUndoCount),
    //FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO_COUNT, FXUndoList::onUpdRedoCount),
    //FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_CLEAR,      FXUndoList::onUpdClear),
    //FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REVERT,     FXUndoList::onUpdRevert),
    FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO_ALL,   GNEUndoList::p_onUpdUndo),
    FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO_ALL,   GNEUndoList::p_onUpdRedo),
    FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO,       GNEUndoList::p_onUpdUndo),
    FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO,       GNEUndoList::p_onUpdRedo)
};


// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEUndoList, FXUndoList, GNEUndoListMap, ARRAYNUMBER(GNEUndoListMap))


// ===========================================================================
// member method definitions
// ===========================================================================

GNEUndoList::GNEUndoList(GNEApplicationWindow* parent) :
    FXUndoList(),
    myGNEApplicationWindowParent(parent) {
}


void
GNEUndoList::p_begin(const std::string& description) {
    myCommandGroups.push(new CommandGroup(description));
    begin(myCommandGroups.top());
}


void
GNEUndoList::p_end() {
    myCommandGroups.pop();
    // check if net has to be updated
    if (myCommandGroups.empty() && myGNEApplicationWindowParent->getViewNet()) {
        myGNEApplicationWindowParent->getViewNet()->updateViewNet();
    }
    end();
}


void
GNEUndoList::p_clear() {
    // disable updating of interval bar (check viewNet due #7252)
    if (myGNEApplicationWindowParent->getViewNet()) {
        myGNEApplicationWindowParent->getViewNet()->getIntervalBar().disableIntervalBarUpdate();
    }
    p_abort();
    clear();
    // enable updating of interval bar again (check viewNet due #7252)
    if (myGNEApplicationWindowParent->getViewNet()) {
        myGNEApplicationWindowParent->getViewNet()->getIntervalBar().enableIntervalBarUpdate();
    }
}


void
GNEUndoList::p_abort() {
    while (hasCommandGroup()) {
        myCommandGroups.top()->undo();
        myCommandGroups.pop();
        abort();
    }
}


void
GNEUndoList::p_abortLastCommandGroup() {
    if (myCommandGroups.size() > 0) {
        myCommandGroups.top()->undo();
        myCommandGroups.pop();
        abort();
    }
}


void
GNEUndoList::undo() {
    WRITE_DEBUG("Calling GNEUndoList::undo()");
    FXUndoList::undo();
    // update specific controls
    myGNEApplicationWindowParent->updateControls();
}


void
GNEUndoList::redo() {
    WRITE_DEBUG("Calling GNEUndoList::redo()");
    FXUndoList::redo();
    // update specific controls
    myGNEApplicationWindowParent->updateControls();
}


void
GNEUndoList::p_add(GNEChange_Attribute* cmd) {
    if (cmd->trueChange()) {
        add(cmd, true);
    } else {
        delete cmd;
    }
}


int
GNEUndoList::currentCommandGroupSize() const {
    if (myCommandGroups.size() > 0) {
        return myCommandGroups.top()->size();
    } else {
        return 0;
    }
}


long
GNEUndoList::p_onUpdUndo(FXObject* sender, FXSelector, void*) {
    // first check if Undo Menu command or button has to be disabled
    bool enable = canUndo() && !hasCommandGroup() && myGNEApplicationWindowParent->isUndoRedoEnabled().empty();
    // cast button (see #6209)
    FXButton* button = dynamic_cast<FXButton*>(sender);
    // enable or disable depending of "enable" flag
    if (button) {
        // avoid unnnecesary enables/disables (due flickering)
        if (enable && !button->isEnabled()) {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
            button->update();
        } else if (!enable && button->isEnabled()) {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
            button->update();
        }
    } else {
        sender->handle(this, enable ? FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE) : FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    }
    // cast menu command
    FXMenuCommand* menuCommand = dynamic_cast<FXMenuCommand*>(sender);
    // only set caption on menu command item
    if (menuCommand) {
        // change caption of FXMenuCommand
        FXString caption = undoName();
        // set caption of FXmenuCommand edit/undo
        if (myGNEApplicationWindowParent->isUndoRedoEnabled().size() > 0) {
            caption = ("Cannot Undo in the middle of " + myGNEApplicationWindowParent->isUndoRedoEnabled()).c_str();
        } else if (hasCommandGroup()) {
            caption = ("Cannot Undo in the middle of " + myCommandGroups.top()->getDescription()).c_str();
        } else if (!canUndo()) {
            caption = "Undo";
        }
        menuCommand->handle(this, FXSEL(SEL_COMMAND, FXMenuCaption::ID_SETSTRINGVALUE), (void*)&caption);
        menuCommand->update();
    }
    return 1;
}


long
GNEUndoList::p_onUpdRedo(FXObject* sender, FXSelector, void*) {
    // first check if Redo Menu command or button has to be disabled
    bool enable = canRedo() && !hasCommandGroup() && myGNEApplicationWindowParent->isUndoRedoEnabled().empty();
    // cast button (see #6209)
    FXButton* button = dynamic_cast<FXButton*>(sender);
    // enable or disable depending of "enable" flag
    if (button) {
        // avoid unnnecesary enables/disables (due flickering)
        if (enable && !button->isEnabled()) {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
            button->update();
        } else if (!enable && button->isEnabled()) {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
            button->update();
        }
    } else {
        sender->handle(this, enable ? FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE) : FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    }
    // cast menu command
    FXMenuCommand* menuCommand = dynamic_cast<FXMenuCommand*>(sender);
    // only set caption on menu command item
    if (menuCommand) {
        // change caption of FXMenuCommand
        FXString caption = redoName();
        // set caption of FXmenuCommand edit/undo
        if (myGNEApplicationWindowParent->isUndoRedoEnabled().size() > 0) {
            caption = ("Cannot Redo in the middle of " + myGNEApplicationWindowParent->isUndoRedoEnabled()).c_str();
        } else if (hasCommandGroup()) {
            caption = ("Cannot Redo in the middle of " + myCommandGroups.top()->getDescription()).c_str();
        } else if (!canRedo()) {
            caption = "Redo";
        }
        menuCommand->handle(this, FXSEL(SEL_COMMAND, FXMenuCaption::ID_SETSTRINGVALUE), (void*)&caption);
        menuCommand->update();
    }
    return 1;
}


bool
GNEUndoList::hasCommandGroup() const {
    return myCommandGroups.size() != 0;
}


GNEUndoList::CommandGroup::CommandGroup(std::string description) :
    myDescription(description) {
}


const std::string&
GNEUndoList::CommandGroup::getDescription() {
    return myDescription;
}


FXString
GNEUndoList::CommandGroup::undoName() const {
    return ("Undo " + myDescription).c_str();
}


FXString
GNEUndoList::CommandGroup::redoName() const {
    return ("Redo " + myDescription).c_str();
}
