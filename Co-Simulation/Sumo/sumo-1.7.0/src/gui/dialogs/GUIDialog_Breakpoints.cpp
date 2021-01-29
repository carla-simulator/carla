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
/// @file    GUIDialog_Breakpoints.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 17 Jun 2004
///
// Editor for simulation breakpoints
/****************************************************************************/
#include <config.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <set>
#include <gui/GUIApplicationWindow.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/foxtools/MFXUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/settings/GUISettingsHandler.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/gui/div/GUIIOGlobals.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/iodevices/OutputDevice.h>
#include "GUIDialog_Breakpoints.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GUIDialog_Breakpoints) GUIDialog_BreakpointsMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_LOAD,  GUIDialog_Breakpoints::onCmdLoad),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_SAVE,  GUIDialog_Breakpoints::onCmdSave),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_CLEAR, GUIDialog_Breakpoints::onCmdClear),
    FXMAPFUNC(SEL_COMMAND,  MID_CANCEL,        GUIDialog_Breakpoints::onCmdClose),
    FXMAPFUNC(SEL_REPLACED, MID_TABLE,         GUIDialog_Breakpoints::onCmdEditTable),
};


FXIMPLEMENT(GUIDialog_Breakpoints, FXMainWindow, GUIDialog_BreakpointsMap, ARRAYNUMBER(GUIDialog_BreakpointsMap))

// ===========================================================================
// method definitions
// ===========================================================================

GUIDialog_Breakpoints::GUIDialog_Breakpoints(GUIMainWindow* parent, std::vector<SUMOTime>& breakpoints, FXMutex& breakpointLock) :
    FXMainWindow(parent->getApp(), "Breakpoints Editor", GUIIconSubSys::getIcon(GUIIcon::APP_BREAKPOINTS), nullptr, GUIDesignChooserDialog),
    myParent(parent), myBreakpoints(&breakpoints), myBreakpointLock(&breakpointLock) {
    // build main Frame
    FXHorizontalFrame* hbox = new FXHorizontalFrame(this, GUIDesignAuxiliarFrame);
    // build the table
    FXVerticalFrame* layoutLeft = new FXVerticalFrame(hbox, GUIDesignChooserLayoutLeft);
    myTable = new FXTable(layoutLeft, this, MID_TABLE, GUIDesignBreakpointTable);
    myTable->setVisibleRows(20);
    myTable->setVisibleColumns(1);
    myTable->setTableSize(20, 1);
    myTable->setBackColor(FXRGB(255, 255, 255));
    myTable->getRowHeader()->setWidth(0);
    myBreakpointLock->lock();
    rebuildList();
    myBreakpointLock->unlock();
    // build the layout
    FXVerticalFrame* layoutRight = new FXVerticalFrame(hbox, GUIDesignChooserLayoutRight);
    // create buttons ('&' in the label creates a hot key)
    // "Load"
    new FXButton(layoutRight, "&Load\t\t", GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG), this, MID_CHOOSEN_LOAD, GUIDesignChooserButtons);
    // "Save"
    new FXButton(layoutRight, "&Save\t\t", GUIIconSubSys::getIcon(GUIIcon::SAVE), this, MID_CHOOSEN_SAVE, GUIDesignChooserButtons);
    new FXHorizontalSeparator(layoutRight, GUIDesignHorizontalSeparator);
    // "Clear List"
    new FXButton(layoutRight, "Clea&r\t\t", GUIIconSubSys::getIcon(GUIIcon::CLEANJUNCTIONS), this, MID_CHOOSEN_CLEAR, GUIDesignChooserButtons);
    new FXHorizontalSeparator(layoutRight, GUIDesignHorizontalSeparator);
    // "Close"
    new FXButton(layoutRight, "&Close\t\t", GUIIconSubSys::getIcon(GUIIcon::NO), this, MID_CANCEL, GUIDesignChooserButtons);
    // add this dialog as child of GUIMainWindow parent
    myParent->addChild(this);
}


GUIDialog_Breakpoints::~GUIDialog_Breakpoints() {
    // remove this dialog as child of GUIMainWindow parent
    myParent->removeChild(this);
}


void
GUIDialog_Breakpoints::show() {
    FXMainWindow::show();
    myTable->startInput((int)myBreakpoints->size(), 0);
}


void
GUIDialog_Breakpoints::rebuildList() {
    myTable->clearItems();
    sort(myBreakpoints->begin(), myBreakpoints->end());
    // set table attributes
    myTable->setTableSize((FXint)myBreakpoints->size() + 1, 1);
    myTable->setColumnText(0, "Time");
    FXHeader* header = myTable->getColumnHeader();
    header->setHeight(GUIDesignBreakpointTableHeaderHeight);
    header->setItemJustify(0, JUSTIFY_CENTER_X);
    // insert into table
    for (int row = 0; row < (int)myBreakpoints->size(); row++) {
        myTable->setItemText(row, 0, time2string((*myBreakpoints)[row]).c_str());
    }
    // insert dummy last field
    myTable->setItemText((int)myBreakpoints->size(), 0, " ");
}


long
GUIDialog_Breakpoints::onCmdLoad(FXObject*, FXSelector, void*) {
    FXFileDialog opendialog(this, "Load Breakpoints");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_ANY);
    opendialog.setPatternList("*.txt");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        std::vector<SUMOTime> newBreakpoints = GUISettingsHandler::loadBreakpoints(file);
        FXMutexLock lock(*myBreakpointLock);
        myBreakpoints->assign(newBreakpoints.begin(), newBreakpoints.end());
        rebuildList();
    }
    return 1;
}


long
GUIDialog_Breakpoints::onCmdSave(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this, "Save Breakpoints", ".txt", GUIIconSubSys::getIcon(GUIIcon::EMPTY), gCurrentFolder);
    if (file == "") {
        return 1;
    }
    std::string content = encode2TXT();
    try {
        OutputDevice& dev = OutputDevice::getDevice(file.text());
        dev << content;
        dev.close();
    } catch (IOError& e) {
        FXMessageBox::error(this, MBOX_OK, "Storing failed!", "%s", e.what());
    }
    return 1;
}


std::string
GUIDialog_Breakpoints::encode2TXT() {
    FXMutexLock lock(*myBreakpointLock);
    std::ostringstream strm;
    std::sort(myBreakpoints->begin(), myBreakpoints->end());
    for (std::vector<SUMOTime>::iterator j = myBreakpoints->begin(); j != myBreakpoints->end(); ++j) {
        strm << time2string(*j) << std::endl;
    }
    return strm.str();
}


long
GUIDialog_Breakpoints::onCmdClear(FXObject*, FXSelector, void*) {
    FXMutexLock lock(*myBreakpointLock);
    myBreakpoints->clear();
    rebuildList();
    return 1;
}



long
GUIDialog_Breakpoints::onCmdClose(FXObject*, FXSelector, void*) {
    close(true);
    return 1;
}


long
GUIDialog_Breakpoints::onCmdEditTable(FXObject*, FXSelector, void* ptr) {
    FXMutexLock lock(*myBreakpointLock);
    const FXTablePos* const i = (FXTablePos*) ptr;
    const std::string value = StringUtils::prune(myTable->getItemText(i->row, i->col).text());
    // check whether the inserted value is empty
    const bool empty = value.find_first_not_of(" ") == std::string::npos;
    try {
        if (i->row == (int)myBreakpoints->size()) {
            if (!empty) {
                myBreakpoints->push_back(string2time(value));
            }
        } else {
            if (empty) {
                myBreakpoints->erase(myBreakpoints->begin() + i->row);
            } else {
                (*myBreakpoints)[i->row] = string2time(value);
            }
        }
    } catch (NumberFormatException&) {
        std::string msg = "The value must be a number, is:" + value;
        FXMessageBox::error(this, MBOX_OK, "Time format error", "%s", msg.c_str());
    } catch (ProcessError&) {
        std::string msg = "The value must be a number or a string of the form hh:mm:ss, is:" + value;
        FXMessageBox::error(this, MBOX_OK, "Time format error", "%s", msg.c_str());
    }
    rebuildList();
    return 1;
}


void
GUIDialog_Breakpoints::layout() {
    FXMainWindow::layout();
    myTable->setColumnWidth(0, myTable->getWidth() - 1);
}


/****************************************************************************/
