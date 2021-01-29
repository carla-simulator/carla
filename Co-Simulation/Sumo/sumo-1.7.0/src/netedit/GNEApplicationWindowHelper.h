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
/// @file    GNEApplicationWindowHelper.h
/// @author  Pablo Alvarez Lopez
/// @date    mar 2020
///
// Functions from main window of NETEDIT
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOTime.h>
#include <utils/foxtools/FXSynchQue.h>
#include <utils/foxtools/FXThreadEvent.h>
#include <utils/foxtools/MFXInterThreadEventClient.h>
#include <utils/geom/Position.h>
#include <utils/gui/div/GUIMessageWindow.h>
#include <utils/gui/windows/GUIMainWindow.h>
#include <utils/shapes/ShapeHandler.h>

#include "GNEViewNetHelper.h"


// ===========================================================================
// class declarations
// ===========================================================================
class GNEApplicationWindow;
class GNELoadThread;
class GNEUndoList;
class GNENet;
class GNEViewNet;
class GUIEvent;


// ===========================================================================
// class definition
// ===========================================================================

struct GNEApplicationWindowHelper {

    /// @brief struct for griped toolbars
    struct ToolbarsGrip {

        /// @brief constructor
        ToolbarsGrip(GNEApplicationWindow* GNEApp);

        /// @brief build menu toolbar grips
        void buildMenuToolbarsGrip();

        /// @brief build toolbars grips
        void buildViewParentToolbarsGrips();

        /// @brief build toolbars grips
        void destroyParentToolbarsGrips();

        /// @brief The application menu bar (for file, edit, processing...)
        FXMenuBar* menu;

        /// @brief The application menu bar for supermodes (network and demand)
        FXMenuBar* superModes;

        /// @brief The application menu bar for save elements (NetworkElements, additionals and demand elements)
        FXMenuBar* saveElements;

        /// @brief The application menu bar for navigation (zoom, coloring...)
        FXMenuBar* navigation;

        /// @brief The application menu bar (for select, inspect...)
        FXMenuBar* modes;

        /// @brief The application menu bar for mode options (show connections, select edges...)
        FXMenuBar* modeOptions;

        /// @brief The application menu bar for mode options (show connections, select edges...)
        FXMenuBar* intervalBar;

        FXDockSite* myTopDock;
    private:
        /// @brief pointer to current GNEApplicationWindow
        GNEApplicationWindow* myGNEApp;
        /// @brief menu bar drag (for file, edit, processing...)
        FXToolBarShell* myToolBarShellMenu;

        /// @brief menu bar drag for modes (network and demand)
        FXToolBarShell* myToolBarShellSuperModes;

        /// @brief menu bar drag for save elements (NetworkElements, additionals and demand elements)
        FXToolBarShell* myToolBarShellSaveElements;

        /// @brief menu bar drag for navigation (Zoom, coloring...)
        FXToolBarShell* myToolBarShellNavigation;

        /// @brief menu bar drag for modes (select, inspect, delete...)
        FXToolBarShell* myToolBarShellModes;

        /// @brief menu bar drag for mode options (show connections, select edges...)
        FXToolBarShell* myToolBarShellModeOptions;

        /// @brief menu bar drag for interval bar
        FXToolBarShell* myToolBarShellIntervalBar;
    };

    /// @brief struct for menu bar file
    struct MenuBarFile {

        /// @brief constructor
        MenuBarFile(GNEApplicationWindow* GNEApp);

        /// @brief build recent files
        void buildRecentFiles(FXMenuPane* fileMenu);

        /// @brief List of recent config files
        FXRecentFiles myRecentConfigs;

        /// @brief List of recent nets
        FXRecentFiles myRecentNets;

    private:
        /// @brief pointer to current GNEApplicationWindow
        GNEApplicationWindow* myGNEApp;
    };

    /// @brief struct for File menu commands
    struct FileMenuCommands {

        /// @brief constructor
        FileMenuCommands(GNEApplicationWindow* GNEApp);

        /// @brief build menu commands
        void buildFileMenuCommands(FXMenuPane* fileMenu, FXMenuPane* fileMenuTLS, FXMenuPane* fileMenuAdditionals,
                                   FXMenuPane* fileMenuDemandElements, FXMenuPane* fileMenuDataElements);

        /// @brief FXMenuCommand for enable or disable save additionals
        FXMenuCommand* saveAdditionals;

        /// @brief FXMenuCommand for enable or disable save additionals As
        FXMenuCommand* saveAdditionalsAs;

        /// @brief FXMenuCommand for enable or disable save additionals
        FXMenuCommand* saveTLSPrograms;

        /// @brief FXMenuCommand for enable or disable save demand elements
        FXMenuCommand* saveDemandElements;

        /// @brief FXMenuCommand for enable or disable save demand elements as
        FXMenuCommand* saveDemandElementsAs;

        /// @brief FXMenuCommand for enable or disable save data elements
        FXMenuCommand* saveDataElements;

        /// @brief FXMenuCommand for enable or disable save data elements as
        FXMenuCommand* saveDataElementsAs;

    private:
        /// @brief pointer to current GNEApplicationWindows
        GNEApplicationWindow* myGNEApp;
    };

    /// @brief struct for edit menu commands
    struct EditMenuCommands {

        /// @brief struct for network menu commands
        struct NetworkMenuCommands {

            /// @brief constructor
            NetworkMenuCommands(const EditMenuCommands* editMenuCommandsParent);

            /// @brief build menu commands
            void buildNetworkMenuCommands(FXMenuPane* editMenu);

            /// @brief show all menu commands
            void showNetworkMenuCommands();

            /// @brief hide all menu commands
            void hideNetworkMenuCommands();

            /// @brief menu command for create edge
            FXMenuCommand* createEdgeMode;

            /// @brief menu command for move mode
            FXMenuCommand* moveMode;

            /// @brief menu command for delete mode
            FXMenuCommand* deleteMode;

            /// @brief menu command for inspect mode
            FXMenuCommand* inspectMode;

            /// @brief menu command for select mode
            FXMenuCommand* selectMode;

            /// @brief menu command for connect mode
            FXMenuCommand* connectMode;

            /// @brief menu command for prohibition mode
            FXMenuCommand* prohibitionMode;

            /// @brief menu command for TLS Mode
            FXMenuCommand* TLSMode;

            /// @brief menu command for additional mode
            FXMenuCommand* additionalMode;

            /// @brief menu command for crossing mode
            FXMenuCommand* crossingMode;

            /// @brief menu command for TAZ mode
            FXMenuCommand* TAZMode;

            /// @brief menu command for shape mode
            FXMenuCommand* shapeMode;

        private:
            /// @brief reference to EditMenuCommands
            const EditMenuCommands* myEditMenuCommandsParent;

            /// @brief separator between sets of FXMenuCommand
            FXMenuSeparator* myHorizontalSeparator;
        };

        /// @brief struct for Demand menu commands
        struct DemandMenuCommands {

            /// @brief constructor
            DemandMenuCommands(const EditMenuCommands* editMenuCommandsParent);

            /// @brief build menu commands
            void buildDemandMenuCommands(FXMenuPane* editMenu);

            /// @brief show all menu commands
            void showDemandMenuCommands();

            /// @brief hide all menu commands
            void hideDemandMenuCommands();

            /// @brief menu command for route mode
            FXMenuCommand* routeMode;

            /// @brief menu command for vehicle mode
            FXMenuCommand* vehicleMode;

            /// @brief menu command for vehicle type mode
            FXMenuCommand* vehicleTypeMode;

            /// @brief menu command for stop mode
            FXMenuCommand* stopMode;

            /// @brief menu command for person type mode
            FXMenuCommand* personTypeMode;

            /// @brief menu command for person mode
            FXMenuCommand* personMode;

            /// @brief menu command for person plan mode
            FXMenuCommand* personPlanMode;

        private:
            /// @brief reference to EditMenuCommands
            const EditMenuCommands* myEditMenuCommandsParent;

            /// @brief separator between sets of FXMenuCommand
            FXMenuSeparator* myHorizontalSeparator;
        };


        /// @brief struct for Data menu commands
        struct DataMenuCommands {

            /// @brief constructor
            DataMenuCommands(const EditMenuCommands* editMenuCommandsParent);

            /// @brief build menu commands
            void buildDataMenuCommands(FXMenuPane* editMenu);

            /// @brief show all menu commands
            void showDataMenuCommands();

            /// @brief hide all menu commands
            void hideDataMenuCommands();

            /// @brief menu command for edge mode
            FXMenuCommand* edgeData;

            /// @brief menu command for edge rel mode
            FXMenuCommand* edgeRelData;

            /// @brief menu command for TAZ rel mode
            FXMenuCommand* TAZRelData;

        private:
            /// @brief reference to EditMenuCommands
            const EditMenuCommands* myEditMenuCommandsParent;

            /// @brief separator between sets of FXMenuCommand
            FXMenuSeparator* myHorizontalSeparator;
        };

        /// @brief constructor
        EditMenuCommands(GNEApplicationWindow* GNEApp);

        /// @brief build edit menu commands
        void buildEditMenuCommands(FXMenuPane* editMenu);

    private:
        /// @brief pointer to current GNEApplicationWindows
        GNEApplicationWindow* myGNEApp;

    public:
        /// @brief Network Menu Commands
        NetworkMenuCommands networkMenuCommands;

        /// @brief Demand Menu Commands
        DemandMenuCommands demandMenuCommands;

        /// @brief Data Menu Commands
        DataMenuCommands dataMenuCommands;

        /// @brief FXMenuCommand for undo last change
        FXMenuCommand* undoLastChange;

        /// @brief FXMenuCommand for redo last change
        FXMenuCommand* redoLastChange;

        /// @brief FXMenuCommand for edit view scheme
        FXMenuCommand* editViewScheme;

        /// @brief FXMenuCommand for edit view port
        FXMenuCommand* editViewPort;

        /// @brief FXMenuCommand for toogle grid
        FXMenuCommand* toogleGrid;

        /// @brief FXMenuCommand for clear Front element
        FXMenuCommand* clearFrontElement;

        /// @brief menu check for load additionals in SUMO GUI
        FXMenuCheck* loadAdditionalsInSUMOGUI;

        /// @brief menu check for load demand in SUMO GUI
        FXMenuCheck* loadDemandInSUMOGUI;

        /// @brief FXMenuCommand for open in SUMO GUI
        FXMenuCommand* openInSUMOGUI;
    };

    /// @brief struct for processing menu commands
    struct ProcessingMenuCommands {

        /// @brief constructor
        ProcessingMenuCommands(GNEApplicationWindow* GNEApp);

        /// @brief build menu commands
        void buildProcessingMenuCommands(FXMenuPane* editMenu);

        /// @brief show network processing menu commands
        void showNetworkProcessingMenuCommands();

        /// @brief show network processing menu commands
        void hideNetworkProcessingMenuCommands();

        /// @brief show demand processing menu commands
        void showDemandProcessingMenuCommands();

        /// @brief show demand processing menu commands
        void hideDemandProcessingMenuCommands();

        /// @brief show data processing menu commands
        void showDataProcessingMenuCommands();

        /// @brief show data processing menu commands
        void hideDataProcessingMenuCommands();

        /// @name Processing FXMenuCommands for Network mode
        /// @{
        /// @brief FXMenuCommand for compute network
        FXMenuCommand* computeNetwork;

        /// @brief FXMenuCommand for compute network with volatile options
        FXMenuCommand* computeNetworkVolatile;

        /// @brief FXMenuCommand for clean junctions without edges
        FXMenuCommand* cleanJunctions;

        /// @brief FXMenuCommand for join selected junctions
        FXMenuCommand* joinJunctions;

        /// @brief FXMenuCommand for clear invalid crosings
        FXMenuCommand* clearInvalidCrossings;
        /// @}

        /// @name Processing FXMenuCommands for Demand mode
        /// @{
        /// @brief FXMenuCommand for compute demand elements
        FXMenuCommand* computeDemand;

        /// @brief FXMenuCommand for clean routes without vehicles
        FXMenuCommand* cleanRoutes;

        /// @brief FXMenuCommand for join routes
        FXMenuCommand* joinRoutes;

        /// @brief FXMenuCommand for clear invalid demand elements
        FXMenuCommand* clearInvalidDemandElements;
        /// @}

        /// @name Processing FXMenuCommands for Data mode
        /// @{

        /// @}

        /// @brief FXMenuCommand for open option menus
        FXMenuCommand* optionMenus;

    private:
        /// @brief pointer to current GNEApplicationWindows
        GNEApplicationWindow* myGNEApp;
    };

    /// @brief struct for locate menu commands
    struct LocateMenuCommands {

        /// @brief constructor
        LocateMenuCommands(GNEApplicationWindow* GNEApp);

        /// @brief build menu commands
        void buildLocateMenuCommands(FXMenuPane* locateMenu);

    private:
        /// @brief pointer to current GNEApplicationWindows
        GNEApplicationWindow* myGNEApp;
    };

    /// @brief struct for windows menu commands
    struct WindowsMenuCommands {

        /// @brief constructor
        WindowsMenuCommands(GNEApplicationWindow* GNEApp);

        /// @brief build menu commands
        void buildWindowsMenuCommands(FXMenuPane* windowsMenu, FXStatusBar* statusbar, GUIMessageWindow* messageWindow);

    private:
        /// @brief pointer to current GNEApplicationWindows
        GNEApplicationWindow* myGNEApp;
    };

    /// @brief struct for supermode commands
    struct SupermodeCommands {

        /// @brief constructor
        SupermodeCommands(GNEApplicationWindow* GNEApp);

        /// @brief build menu commands
        void buildSupermodeCommands(FXMenuPane* editMenu);

        /// @brief show all menu commands
        void showSupermodeCommands();

        /// @brief hide all menu commands
        void hideSupermodeCommands();

        /// @brief FXMenuCommand for network supermode
        FXMenuCommand* networkMode;

        /// @brief FXMenuCommand for demand supermode
        FXMenuCommand* demandMode;

        /// @brief FXMenuCommand for data supermode
        FXMenuCommand* dataMode;

    private:
        /// @brief pointer to current GNEApplicationWindows
        GNEApplicationWindow* myGNEApp;

        /// @brief separator between sets of FXMenuCommand
        FXMenuSeparator* myHorizontalSeparator;
    };

    /// @brief toogle edit options Network menu commands (called in GNEApplicationWindow::onCmdToogleEditOptions)
    static bool toogleEditOptionsNetwork(GNEViewNet* viewNet, const FXMenuCheck* menuCheck,
                                         const int numericalKeyPressed, FXObject* obj, FXSelector sel);

    /// @brief toogle edit options Demand menu commands (called in GNEApplicationWindow::onCmdToogleEditOptions)
    static bool toogleEditOptionsDemand(GNEViewNet* viewNet, const FXMenuCheck* menuCheck,
                                        const int numericalKeyPressed, FXObject* obj, FXSelector sel);

    /// @brief toogle edit options Data menu commands (called in GNEApplicationWindow::onCmdToogleEditOptions)
    static bool toogleEditOptionsData(GNEViewNet* viewNet, const FXMenuCheck* menuCheck,
                                      const int numericalKeyPressed, FXObject* obj, FXSelector sel);
};
