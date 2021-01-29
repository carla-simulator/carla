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
/// @file    GUIApplicationWindow.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Andreas Gaubatz
/// @date    Sept 2002
///
// The main window of the SUMO-gui.
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include <guisim/GUINet.h>
#include <guisim/GUILane.h>
#include <netload/NLHandler.h>
#include <traci-server/TraCIServer.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSStateHandler.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/transportables/MSTransportableControl.h>

#include "GUISUMOViewParent.h"
#include "GUILoadThread.h"
#include "GUIRunThread.h"
#include "GUIApplicationWindow.h"
#include "GUIEvent_SimulationLoaded.h"
#include "GUIEvent_SimulationEnded.h"

#include <utils/common/ToString.h>
#include <utils/common/RandHelper.h>
#include <utils/foxtools/MFXUtils.h>
#include <utils/foxtools/FXLCDLabel.h>
#include <utils/foxtools/FXThreadEvent.h>
#include <utils/foxtools/FXLinkLabel.h>

#include <utils/xml/XMLSubSys.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/events/GUIEvent_SimulationStep.h>
#include <utils/gui/events/GUIEvent_Message.h>
#include <utils/gui/div/GUIMessageWindow.h>
#include <utils/gui/div/GUIDialog_GLChosenEditor.h>
#include <utils/gui/tracker/GUIParameterTracker.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/cursors/GUICursorSubSys.h>
#include <utils/gui/shortcuts/GUIShortcutsSubSys.h>
#include <utils/gui/div/GUIIOGlobals.h>
#include <utils/gui/div/GUIUserIO.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/globjects/GUIShapeContainer.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/settings/GUISettingsHandler.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUIPerspectiveChanger.h>
#include <utils/options/OptionsCont.h>
#include "GUIGlobals.h"
#include "dialogs/GUIDialog_AboutSUMO.h"
#include "dialogs/GUIDialog_AppSettings.h"
#include "dialogs/GUIDialog_Breakpoints.h"


//#define HAVE_DANGEROUS_SOUNDS

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXDEFMAP(GUIApplicationWindow) GUIApplicationWindowMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_Q_CLOSE,    GUIApplicationWindow::onCmdQuit),
    FXMAPFUNC(SEL_SIGNAL,   MID_HOTKEY_CTRL_Q_CLOSE,    GUIApplicationWindow::onCmdQuit),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_F4_CLOSE,    GUIApplicationWindow::onCmdQuit),
    FXMAPFUNC(SEL_CLOSE,    MID_WINDOW,                 GUIApplicationWindow::onCmdQuit),

    FXMAPFUNC(SEL_COMMAND,  MID_OPEN_CONFIG,                                    GUIApplicationWindow::onCmdOpenConfiguration),
    FXMAPFUNC(SEL_COMMAND,  MID_OPEN_NETWORK,                                   GUIApplicationWindow::onCmdOpenNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_P,                                  GUIApplicationWindow::onCmdOpenShapes),
    FXMAPFUNC(SEL_COMMAND,  MID_OPEN_EDGEDATA,                                  GUIApplicationWindow::onCmdOpenEdgeData),
    FXMAPFUNC(SEL_COMMAND,  MID_RECENTFILE,                                     GUIApplicationWindow::onCmdOpenRecent),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_R_RELOAD,                           GUIApplicationWindow::onCmdReload),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_SHIFT_S_SAVENETWORK_AS,             GUIApplicationWindow::onCmdSaveConfig),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_W_CLOSESIMULATION,                  GUIApplicationWindow::onCmdClose),
    FXMAPFUNC(SEL_COMMAND,  MID_EDITCHOSEN,                                     GUIApplicationWindow::onCmdEditChosen),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_B_EDITBREAKPOINT_OPENDATAELEMENTS,  GUIApplicationWindow::onCmdEditBreakpoints),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F9_EDIT_VIEWSCHEME,                      GUIApplicationWindow::onCmdEditViewScheme),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_I_EDITVIEWPORT,                     GUIApplicationWindow::onCmdEditViewport),
    FXMAPFUNC(SEL_COMMAND,  MID_NETEDIT,                                        GUIApplicationWindow::onCmdNetedit),

    FXMAPFUNC(SEL_COMMAND,  MID_APPSETTINGS,                            GUIApplicationWindow::onCmdAppSettings),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_G_GAMINGMODE_TOOGLEGRID,    GUIApplicationWindow::onCmdGaming),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_F_FULSCREENMODE,            GUIApplicationWindow::onCmdFullScreen),
    FXMAPFUNC(SEL_COMMAND,  MID_LISTINTERNAL,                           GUIApplicationWindow::onCmdListInternal),
    FXMAPFUNC(SEL_COMMAND,  MID_LISTPARKING,                            GUIApplicationWindow::onCmdListParking),
    FXMAPFUNC(SEL_COMMAND,  MID_LISTTELEPORTING,                        GUIApplicationWindow::onCmdListTeleporting),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F12_ABOUT,                       GUIApplicationWindow::onCmdAbout),
    FXMAPFUNC(SEL_COMMAND,  MID_NEW_MICROVIEW,                          GUIApplicationWindow::onCmdNewView),
#ifdef HAVE_OSG
    FXMAPFUNC(SEL_COMMAND,  MID_NEW_OSGVIEW,    GUIApplicationWindow::onCmdNewOSG),
#endif
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_A_STARTSIMULATION_OPENADDITIONALS,          GUIApplicationWindow::onCmdStart),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_S_STOPSIMULATION_SAVENETWORK,               GUIApplicationWindow::onCmdStop),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_D_SINGLESIMULATIONSTEP_OPENDEMANDELEMENTS,  GUIApplicationWindow::onCmdStep),
    FXMAPFUNC(SEL_COMMAND,  MID_SIMSAVE,                                                GUIApplicationWindow::onCmdSaveState),
    FXMAPFUNC(SEL_COMMAND,  MID_TIME_TOOGLE,                                            GUIApplicationWindow::onCmdTimeToggle),
    FXMAPFUNC(SEL_COMMAND,  MID_DELAY_TOOGLE,                                           GUIApplicationWindow::onCmdDelayToggle),
    FXMAPFUNC(SEL_COMMAND,  MID_DEMAND_SCALE,                                           GUIApplicationWindow::onCmdDemandScale),
    FXMAPFUNC(SEL_COMMAND,  MID_CLEARMESSAGEWINDOW,                                     GUIApplicationWindow::onCmdClearMsgWindow),

    FXMAPFUNC(SEL_COMMAND,  MID_SHOWNETSTATS,       GUIApplicationWindow::onCmdShowStats),
    FXMAPFUNC(SEL_COMMAND,  MID_SHOWVEHSTATS,       GUIApplicationWindow::onCmdShowStats),
    FXMAPFUNC(SEL_COMMAND,  MID_SHOWPERSONSTATS,    GUIApplicationWindow::onCmdShowStats),

    FXMAPFUNC(SEL_UPDATE,   MID_OPEN_CONFIG,            GUIApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_UPDATE,   MID_OPEN_NETWORK,           GUIApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_UPDATE,   MID_OPEN_NETWORK,           GUIApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_P,          GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_OPEN_EDGEDATA,          GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_R_RELOAD,   GUIApplicationWindow::onUpdReload),
    FXMAPFUNC(SEL_UPDATE,   MID_RECENTFILE,             GUIApplicationWindow::onUpdOpenRecent),
    FXMAPFUNC(SEL_UPDATE,   MID_NEW_MICROVIEW,          GUIApplicationWindow::onUpdAddView),
#ifdef HAVE_OSG
    FXMAPFUNC(SEL_UPDATE,   MID_NEW_OSGVIEW,    GUIApplicationWindow::onUpdAddView),
#endif
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_A_STARTSIMULATION_OPENADDITIONALS,          GUIApplicationWindow::onUpdStart),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_S_STOPSIMULATION_SAVENETWORK,               GUIApplicationWindow::onUpdStop),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_D_SINGLESIMULATIONSTEP_OPENDEMANDELEMENTS,  GUIApplicationWindow::onUpdStep),
    FXMAPFUNC(SEL_UPDATE,   MID_SIMSAVE,                                                GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_EDITCHOSEN,                                             GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_B_EDITBREAKPOINT_OPENDATAELEMENTS,          GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_F9_EDIT_VIEWSCHEME,                              GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_I_EDITVIEWPORT,                             GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_NETEDIT,                                                GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_DEMAND_SCALE,                                           GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_G_GAMINGMODE_TOOGLEGRID,                    GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_F_FULSCREENMODE,                            GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_TRACI_STATUS,                                           GUIApplicationWindow::onUpdTraCIStatus),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F1_ONLINEDOCUMENTATION,                          GUIApplicationWindow::onCmdHelp),

    // forward requests to the active view
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEJUNCTION, GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEEDGE,     GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEVEHICLE,  GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPERSON,   GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATECONTAINER, GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATETLS,      GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEADD,      GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPOI,      GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPOLY,     GUIApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEJUNCTION, GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEEDGE,     GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEVEHICLE,  GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEPERSON,   GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATECONTAINER, GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATETLS,      GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEADD,      GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEPOI,      GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEPOLY,     GUIApplicationWindow::onUpdNeedsSimulation),
    FXMAPFUNC(SEL_KEYPRESS,              0,     GUIApplicationWindow::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE,            0,     GUIApplicationWindow::onKeyRelease),

    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, GUIApplicationWindow::onClipboardRequest),

    FXMAPFUNC(FXEX::SEL_THREAD_EVENT, ID_LOADTHREAD_EVENT,  GUIApplicationWindow::onLoadThreadEvent),
    FXMAPFUNC(FXEX::SEL_THREAD_EVENT, ID_RUNTHREAD_EVENT,   GUIApplicationWindow::onRunThreadEvent),
    FXMAPFUNC(FXEX::SEL_THREAD,       ID_LOADTHREAD_EVENT,  GUIApplicationWindow::onLoadThreadEvent),
    FXMAPFUNC(FXEX::SEL_THREAD,       ID_RUNTHREAD_EVENT,   GUIApplicationWindow::onRunThreadEvent),
};

// Object implementation
FXIMPLEMENT(GUIApplicationWindow, FXMainWindow, GUIApplicationWindowMap, ARRAYNUMBER(GUIApplicationWindowMap))

// ===========================================================================
// static members
// ===========================================================================
std::mt19937 GUIApplicationWindow::myGamingRNG;

// ===========================================================================
// member method definitions
// ===========================================================================
GUIApplicationWindow::GUIApplicationWindow(FXApp* a, const std::string& configPattern) :
    GUIMainWindow(a),
    myLoadThread(nullptr), myRunThread(nullptr),
    myAmLoading(false),
    myAlternateSimDelay(0.),
    myRecentFiles(a, "files"),
    myConfigPattern(configPattern),
    hadDependentBuild(false),
    myShowTimeAsHMS(false),
    myHaveNotifiedAboutSimEnd(false),
    // game specific
    myJamSoundTime(60),
    myPreviousCollisionNumber(0),
    myWaitingTime(0),
    myTimeLoss(0),
    myEmergencyVehicleCount(0),
    myTotalDistance(0) {
    // init icons
    GUIIconSubSys::initIcons(a);
    // init cursors
    GUICursorSubSys::initCursors(a);
}


void
GUIApplicationWindow::dependentBuild() {
    // don't do this twice
    if (hadDependentBuild) {
        return;
    }
    hadDependentBuild = true;

    setTarget(this);
    setSelector(MID_WINDOW);

    // build menu bar
    myMenuBarDrag = new FXToolBarShell(this, GUIDesignToolBar);
    myMenuBar = new FXMenuBar(myTopDock, myMenuBarDrag, GUIDesignToolbarMenuBar);
    new FXToolBarGrip(myMenuBar, myMenuBar, FXMenuBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
    buildToolBars();
    // build the thread - io
    myLoadThreadEvent.setTarget(this),  myLoadThreadEvent.setSelector(ID_LOADTHREAD_EVENT);
    myRunThreadEvent.setTarget(this), myRunThreadEvent.setSelector(ID_RUNTHREAD_EVENT);

    // build the status bar
    myStatusbar = new FXStatusBar(this, GUIDesignStatusBar);
    {
        myGeoFrame = new FXHorizontalFrame(myStatusbar, GUIDesignHorizontalFrameStatusBar);
        myGeoCoordinate = new FXLabel(myGeoFrame, "N/A\t\tOriginal coordinate (before coordinate transformation in NETCONVERT)", nullptr, LAYOUT_CENTER_Y);
        myCartesianFrame = new FXHorizontalFrame(myStatusbar, GUIDesignHorizontalFrameStatusBar);
        myCartesianCoordinate = new FXLabel(myCartesianFrame, "N/A\t\tNetwork coordinate", nullptr, LAYOUT_CENTER_Y);
        myStatButtons.push_back(new FXButton(myStatusbar, "-", GUIIconSubSys::getIcon(GUIIcon::GREENVEHICLE), this, MID_SHOWVEHSTATS));
        myStatButtons.push_back(new FXButton(myStatusbar, "-", GUIIconSubSys::getIcon(GUIIcon::GREENPERSON), this, MID_SHOWPERSONSTATS));
        myStatButtons.back()->hide();
        myStatButtons.push_back(new FXButton(myStatusbar, "-", GUIIconSubSys::getIcon(GUIIcon::GREENEDGE), this, MID_SHOWVEHSTATS));
        myStatButtons.back()->hide();
    }

    // make the window a mdi-window
    myMainSplitter = new FXSplitter(this, GUIDesignSplitter | SPLITTER_VERTICAL | SPLITTER_REVERSED);
    myMDIClient = new FXMDIClient(myMainSplitter, GUIDesignSplitterMDI);
    myMDIMenu = new FXMDIMenu(this, myMDIClient);
    new FXMDIWindowButton(myMenuBar, myMDIMenu, myMDIClient, FXMDIClient::ID_MDI_MENUWINDOW, GUIDesignMDIButtonLeft);
    new FXMDIDeleteButton(myMenuBar, myMDIClient, FXMDIClient::ID_MDI_MENUCLOSE, GUIDesignMDIButtonRight);
    new FXMDIRestoreButton(myMenuBar, myMDIClient, FXMDIClient::ID_MDI_MENURESTORE, GUIDesignMDIButtonRight);
    new FXMDIMinimizeButton(myMenuBar, myMDIClient, FXMDIClient::ID_MDI_MENUMINIMIZE, GUIDesignMDIButtonRight);

    // build the message window
    myMessageWindow = new GUIMessageWindow(myMainSplitter);
    // fill menu and tool bar
    fillMenuBar();
    myToolBar6->hide();
    myToolBar7->hide();
    myToolBar9->hide();
    myToolBar10->hide();
    // build additional threads
    myLoadThread = new GUILoadThread(getApp(), this, myEvents, myLoadThreadEvent);
    myRunThread = new GUIRunThread(getApp(), this, mySimDelay, myEvents, myRunThreadEvent);
    // set the status bar
    myStatusbar->getStatusLine()->setText("Ready.");
    // set the caption
    setTitle(MFXUtils::getTitleText("SUMO " VERSION_STRING));

    // start the simulation-thread (it will loop until the application ends deciding by itself whether to perform a step or not)
    myRunThread->start();
    setIcon(GUIIconSubSys::getIcon(GUIIcon::SUMO));
    setMiniIcon(GUIIconSubSys::getIcon(GUIIcon::SUMO_MINI));
}


void
GUIApplicationWindow::create() {
    setWindowSizeAndPos();
    gCurrentFolder = getApp()->reg().readStringEntry("SETTINGS", "basedir", "");
    FXMainWindow::create();
    myMenuBarDrag->create();
    myToolBarDrag1->create();
    myToolBarDrag2->create();
    myToolBarDrag3->create();
    myToolBarDrag4->create();
    myToolBarDrag5->create();
    myToolBarDrag6->create();
    myToolBarDrag7->create();
    myFileMenu->create();
    mySelectByPermissions->create();
    myEditMenu->create();
    mySettingsMenu->create();
    myLocatorMenu->create();
    myControlMenu->create();
    myWindowsMenu->create();
    myHelpMenu->create();

    FXint textWidth = getApp()->getNormalFont()->getTextWidth("8", 1) * 24;
    myCartesianFrame->setWidth(textWidth);
    myGeoFrame->setWidth(textWidth);

    show(PLACEMENT_DEFAULT);
    if (!OptionsCont::getOptions().isSet("window-size")) {
        if (getApp()->reg().readIntEntry("SETTINGS", "maximized", 0) == 1) {
            maximize();
        }
    }
    myShowTimeAsHMS = (getApp()->reg().readIntEntry("gui", "timeasHMS", 0) == 1);
    myAlternateSimDelay = getApp()->reg().readIntEntry("gui", "alternateSimDelay", 100);
}


GUIApplicationWindow::~GUIApplicationWindow() {
    myRunThread->prepareDestruction();
    myRunThread->join();
    closeAllWindows();
    //
    GUIIconSubSys::close();
    delete myGLVisual;
    // delete some non-parented windows
    delete myToolBarDrag1;
    //
    delete myRunThread;
    delete myFileMenu;
    delete myEditMenu;
    delete mySelectByPermissions;
    delete mySettingsMenu;
    delete myLocatorMenu;
    delete myControlMenu;
    delete myWindowsMenu;
    delete myHelpMenu;

    delete myLoadThread;

    while (!myEvents.empty()) {
        // get the next event
        GUIEvent* e = myEvents.top();
        myEvents.pop();
        delete e;
    }
}


void
GUIApplicationWindow::detach() {
    FXMainWindow::detach();
    myMenuBarDrag->detach();
    myToolBarDrag1->detach();
}


void
GUIApplicationWindow::fillMenuBar() {
    // build file menu
    myFileMenu = new FXMenuPane(this);
    new FXMenuTitle(myMenuBar, "&File", nullptr, myFileMenu);
    new FXMenuCommand(myFileMenu,
                      "&Open Simulation...\tCtrl+O\tOpen a simulation (Configuration file).",
                      GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG), this, MID_OPEN_CONFIG);
    new FXMenuCommand(myFileMenu,
                      "Open &Network...\tCtrl+N\tOpen a network.",
                      GUIIconSubSys::getIcon(GUIIcon::OPEN_NET), this, MID_OPEN_NETWORK);
    new FXMenuCommand(myFileMenu,
                      "Open Shapes \tCtrl+P\tLoad POIs and Polygons for visualization.",
                      GUIIconSubSys::getIcon(GUIIcon::OPEN_SHAPES), this, MID_HOTKEY_CTRL_P);
    new FXMenuCommand(myFileMenu,
                      "Open EdgeData \tCtrl+U\tLoad edge related data for visualization.",
                      GUIIconSubSys::getIcon(GUIIcon::OPEN_NET), this, MID_OPEN_EDGEDATA);
    new FXMenuCommand(myFileMenu,
                      "&Reload\tCtrl+R\tReloads the simulation / the network.",
                      GUIIconSubSys::getIcon(GUIIcon::RELOAD), this, MID_HOTKEY_CTRL_R_RELOAD);
    new FXMenuSeparator(myFileMenu);
    new FXMenuCommand(myFileMenu,
                      "Save Configuration\tCtrl+Shift+S\tSave current options as a configuration file.",
                      GUIIconSubSys::getIcon(GUIIcon::SAVE), this, MID_HOTKEY_CTRL_SHIFT_S_SAVENETWORK_AS);
    new FXMenuCommand(myFileMenu,
                      "Close\tCtrl+W\tClose the simulation.",
                      GUIIconSubSys::getIcon(GUIIcon::CLOSE), this, MID_HOTKEY_CTRL_W_CLOSESIMULATION);
    // Recent files
    FXMenuSeparator* sep1 = new FXMenuSeparator(myFileMenu);
    sep1->setTarget(&myRecentFiles);
    sep1->setSelector(FXRecentFiles::ID_ANYFILES);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_1);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_2);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_3);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_4);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_5);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_6);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_7);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_8);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_9);
    new FXMenuCommand(myFileMenu, "", nullptr, &myRecentFiles, FXRecentFiles::ID_FILE_10);
    new FXMenuCommand(myFileMenu, "C&lear Recent Files", nullptr, &myRecentFiles, FXRecentFiles::ID_CLEAR);
    myRecentFiles.setTarget(this);
    myRecentFiles.setSelector(MID_RECENTFILE);
    new FXMenuSeparator(myFileMenu);
    new FXMenuCommand(myFileMenu,
                      "&Quit\tCtrl+Q\tQuit the Application.",
                      nullptr, this, MID_HOTKEY_CTRL_Q_CLOSE, 0);

    // build edit menu
    mySelectByPermissions = new FXMenuPane(this);
    std::vector<std::string> vehicleClasses = SumoVehicleClassStrings.getStrings();
    for (auto i : vehicleClasses) {
        new FXMenuCommand(mySelectByPermissions, i.c_str(), nullptr, this, MID_EDITCHOSEN);
    }

    myEditMenu = new FXMenuPane(this);
    new FXMenuTitle(myMenuBar, "&Edit", nullptr, myEditMenu);
    new FXMenuCommand(myEditMenu,
                      "Edit Selected...\tCtrl+E\tOpens a dialog for editing the list of selected items.",
                      GUIIconSubSys::getIcon(GUIIcon::FLAG), this, MID_EDITCHOSEN);
    mySelectLanesMenuCascade = new FXMenuCascade(myEditMenu,
            "Select lanes which allow...\t\tOpens a menu for selecting a vehicle class by which to selected lanes.",
            GUIIconSubSys::getIcon(GUIIcon::FLAG), mySelectByPermissions);
    new FXMenuSeparator(myEditMenu);
    new FXMenuCommand(myEditMenu,
                      "Edit Breakpoints\tCtrl+B\tOpens a dialog for editing breakpoints.",
                      nullptr, this, MID_HOTKEY_CTRL_B_EDITBREAKPOINT_OPENDATAELEMENTS);
    new FXMenuCommand(myEditMenu,
                      "Edit Visualisation\tF9\tOpens a dialog for editing visualization settings.",
                      nullptr, this, MID_HOTKEY_F9_EDIT_VIEWSCHEME);
    new FXMenuCommand(myEditMenu,
                      "Edit Viewport\tCtrl+I\tOpens a dialog for editing viewing area, zoom and rotation.",
                      nullptr, this, MID_HOTKEY_CTRL_I_EDITVIEWPORT);
    new FXMenuSeparator(myEditMenu);
    new FXMenuCommand(myEditMenu,
                      "Open in netedit\tCtrl+T\tOpens the netedit application with the current network.",
                      GUIIconSubSys::getIcon(GUIIcon::NETEDIT_MINI), this, MID_NETEDIT);

    // build settings menu
    mySettingsMenu = new FXMenuPane(this);
    new FXMenuTitle(myMenuBar, "&Settings", nullptr, mySettingsMenu);
    new FXMenuCommand(mySettingsMenu,
                      "Application Settings...\t\tOpen a Dialog for Application Settings editing.",
                      nullptr, this, MID_APPSETTINGS);
    new FXMenuCheck(mySettingsMenu,
                    "Gaming Mode\tCtrl+G\tToggle gaming mode on/off.",
                    this, MID_HOTKEY_CTRL_G_GAMINGMODE_TOOGLEGRID);
    new FXMenuCheck(mySettingsMenu,
                    "Full Screen Mode\tCtrl+F\tToggle full screen mode on/off.",
                    this, MID_HOTKEY_CTRL_F_FULSCREENMODE);
    // build Locate menu
    myLocatorMenu = new FXMenuPane(this);
    new FXMenuTitle(myMenuBar, "&Locate", nullptr, myLocatorMenu);
    new FXMenuCommand(myLocatorMenu,
                      "Locate &Junctions\t\tOpen a Dialog for Locating a Junction.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATEJUNCTION), this, MID_LOCATEJUNCTION);
    new FXMenuCommand(myLocatorMenu,
                      "Locate &Edges\t\tOpen a Dialog for Locating an Edge.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATEEDGE), this, MID_LOCATEEDGE);
    if (!MSGlobals::gUseMesoSim) { // there are no gui-vehicles in mesosim
        new FXMenuCommand(myLocatorMenu,
                          "Locate &Vehicles\t\tOpen a Dialog for Locating a Vehicle.",
                          GUIIconSubSys::getIcon(GUIIcon::LOCATEVEHICLE), this, MID_LOCATEVEHICLE);
    }
    new FXMenuCommand(myLocatorMenu,
                      "Locate &Persons\t\tOpen a Dialog for Locating a Person.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATEPERSON), this, MID_LOCATEPERSON);
    new FXMenuCommand(myLocatorMenu,
                      "Locate &Container\t\tOpen a Dialog for Locating a Container.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATECONTAINER), this, MID_LOCATECONTAINER);
    new FXMenuCommand(myLocatorMenu,
                      "Locate &TLS\t\tOpen a Dialog for Locating a Traffic Light.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATETLS), this, MID_LOCATETLS);
    new FXMenuCommand(myLocatorMenu,
                      "Locate &Additional\t\tOpen a Dialog for Locating an Additional Structure.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATEADD), this, MID_LOCATEADD);
    new FXMenuCommand(myLocatorMenu,
                      "Locate P&oI\t\tOpen a Dialog for Locating a Point of Intereset.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATEPOI), this, MID_LOCATEPOI);
    new FXMenuCommand(myLocatorMenu,
                      "Locate Po&lygon\t\tOpen a Dialog for Locating a Polygon.",
                      GUIIconSubSys::getIcon(GUIIcon::LOCATEPOLY), this, MID_LOCATEPOLY);
    new FXMenuSeparator(myLocatorMenu);
    new FXMenuCheck(myLocatorMenu,
                    "Show Internal Structures\t\tShow internal junctions and streets in locator dialog.",
                    this, MID_LISTINTERNAL);
    FXMenuCheck* listParking = new FXMenuCheck(myLocatorMenu,
            "Show Parking Vehicles\t\tShow parking vehicles in locator dialog.",
            this, MID_LISTPARKING);
    listParking->setCheck(myListParking);
    new FXMenuCheck(myLocatorMenu,
                    "Show vehicles outside the road network\t\tShow vehicles that are teleporting or driving remote-controlled outside the road network in locator dialog.",
                    this, MID_LISTTELEPORTING);
    // build control menu
    myControlMenu = new FXMenuPane(this);
    new FXMenuTitle(myMenuBar, "Simulation", nullptr, myControlMenu);
    new FXMenuCommand(myControlMenu,
                      "Run\tCtrl+A\tStart running the simulation.",
                      GUIIconSubSys::getIcon(GUIIcon::START), this, MID_HOTKEY_CTRL_A_STARTSIMULATION_OPENADDITIONALS);
    new FXMenuCommand(myControlMenu,
                      "Stop\tCtrl+S\tStop running the simulation.",
                      GUIIconSubSys::getIcon(GUIIcon::STOP), this, MID_HOTKEY_CTRL_S_STOPSIMULATION_SAVENETWORK);
    new FXMenuCommand(myControlMenu,
                      "Step\tCtrl+D\tPerform one simulation step.",
                      GUIIconSubSys::getIcon(GUIIcon::STEP), this, MID_HOTKEY_CTRL_D_SINGLESIMULATIONSTEP_OPENDEMANDELEMENTS);
    new FXMenuCommand(myControlMenu,
                      "Save\t\tSave the current simulation state to a file.",
                      GUIIconSubSys::getIcon(GUIIcon::SAVE), this, MID_SIMSAVE);

    // build windows menu
    myWindowsMenu = new FXMenuPane(this);
    new FXMenuTitle(myMenuBar, "&Windows", nullptr, myWindowsMenu);
    new FXMenuCheck(myWindowsMenu,
                    "Show Status Line\t\tToggle the Status Bar on/off.",
                    myStatusbar, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(myWindowsMenu,
                    "Show Message Window\t\tToggle the Message Window on/off.",
                    myMessageWindow, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(myWindowsMenu,
                    "Show Simulation Time\t\tToggle the Simulation Time on/off.",
                    myToolBar3, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(myWindowsMenu,
                    "Show Simulation Delay\t\tToggle the Simulation Delay Entry on/off.",
                    myToolBar4, FXWindow::ID_TOGGLESHOWN);
    addToWindowsMenu(myWindowsMenu);
    new FXMenuSeparator(myWindowsMenu);
    new FXMenuCommand(myWindowsMenu,
                      "Tile &Horizontally",
                      GUIIconSubSys::getIcon(GUIIcon::WINDOWS_TILE_HORI), myMDIClient, FXMDIClient::ID_MDI_TILEHORIZONTAL);
    new FXMenuCommand(myWindowsMenu,
                      "Tile &Vertically",
                      GUIIconSubSys::getIcon(GUIIcon::WINDOWS_TILE_VERT), myMDIClient, FXMDIClient::ID_MDI_TILEVERTICAL);
    new FXMenuCommand(myWindowsMenu,
                      "Cascade",
                      GUIIconSubSys::getIcon(GUIIcon::WINDOWS_CASCADE),
                      myMDIClient, FXMDIClient::ID_MDI_CASCADE);
    new FXMenuCommand(myWindowsMenu,
                      "&Close",
                      nullptr, myMDIClient, FXMDIClient::ID_MDI_CLOSE);
    sep1 = new FXMenuSeparator(myWindowsMenu);
    sep1->setTarget(myMDIClient);
    sep1->setSelector(FXMDIClient::ID_MDI_ANY);
    new FXMenuCommand(myWindowsMenu, "", nullptr, myMDIClient, FXMDIClient::ID_MDI_1);
    new FXMenuCommand(myWindowsMenu, "", nullptr, myMDIClient, FXMDIClient::ID_MDI_2);
    new FXMenuCommand(myWindowsMenu, "", nullptr, myMDIClient, FXMDIClient::ID_MDI_3);
    new FXMenuCommand(myWindowsMenu, "", nullptr, myMDIClient, FXMDIClient::ID_MDI_4);
    new FXMenuCommand(myWindowsMenu, "&Others...", nullptr, myMDIClient, FXMDIClient::ID_MDI_OVER_5);
    new FXMenuSeparator(myWindowsMenu);
    new FXMenuCommand(myWindowsMenu,
                      "Clear Message Window\t\tClear the message window.",
                      nullptr, this, MID_CLEARMESSAGEWINDOW);

    // build help menu
    myHelpMenu = new FXMenuPane(this);
    new FXMenuTitle(myMenuBar,
                    "&Help",
                    nullptr, myHelpMenu);
    new FXMenuCommand(myHelpMenu,
                      "&Online Documentation\tF1\tOpen Online documentation.",
                      nullptr, this, MID_HOTKEY_F1_ONLINEDOCUMENTATION);
    new FXMenuCommand(myHelpMenu,
                      "&About\tF12\tAbout sumo-gui.",
                      GUIIconSubSys::getIcon(GUIIcon::SUMO_MINI), this, MID_HOTKEY_F12_ABOUT);

    //new FXButton(myMenuBar, "\t\tShows TraCI status", GUIIconSubSys::getIcon(GUIIcon::ADD), this, MID_TRACI_STATUS, 0, 0, 0, 14, 14, 0, 0, 0, 0);

    // build SUMO Accelerators (hotkeys)
    GUIShortcutsSubSys::buildSUMOAccelerators(this);
}


void
GUIApplicationWindow::buildToolBars() {
    // build tool bars
    {
        // file and simulation tool bar
        myToolBarDrag1 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar1 = new FXToolBar(myTopDock, myToolBarDrag1, GUIDesignToolBarRaisedNextTop);
        new FXToolBarGrip(myToolBar1, myToolBar1, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        // build file tools
        new FXButton(myToolBar1, "\t\tOpen a simulation (Configuration file).", GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG), this, MID_OPEN_CONFIG, GUIDesignButtonToolbar);
        new FXButton(myToolBar1, "\t\tOpen a network.", GUIIconSubSys::getIcon(GUIIcon::OPEN_NET), this, MID_OPEN_NETWORK, GUIDesignButtonToolbar);
        new FXButton(myToolBar1, "\t\tReloads the simulation / the network.", GUIIconSubSys::getIcon(GUIIcon::RELOAD), this, MID_HOTKEY_CTRL_R_RELOAD, GUIDesignButtonToolbar);
    }
    {
        // simulation toolbar
        myToolBarDrag2 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar2 = new FXToolBar(myTopDock, myToolBarDrag2, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar2, myToolBar2, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        // build simulation tools
        new FXButton(myToolBar2, "\t\tStart the loaded simulation.", GUIIconSubSys::getIcon(GUIIcon::START), this, MID_HOTKEY_CTRL_A_STARTSIMULATION_OPENADDITIONALS, GUIDesignButtonToolbar);
        new FXButton(myToolBar2, "\t\tStop the running simulation.", GUIIconSubSys::getIcon(GUIIcon::STOP), this, MID_HOTKEY_CTRL_S_STOPSIMULATION_SAVENETWORK, GUIDesignButtonToolbar);
        new FXButton(myToolBar2, "\t\tPerform a single simulation step.", GUIIconSubSys::getIcon(GUIIcon::STEP), this, MID_HOTKEY_CTRL_D_SINGLESIMULATIONSTEP_OPENDEMANDELEMENTS, GUIDesignButtonToolbar);
    }
    {
        // Simulation Step Display
        myToolBarDrag3 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar3 = new FXToolBar(myTopDock, myToolBarDrag3, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar3, myToolBar3, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        new FXButton(myToolBar3, "Time:\t\tToggle between seconds and hour:minute:seconds display", nullptr, this, MID_TIME_TOOGLE, GUIDesignButtonToolbarText);

        myLCDLabel = new FXEX::FXLCDLabel(myToolBar3, 16, nullptr, 0, JUSTIFY_RIGHT);
        myLCDLabel->setHorizontal(2);
        myLCDLabel->setVertical(6);
        myLCDLabel->setThickness(2);
        myLCDLabel->setGroove(2);
        myLCDLabel->setText("----------------");
    }
    {
        // Simulation Delay
        myToolBarDrag4 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar4 = new FXToolBar(myTopDock, myToolBarDrag4, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar4, myToolBar4, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        new FXButton(myToolBar4, "Delay (ms):\t\tDelay per simulated second. Click to toggle between the last two delay values", nullptr, this, MID_DELAY_TOOGLE, GUIDesignButtonToolbarText);

        mySimDelay = 0;
        mySimDelayTarget = new FXDataTarget(mySimDelay);
        mySimDelaySpinner = new FXRealSpinner(myToolBar4, 7, mySimDelayTarget, FXDataTarget::ID_VALUE, GUIDesignSpinDial);
        mySimDelaySlider = new FXSlider(myToolBar4, mySimDelayTarget, FXDataTarget::ID_VALUE, LAYOUT_FIX_WIDTH | SLIDER_ARROW_UP | SLIDER_TICKS_TOP, 0, 0, 300, 10, 0, 0, 5, 0);
        mySimDelaySlider->setRange(0, 1000);
        mySimDelaySlider->setHeadSize(10);
        mySimDelaySlider->setIncrement(50);
        mySimDelaySlider->setTickDelta(100);
        mySimDelaySlider->setValue((int)mySimDelay);
        //mySimDelayTarget->setNumberFormat(0);
        //mySimDelayTarget->setIncrements(1, 10, 10);
        mySimDelaySpinner->setIncrement(10);
        mySimDelaySpinner->setRange(0, 10000);
        mySimDelaySpinner->setValue(mySimDelay);
    }
    {
        // Scale traffic (flows and incrementally loaded vehicles)
        myToolBarDrag8 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar8 = new FXToolBar(myTopDock, myToolBarDrag8, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar8, myToolBar8, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        new FXLabel(myToolBar8, "Scale Traffic:\t\tScale traffic from flows and vehicles that are loaded incrementally from route files", nullptr, LAYOUT_TOP | LAYOUT_LEFT);
        myDemandScaleSpinner = new FXRealSpinner(myToolBar8, 7, this, MID_DEMAND_SCALE, GUIDesignSpinDial);
        myDemandScaleSpinner->setIncrement(0.5);
        myDemandScaleSpinner->setRange(0, 1000);
        myDemandScaleSpinner->setValue(1);
    }
    {
        // Views
        myToolBarDrag5 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar5 = new FXToolBar(myTopDock, myToolBarDrag5, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar5, myToolBar5, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        // build view tools
        new FXButton(myToolBar5, "\t\tOpen a new microscopic view.",
                     GUIIconSubSys::getIcon(GUIIcon::MICROVIEW), this, MID_NEW_MICROVIEW, GUIDesignButtonToolbar);
#ifdef HAVE_OSG
        new FXButton(myToolBar5, "\t\tOpen a new 3D view.",
                     GUIIconSubSys::getIcon(GUIIcon::OSGVIEW), this, MID_NEW_OSGVIEW, GUIDesignButtonToolbar);
#endif
    }
    {
        /// game specific stuff
        // total waitingTime
        myToolBarDrag6 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar6 = new FXToolBar(myTopDock, myToolBarDrag6, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar6, myToolBar6, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        new FXLabel(myToolBar6, "Waiting Time:\t\tTime spent waiting accumulated for all vehicles", nullptr, LAYOUT_TOP | LAYOUT_LEFT);
        myWaitingTimeLabel = new FXEX::FXLCDLabel(myToolBar6, 13, nullptr, 0, JUSTIFY_RIGHT);
        myWaitingTimeLabel->setHorizontal(2);
        myWaitingTimeLabel->setVertical(6);
        myWaitingTimeLabel->setThickness(2);
        myWaitingTimeLabel->setGroove(2);
        myWaitingTimeLabel->setText("-------------");

        // idealistic time loss
        myToolBarDrag7 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar7 = new FXToolBar(myTopDock, myToolBarDrag7, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar7, myToolBar7, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        new FXLabel(myToolBar7, "Time Loss:\t\tTime lost due to being unable to drive with maximum speed for all vehicles", nullptr, LAYOUT_TOP | LAYOUT_LEFT);
        myTimeLossLabel = new FXEX::FXLCDLabel(myToolBar7, 13, nullptr, 0, JUSTIFY_RIGHT);
        myTimeLossLabel->setHorizontal(2);
        myTimeLossLabel->setVertical(6);
        myTimeLossLabel->setThickness(2);
        myTimeLossLabel->setGroove(2);
        myTimeLossLabel->setText("-------------");

        // total driving distance
        myToolBarDrag9 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar9 = new FXToolBar(myTopDock, myToolBarDrag9, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar9, myToolBar9, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        new FXLabel(myToolBar9, "Distance (km):\t\tTotal distance driven by DRT vehicles", nullptr, LAYOUT_TOP | LAYOUT_LEFT);
        myTotalDistanceLabel = new FXEX::FXLCDLabel(myToolBar9, 13, nullptr, 0, JUSTIFY_RIGHT);
        myTotalDistanceLabel->setHorizontal(2);
        myTotalDistanceLabel->setVertical(6);
        myTotalDistanceLabel->setThickness(2);
        myTotalDistanceLabel->setGroove(2);
        myTotalDistanceLabel->setText("-------------");

        // emergency vehicle counts
        myToolBarDrag10 = new FXToolBarShell(this, GUIDesignToolBar);
        myToolBar10 = new FXToolBar(myTopDock, myToolBarDrag10, GUIDesignToolBarRaisedSameTop);
        new FXToolBarGrip(myToolBar10, myToolBar10, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
        new FXLabel(myToolBar10, "Emergency Vehicle waiting time:\t\tTime spent waiting accumulated for emergency vehicles", nullptr, LAYOUT_TOP | LAYOUT_LEFT);
        myEmergencyVehicleLabel = new FXEX::FXLCDLabel(myToolBar10, 13, nullptr, 0, JUSTIFY_RIGHT);
        myEmergencyVehicleLabel->setHorizontal(2);
        myEmergencyVehicleLabel->setVertical(6);
        myEmergencyVehicleLabel->setThickness(2);
        myEmergencyVehicleLabel->setGroove(2);
        myEmergencyVehicleLabel->setText("-------------");
    }
}


long
GUIApplicationWindow::onCmdQuit(FXObject*, FXSelector, void*) {
    storeWindowSizeAndPos();
    getApp()->reg().writeStringEntry("SETTINGS", "basedir", gCurrentFolder.text());
    getApp()->reg().writeIntEntry("SETTINGS", "maximized", isMaximized() ? 1 : 0);
    getApp()->reg().writeIntEntry("gui", "timeasHMS", myShowTimeAsHMS ? 1 : 0);
    getApp()->reg().writeIntEntry("gui", "alternateSimDelay", (int)myAlternateSimDelay);
    getApp()->exit(0);
    return 1;
}


long
GUIApplicationWindow::onCmdEditChosen(FXObject* menu, FXSelector, void*) {
    FXMenuCommand* mc = dynamic_cast<FXMenuCommand*>(menu);
    if (mc->getText() == "Edit Selected...") {
        GUIDialog_GLChosenEditor* chooser =
            new GUIDialog_GLChosenEditor(this, &gSelected);
        chooser->create();
        chooser->show();
    } else {
        if (!myAmLoading && myRunThread->simulationAvailable()) {
            const SUMOVehicleClass svc = SumoVehicleClassStrings.get(mc->getText().text());
            for (MSEdgeVector::const_iterator i = MSEdge::getAllEdges().begin(); i != MSEdge::getAllEdges().end(); ++i) {
                const std::vector<MSLane*>& lanes = (*i)->getLanes();
                for (std::vector<MSLane*>::const_iterator it = lanes.begin(); it != lanes.end(); ++it) {
                    GUILane* lane = dynamic_cast<GUILane*>(*it);
                    assert(lane != 0);
                    if ((lane->getPermissions() & svc) != 0) {
                        gSelected.select(lane->getGlID());
                    }
                }
            }
            if (myMDIClient->numChildren() > 0) {
                GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
                if (w != nullptr) {
                    // color by selection
                    w->getView()->getVisualisationSettings().laneColorer.setActive(1);
                }
            }
        }
        updateChildren();
    }
    return 1;
}


long
GUIApplicationWindow::onCmdEditBreakpoints(FXObject*, FXSelector, void*) {
    GUIDialog_Breakpoints* chooser = new GUIDialog_Breakpoints(this, myRunThread->getBreakpoints(), myRunThread->getBreakpointLock());
    chooser->create();
    chooser->show();
    return 1;
}

long
GUIApplicationWindow::onCmdEditViewport(FXObject*, FXSelector, void*) {
    if (!myGLWindows.empty()) {
        myGLWindows[0]->getView()->showViewportEditor();
    }
    return 1;
}


long
GUIApplicationWindow::onCmdEditViewScheme(FXObject*, FXSelector, void*) {
    if (!myGLWindows.empty()) {
        myGLWindows[0]->getView()->showViewschemeEditor();
    }
    return 1;
}


long
GUIApplicationWindow::onCmdHelp(FXObject*, FXSelector, void*) {
    FXLinkLabel::fxexecute("https://sumo.dlr.de/docs/sumo-gui.html");
    return 1;
}


long
GUIApplicationWindow::onCmdNetedit(FXObject*, FXSelector, void*) {
    if (myGLWindows.empty()) {
        return 1;
    }
    FXRegistry reg("SUMO netedit", "Eclipse");
    reg.read();
    const GUISUMOAbstractView* const v = myGLWindows[0]->getView();
    reg.writeRealEntry("viewport", "x", v->getChanger().getXPos());
    reg.writeRealEntry("viewport", "y", v->getChanger().getYPos());
    reg.writeRealEntry("viewport", "z", v->getChanger().getZPos());
    reg.write();
    std::string netedit = "netedit";
    const char* sumoPath = getenv("SUMO_HOME");
    if (sumoPath != nullptr) {
        std::string newPath = std::string(sumoPath) + "/bin/netedit";
        if (FileHelpers::isReadable(newPath) || FileHelpers::isReadable(newPath + ".exe")) {
            netedit = "\"" + newPath + "\"";
        }
    }
    std::string cmd = netedit + " --registry-viewport -s " + "\"" + OptionsCont::getOptions().getString("net-file") + "\"";
    // start in background
#ifndef WIN32
    cmd = cmd + " &";
#else
    // see "help start" for the parameters
    cmd = "start /B \"\" " + cmd;
#endif
    WRITE_MESSAGE("Running " + cmd + ".");
    // yay! fun with dangerous commands... Never use this over the internet
    SysUtils::runHiddenCommand(cmd);
    return 1;
}


long
GUIApplicationWindow::onCmdOpenConfiguration(FXObject*, FXSelector, void*) {
    // get the new file name
    FXFileDialog opendialog(this, "Open Simulation Configuration");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList(myConfigPattern.c_str());
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        loadConfigOrNet(file);
        myRecentFiles.appendFile(file.c_str());
    }
    return 1;
}


long
GUIApplicationWindow::onCmdOpenNetwork(FXObject*, FXSelector, void*) {
    // get the new file name
    FXFileDialog opendialog(this, "Open Network");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("SUMO nets (*.net.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        loadConfigOrNet(file);
        myRecentFiles.appendFile(file.c_str());
    }
    return 1;
}


long
GUIApplicationWindow::onCmdOpenShapes(FXObject*, FXSelector, void*) {
    // get the shape file name
    FXFileDialog opendialog(this, "Open Shapes");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("Additional files (*.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();

        dynamic_cast<GUIShapeContainer&>(myRunThread->getNet().getShapeContainer()).allowReplacement();
        NLShapeHandler handler(file, myRunThread->getNet().getShapeContainer());
        if (!XMLSubSys::runParser(handler, file, false)) {
            WRITE_MESSAGE("Loading of " + file + " failed.");
        }
        update();
        if (myMDIClient->numChildren() > 0) {
            GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
            if (w != nullptr) {
                w->getView()->update();
            }
        }
    }
    return 1;
}

long
GUIApplicationWindow::onCmdOpenEdgeData(FXObject*, FXSelector, void*) {
    // get the shape file name
    FXFileDialog opendialog(this, "Open EdgeData");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("EdgeData files (*.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        if (!GUINet::getGUIInstance()->loadEdgeData(file)) {
            WRITE_MESSAGE("Loading of " + file + " failed.");
        }
        update();
        if (myMDIClient->numChildren() > 0) {
            GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
            if (w != nullptr) {
                w->getView()->update();
            }
        }
    }
    return 1;
}

long
GUIApplicationWindow::onCmdReload(FXObject*, FXSelector, void*) {
    if (!myAmLoading) {
        storeWindowSizeAndPos();
        getApp()->beginWaitCursor();
        myAmLoading = true;
        closeAllWindows();
        myLoadThread->start();
        setStatusBarText("Reloading.");
        update();
    }
    return 1;
}


long
GUIApplicationWindow::onCmdOpenRecent(FXObject* /* sender */, FXSelector, void* ptr) {
    if (myAmLoading) {
        myStatusbar->getStatusLine()->setText("Already loading!");
        return 1;
    }
    std::string file((const char*)ptr);
    loadConfigOrNet(file);
    return 1;
}


long
GUIApplicationWindow::onCmdSaveConfig(FXObject*, FXSelector, void*) {
    // get the new file name
    FXFileDialog opendialog(this, "Save Sumo Configuration");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::SAVE));
    opendialog.setSelectMode(SELECTFILE_ANY);
    opendialog.setPatternList("Config (*.sumocfg)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (!opendialog.execute() || !MFXUtils::userPermitsOverwritingWhenFileExists(this, opendialog.getFilename())) {
        return 1;
    }
    std::string file = MFXUtils::assureExtension(opendialog.getFilename(),
                       opendialog.getPatternText(opendialog.getCurrentPattern()).after('.').before(')')).text();
    std::ofstream out(file);
    if (out.good()) {
        OptionsCont::getOptions().writeConfiguration(out, true, false, false);
        setStatusBarText("Configuration saved to " + file);
    } else {
        setStatusBarText("Could not save onfiguration to " + file);
    }
    out.close();
    return 1;
}


long
GUIApplicationWindow::onCmdClose(FXObject*, FXSelector, void*) {
    closeAllWindows();
    return 1;
}


long
GUIApplicationWindow::onUpdOpen(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myAmLoading ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    return 1;
}


long
GUIApplicationWindow::onUpdReload(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myAmLoading || myLoadThread->getFileName() == "" || TraCIServer::getInstance() != nullptr
                   ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    return 1;
}


long
GUIApplicationWindow::onUpdOpenRecent(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myAmLoading ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    return 1;
}


long
GUIApplicationWindow::onUpdAddView(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myAmLoading || !myRunThread->simulationAvailable()
                   ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    return 1;
}


long
GUIApplicationWindow::onCmdStart(FXObject*, FXSelector, void*) {
    // check whether a net was loaded successfully
    if (!myRunThread->simulationAvailable()) {
        myStatusbar->getStatusLine()->setText("No simulation loaded!");
        return 1;
    }
    // check whether it was started before and paused;
    if (!myWasStarted) {
        myRunThread->begin();
        myWasStarted = true;
    }
    myRunThread->resume();
    getApp()->forceRefresh(); // only callking myToolBar2->forceRefresh somehow loses keyboard focus
    return 1;
}


long
GUIApplicationWindow::onCmdStop(FXObject*, FXSelector, void*) {
    myRunThread->stop();
    getApp()->forceRefresh(); // only callking myToolBar2->forceRefresh somehow loses keyboard focus
    return 1;
}


long
GUIApplicationWindow::onCmdStep(FXObject*, FXSelector, void*) {
    // check whether a net was loaded successfully
    if (!myRunThread->simulationAvailable()) {
        myStatusbar->getStatusLine()->setText("No simulation loaded!");
        return 1;
    }
    // check whether it was started before and paused;
    if (!myWasStarted) {
        myRunThread->begin();
        myWasStarted = true;
    }
    myRunThread->singleStep();
    return 1;
}


long
GUIApplicationWindow::onCmdSaveState(FXObject*, FXSelector, void*) {
    // get the new file name
    FXFileDialog opendialog(this, "Save Simulation State");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::SAVE));
    opendialog.setSelectMode(SELECTFILE_ANY);
    opendialog.setPatternList("GZipped State (*.xml.gz)\nXML State (*.xml)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (!opendialog.execute() || !MFXUtils::userPermitsOverwritingWhenFileExists(this, opendialog.getFilename())) {
        return 1;
    }

    const std::string file = MFXUtils::assureExtension(opendialog.getFilename(),
                             opendialog.getPatternText(opendialog.getCurrentPattern()).after('.').before(')')).text();
    MSStateHandler::saveState(file, MSNet::getInstance()->getCurrentTimeStep());
    setStatusBarText("Simulation saved to " + file);
    return 1;
}


long
GUIApplicationWindow::onCmdTimeToggle(FXObject*, FXSelector, void*) {
    myShowTimeAsHMS = !myShowTimeAsHMS;
    if (myRunThread->simulationAvailable()) {
        updateTimeLCD(myRunThread->getNet().getCurrentTimeStep());
    }
    return 1;
}


long
GUIApplicationWindow::onCmdDelayToggle(FXObject*, FXSelector, void*) {
    const double tmp = myAlternateSimDelay;
    myAlternateSimDelay = mySimDelay;
    mySimDelay = tmp;
    return 1;
}


long
GUIApplicationWindow::onCmdDemandScale(FXObject*, FXSelector, void*) {
    if (myRunThread->simulationAvailable()) {
        myRunThread->getNet().getVehicleControl().setScale(myDemandScaleSpinner->getValue());
    }
    return 1;
}


long
GUIApplicationWindow::onCmdClearMsgWindow(FXObject*, FXSelector, void*) {
    myMessageWindow->clear();
    return 1;
}


long
GUIApplicationWindow::onUpdStart(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   !myRunThread->simulationIsStartable() || myAmLoading
                   ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    return 1;
}


long
GUIApplicationWindow::onUpdStop(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   !myRunThread->simulationIsStopable() || myAmLoading
                   ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    return 1;
}


long
GUIApplicationWindow::onUpdStep(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   !myRunThread->simulationIsStepable() || myAmLoading
                   ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    return 1;
}


long
GUIApplicationWindow::onUpdNeedsSimulation(FXObject* sender, FXSelector, void* ptr) {
    bool disable = !myRunThread->simulationAvailable() || myAmLoading;
    sender->handle(this, disable ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    // mySelectLanesMenuCascade has to be disabled manually
    if (disable) {
        mySelectLanesMenuCascade->disable();
    } else {
        mySelectLanesMenuCascade->enable();
    }
    return 1;
}

long
GUIApplicationWindow::onUpdTraCIStatus(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this, TraCIServer::getInstance() == nullptr ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    return 1;
}


long
GUIApplicationWindow::onCmdLocate(FXObject*, FXSelector sel, void*) {
    if (myMDIClient->numChildren() > 0) {
        GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            w->onCmdLocate(nullptr, sel, nullptr);
        }
    }
    return 1;
}


long
GUIApplicationWindow::onCmdShowStats(FXObject*, FXSelector, void*) {
    if (myMDIClient->numChildren() > 0) {
        GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
        GUINet::getGUIInstance()->getParameterWindow(*this, *w->getView());
    }
    return 1;
}


long
GUIApplicationWindow::onCmdAppSettings(FXObject*, FXSelector, void*) {
    GUIDialog_AppSettings* d = new GUIDialog_AppSettings(this);
    d->create();
    d->show(PLACEMENT_OWNER);
    return 1;
}


long
GUIApplicationWindow::onCmdGaming(FXObject*, FXSelector, void*) {
    if (myGLWindows.empty()) {
        return 1;
    }
    myAmGaming = !myAmGaming;
    myGLWindows[0]->getView()->getVisualisationSettings().gaming = myAmGaming;
    if (myAmGaming) {
        myMenuBar->hide();
        myStatusbar->hide();
        myToolBar1->hide();
        myToolBar2->hide();
        myToolBar4->hide();
        myToolBar5->hide();
        myToolBar6->show();
        myToolBar8->hide();
        myToolBar10->show();
        if (myTLSGame) {
            myToolBar7->show();
        } else {
            myToolBar9->show();
        }
        myMessageWindow->hide();
        myLCDLabel->setFgColor(MFXUtils::getFXColor(RGBColor::RED));
        myWaitingTimeLabel->setFgColor(MFXUtils::getFXColor(RGBColor::RED));
        myTimeLossLabel->setFgColor(MFXUtils::getFXColor(RGBColor::RED));
        myEmergencyVehicleLabel->setFgColor(MFXUtils::getFXColor(RGBColor::RED));
        myTotalDistanceLabel->setFgColor(MFXUtils::getFXColor(RGBColor::RED));
    } else {
        myMenuBar->show();
        myStatusbar->show();
        myToolBar1->show();
        myToolBar2->show();
        myToolBar4->show();
        myToolBar5->show();
        myToolBar6->hide();
        myToolBar7->hide();
        myToolBar8->show();
        myToolBar9->hide();
        myToolBar10->hide();
        myMessageWindow->show();
        myLCDLabel->setFgColor(MFXUtils::getFXColor(RGBColor::GREEN));
    }
    if (myMDIClient->numChildren() > 0) {
        GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            w->setToolBarVisibility(!myAmGaming && !myAmFullScreen);
        }
    }
    update();
    return 1;
}


long
GUIApplicationWindow::onCmdFullScreen(FXObject*, FXSelector, void*) {
    if (myGLWindows.empty()) {
        return 1;
    }
    myAmFullScreen = !myAmFullScreen;
    if (myAmFullScreen) {
        getApp()->reg().writeIntEntry("SETTINGS", "x", getX());
        getApp()->reg().writeIntEntry("SETTINGS", "y", getY());
        getApp()->reg().writeIntEntry("SETTINGS", "width", getWidth());
        getApp()->reg().writeIntEntry("SETTINGS", "height", getHeight());
        maximize();
        setDecorations(DECOR_NONE);
        place(PLACEMENT_MAXIMIZED);
        myMenuBar->hide();
        myStatusbar->hide();
        myToolBar1->hide();
        myToolBar2->hide();
        myToolBar3->hide();
        myToolBar4->hide();
        myToolBar5->hide();
        myToolBar6->hide();
        myToolBar7->hide();
        myToolBar8->hide();
        myMessageWindow->hide();
        if (myMDIClient->numChildren() > 0) {
            GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
            if (w != nullptr) {
                w->setToolBarVisibility(false);
            }
        }
        update();
    } else {
        place(PLACEMENT_VISIBLE);
        setDecorations(DECOR_ALL);
        restore();
        myToolBar3->show();
        myAmGaming = !myAmGaming;
        onCmdGaming(nullptr, 0, nullptr);
        setWidth(getApp()->reg().readIntEntry("SETTINGS", "width", 600));
        setHeight(getApp()->reg().readIntEntry("SETTINGS", "height", 400));
        setX(getApp()->reg().readIntEntry("SETTINGS", "x", 150));
        setY(getApp()->reg().readIntEntry("SETTINGS", "y", 150));
    }
    return 1;
}


long
GUIApplicationWindow::onCmdListInternal(FXObject*, FXSelector, void*) {
    myListInternal = !myListInternal;
    return 1;
}


long
GUIApplicationWindow::onCmdListParking(FXObject*, FXSelector, void*) {
    myListParking = !myListParking;
    return 1;
}

long
GUIApplicationWindow::onCmdListTeleporting(FXObject*, FXSelector, void*) {
    myListTeleporting = !myListTeleporting;
    return 1;
}


long
GUIApplicationWindow::onCmdNewView(FXObject*, FXSelector, void*) {
    openNewView(GUISUMOViewParent::VIEW_2D_OPENGL);
    return 1;
}


#ifdef HAVE_OSG
long
GUIApplicationWindow::onCmdNewOSG(FXObject*, FXSelector, void*) {
    openNewView(GUISUMOViewParent::VIEW_3D_OSG);
    return 1;
}
#endif


long
GUIApplicationWindow::onCmdAbout(FXObject*, FXSelector, void*) {
    GUIDialog_AboutSUMO* about = new GUIDialog_AboutSUMO(this);
    about->create();
    about->show(PLACEMENT_OWNER);
    return 1;
}


long GUIApplicationWindow::onClipboardRequest(FXObject* /* sender */, FXSelector /* sel */, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    FXString string = GUIUserIO::clipped.c_str();
    setDNDData(FROM_CLIPBOARD, event->target, string);
    return 1;
}


long
GUIApplicationWindow::onLoadThreadEvent(FXObject*, FXSelector, void*) {
    eventOccurred();
    return 1;
}


long
GUIApplicationWindow::onRunThreadEvent(FXObject*, FXSelector, void*) {
    eventOccurred();
    return 1;
}


void
GUIApplicationWindow::eventOccurred() {
    while (!myEvents.empty()) {
        // get the next event
        GUIEvent* e = myEvents.top();
        myEvents.pop();
        // process
        switch (e->getOwnType()) {
            case EVENT_SIMULATION_LOADED:
                handleEvent_SimulationLoaded(e);
                break;
            case EVENT_SIMULATION_STEP:
                if (myRunThread->simulationAvailable()) { // avoid race-condition related crash if reload was pressed
                    handleEvent_SimulationStep(e);
                }
                break;
            case EVENT_MESSAGE_OCCURRED:
            case EVENT_WARNING_OCCURRED:
            case EVENT_ERROR_OCCURRED:
            case EVENT_DEBUG_OCCURRED:
            case EVENT_GLDEBUG_OCCURRED:
            case EVENT_STATUS_OCCURRED:
                handleEvent_Message(e);
                break;
            case EVENT_SIMULATION_ENDED:
                handleEvent_SimulationEnded(e);
                break;
            default:
                break;
        }
        delete e;
    }
    myToolBar2->forceRefresh();
    myToolBar3->forceRefresh();
}


void
GUIApplicationWindow::handleEvent_SimulationLoaded(GUIEvent* e) {
    myAmLoading = false;
    GUIEvent_SimulationLoaded* ec = static_cast<GUIEvent_SimulationLoaded*>(e);
    // check whether the loading was successfull
    if (ec->myNet == nullptr) {
        // report failure
        setStatusBarText("Loading of '" + ec->myFile + "' failed!");
        if (GUIGlobals::gQuitOnEnd) {
            closeAllWindows();
            getApp()->exit(1);
        }
    } else {
        // initialise simulation thread
        if (!myRunThread->init(ec->myNet, ec->myBegin, ec->myEnd)) {
            if (GUIGlobals::gQuitOnEnd) {
                closeAllWindows();
                getApp()->exit(1);
            }
        } else {
            // report success
            setStatusBarText("'" + ec->myFile + "' loaded.");
            setWindowSizeAndPos();
            myWasStarted = false;
            myHaveNotifiedAboutSimEnd = false;
            // initialise views
            myViewNumber = 0;
            const GUISUMOViewParent::ViewType defaultType = ec->myOsgView ? GUISUMOViewParent::VIEW_3D_OSG : GUISUMOViewParent::VIEW_2D_OPENGL;
            if (ec->mySettingsFiles.size() > 0) {
                // open a view for each file and apply settings
                for (std::vector<std::string>::const_iterator it = ec->mySettingsFiles.begin(); it != ec->mySettingsFiles.end(); ++it) {
                    GUISettingsHandler settings(*it);
                    GUISUMOViewParent::ViewType vt = defaultType;
                    if (settings.getViewType() == "osg" || settings.getViewType() == "3d") {
                        vt = GUISUMOViewParent::VIEW_3D_OSG;
                    }
                    if (settings.getViewType() == "opengl" || settings.getViewType() == "2d") {
                        vt = GUISUMOViewParent::VIEW_2D_OPENGL;
                    }
                    GUISUMOAbstractView* view = openNewView(vt);
                    if (view == nullptr) {
                        break;
                    }
                    std::string settingsName = settings.addSettings(view);
                    view->addDecals(settings.getDecals());
                    settings.applyViewport(view);
                    settings.setSnapshots(view);
                    if (settings.getDelay() > 0.) {
                        mySimDelay = settings.getDelay();
                    }
                    if (settings.getBreakpoints().size() > 0) {
                        myRunThread->getBreakpointLock().lock();
                        myRunThread->getBreakpoints().assign(settings.getBreakpoints().begin(), settings.getBreakpoints().end());
                        myRunThread->getBreakpointLock().unlock();
                    }
                    if (!OptionsCont::getOptions().isDefault("breakpoints")) {
                        std::vector<SUMOTime> breakpoints;
                        for (const std::string& val : OptionsCont::getOptions().getStringVector("breakpoints")) {
                            breakpoints.push_back(string2time(val));
                        }
                        std::sort(breakpoints.begin(), breakpoints.end());
                        myRunThread->getBreakpointLock().lock();
                        myRunThread->getBreakpoints().assign(breakpoints.begin(), breakpoints.end());
                        myRunThread->getBreakpointLock().unlock();
                    }
                    myJamSounds = settings.getEventDistribution("jam");
                    myCollisionSounds = settings.getEventDistribution("collision");
                    if (settings.getJamSoundTime() > 0) {
                        myJamSoundTime = settings.getJamSoundTime();
                    }
                }
            } else {
                openNewView(defaultType);
            }
            if (!OptionsCont::getOptions().isDefault("breakpoints")) {
                std::vector<SUMOTime> breakpoints;
                for (const std::string& val : OptionsCont::getOptions().getStringVector("breakpoints")) {
                    breakpoints.push_back(string2time(val));
                }
                std::sort(breakpoints.begin(), breakpoints.end());
                myRunThread->getBreakpointLock().lock();
                myRunThread->getBreakpoints().assign(breakpoints.begin(), breakpoints.end());
                myRunThread->getBreakpointLock().unlock();
            }

            if (OptionsCont::getOptions().getBool("game")) {
                if (OptionsCont::getOptions().getString("game.mode") == "tls") {
                    myTLSGame = true;
                    setTitle("SUMO Interactive Traffic Light");
                } else {
                    myTLSGame = false;
                    setTitle("SUMO Interactive Demand-Responsive-Transport");
                }
                onCmdGaming(nullptr, 0, nullptr);
            } else {
                // set simulation name on the caption
                setTitle(MFXUtils::getTitleText("SUMO " VERSION_STRING, ec->myFile.c_str()));
            }
            if (ec->myViewportFromRegistry) {
                Position off;
                off.set(getApp()->reg().readRealEntry("viewport", "x"),
                        getApp()->reg().readRealEntry("viewport", "y"),
                        getApp()->reg().readRealEntry("viewport", "z"));
                Position p(off.x(), off.y(), 0);
                GUISUMOAbstractView* view = myGLWindows[0]->getView();
                view->setViewportFromToRot(off, p, 0);
            }
            // set simulation step begin information
            myLCDLabel->setText("----------------");
            for (std::vector<FXButton*>::const_iterator it = myStatButtons.begin(); it != myStatButtons.end(); ++it) {
                (*it)->setText("-");
            }
            // initialize scale from options unless already set in the UI
            if (myDemandScaleSpinner->getValue() == 1 || !OptionsCont::getOptions().isDefault("scale")) {
                myDemandScaleSpinner->setValue(OptionsCont::getOptions().getFloat("scale"));
            }
        }
    }
    getApp()->endWaitCursor();
    // start if wished
    if (GUIGlobals::gRunAfterLoad && ec->myNet != nullptr && myRunThread->simulationIsStartable()) {
        onCmdStart(nullptr, 0, nullptr);
    }
    update();
}


void
GUIApplicationWindow::handleEvent_SimulationStep(GUIEvent*) {
    updateTimeLCD(myRunThread->getNet().getCurrentTimeStep());
    const int running = myRunThread->getNet().getVehicleControl().getRunningVehicleNo();
    const int backlog = myRunThread->getNet().getInsertionControl().getWaitingVehicleNo();
    if (backlog > running) {
        if (myStatButtons.front()->getIcon() == GUIIconSubSys::getIcon(GUIIcon::GREENVEHICLE)) {
            myStatButtons.front()->setIcon(GUIIconSubSys::getIcon(GUIIcon::YELLOWVEHICLE));
        }
    } else {
        if (myStatButtons.front()->getIcon() == GUIIconSubSys::getIcon(GUIIcon::YELLOWVEHICLE)) {
            myStatButtons.front()->setIcon(GUIIconSubSys::getIcon(GUIIcon::GREENVEHICLE));
        }
    }
    myStatButtons.front()->setText(toString(running).c_str());
    if (myRunThread->getNet().hasPersons()) {
        if (!myStatButtons[1]->shown()) {
            myStatButtons[1]->show();
        }
        myStatButtons[1]->setText(toString(myRunThread->getNet().getPersonControl().getRunningNumber()).c_str());
    }
    if (myRunThread->getNet().hasContainers()) {
        if (!myStatButtons[2]->shown()) {
            myStatButtons[2]->show();
        }
        myStatButtons[2]->setText(toString(myRunThread->getNet().getContainerControl().getRunningNumber()).c_str());
    }
    if (myAmGaming) {
        if (myTLSGame) {
            checkGamingEvents();
        } else {
            checkGamingEventsDRT();
        }
    }
    if (myRunThread->simulationIsStartable()) {
        getApp()->forceRefresh(); // restores keyboard focus
    }
    updateChildren();
    update();
}


void
GUIApplicationWindow::handleEvent_Message(GUIEvent* e) {
    GUIEvent_Message* ec = static_cast<GUIEvent_Message*>(e);
    if (ec->getOwnType() == EVENT_STATUS_OCCURRED) {
        setStatusBarText(ec->getMsg());
    } else {
        myMessageWindow->appendMsg(ec->getOwnType(), ec->getMsg());
    }
}


void
GUIApplicationWindow::handleEvent_SimulationEnded(GUIEvent* e) {
    GUIEvent_SimulationEnded* ec = static_cast<GUIEvent_SimulationEnded*>(e);
    onCmdStop(nullptr, 0, nullptr);
    if (ec->getReason() == MSNet::SIMSTATE_LOADING) {
        onCmdReload(nullptr, 0, nullptr);
    } else if (GUIGlobals::gQuitOnEnd) {
        closeAllWindows();
        getApp()->exit(ec->getReason() == MSNet::SIMSTATE_ERROR_IN_SIM);
    } else if (GUIGlobals::gDemoAutoReload) {
        onCmdReload(nullptr, 0, nullptr);
    } else if (!myHaveNotifiedAboutSimEnd) {
        // build the text
        const std::string text = "Simulation ended at time: " + time2string(ec->getTimeStep()) +
                                 ".\nReason: " + MSNet::getStateMessage(ec->getReason()) +
                                 "\nDo you want to close all open files and views?";
        FXuint answer = FXMessageBox::question(this, MBOX_YES_NO, "Simulation ended", "%s", text.c_str());
        if (answer == 1) { //1:yes, 2:no, 4:esc
            closeAllWindows();
        }
        myHaveNotifiedAboutSimEnd = true;
    }
}


void
GUIApplicationWindow::checkGamingEvents() {
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt it = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();
#ifdef HAVE_DANGEROUS_SOUNDS // disable user-configurable command execution for public build
    if (myJamSounds.getOverallProb() > 0) {
        // play honking sound if some vehicle is waiting too long
        for (; it != end; ++it) {
            // XXX use impatience instead of waiting time ?
            if (it->second->getWaitingTime() > TIME2STEPS(myJamSoundTime)) {
                const std::string cmd = myJamSounds.get(&myGamingRNG);
                if (cmd != "") {
                    // yay! fun with dangerous commands... Never use this over the internet
                    SysUtils::runHiddenCommand(cmd);
                    // one sound per simulation step is enough
                    break;
                }
            }
        }
    }
    if (myCollisionSounds.getOverallProb() > 0) {
        int collisions = MSNet::getInstance()->getVehicleControl().getCollisionCount();
        if (myPreviousCollisionNumber != collisions) {
            const std::string cmd = myCollisionSounds.get(&myGamingRNG);
            if (cmd != "") {
                // yay! fun with dangerous commands... Never use this over the internet
                SysUtils::runHiddenCommand(cmd);
            }
            myPreviousCollisionNumber = collisions;
        }
    }
#endif

    // update performance indicators
    for (it = vc.loadedVehBegin(); it != end; ++it) {
        const MSVehicle* veh = dynamic_cast<MSVehicle*>(it->second);
        assert(veh != 0);
        if (veh->isOnRoad() && !veh->isStopped()) {
            const double vmax = MIN2(veh->getVehicleType().getMaxSpeed(), veh->getEdge()->getSpeedLimit());
            if (veh->getSpeed() < SUMO_const_haltingSpeed) {
                myWaitingTime += DELTA_T;
                if (veh->getID().rfind("emergency", 0) == 0) {
                    myEmergencyVehicleCount += DELTA_T;
                }
            }
            myTimeLoss += TIME2STEPS(TS * (vmax - veh->getSpeed()) / vmax); // may be negative with speedFactor > 1
        }

    }
    myWaitingTimeLabel->setText(time2string(myWaitingTime).c_str());
    myTimeLossLabel->setText(time2string(myTimeLoss).c_str());
    myEmergencyVehicleLabel->setText(time2string(myEmergencyVehicleCount).c_str());
}

void
GUIApplicationWindow::checkGamingEventsDRT() {
    // update performance indicators
    MSTransportableControl& pc = myRunThread->getNet().getPersonControl();
    myWaitingTime += pc.getWaitingForVehicleNumber() * DELTA_T;
    myWaitingTimeLabel->setText(time2string(myWaitingTime).c_str());

    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();
    for (auto it = vc.loadedVehBegin(); it != end; ++it) {
        const MSVehicle* veh = dynamic_cast<MSVehicle*>(it->second);
        assert(veh != 0);
        if (veh->isOnRoad() && !veh->isStopped()) {
            myTotalDistance += SPEED2DIST(veh->getSpeed());
        }
    }
    myTotalDistanceLabel->setText(toString(myTotalDistance / 100).c_str());
}

void
GUIApplicationWindow::loadConfigOrNet(const std::string& file) {
    if (!myAmLoading) {
        storeWindowSizeAndPos();
        getApp()->beginWaitCursor();
        myAmLoading = true;
        closeAllWindows();
        gSchemeStorage.saveViewport(0, 0, -1, 0); // recenter view
        myLoadThread->loadConfigOrNet(file);
        setStatusBarText("Loading '" + file + "'.");
        update();
    }
}


GUISUMOAbstractView*
GUIApplicationWindow::openNewView(GUISUMOViewParent::ViewType vt) {
    if (!myRunThread->simulationAvailable()) {
        myStatusbar->getStatusLine()->setText("No simulation loaded!");
        return nullptr;
    }
    GUISUMOAbstractView* oldView = nullptr;
    if (myMDIClient->numChildren() > 0) {
        GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            oldView = w->getView();
        }
    }
    std::string caption = "View #" + toString(myViewNumber++);
    FXuint opts = MDI_TRACKING;
    GUISUMOViewParent* w = new GUISUMOViewParent(myMDIClient, myMDIMenu, FXString(caption.c_str()),
            this, GUIIconSubSys::getIcon(GUIIcon::SUMO_MINI), opts, 10, 10, 300, 200);
    GUISUMOAbstractView* v = w->init(getBuildGLCanvas(), myRunThread->getNet(), vt);
    if (oldView != nullptr) {
        // copy viewport
        oldView->copyViewportTo(v);
    }
    w->create();
    if (myMDIClient->numChildren() == 1) {
        w->maximize();
    } else {
        myMDIClient->vertical(true);
    }
    myMDIClient->setActiveChild(w);

    return v;
}


FXGLCanvas*
GUIApplicationWindow::getBuildGLCanvas() const {
    if (myMDIClient->numChildren() == 0) {
        return nullptr;
    }
    GUISUMOViewParent* share_tmp1 =
        static_cast<GUISUMOViewParent*>(myMDIClient->childAtIndex(0));
    return share_tmp1->getBuildGLCanvas();
}


void
GUIApplicationWindow::closeAllWindows() {
    myTrackerLock.lock();
    myLCDLabel->setText("----------------");
    for (std::vector<FXButton*>::const_iterator it = myStatButtons.begin(); it != myStatButtons.end(); ++it) {
        (*it)->setText("-");
        if (it != myStatButtons.begin()) {
            (*it)->hide();
        }
    }
    // delete the simulation
    myRunThread->deleteSim();
    // reset the caption
    setTitle(MFXUtils::getTitleText("SUMO " VERSION_STRING));
    // remove trackers and other external windows (must be delayed until deleteSim)
    while (!myGLWindows.empty()) {
        delete myGLWindows.front();
    }
    // make a copy because deleting modifyes the vector;
    std::vector<FXMainWindow*> trackerWindows = myTrackerWindows;;
    for (FXMainWindow* const window : trackerWindows) {
        delete window;
    }
    myTrackerWindows.clear();
    // clear selected items
    gSelected.clear();
    // add a separator to the log
    myMessageWindow->addSeparator();
    myTrackerLock.unlock();
    // remove coordinate information
    myGeoCoordinate->setText("N/A");
    myCartesianCoordinate->setText("N/A");
    //
    GUITexturesHelper::clearTextures();
    GLHelper::resetFont();
    update();
}


FXCursor*
GUIApplicationWindow::getDefaultCursor() {
    return getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
}


SUMOTime
GUIApplicationWindow::getCurrentSimTime() const {
    return myRunThread->getNet().getCurrentTimeStep();
}


double
GUIApplicationWindow::getTrackerInterval() const {
    return GUIGlobals::gTrackerInterval;
}


void
GUIApplicationWindow::loadOnStartup() {
    loadConfigOrNet("");
}


void
GUIApplicationWindow::setStatusBarText(const std::string& text) {
    myStatusbar->getStatusLine()->setText(text.c_str());
    myStatusbar->getStatusLine()->setNormalText(text.c_str());
}


void
GUIApplicationWindow::addRecentFile(const FX::FXString& f) {
    myRecentFiles.appendFile(f);
}


void
GUIApplicationWindow::updateTimeLCD(SUMOTime time) {
    time -= DELTA_T; // synchronize displayed time with netstate output
    if (time < 0) {
        myLCDLabel->setText("----------------");
        return;
    }
    if (myAmGaming) {
        // show time counting backwards
        time = myRunThread->getSimEndTime() - time;
    }
    std::ostringstream str;
    str << std::setfill('0');
    const bool hideFraction = myAmGaming || DELTA_T % 1000 == 0;
    if (myShowTimeAsHMS) {
        SUMOTime day = time / 86400000;
        if (day > 0) {
            str << day << '-';
            time %= 86400000;
        }
        str << std::setw(2);
        str << time / 3600000 << '-';
        time %= 3600000;
        str << std::setw(2) << time / 60000 << '-';
        time %= 60000;
    }
    str << std::setw(2) << time / 1000;
    if (!hideFraction) {
        str << '.' << std::setw(3) << time % 1000;
    }
    myLCDLabel->setText(str.str().c_str());
}


long
GUIApplicationWindow::onKeyPress(FXObject* o, FXSelector sel, void* ptr) {
    const long handled = FXMainWindow::onKeyPress(o, sel, ptr);
    if (handled == 0 && myMDIClient->numChildren() > 0) {
        GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            w->onKeyPress(nullptr, sel, ptr);
        }
    }
    return 0;
}


long
GUIApplicationWindow::onKeyRelease(FXObject* o, FXSelector sel, void* ptr) {
    const long handled = FXMainWindow::onKeyRelease(o, sel, ptr);
    if (handled == 0 && myMDIClient->numChildren() > 0) {
        GUISUMOViewParent* w = dynamic_cast<GUISUMOViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            w->onKeyRelease(nullptr, sel, ptr);
        }
    }
    return 0;
}


void
GUIApplicationWindow::sendBlockingEvent(GUIEvent* event) {
    myEventMutex.lock();
    myEvents.push_back(event);
    myRunThreadEvent.signal();
    myEventCondition.wait(myEventMutex);
    myEventMutex.unlock();
}

void
GUIApplicationWindow::setBreakpoints(const std::vector<SUMOTime>& breakpoints) {
    if (myRunThread != nullptr) {
        myRunThread->getBreakpointLock().lock();
        myRunThread->getBreakpoints().assign(breakpoints.begin(), breakpoints.end());
        myRunThread->getBreakpointLock().unlock();
    }
}

const std::vector<SUMOTime>
GUIApplicationWindow::retrieveBreakpoints() const {
    myRunThread->getBreakpointLock().lock();
    std::vector<SUMOTime> result = myRunThread->getBreakpoints();
    myRunThread->getBreakpointLock().unlock();
    return result;
}


/****************************************************************************/
