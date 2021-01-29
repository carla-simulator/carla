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
/// @file    GUIGlChildWindow.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 29.05.2005
///
//
/****************************************************************************/
#include <config.h>

#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIMainWindow.h>

#include "GUIGlChildWindow.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUIGlChildWindow) GUIGlChildWindowMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_RECENTERVIEW,                   GUIGlChildWindow::onCmdRecenterView),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_I_EDITVIEWPORT,     GUIGlChildWindow::onCmdEditViewport),
    FXMAPFUNC(SEL_COMMAND,  MID_SHOWTOOLTIPS,                   GUIGlChildWindow::onCmdShowToolTips),
    FXMAPFUNC(SEL_COMMAND,  MID_ZOOM_STYLE,                     GUIGlChildWindow::onCmdZoomStyle),
    FXMAPFUNC(SEL_COMMAND,  MID_COLOURSCHEMECHANGE,             GUIGlChildWindow::onCmdChangeColorScheme),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F9_EDIT_VIEWSCHEME,      GUIGlChildWindow::onCmdEditViewScheme),
};

FXIMPLEMENT(GUIGlChildWindow, FXMDIChild, GUIGlChildWindowMap, ARRAYNUMBER(GUIGlChildWindowMap))


// ===========================================================================
// member method definitions
// ===========================================================================
GUIGlChildWindow::GUIGlChildWindow(FXMDIClient* p, GUIMainWindow* parentWindow, FXMDIMenu* mdimenu,
                                   const FXString& name, FXMenuBar* gripNavigationToolbar, FXIcon* ic, FXuint opts, FXint x, FXint y, FXint w, FXint h) :
    FXMDIChild(p, name, ic, mdimenu, opts, x, y, w, h),
    myParent(parentWindow),
    myGripNavigationToolbar(gripNavigationToolbar),
    myStaticNavigationToolBar(nullptr),
    myView(nullptr) {
    // Make MDI Window Menu
    setTracking();
    // create a vertical frame to add elements
    myContentFrame = new FXVerticalFrame(this, GUIDesignFrameArea);
    // if menuBarGripElements isn't NULL, use it to create a grip navigation elements. In other cas, create a static FXToolbar
    if (myGripNavigationToolbar == nullptr) {
        myStaticNavigationToolBar = new FXToolBar(myContentFrame, GUIDesignToolbarMenuBar);
    }
    // build the tool bars
    buildNavigationToolBar(); // always there (recenter)
    buildColoringToolBar(); // always there (coloring)
    buildScreenshotToolBar(); // always there (screenshot)
}


GUIGlChildWindow::~GUIGlChildWindow() {
    delete myLocatorPopup;
    // only delete static navigation bar if it was created
    if (myStaticNavigationToolBar) {
        delete myStaticNavigationToolBar;
    }
}


void
GUIGlChildWindow::create() {
    FXMDIChild::create();
    // only create static navigation bar if it was created
    if (myStaticNavigationToolBar) {
        myStaticNavigationToolBar->create();
    } else {
        myGripNavigationToolbar->create();
    }
    myLocatorPopup->create();
    myView->create();
}


GUISUMOAbstractView*
GUIGlChildWindow::getView() const {
    return myView;
}


GUIMainWindow*
GUIGlChildWindow::getParent() {
    return myParent;
}


void
GUIGlChildWindow::buildNavigationToolBar() {
    // build the view settings
    // recenter view
    new FXButton(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
                 "\tRecenter View\tRecenter view to the simulated area.",
                 GUIIconSubSys::getIcon(GUIIcon::RECENTERVIEW), this, MID_RECENTERVIEW, GUIDesignButtonToolbar);
    // add viewport button
    new FXButton(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
                 "\tEdit ViewporttOpens a menu which lets you edit the viewport.",
                 GUIIconSubSys::getIcon(GUIIcon::EDITVIEWPORT), this, MID_HOTKEY_CTRL_I_EDITVIEWPORT, GUIDesignButtonToolbar);
    // toggle button for zooming style
    MFXCheckableButton* zoomBut = new MFXCheckableButton(false, myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
            "\tToggles Zooming Style\tToggles whether zooming is based at cursor position or at the center of the view.",
            GUIIconSubSys::getIcon(GUIIcon::ZOOMSTYLE), this, MID_ZOOM_STYLE, GUIDesignButtonToolbarCheckable);
    zoomBut->setChecked(getApp()->reg().readIntEntry("gui", "zoomAtCenter", 0) != 1);
    // build the locator popup
    myLocatorPopup = new FXPopup(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar, POPUP_VERTICAL);
    // build locator button
    myLocatorButton = new FXMenuButton(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
                                       "\tLocate Structures\tLocate structures within the network.",
                                       GUIIconSubSys::getIcon(GUIIcon::LOCATE), myLocatorPopup, GUIDesignButtonToolbarLocator);
    // add toggle button for tool-tips on/off
    new MFXCheckableButton(false, myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
                           "\tToggles Tool Tips\tToggles whether tool tips shall be shown.",
                           GUIIconSubSys::getIcon(GUIIcon::SHOWTOOLTIPS), this, MID_SHOWTOOLTIPS, GUIDesignButtonToolbarCheckable);
}


void
GUIGlChildWindow::buildColoringToolBar() {
    // Create Vertical separator
    new FXVerticalSeparator(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar, GUIDesignVerticalSeparator);
    // build coloring tools
    myColoringSchemes = new FXComboBox(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
                                       GUIDesignComboBoxNCol, this, MID_COLOURSCHEMECHANGE, GUIDesignComboBoxStatic);
    // editor
    new FXButton(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
                 "\tEdit Coloring Schemes\tOpens a menu which lets you edit the coloring schemes.",
                 GUIIconSubSys::getIcon(GUIIcon::COLORWHEEL), this, MID_HOTKEY_F9_EDIT_VIEWSCHEME, GUIDesignButtonToolbar);
}


void
GUIGlChildWindow::buildScreenshotToolBar() {
    // Create Vertical separator
    new FXVerticalSeparator(myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar, GUIDesignVerticalSeparator);
    // snapshot
    new MFXCheckableButton(false, myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar,
                           "\tMake Snapshot\tMakes a snapshot of the view.",
                           GUIIconSubSys::getIcon(GUIIcon::CAMERA), this, MID_MAKESNAPSHOT, GUIDesignButtonToolbar);
}


FXGLCanvas*
GUIGlChildWindow::getBuildGLCanvas() const {
    return myView;
}


FXToolBar*
GUIGlChildWindow::getNavigationToolBar(GUISUMOAbstractView&) {
    return myGripNavigationToolbar ? myGripNavigationToolbar : myStaticNavigationToolBar;
}


FXPopup*
GUIGlChildWindow::getLocatorPopup() {
    return myLocatorPopup;
}


FXComboBox*
GUIGlChildWindow::getColoringSchemesCombo() {
    return myColoringSchemes;
}


long
GUIGlChildWindow::onCmdRecenterView(FXObject*, FXSelector, void*) {
    myView->recenterView();
    myView->update();
    return 1;
}


long
GUIGlChildWindow::onCmdEditViewport(FXObject*, FXSelector, void*) {
    myView->showViewportEditor();
    return 1;
}


long
GUIGlChildWindow::onCmdEditViewScheme(FXObject*, FXSelector, void*) {
    myView->showViewschemeEditor();
    return 1;
}


long
GUIGlChildWindow::onCmdShowToolTips(FXObject* sender, FXSelector, void*) {
    MFXCheckableButton* button = dynamic_cast<MFXCheckableButton*>(sender);
    // check if button was sucesfully casted
    if (button) {
        button->setChecked(!button->amChecked());
        myView->showToolTips(button->amChecked());
        update();
        myView->update();
    }
    return 1;
}


long
GUIGlChildWindow::onCmdZoomStyle(FXObject* sender, FXSelector, void*) {
    MFXCheckableButton* button = dynamic_cast<MFXCheckableButton*>(sender);
    if (button) {
        button->setChecked(!button->amChecked());
        getApp()->reg().writeIntEntry("gui", "zoomAtCenter",
                                      button->amChecked() ? 0 : 1);
        update();
        myView->update();
    }
    return 1;
}


long
GUIGlChildWindow::onCmdChangeColorScheme(FXObject*, FXSelector, void* data) {
    myView->setColorScheme((char*) data);
    return 1;
}


void
GUIGlChildWindow::setView(GUIGlID id) {
    // this is used by the locator widget. zooming to bounding box
    myView->centerTo(id, true, -1);
    myView->update();
}


bool
GUIGlChildWindow::isSelected(GUIGlObject* o) const {
    return gSelected.isSelected(o->getType(), o->getGlID());
}


/****************************************************************************/
