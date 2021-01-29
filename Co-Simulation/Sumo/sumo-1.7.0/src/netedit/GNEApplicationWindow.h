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
/// @file    GNEApplicationWindow.h
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// The main window of Netedit (adapted from GUIApplicationWindow)
/****************************************************************************/
#pragma once

#include "GNEApplicationWindowHelper.h"


// ===========================================================================
// class definition
// ===========================================================================
/**
 * @class GNEApplicationWindow
 * @brief The main window of the Netedit.
 *
 * Contains the file opening support and a canvas to display the network in.
 *
 * Beside views on the simulation, shown within a MDI-window, the main window
 * may also have some further views (children) assigned which are stored
 * within a separate list.
 */
class GNEApplicationWindow : public GUIMainWindow, public MFXInterThreadEventClient {
    /// @brief FOX-declaration
    FXDECLARE(GNEApplicationWindow)

public:

    /**@brief Constructor
     * @param[in] a The FOX application
     * @param[in] configPattern The pattern used for loading configurations
     */
    GNEApplicationWindow(FXApp* a, const std::string& configPattern);

    /// @brief Destructor
    ~GNEApplicationWindow();

    /// @brief Creates the main window (required by FOX)
    void create();

    /// @brief load net on startup
    void loadOptionOnStartup();

    /// @brief build dependent
    void dependentBuild();

    /// @brief set text of the statusBar
    void setStatusBarText(const std::string& statusBarText);

    /// @brief called if the user selects Processing->compute junctions with volatile options
    long computeJunctionWithVolatileOptions();

    /// @brief enable save TLS Programs
    void enableSaveTLSProgramsMenu();

    /// @brief enable save additionals
    void enableSaveAdditionalsMenu();

    /// @brief disable save additionals
    void disableSaveAdditionalsMenu();

    /// @brief enable save demand elements
    void enableSaveDemandElementsMenu();

    /// @brief disable save demand elements
    void disableSaveDemandElementsMenu();

    /// @brief enable save data elements
    void enableSaveDataElementsMenu();

    /// @brief disable save data elements
    void disableSaveDataElementsMenu();

    /// @name Inter-thread event handling
    /// @{
    /// @brief called when an event occurred
    void eventOccurred();

    /// @brief handle event of type Network loaded
    void handleEvent_NetworkLoaded(GUIEvent* e);

    /// @brief hanlde event of type message
    void handleEvent_Message(GUIEvent* e);
    /// @}

    /// @name FOX-callbacks
    /// @{
    /// @brief called when the command/FXCall new network is executed
    long onCmdNewNetwork(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open configuration is executed
    long onCmdOpenConfiguration(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open network is executed
    long onCmdOpenNetwork(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open foreign is executed
    long onCmdOpenForeign(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open additionals is executed
    long onCmdOpenAdditionals(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open additionals is executed
    long onCmdOpenTLSPrograms(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open demand is executed
    long onCmdOpenDemandElements(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open data is executed
    long onCmdOpenDataElements(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall reload is executed
    long onCmdReload(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall open recent is executed
    long onCmdOpenRecent(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall close is executed
    long onCmdClose(FXObject*, FXSelector, void*);

    /// @brief Called on menu Edit->Visualization
    long onCmdEditViewScheme(FXObject*, FXSelector, void*);

    /// @brief Called on menu Edit->Viewport
    long onCmdEditViewport(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall locate is executed
    long onCmdLocate(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save network is executed
    long onCmdSaveNetwork(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save TLSPrograms is executed
    long onCmdSaveTLSPrograms(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save TLSPrograms as is executed
    long onCmdSaveTLSProgramsAs(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save additionals is executed
    long onCmdSaveAdditionals(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save additionals as is executed
    long onCmdSaveAdditionalsAs(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save demand elements is executed
    long onCmdSaveDemandElements(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save demand elements as is executed
    long onCmdSaveDemandElementsAs(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save data elements is executed
    long onCmdSaveDataElements(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save data elements as is executed
    long onCmdSaveDataElementsAs(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save network as is executed
    long onCmdSaveAsNetwork(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall needs network is executed
    long onUpdNeedsNetwork(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall needs front element is executed
    long onUpdNeedsFrontElement(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall reload is executed
    long onUpdReload(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall save network is executed
    long onUpdSaveNetwork(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall save additionals is executed
    long onUpdSaveAdditionals(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall save demand elements is executed
    long onUpdSaveDemandElements(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall save data elements is executed
    long onUpdSaveDataElements(FXObject*, FXSelector, void*);

    /// @brief called when the update/FXCall undo is executed
    long onUpdUndo(FXObject* obj, FXSelector sel, void* ptr);

    /// @brief called when the update/FXCall redo is executed
    long onUpdRedo(FXObject* obj, FXSelector sel, void* ptr);

    /// @brief called when the command/FXCall save as plain xml is executed
    long onCmdSaveAsPlainXML(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall save joined is executed
    long onCmdSaveJoined(FXObject*, FXSelector, void*);

    /// @brief called when a key is pressed
    long onKeyPress(FXObject* o, FXSelector sel, void* data);

    /// @brief called when a key is released
    long onKeyRelease(FXObject* o, FXSelector sel, void* data);

    /**@brief Called by FOX if the application shall be closed
     * @note Called either by FileMenu->Quit, the normal close-menu or SIGINT
     */
    long onCmdQuit(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall edit chosen is executed
    long onCmdEditChosen(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall show about dialog is executed
    long onCmdAbout(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall on update open executed
    long onUpdOpen(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall clear message windows is executed
    long onCmdClearMsgWindow(FXObject*, FXSelector, void*);

    /// @brief called when user toogle windows checkbox "load additionals"
    long onCmdLoadAdditionalsInSUMOGUI(FXObject*, FXSelector, void*);

    /// @brief called when user toogle windows checkbox "load demand"
    long onCmdLoadDemandInSUMOGUI(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall load thread is executed
    long onLoadThreadEvent(FXObject*, FXSelector, void*);

    /// @brief called when the command/FXCall clipboard request is executed
    long onClipboardRequest(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user hits an edit-supermode hotkey
    long onCmdSetSuperMode(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user hits an edit-mode hotkey
    long onCmdSetMode(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called when user press a process button (or a shortcut)
    long onCmdProcessButton(FXObject*, FXSelector sel, void*);

    /// @brief called if the user hints ctrl + T
    long onCmdOpenSUMOGUI(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user hits esc
    long onCmdAbort(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user hits del
    long onCmdDel(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user hits enter
    long onCmdEnter(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user hits backspace
    long onCmdBackspace(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user hits f
    long onCmdFocusFrame(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user press key combination Ctrl + G to toogle grid
    long onCmdToogleGrid(FXObject*, FXSelector, void*);

    /// @brief called if the user call set front element
    long onCmdSetFrontElement(FXObject*, FXSelector, void*);

    /// @brief called if the user press key combination Alt + <0-9>
    long onCmdToogleEditOptions(FXObject*, FXSelector, void*);

    /// @brief called if the user selects help->Documentation
    long onCmdHelp(FXObject* sender, FXSelector sel, void* ptr);

    /// @brief called if the user selects Processing->Configure Options
    long onCmdOptions(FXObject*, FXSelector, void*);

    /// @brief called when user press Ctrl+Z
    long onCmdUndo(FXObject*, FXSelector, void*);

    // @brief called when user press Ctrl+Y
    long onCmdRedo(FXObject*, FXSelector, void*);

    /// @brief called when user press Ctrl+Z
    long onCmdCut(FXObject*, FXSelector, void*);

    // @brief called when user press Ctrl+Y
    long onCmdCopy(FXObject*, FXSelector, void*);

    // @brief called when user press Ctrl+Y
    long onCmdPaste(FXObject*, FXSelector, void*);

    // @brief called when user press Ctrl+F1
    long onCmdSetTemplate(FXObject*, FXSelector, void*);

    // @brief called when user press Ctrl+F2
    long onCmdCopyTemplate(FXObject*, FXSelector, void*);

    // @brief called when user press Ctrl+F3
    long onCmdClearTemplate(FXObject*, FXSelector, void*);

    /// @}

    /// @name inherited from GUIMainWindow
    /// @{
    /// @brief get build OpenGL Canvas (due NETEDIT only uses a single View, it always return nullptr)
    FXGLCanvas* getBuildGLCanvas() const;

    /// @brief get current simulation time (pure virtual but we don't need it)
    SUMOTime getCurrentSimTime() const;

    /// @brief get current tracker interval (pure virtual but we don't need it)
    double getTrackerInterval() const;
    /// @}

    /// @brief get default cursor
    FXCursor* getDefaultCursor();

    /// @brief get pointer to undoList
    GNEUndoList* getUndoList();

    /// @brief get pointer to viewNet
    GNEViewNet* getViewNet();

    /// @brief get ToolbarsGrip
    GNEApplicationWindowHelper::ToolbarsGrip& getToolbarsGrip();

    /// @brief update control contents after undo/redo or recompute
    void updateControls();

    /// @brief update FXMenuCommands depending of supermode
    void updateSuperModeMenuCommands(const Supermode supermode);

    /// @brief disable undo-redo giving a string with the reason
    void disableUndoRedo(const std::string& reason);

    /// @brief disable undo-redo
    void enableUndoRedo();

    /// @brief check if undo-redo is enabled
    const std::string& isUndoRedoEnabled() const;

protected:
    /// @brief FOX needs this for static members
    GNEApplicationWindow();

    /// @brief the thread that loads the network
    GNELoadThread* myLoadThread;

    /// @brief information whether the gui is currently loading and the load-options shall be greyed out
    bool myAmLoading;

    /// @brief the submenus
    FXMenuPane* myFileMenu,
                *myFileMenuTLS,
                *myFileMenuAdditionals,
                *myFileMenuDemandElements,
                *myFileMenuDataElements,
                *myEditMenu,
                *myProcessingMenu,
                *myLocatorMenu,
                *myWindowsMenu,
                *myHelpMenu;

    /// @brief A window to display messages, warnings and error in
    GUIMessageWindow* myMessageWindow;

    /// @brief The splitter that divides the main window into view and the log window
    FXSplitter* myMainSplitter;

    /// @brief List of got requests
    FXSynchQue<GUIEvent*> myEvents;

    /// @brief io-event with the load-thread
    FXEX::FXThreadEvent myLoadThreadEvent;

    /// @brief check if had dependent build
    bool hadDependentBuild;

    /// @brief we are responsible for the net
    GNENet* myNet;

    /// @brief the one and only undo list
    GNEUndoList* myUndoList;

    /// @brief Input file pattern
    std::string myConfigPattern;

    /// @brief string to check if undo/redo list is enabled (a String is used to keep the disabling reason)
    std::string myUndoRedoListEnabled;

private:
    /// @brief Toolbars Grip
    GNEApplicationWindowHelper::ToolbarsGrip myToolbarsGrip;

    /// @brief MenuBarFile
    GNEApplicationWindowHelper::MenuBarFile myMenuBarFile;

    /// @brief File Menu Commands
    GNEApplicationWindowHelper::FileMenuCommands myFileMenuCommands;

    /// @brief Edit Menu Commands
    GNEApplicationWindowHelper::EditMenuCommands myEditMenuCommands;

    /// @brief Edit Menu Commands
    GNEApplicationWindowHelper::ProcessingMenuCommands myProcessingMenuCommands;

    /// @brief Locate Menu Commands
    GNEApplicationWindowHelper::LocateMenuCommands myLocateMenuCommands;

    /// @brief Windows Menu Commands
    GNEApplicationWindowHelper::WindowsMenuCommands myWindowsMenuCommands;

    /// @brief Supermode Commands
    GNEApplicationWindowHelper::SupermodeCommands mySupermodeCommands;

    /// @brief pointer to current view net
    GNEViewNet* myViewNet;

    /// @brief the prefix for the window title
    const FXString myTitlePrefix;

    /// @brief The menu used for the MDI-windows
    FXMDIMenu* myMDIMenu;

    /// @brief Builds the menu bar
    void fillMenuBar();

    /// @brief starts to load a netimport configuration or a network */
    void loadConfigOrNet(const std::string file, bool isNet, bool isReload = false, bool useStartupOptions = false, bool newNet = false);

    /// @brief this method closes all windows and deletes the current simulation */
    void closeAllWindows();

    /// @brief warns about unsaved changes and gives the user the option to abort
    bool continueWithUnsavedChanges(const std::string& operation);

    /// @brief warns about unsaved changes in additionals and gives the user the option to abort
    bool continueWithUnsavedAdditionalChanges(const std::string& operation);

    /// @brief warns about unsaved changes in demand elements and gives the user the option to abort
    bool continueWithUnsavedDemandElementChanges(const std::string& operation);
};
