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
/// @file    GUIMainWindow.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 29.05.2005
///
//
/****************************************************************************/
#include <config.h>

#include <string>
#include <algorithm>
#include <fx.h>
// fx3d includes windows.h so we need to guard against macro pollution
#ifdef WIN32
#define NOMINMAX
#endif
#include <fx3d.h>
#ifdef WIN32
#undef NOMINMAX
#endif
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/options/OptionsCont.h>
#include "GUIAppEnum.h"
#include "GUIMainWindow.h"
#include "GUIGlChildWindow.h"


// ===========================================================================
// static member definitions
// ===========================================================================
GUIMainWindow* GUIMainWindow::myInstance = nullptr;

// ===========================================================================
// member method definitions
// ===========================================================================
GUIMainWindow::GUIMainWindow(FXApp* a) :
    FXMainWindow(a, "sumo-gui main window", nullptr, nullptr, DECOR_ALL, 20, 20, 600, 400),
    myAmFullScreen(false),
    myTrackerLock(true),
    myGLVisual(new FXGLVisual(a, VISUAL_DOUBLEBUFFER)),
    myAmGaming(false),
    myListInternal(false),
    myListParking(true),
    myListTeleporting(false) {

    FXFontDesc fdesc;
    getApp()->getNormalFont()->getFontDesc(fdesc);
    fdesc.weight = FXFont::Bold;
    myBoldFont = new FXFont(getApp(), fdesc);

    myTopDock = new FXDockSite(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    myBottomDock = new FXDockSite(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);
    myLeftDock = new FXDockSite(this, LAYOUT_SIDE_LEFT | LAYOUT_FILL_Y);
    myRightDock = new FXDockSite(this, LAYOUT_SIDE_RIGHT | LAYOUT_FILL_Y);
    if (myInstance != nullptr) {
        throw ProcessError("MainWindow initialized twice");
    }
    myInstance = this;
    //myGLVisual->setStencilSize(8); // enable stencil buffer
}


GUIMainWindow::~GUIMainWindow() {
    delete myBoldFont;
    delete myTopDock;
    delete myBottomDock;
    delete myLeftDock;
    delete myRightDock;
}



void
GUIMainWindow::addGLChild(GUIGlChildWindow* child) {
    myGLWindows.push_back(child);
}


void
GUIMainWindow::removeGLChild(GUIGlChildWindow* child) {
    std::vector<GUIGlChildWindow*>::iterator i = std::find(myGLWindows.begin(), myGLWindows.end(), child);
    if (i != myGLWindows.end()) {
        myGLWindows.erase(i);
    }
}


void
GUIMainWindow::addChild(FXMainWindow* child) {
    myTrackerLock.lock();
    myTrackerWindows.push_back(child);
    myTrackerLock.unlock();
}


void
GUIMainWindow::removeChild(FXMainWindow* child) {
    myTrackerLock.lock();
    std::vector<FXMainWindow*>::iterator i = std::find(myTrackerWindows.begin(), myTrackerWindows.end(), child);
    myTrackerWindows.erase(i);
    myTrackerLock.unlock();
}


std::vector<std::string>
GUIMainWindow::getViewIDs() const {
    std::vector<std::string> ret;
    for (GUIGlChildWindow* const window : myGLWindows) {
        ret.push_back(window->getTitle().text());
    }
    return ret;
}


GUIGlChildWindow*
GUIMainWindow::getViewByID(const std::string& id) const {
    for (GUIGlChildWindow* const window : myGLWindows) {
        if (std::string(window->getTitle().text()) == id) {
            return window;
        }
    }
    return nullptr;
}


FXFont*
GUIMainWindow::getBoldFont() {
    return myBoldFont;
}


void
GUIMainWindow::updateChildren() {
    // inform views
    myMDIClient->forallWindows(this, FXSEL(SEL_COMMAND, MID_SIMSTEP), nullptr);
    // inform other windows
    myTrackerLock.lock();
    for (int i = 0; i < (int)myTrackerWindows.size(); i++) {
        myTrackerWindows[i]->handle(this, FXSEL(SEL_COMMAND, MID_SIMSTEP), nullptr);
    }
    myTrackerLock.unlock();
}


FXGLVisual*
GUIMainWindow::getGLVisual() const {
    return myGLVisual;
}


FXLabel&
GUIMainWindow::getCartesianLabel() {
    return *myCartesianCoordinate;
}


FXLabel&
GUIMainWindow::getGeoLabel() {
    return *myGeoCoordinate;
}


GUIMainWindow*
GUIMainWindow::getInstance() {
    if (myInstance != nullptr) {
        return myInstance;
    }
    throw ProcessError("A GUIMainWindow instance was not yet constructed.");
}


GUISUMOAbstractView*
GUIMainWindow::getActiveView() const {
    GUIGlChildWindow* w = dynamic_cast<GUIGlChildWindow*>(myMDIClient->getActiveChild());
    if (w != nullptr) {
        return w->getView();
    }
    return nullptr;
}


void
GUIMainWindow::setWindowSizeAndPos() {
    int windowWidth = getApp()->reg().readIntEntry("SETTINGS", "width", 600);
    int windowHeight = getApp()->reg().readIntEntry("SETTINGS", "height", 400);
    const OptionsCont& oc = OptionsCont::getOptions();
    if (oc.isSet("window-size")) {
        std::vector<std::string> windowSize = oc.getStringVector("window-size");
        if (windowSize.size() != 2) {
            WRITE_ERROR("option window-size requires INT,INT");
        } else {
            try {
                windowWidth = StringUtils::toInt(windowSize[0]);
                windowHeight = StringUtils::toInt(windowSize[1]);
            } catch (NumberFormatException& e) {
                WRITE_ERROR("option window-size requires INT,INT " + toString(e.what()));
            }
        }
    }
    if (oc.isSet("window-size") || getApp()->reg().readIntEntry("SETTINGS", "maximized", 0) == 0 || oc.isSet("window-pos")) {
        // when restoring previous pos, make sure the window fits fully onto the current screen
        int x = MAX2(0, MIN2(getApp()->reg().readIntEntry("SETTINGS", "x", 150), getApp()->getRootWindow()->getWidth() - windowWidth));
        int y = MAX2(50, MIN2(getApp()->reg().readIntEntry("SETTINGS", "y", 150), getApp()->getRootWindow()->getHeight() - windowHeight));
        if (oc.isSet("window-pos")) {
            std::vector<std::string> windowPos = oc.getStringVector("window-pos");
            if (windowPos.size() != 2) {
                WRITE_ERROR("option window-pos requires INT,INT");
            } else {
                try {
                    x = StringUtils::toInt(windowPos[0]);
                    y = StringUtils::toInt(windowPos[1]);
                } catch (NumberFormatException& e) {
                    WRITE_ERROR("option window-pos requires INT,INT " + toString(e.what()));
                }
            }
        }
        move(x, y);
        resize(windowWidth, windowHeight);
    }
}

void
GUIMainWindow::storeWindowSizeAndPos() {
    if (!myAmFullScreen) {
        getApp()->reg().writeIntEntry("SETTINGS", "x", getX());
        getApp()->reg().writeIntEntry("SETTINGS", "y", getY());
        getApp()->reg().writeIntEntry("SETTINGS", "width", getWidth());
        getApp()->reg().writeIntEntry("SETTINGS", "height", getHeight());
    }
}


/****************************************************************************/
