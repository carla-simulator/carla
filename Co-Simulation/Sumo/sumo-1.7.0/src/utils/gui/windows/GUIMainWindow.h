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
/// @file    GUIMainWindow.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Fri, 29.04.2005
///
//
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <vector>
#include <string>
#include <map>
#include <utils/common/StdDefs.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUIEvent;
class GUIGlChildWindow;
class GUISUMOAbstractView;


// ===========================================================================
// class definitions
// ===========================================================================
class GUIMainWindow : public FXMainWindow {
public:
    GUIMainWindow(FXApp* a);
    virtual ~GUIMainWindow();
    /// Adds a further child window to the list
    void addGLChild(GUIGlChildWindow* child);
    void addChild(FXMainWindow* child);

    /// removes the given child window from the list
    void removeGLChild(GUIGlChildWindow* child);
    void removeChild(FXMainWindow*  child);

    std::vector<std::string> getViewIDs() const;
    GUIGlChildWindow* getViewByID(const std::string& id) const;
    const std::vector<GUIGlChildWindow*>& getViews() const {
        return myGLWindows;
    }

    void updateChildren();

    FXFont* getBoldFont();

    FXGLVisual* getGLVisual() const;

    virtual FXGLCanvas* getBuildGLCanvas() const = 0;

    virtual SUMOTime getCurrentSimTime() const = 0;

    virtual double getTrackerInterval() const = 0;

    virtual void setStatusBarText(const std::string&) { }

    FXLabel& getCartesianLabel();
    FXLabel& getGeoLabel();

    /// @brief return whether the gui is in gaming mode
    bool isGaming() const {
        return myAmGaming;
    }

    /// @brief return whether to list internal structures
    bool listInternal() const {
        return myListInternal;
    }

    /// @brief return whether to list parking vehicles
    bool listParking() const {
        return myListParking;
    }

    /// @brief return whether to list teleporting vehicles
    bool listTeleporting() const {
        return myListTeleporting;
    }

    static GUIMainWindow* getInstance();

    /** @brief Returns the delay (should be overwritten by subclasses if applicable)
     * @return parsed delay in milliseconds
     */
    virtual double getDelay() const {
        return 0.;
    }

    /** @brief Sets the delay of the parent application
     */
    virtual void setDelay(double) {}

    /** @brief Sets the breakpoints of the parent application
     */
    virtual void setBreakpoints(const std::vector<SUMOTime>&) {}

    /** @brief Sends an event from the application thread to the GUI and waits until it is handled
     * @param event the event to send
     */
    virtual void sendBlockingEvent(GUIEvent* event) {
        UNUSED_PARAMETER(event);
    }

    /** @brief get the active view or 0 */
    GUISUMOAbstractView* getActiveView() const;

    /// @brief Toggle full screen mode
    virtual long onCmdFullScreen(FXObject*, FXSelector, void*) {
        return 1;
    }

    bool isFullScreen() {
        return myAmFullScreen;
    }

    /// @brief retrieve breakpoints if provided by the application
    virtual const std::vector<SUMOTime> retrieveBreakpoints() const {
        return std::vector<SUMOTime>();
    }

protected:
    /// @brief whether to show the window in full screen mode
    bool myAmFullScreen;

    std::vector<GUIGlChildWindow*> myGLWindows;
    std::vector<FXMainWindow*> myTrackerWindows;
    /// A lock to make the removal and addition of trackers secure
    FXMutex myTrackerLock;

    /// Font used for popup-menu titles
    FXFont* myBoldFont;

    /// The multi view panel
    FXMDIClient* myMDIClient;

    /// The status bar
    FXStatusBar* myStatusbar;

    /// Labels for the current cartesian and geo-coordinate
    FXLabel* myCartesianCoordinate, *myGeoCoordinate;
    FXHorizontalFrame* myCartesianFrame, *myGeoFrame;

    /// The gl-visual used
    FXGLVisual* myGLVisual;

    FXDockSite* myTopDock, *myBottomDock, *myLeftDock, *myRightDock;

    /// information whether the gui is currently in gaming mode
    bool myAmGaming;

    /// information whether the locator should list internal structures
    bool myListInternal;

    /// information whether the locator should list parking vehicles
    bool myListParking;

    /// information whether the locator should list teleporting vehicles
    bool myListTeleporting;

    /// the singleton window instance
    static GUIMainWindow* myInstance;

protected:
    FOX_CONSTRUCTOR(GUIMainWindow)

    /// @brief perform initial window positioning and sizing according to user options / previous call
    void setWindowSizeAndPos();

    /// @brief record window position and size in registry
    void storeWindowSizeAndPos();

};
