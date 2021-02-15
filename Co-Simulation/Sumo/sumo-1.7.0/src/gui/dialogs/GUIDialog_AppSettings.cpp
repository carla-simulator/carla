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
/// @file    GUIDialog_AppSettings.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 08.03.2004
///
// The application-settings dialog
/****************************************************************************/
#include <config.h>

#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIMessageWindow.h>
#include <gui/GUIGlobals.h>
#include "GUIDialog_AppSettings.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUIDialog_AppSettings) GUIDialog_AppSettingsMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_QUITONSIMEND,    GUIDialog_AppSettings::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, MID_AUTOSTART,       GUIDialog_AppSettings::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, MID_DEMO,            GUIDialog_AppSettings::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, MID_ALLOWTEXTURES,   GUIDialog_AppSettings::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, MID_LOCATELINKS,     GUIDialog_AppSettings::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, MID_SETTINGS_OK,     GUIDialog_AppSettings::onCmdOk),
    FXMAPFUNC(SEL_COMMAND, MID_SETTINGS_CANCEL, GUIDialog_AppSettings::onCmdCancel),
};

FXIMPLEMENT(GUIDialog_AppSettings, FXDialogBox, GUIDialog_AppSettingsMap, ARRAYNUMBER(GUIDialog_AppSettingsMap))


// ===========================================================================
// method definitions
// ===========================================================================
GUIDialog_AppSettings::GUIDialog_AppSettings(FXMainWindow* parent)
    : FXDialogBox(parent, "Application Settings"),
      myAppQuitOnEnd(GUIGlobals::gQuitOnEnd),
      myAppAutoStart(GUIGlobals::gRunAfterLoad),
      myAppDemo(GUIGlobals::gDemoAutoReload),
      myAllowTextures(GUITexturesHelper::texturesAllowed()),
      myLocateLinks(GUIMessageWindow::locateLinksEnabled()) {
    FXCheckButton* b = nullptr;
    FXVerticalFrame* f1 = new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0);
    b = new FXCheckButton(f1, "Quit on Simulation End", this, MID_QUITONSIMEND);
    b->setCheck(myAppQuitOnEnd);
    b = new FXCheckButton(f1, "Autostart Simulation on Load and Reload", this, MID_AUTOSTART);
    b->setCheck(myAppAutoStart);
    b = new FXCheckButton(f1, "Reload Simulation after finish (Demo mode)", this, MID_DEMO);
    b->setCheck(myAppDemo);
    b = new FXCheckButton(f1, "Locate elements when clicking on messages", this, MID_LOCATELINKS);
    b->setCheck(myLocateLinks);
    new FXHorizontalSeparator(f1, SEPARATOR_GROOVE | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_X);
    b = new FXCheckButton(f1, "Allow Textures", this, MID_ALLOWTEXTURES);
    b->setCheck(myAllowTextures);
    FXHorizontalFrame* f2 = new FXHorizontalFrame(f1, LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | PACK_UNIFORM_WIDTH, 0, 0, 0, 0, 10, 10, 5, 5);
    FXButton* initial = new FXButton(f2, "&OK", nullptr, this, MID_SETTINGS_OK, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, 30, 30, 4, 4);
    new FXButton(f2, "&Cancel", nullptr, this, MID_SETTINGS_CANCEL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, 30, 30, 4, 4);
    initial->setFocus();
    setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
}


GUIDialog_AppSettings::~GUIDialog_AppSettings() {}


long
GUIDialog_AppSettings::onCmdOk(FXObject*, FXSelector, void*) {
    GUIGlobals::gQuitOnEnd = myAppQuitOnEnd;
    GUIGlobals::gRunAfterLoad = myAppAutoStart;
    GUIGlobals::gDemoAutoReload = myAppDemo;
    GUIGlobals::gRunAfterLoad = myAppAutoStart;
    GUITexturesHelper::allowTextures(myAllowTextures);
    GUIMessageWindow::enableLocateLinks(myLocateLinks);
    destroy();
    return 1;
}


long
GUIDialog_AppSettings::onCmdCancel(FXObject*, FXSelector, void*) {
    destroy();
    return 1;
}


long
GUIDialog_AppSettings::onCmdSelect(FXObject*, FXSelector sel, void*) {
    switch (FXSELID(sel)) {
        case MID_QUITONSIMEND:
            myAppQuitOnEnd = !myAppQuitOnEnd;
            break;
        case MID_AUTOSTART:
            myAppAutoStart = !myAppAutoStart;
            break;
        case MID_DEMO:
            myAppDemo = !myAppDemo;
            break;
        case MID_LOCATELINKS:
            myLocateLinks = !myLocateLinks;
            break;
        case MID_ALLOWTEXTURES:
            myAllowTextures = !myAllowTextures;
            break;
    }
    return 1;
}


/****************************************************************************/
