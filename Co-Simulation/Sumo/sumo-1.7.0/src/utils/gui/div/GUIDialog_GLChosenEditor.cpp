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
/// @file    GUIDialog_GLChosenEditor.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 11.03.2004
///
// Editor for the list of chosen objects
/****************************************************************************/
#include <config.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/foxtools/MFXUtils.h>
#include "GUIDialog_GLChosenEditor.h"
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/div/GUIIOGlobals.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIMainWindow.h>
#include <utils/gui/images/GUIIconSubSys.h>


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUIDialog_GLChosenEditor) GUIDialog_GLChosenEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_LOAD,       GUIDialog_GLChosenEditor::onCmdLoad),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_SAVE,       GUIDialog_GLChosenEditor::onCmdSave),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_DESELECT,   GUIDialog_GLChosenEditor::onCmdDeselect),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_CLEAR,      GUIDialog_GLChosenEditor::onCmdClear),
    FXMAPFUNC(SEL_COMMAND,  MID_CANCEL,             GUIDialog_GLChosenEditor::onCmdClose),
};

FXIMPLEMENT(GUIDialog_GLChosenEditor, FXMainWindow, GUIDialog_GLChosenEditorMap, ARRAYNUMBER(GUIDialog_GLChosenEditorMap))


// ===========================================================================
// method definitions
// ===========================================================================

GUIDialog_GLChosenEditor::GUIDialog_GLChosenEditor(GUIMainWindow* parent, GUISelectedStorage* str) :
    FXMainWindow(parent->getApp(), "List of Selected Items", GUIIconSubSys::getIcon(GUIIcon::APP_SELECTOR), nullptr, GUIDesignChooserDialog),
    myParent(parent), myStorage(str) {
    myStorage->add2Update(this);
    FXHorizontalFrame* hbox = new FXHorizontalFrame(this, GUIDesignAuxiliarFrame);
    // create layout left
    FXVerticalFrame* layoutLeft = new FXVerticalFrame(hbox, GUIDesignChooserLayoutLeft);
    // create frame for list
    FXVerticalFrame* layoutList = new FXVerticalFrame(layoutLeft, GUIDesignChooserLayoutList);
    // build the list and rebuild it
    myList = new FXList(layoutList, this, MID_CHOOSER_LIST, GUIDesignChooserListMultiple);
    rebuildList();
    // build the layout
    FXVerticalFrame* layout = new FXVerticalFrame(hbox, GUIDesignChooserLayoutRight);
    // "Load"
    new FXButton(layout, "&Load selection\t\t", GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG), this, MID_CHOOSEN_LOAD, GUIDesignChooserButtons);
    // "Save"
    new FXButton(layout, "&Save selection\t\t", GUIIconSubSys::getIcon(GUIIcon::SAVE), this, MID_CHOOSEN_SAVE, GUIDesignChooserButtons);
    // extra separator
    new FXHorizontalSeparator(layout, GUIDesignHorizontalSeparator);
    // "Deselect Chosen"
    new FXButton(layout, "&Deselect chosen\t\t", GUIIconSubSys::getIcon(GUIIcon::FLAG), this, MID_CHOOSEN_DESELECT, GUIDesignChooserButtons);
    // "Clear List"
    new FXButton(layout, "&Clear selection\t\t", GUIIconSubSys::getIcon(GUIIcon::FLAG), this, MID_CHOOSEN_CLEAR, GUIDesignChooserButtons);
    // extra separator
    new FXHorizontalSeparator(layout, GUIDesignHorizontalSeparator);
    // "Close"
    new FXButton(layout, "Cl&ose\t\t", GUIIconSubSys::getIcon(GUIIcon::NO), this, MID_CANCEL, GUIDesignChooserButtons);
    myParent->addChild(this);
}


GUIDialog_GLChosenEditor::~GUIDialog_GLChosenEditor() {
    myStorage->remove2Update();
    myParent->removeChild(this);
}


void
GUIDialog_GLChosenEditor::rebuildList() {
    myList->clearItems();
    const std::set<GUIGlID>& chosen = gSelected.getSelected();
    for (auto i : chosen) {
        GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(i);
        if (object != nullptr) {
            std::string name = object->getFullName();
            FXListItem* item = myList->getItem(myList->appendItem(name.c_str()));
            item->setData(object);
            GUIGlObjectStorage::gIDStorage.unblockObject(i);
        }
    }
}


void
GUIDialog_GLChosenEditor::selectionUpdated() {
    rebuildList();
    FXMainWindow::update();
}


long
GUIDialog_GLChosenEditor::onCmdLoad(FXObject*, FXSelector, void*) {
    // get the new file name
    FXFileDialog opendialog(this, "Open List of Selected Items");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("*.txt\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        std::string msg = gSelected.load(file);
        if (msg != "") {
            FXMessageBox::error(this, MBOX_OK, "Errors while loading Selection", "%s", msg.c_str());
        }
        rebuildList();
    }
    return 1;
}


long
GUIDialog_GLChosenEditor::onCmdSave(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this, "Save List of selected Items", ".txt", GUIIconSubSys::getIcon(GUIIcon::EMPTY), gCurrentFolder);
    if (file == "") {
        return 1;
    }
    try {
        gSelected.save(file.text());
    } catch (IOError& e) {
        FXMessageBox::error(this, MBOX_OK, "Storing failed!", "%s", e.what());
    }
    return 1;
}


long
GUIDialog_GLChosenEditor::onCmdDeselect(FXObject*, FXSelector, void*) {
    FXint no = myList->getNumItems();
    FXint i;
    std::vector<GUIGlID> selected;
    for (i = 0; i < no; ++i) {
        if (myList->getItem(i)->isSelected()) {
            selected.push_back(static_cast<GUIGlObject*>(myList->getItem(i)->getData())->getGlID());
        }
    }
    // remove items from list
    for (i = 0; i < (FXint) selected.size(); ++i) {
        gSelected.deselect(selected[i]);
    }
    // rebuild list
    rebuildList();
    myParent->updateChildren();
    return 1;
}


long
GUIDialog_GLChosenEditor::onCmdClear(FXObject*, FXSelector, void*) {
    myList->clearItems();
    gSelected.clear();
    myParent->updateChildren();
    return 1;
}


long
GUIDialog_GLChosenEditor::onCmdClose(FXObject*, FXSelector, void*) {
    close(true);
    return 1;
}


/****************************************************************************/
