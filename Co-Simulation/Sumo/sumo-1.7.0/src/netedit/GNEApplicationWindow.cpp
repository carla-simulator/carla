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
/// @file    GNEApplicationWindow.cpp
/// @author  Pablo Alvarez Lopez
/// @date    mar 2020
///
// Functions from main window of NETEDIT
/****************************************************************************/
#include <netbuild/NBFrame.h>
#include <netedit/elements/additional/GNEAdditionalHandler.h>
#include <netedit/elements/data/GNEDataHandler.h>
#include <netedit/elements/demand/GNERouteHandler.h>
#include <netedit/dialogs/GNEAbout.h>
#include <netedit/frames/network/GNETAZFrame.h>
#include <netedit/frames/network/GNETLSEditorFrame.h>
#include <netedit/frames/common/GNEInspectorFrame.h>
#include <netimport/NIFrame.h>
#include <netwrite/NWFrame.h>
#include <utils/common/SystemFrame.h>
#include <utils/foxtools/FXLinkLabel.h>
#include <utils/gui/cursors/GUICursorSubSys.h>
#include <utils/gui/shortcuts/GUIShortcutsSubSys.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/div/GUIDialog_GLChosenEditor.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIUserIO.h>
#include <utils/gui/events/GUIEvent_Message.h>
#include <utils/gui/images/GUITextureSubSys.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/gui/settings/GUISettingsHandler.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUIDialog_Options.h>
#include <utils/gui/windows/GUIPerspectiveChanger.h>
#include <utils/options/OptionsCont.h>
#include <utils/xml/XMLSubSys.h>

#include "GNEApplicationWindow.h"
#include "GNEEvent_NetworkLoaded.h"
#include "GNELoadThread.h"
#include "GNENet.h"
#include "GNEViewNet.h"
#include "GNEUndoList.h"
#include "GNEViewParent.h"

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

// ===========================================================================
// FOX-declarations
// ===========================================================================

FXDEFMAP(GNEApplicationWindow) GNEApplicationWindowMap[] = {
    // quit calls
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_Q_CLOSE,    GNEApplicationWindow::onCmdQuit),
    FXMAPFUNC(SEL_SIGNAL,   MID_HOTKEY_CTRL_Q_CLOSE,    GNEApplicationWindow::onCmdQuit),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_F4_CLOSE,    GNEApplicationWindow::onCmdQuit),
    FXMAPFUNC(SEL_CLOSE,    MID_WINDOW,                 GNEApplicationWindow::onCmdQuit),

    // toolbar file
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_N_NEWNETWORK,                       GNEApplicationWindow::onCmdNewNetwork),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_N_NEWNETWORK,                       GNEApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_COMMAND,  MID_OPEN_NETWORK,                                   GNEApplicationWindow::onCmdOpenNetwork),
    FXMAPFUNC(SEL_UPDATE,   MID_OPEN_NETWORK,                                   GNEApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_COMMAND,  MID_OPEN_CONFIG,                                    GNEApplicationWindow::onCmdOpenConfiguration),
    FXMAPFUNC(SEL_UPDATE,   MID_OPEN_CONFIG,                                    GNEApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TOOLBARFILE_OPENFOREIGN,                    GNEApplicationWindow::onCmdOpenForeign),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_TOOLBARFILE_OPENFOREIGN,                    GNEApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_COMMAND,  MID_RECENTFILE,                                     GNEApplicationWindow::onCmdOpenRecent),
    FXMAPFUNC(SEL_UPDATE,   MID_RECENTFILE,                                     GNEApplicationWindow::onUpdOpen),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_R_RELOAD,                           GNEApplicationWindow::onCmdReload),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_R_RELOAD,                           GNEApplicationWindow::onUpdReload),
    // network
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_S_STOPSIMULATION_SAVENETWORK,       GNEApplicationWindow::onCmdSaveNetwork),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_S_STOPSIMULATION_SAVENETWORK,       GNEApplicationWindow::onUpdSaveNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_SHIFT_S_SAVENETWORK_AS,             GNEApplicationWindow::onCmdSaveAsNetwork),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_SHIFT_S_SAVENETWORK_AS,             GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_L_SAVEASPLAINXML,                   GNEApplicationWindow::onCmdSaveAsPlainXML),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_L_SAVEASPLAINXML,                   GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_J_SAVEJOINEDJUNCTIONS,              GNEApplicationWindow::onCmdSaveJoined),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_J_SAVEJOINEDJUNCTIONS,              GNEApplicationWindow::onUpdNeedsNetwork),
    // TLS
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_K_OPENTLSPROGRAMS,                  GNEApplicationWindow::onCmdOpenTLSPrograms),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_K_OPENTLSPROGRAMS,                  GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_SHIFT_K_SAVETLS,                    GNEApplicationWindow::onCmdSaveTLSPrograms),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_SHIFT_K_SAVETLS,                    GNEApplicationWindow::onUpdNeedsNetwork),
    // additionals
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_A_STARTSIMULATION_OPENADDITIONALS,  GNEApplicationWindow::onCmdOpenAdditionals),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_A_STARTSIMULATION_OPENADDITIONALS,  GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_SHIFT_A_SAVEADDITIONALS,            GNEApplicationWindow::onCmdSaveAdditionals),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_SHIFT_A_SAVEADDITIONALS,            GNEApplicationWindow::onUpdSaveAdditionals),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TOOLBARFILE_SAVEADDITIONALS_AS,             GNEApplicationWindow::onCmdSaveAdditionalsAs),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_TOOLBARFILE_SAVEADDITIONALS_AS,             GNEApplicationWindow::onUpdNeedsNetwork),
    // demand elements
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_D_SINGLESIMULATIONSTEP_OPENDEMANDELEMENTS,  GNEApplicationWindow::onCmdOpenDemandElements),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_D_SINGLESIMULATIONSTEP_OPENDEMANDELEMENTS,  GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_SHIFT_D_SAVEDEMANDELEMENTS,                 GNEApplicationWindow::onCmdSaveDemandElements),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_SHIFT_D_SAVEDEMANDELEMENTS,                 GNEApplicationWindow::onUpdSaveDemandElements),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TOOLBARFILE_SAVEDEMAND_AS,                          GNEApplicationWindow::onCmdSaveDemandElementsAs),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_TOOLBARFILE_SAVEDEMAND_AS,                          GNEApplicationWindow::onUpdNeedsNetwork),
    // data elements
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_B_EDITBREAKPOINT_OPENDATAELEMENTS,  GNEApplicationWindow::onCmdOpenDataElements),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_B_EDITBREAKPOINT_OPENDATAELEMENTS,  GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_SHIFT_B_SAVEDATAELEMENTS,           GNEApplicationWindow::onCmdSaveDataElements),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_SHIFT_B_SAVEDATAELEMENTS,           GNEApplicationWindow::onUpdSaveDataElements),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TOOLBARFILE_SAVEDATA_AS,                    GNEApplicationWindow::onCmdSaveDataElementsAs),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_TOOLBARFILE_SAVEDATA_AS,                    GNEApplicationWindow::onUpdNeedsNetwork),
    // other
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TOOLBARFILE_SAVETLSPROGRAMS_AS,             GNEApplicationWindow::onCmdSaveTLSProgramsAs),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_TOOLBARFILE_SAVETLSPROGRAMS_AS,             GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_W_CLOSESIMULATION,                  GNEApplicationWindow::onCmdClose),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_W_CLOSESIMULATION,                  GNEApplicationWindow::onUpdNeedsNetwork),

    // Toolbar supermode
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F2_SUPERMODE_NETWORK,    GNEApplicationWindow::onCmdSetSuperMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F3_SUPERMODE_DEMAND,     GNEApplicationWindow::onCmdSetSuperMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F4_SUPERMODE_DATA,       GNEApplicationWindow::onCmdSetSuperMode),

    // Toolbar edit
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_E_MODES_EDGE_EDGEDATA,               GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_M_MODES_MOVE,                        GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_D_MODES_DELETE,                      GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_I_MODES_INSPECT,                     GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_S_MODES_SELECT,                      GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_C_MODES_CONNECT_PERSONPLAN,          GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_T_MODES_TLS_VTYPE,                   GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_A_MODES_ADDITIONAL_STOP,             GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_R_MODES_CROSSING_ROUTE_EDGERELDATA,  GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_Z_MODES_TAZ_TAZREL,                  GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_P_MODES_POLYGON_PERSON,              GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_V_MODES_VEHICLE,                     GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_W_MODES_PROHIBITION_PERSONTYPE,      GNEApplicationWindow::onCmdSetMode),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F9_EDIT_VIEWSCHEME,                  GNEApplicationWindow::onCmdEditViewScheme),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_F9_EDIT_VIEWSCHEME,                  GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_I_EDITVIEWPORT,                 GNEApplicationWindow::onCmdEditViewport),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_I_EDITVIEWPORT,                 GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_Z_UNDO,                         GNEApplicationWindow::onCmdUndo),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_Z_UNDO,                         GNEApplicationWindow::onUpdUndo),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_Y_REDO,                         GNEApplicationWindow::onCmdRedo),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_Y_REDO,                         GNEApplicationWindow::onUpdRedo),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_G_GAMINGMODE_TOOGLEGRID,        GNEApplicationWindow::onCmdToogleGrid),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_G_GAMINGMODE_TOOGLEGRID,        GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F11_FRONTELEMENT,                    GNEApplicationWindow::onCmdSetFrontElement),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_F11_FRONTELEMENT,                    GNEApplicationWindow::onUpdNeedsFrontElement),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TOOLBAREDIT_LOADADDITIONALS,            GNEApplicationWindow::onCmdLoadAdditionalsInSUMOGUI),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_TOOLBAREDIT_LOADADDITIONALS,            GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TOOLBAREDIT_LOADDEMAND,                 GNEApplicationWindow::onCmdLoadDemandInSUMOGUI),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_TOOLBAREDIT_LOADDEMAND,                 GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_T_OPENSUMONETEDIT,              GNEApplicationWindow::onCmdOpenSUMOGUI),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_CTRL_T_OPENSUMONETEDIT,              GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_X_CUT,                          GNEApplicationWindow::onCmdCut),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_C_COPY,                         GNEApplicationWindow::onCmdCopy),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_CTRL_V_PASTE,                        GNEApplicationWindow::onCmdPaste),

    // Toolbar processing
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F5_COMPUTE_NETWORK_DEMAND,                   GNEApplicationWindow::onCmdProcessButton),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_F5_COMPUTE_NETWORK_DEMAND,                   GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_SHIFT_F5_COMPUTEJUNCTIONS_VOLATILE,          GNEApplicationWindow::onCmdProcessButton),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_SHIFT_F5_COMPUTEJUNCTIONS_VOLATILE,          GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F6_CLEAN_SOLITARYJUNCTIONS_UNUSEDROUTES,     GNEApplicationWindow::onCmdProcessButton),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_F6_CLEAN_SOLITARYJUNCTIONS_UNUSEDROUTES,     GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F7_JOIN_SELECTEDJUNCTIONS_ROUTES,            GNEApplicationWindow::onCmdProcessButton),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_F7_JOIN_SELECTEDJUNCTIONS_ROUTES,            GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F8_CLEANINVALID_CROSSINGS_DEMANDELEMENTS,    GNEApplicationWindow::onCmdProcessButton),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_F8_CLEANINVALID_CROSSINGS_DEMANDELEMENTS,    GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F10_OPTIONSMENU,                             GNEApplicationWindow::onCmdOptions),

    // Toolbar locate
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEJUNCTION,     GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEJUNCTION,     GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEEDGE,         GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEEDGE,         GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEVEHICLE,      GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEVEHICLE,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPERSON,       GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEPERSON,       GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEROUTE,        GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEROUTE,        GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATESTOP,         GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATESTOP,         GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATETLS,          GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATETLS,          GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEADD,          GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEADD,          GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPOI,          GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEPOI,          GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPOLY,         GNEApplicationWindow::onCmdLocate),
    FXMAPFUNC(SEL_UPDATE,   MID_LOCATEPOLY,         GNEApplicationWindow::onUpdNeedsNetwork),

    // toolbar windows
    FXMAPFUNC(SEL_COMMAND,  MID_CLEARMESSAGEWINDOW,                     GNEApplicationWindow::onCmdClearMsgWindow),

    // toolbar help
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F1_ONLINEDOCUMENTATION,  GNEApplicationWindow::onCmdHelp),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_F12_ABOUT,               GNEApplicationWindow::onCmdAbout),

    // alt + <number>
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_0_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_0_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_1_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_1_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_2_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_2_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_3_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_3_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_4_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_4_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_5_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_5_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_6_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_6_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_7_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_7_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_8_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_8_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_ALT_9_TOOGLEEDITOPTION,      GNEApplicationWindow::onCmdToogleEditOptions),
    FXMAPFUNC(SEL_UPDATE,   MID_HOTKEY_ALT_9_TOOGLEEDITOPTION,      GNEApplicationWindow::onUpdNeedsNetwork),

    // key events
    FXMAPFUNC(SEL_KEYPRESS,     0,                      GNEApplicationWindow::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE,   0,                      GNEApplicationWindow::onKeyRelease),
    FXMAPFUNC(SEL_COMMAND,      MID_HOTKEY_ESC,         GNEApplicationWindow::onCmdAbort),
    FXMAPFUNC(SEL_COMMAND,      MID_HOTKEY_DEL,         GNEApplicationWindow::onCmdDel),
    FXMAPFUNC(SEL_COMMAND,      MID_HOTKEY_ENTER,       GNEApplicationWindow::onCmdEnter),
    FXMAPFUNC(SEL_COMMAND,      MID_HOTKEY_BACKSPACE,   GNEApplicationWindow::onCmdBackspace),

    // threads events
    FXMAPFUNC(FXEX::SEL_THREAD_EVENT,   ID_LOADTHREAD_EVENT,    GNEApplicationWindow::onLoadThreadEvent),
    FXMAPFUNC(FXEX::SEL_THREAD,         ID_LOADTHREAD_EVENT,    GNEApplicationWindow::onLoadThreadEvent),

    // Edge template functions
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_SHIFT_F1_TEMPLATE_SET,       GNEApplicationWindow::onCmdSetTemplate),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_SHIFT_F2_TEMPLATE_COPY,      GNEApplicationWindow::onCmdCopyTemplate),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_SHIFT_F3_TEMPLATE_CLEAR,     GNEApplicationWindow::onCmdClearTemplate),

    // Other
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST,    0,                                      GNEApplicationWindow::onClipboardRequest),
    FXMAPFUNC(SEL_COMMAND,              MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, GNEApplicationWindow::onCmdFocusFrame),
};

// Object implementation
FXIMPLEMENT(GNEApplicationWindow, FXMainWindow, GNEApplicationWindowMap, ARRAYNUMBER(GNEApplicationWindowMap))


// ===========================================================================
// GNEApplicationWindow method definitions
// ===========================================================================

GNEApplicationWindow::GNEApplicationWindow(FXApp* a, const std::string& configPattern) :
    GUIMainWindow(a),
    myLoadThread(nullptr),
    myAmLoading(false),
    myFileMenu(nullptr),
    myFileMenuTLS(nullptr),
    myFileMenuAdditionals(nullptr),
    myFileMenuDemandElements(nullptr),
    myFileMenuDataElements(nullptr),
    myEditMenu(nullptr),
    myProcessingMenu(nullptr),
    myLocatorMenu(nullptr),
    myWindowsMenu(nullptr),
    myHelpMenu(nullptr),
    myMessageWindow(nullptr),
    myMainSplitter(nullptr),
    hadDependentBuild(false),
    myNet(nullptr),
    myUndoList(new GNEUndoList(this)),
    myConfigPattern(configPattern),
    myToolbarsGrip(this),
    myMenuBarFile(this),
    myFileMenuCommands(this),
    myEditMenuCommands(this),
    myProcessingMenuCommands(this),
    myLocateMenuCommands(this),
    myWindowsMenuCommands(this),
    mySupermodeCommands(this),
    myViewNet(nullptr),
    myTitlePrefix("NETEDIT " VERSION_STRING),
    myMDIMenu(nullptr)

{
    // init icons
    GUIIconSubSys::initIcons(a);
    // init Textures
    GUITextureSubSys::initTextures(a);
    // init cursors
    GUICursorSubSys::initCursors(a);
}


void
GNEApplicationWindow::dependentBuild() {
    // do this not twice
    if (hadDependentBuild) {
        WRITE_ERROR("DEBUG: GNEApplicationWindow::dependentBuild called twice");
        return;
    }
    hadDependentBuild = true;
    setTarget(this);
    setSelector(MID_WINDOW);
    // build toolbar menu
    getToolbarsGrip().buildMenuToolbarsGrip();
    // build the thread - io
    myLoadThreadEvent.setTarget(this),  myLoadThreadEvent.setSelector(ID_LOADTHREAD_EVENT);
    // build the status bar
    myStatusbar = new FXStatusBar(this, GUIDesignStatusBar);
    {
        myGeoFrame =
            new FXHorizontalFrame(myStatusbar, GUIDesignHorizontalFrameStatusBar);
        myGeoCoordinate = new FXLabel(myGeoFrame, "N/A\t\tOriginal coordinate (before coordinate transformation in NETCONVERT)", nullptr, LAYOUT_CENTER_Y);
        myCartesianFrame =
            new FXHorizontalFrame(myStatusbar, GUIDesignHorizontalFrameStatusBar);
        myCartesianCoordinate = new FXLabel(myCartesianFrame, "N/A\t\tNetwork coordinate", nullptr, LAYOUT_CENTER_Y);
    }
    // make the window a mdi-window
    myMainSplitter = new FXSplitter(this, GUIDesignSplitter | SPLITTER_VERTICAL | SPLITTER_REVERSED);
    myMDIClient = new FXMDIClient(myMainSplitter, GUIDesignSplitterMDI);
    myMDIMenu = new FXMDIMenu(this, myMDIClient);
    // build the message window
    myMessageWindow = new GUIMessageWindow(myMainSplitter);
    myMainSplitter->setSplit(1, 65);
    // fill menu and tool bar
    fillMenuBar();
    // build additional threads
    myLoadThread = new GNELoadThread(getApp(), this, myEvents, myLoadThreadEvent);
    // set the status bar
    myStatusbar->getStatusLine()->setText("Ready.");
    // set the caption
    setTitle(myTitlePrefix);
    // set Netedit ICON
    setIcon(GUIIconSubSys::getIcon(GUIIcon::NETEDIT));
    // build NETEDIT Accelerators (hotkeys)
    GUIShortcutsSubSys::buildNETEDITAccelerators(this);
}


void
GNEApplicationWindow::create() {
    setWindowSizeAndPos();
    gCurrentFolder = getApp()->reg().readStringEntry("SETTINGS", "basedir", "");
    FXMainWindow::create();
    myFileMenu->create();
    myEditMenu->create();
    myFileMenuTLS->create();
    myFileMenuAdditionals->create();
    myFileMenuDemandElements->create();
    myFileMenuDataElements->create();
    //mySettingsMenu->create();
    myWindowsMenu->create();
    myHelpMenu->create();

    FXint textWidth = getApp()->getNormalFont()->getTextWidth("8", 1) * 22;
    myCartesianFrame->setWidth(textWidth);
    myGeoFrame->setWidth(textWidth);

    show(PLACEMENT_DEFAULT);
    if (!OptionsCont::getOptions().isSet("window-size")) {
        if (getApp()->reg().readIntEntry("SETTINGS", "maximized", 0) == 1) {
            maximize();
        }
    }

}


GNEApplicationWindow::~GNEApplicationWindow() {
    closeAllWindows();
    // Close icons
    GUIIconSubSys::close();
    // Close gifs (Textures)
    GUITextureSubSys::close();
    // delete visuals
    delete myGLVisual;
    // must delete menus to avoid segfault on removing accelerators
    // (http://www.fox-toolkit.net/faq#TOC-What-happens-when-the-application-s)
    delete myFileMenuTLS;
    delete myFileMenuAdditionals;
    delete myFileMenuDemandElements;
    delete myFileMenuDataElements;
    delete myFileMenu;
    delete myEditMenu;
    delete myLocatorMenu;
    delete myProcessingMenu;
    delete myWindowsMenu;
    delete myHelpMenu;
    // Delete load thread
    delete myLoadThread;
    // drop all events
    while (!myEvents.empty()) {
        // get the next event
        GUIEvent* e = myEvents.top();
        myEvents.pop();
        delete e;
    }
    // delte undo list
    delete myUndoList;
}


long
GNEApplicationWindow::onCmdQuit(FXObject*, FXSelector, void*) {
    if (continueWithUnsavedChanges("quit")) {
        storeWindowSizeAndPos();
        getApp()->reg().writeStringEntry("SETTINGS", "basedir", gCurrentFolder.text());
        if (isMaximized()) {
            getApp()->reg().writeIntEntry("SETTINGS", "maximized", 1);
        } else {
            getApp()->reg().writeIntEntry("SETTINGS", "maximized", 0);
        }
        getApp()->exit(0);
    }
    return 1;
}


long
GNEApplicationWindow::onCmdEditChosen(FXObject*, FXSelector, void*) {
    GUIDialog_GLChosenEditor* chooser =
        new GUIDialog_GLChosenEditor(this, &gSelected);
    chooser->create();
    chooser->show();
    return 1;
}


long
GNEApplicationWindow::onCmdNewNetwork(FXObject*, FXSelector, void*) {
    // first check that current edited Net can be closed (und therefore the undo-list cleared, see #5753)
    if (myViewNet && !onCmdClose(0, 0, 0)) {
        return 1;
    } else {
        OptionsCont& oc = OptionsCont::getOptions();
        GNELoadThread::fillOptions(oc);
        GNELoadThread::setDefaultOptions(oc);
        loadConfigOrNet("", true, false, true, true);
        return 1;
    }
}


long
GNEApplicationWindow::onCmdOpenConfiguration(FXObject*, FXSelector, void*) {
    // first check that current edited Net can be closed (und therefore the undo-list cleared, see #5753)
    if (myViewNet && !onCmdClose(0, 0, 0)) {
        return 1;
    } else {
        // get the new file name
        FXFileDialog opendialog(this, "Open Netconvert Configuration");
        opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG));
        opendialog.setSelectMode(SELECTFILE_EXISTING);
        opendialog.setPatternList(myConfigPattern.c_str());
        if (gCurrentFolder.length() != 0) {
            opendialog.setDirectory(gCurrentFolder);
        }
        if (opendialog.execute()) {
            gCurrentFolder = opendialog.getDirectory();
            std::string file = opendialog.getFilename().text();
            loadConfigOrNet(file, false);
            // add it into recent configs
            myMenuBarFile.myRecentConfigs.appendFile(file.c_str());
        }
        return 1;
    }
}


long
GNEApplicationWindow::onCmdOpenNetwork(FXObject*, FXSelector, void*) {
    // first check that current edited Net can be closed (und therefore the undo-list cleared, see #5753)
    if (myViewNet && !onCmdClose(0, 0, 0)) {
        return 1;
    } else {
        // get the new file name
        FXFileDialog opendialog(this, "Open Network");
        opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::OPEN_NET));
        opendialog.setSelectMode(SELECTFILE_EXISTING);
        opendialog.setPatternList("SUMO nets (*.net.xml)\nAll files (*)");
        if (gCurrentFolder.length() != 0) {
            opendialog.setDirectory(gCurrentFolder);
        }
        if (opendialog.execute()) {
            gCurrentFolder = opendialog.getDirectory();
            std::string file = opendialog.getFilename().text();
            loadConfigOrNet(file, true);
            // add it into recent nets
            myMenuBarFile.myRecentNets.appendFile(file.c_str());
            // when a net is loaded, save additionals and TLSPrograms are disabled
            disableSaveAdditionalsMenu();
            myFileMenuCommands.saveTLSPrograms->disable();
        }
        return 1;
    }
}


long
GNEApplicationWindow::onCmdOpenForeign(FXObject*, FXSelector, void*) {
    // first check that current edited Net can be closed (und therefore the undo-list cleared, see #5753)
    if (myViewNet && !onCmdClose(0, 0, 0)) {
        return 1;
    } else {
        // get the new file name
        FXFileDialog opendialog(this, "Import Foreign Network");
        opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::OPEN_NET));
        opendialog.setSelectMode(SELECTFILE_EXISTING);
        FXString osmPattern("OSM net (*.osm.xml,*.osm)");
        opendialog.setPatternText(0, osmPattern);
        if (gCurrentFolder.length() != 0) {
            opendialog.setDirectory(gCurrentFolder);
        }
        if (opendialog.execute()) {
            gCurrentFolder = opendialog.getDirectory();
            std::string file = opendialog.getFilename().text();

            OptionsCont& oc = OptionsCont::getOptions();
            GNELoadThread::fillOptions(oc);
            if (osmPattern.contains(opendialog.getPattern())) {
                // recommended osm options
                // https://sumo.dlr.de/wiki/Networks/Import/OpenStreetMap#Recommended_NETCONVERT_Options
                oc.set("osm-files", file);
                oc.set("geometry.remove", "true");
                oc.set("ramps.guess", "true");
                oc.set("junctions.join", "true");
                oc.set("tls.guess-signals", "true");
                oc.set("tls.discard-simple", "true");
            } else {
                throw ProcessError("Attempted to import unknown file format '" + file + "'.");
            }

            GUIDialog_Options* wizard =
                new GUIDialog_Options(this, "Select Import Options", getWidth(), getHeight());

            if (wizard->execute()) {
                NIFrame::checkOptions(); // needed to set projection parameters
                loadConfigOrNet("", false, false, false);
            }
        }
        return 1;
    }
}


long
GNEApplicationWindow::onCmdOpenAdditionals(FXObject*, FXSelector, void*) {
    // write debug information
    WRITE_DEBUG("Open additional dialog");
    // get the Additional file name
    FXFileDialog opendialog(this, "Open Additionals file");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("Additional files (*.add.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        // close additional dialog
        WRITE_DEBUG("Close additional dialog");
        // udpate current folder
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        // disable validation for additionals
        XMLSubSys::setValidation("never", "auto", "auto");
        // Create additional handler
        GNEAdditionalHandler additionalHandler(file, myNet);
        // begin undoList operation
        myUndoList->p_begin("Loading additionals from '" + file + "'");
        // Run parser for additionals
        if (!XMLSubSys::runParser(additionalHandler, file, false)) {
            WRITE_ERROR("Loading of " + file + " failed.");
        }
        // end undoList operation and update view
        myUndoList->p_end();
        update();
        // restore validation for additionals
        XMLSubSys::setValidation("auto", "auto", "auto");
    } else {
        // write debug information
        WRITE_DEBUG("Cancel additional dialog");
    }
    return 1;
}


long
GNEApplicationWindow::onCmdOpenTLSPrograms(FXObject*, FXSelector, void*) {
    // write debug information
    WRITE_DEBUG("Open TLSProgram dialog");
    // get the shape file name
    FXFileDialog opendialog(this, "Open TLSPrograms file");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::MODETLS));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("TLSProgram files (*.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        // close additional dialog
        WRITE_DEBUG("Close TLSProgram dialog");
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        // Run parser
        myUndoList->p_begin("Loading TLS Programs from '" + file + "'");
        myNet->computeNetwork(this);
        if (myNet->getViewNet()->getViewParent()->getTLSEditorFrame()->parseTLSPrograms(file) == false) {
            // Abort undo/redo
            myUndoList->abort();
        } else {
            // commit undo/redo operation
            myUndoList->p_end();
            update();
        }
    } else {
        // write debug information
        WRITE_DEBUG("Cancel TLSProgram dialog");
    }
    return 1;
}


long
GNEApplicationWindow::onCmdOpenDemandElements(FXObject*, FXSelector, void*) {
    // write debug information
    WRITE_DEBUG("Open demand element dialog");
    // get the demand element file name
    FXFileDialog opendialog(this, "Open demand element file");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::SUPERMODEDEMAND));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("Demand element files (*.rou.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        // close additional dialog
        WRITE_DEBUG("Close demand element dialog");
        // udpate current folder
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        // disable validation for additionals
        XMLSubSys::setValidation("never", "auto", "auto");
        // Create additional handler
        GNERouteHandler demandHandler(file, myNet);
        // begin undoList operation
        myUndoList->p_begin("Loading demand elements from '" + file + "'");
        // Run parser for additionals
        if (!XMLSubSys::runParser(demandHandler, file, false)) {
            WRITE_ERROR("Loading of " + file + " failed.");
        }
        // end undoList operation and update view
        myUndoList->p_end();
        update();
        // restore validation for demand
        XMLSubSys::setValidation("auto", "auto", "auto");
    } else {
        // write debug information
        WRITE_DEBUG("Cancel demand element dialog");
    }
    return 1;
}


long
GNEApplicationWindow::onCmdOpenDataElements(FXObject*, FXSelector, void*) {
    // write debug information
    WRITE_DEBUG("Open data element dialog");
    // get the data element file name
    FXFileDialog opendialog(this, "Open data element file");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::SUPERMODEDATA));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("Data element files (*.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        // close additional dialog
        WRITE_DEBUG("Close data element dialog");
        // udpate current folder
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        // disable validation for additionals
        XMLSubSys::setValidation("never", "auto", "auto");
        // Create additional handler
        GNEDataHandler dataHandler(file, myNet);
        // begin undoList operation
        myUndoList->p_begin("Loading data elements from '" + file + "'");
        // Run parser for additionals
        if (!XMLSubSys::runParser(dataHandler, file, false)) {
            WRITE_ERROR("Loading of " + file + " failed.");
        }
        // end undoList operation and update view
        myUndoList->p_end();
        update();
        // restore validation for data
        XMLSubSys::setValidation("auto", "auto", "auto");
    } else {
        // write debug information
        WRITE_DEBUG("Cancel data element dialog");
    }
    return 1;
}


long
GNEApplicationWindow::onCmdOpenRecent(FXObject* sender, FXSelector, void* fileData) {
    // first check that current edited Net can be closed (und therefore the undo-list cleared, see #5753)
    if (myViewNet && !onCmdClose(0, 0, 0)) {
        return 1;
    } else if (myAmLoading) {
        myStatusbar->getStatusLine()->setText("Already loading!");
        return 1;
    } else {
        std::string file((const char*)fileData);
        loadConfigOrNet(file, sender == &myMenuBarFile.myRecentNets);
        return 1;
    }
}


long
GNEApplicationWindow::onCmdReload(FXObject*, FXSelector, void*) {
    // first check that current edited Net can be closed (und therefore the undo-list cleared, see #5753)
    if (myViewNet) {
        // check if current network can be closed
        if (continueWithUnsavedChanges("reload")) {
            closeAllWindows();
            // disable save additionals and TLS menu
            disableSaveAdditionalsMenu();
            myFileMenuCommands.saveTLSPrograms->disable();
            // hide all Supermode, Network and demand commands
            mySupermodeCommands.hideSupermodeCommands();
            myEditMenuCommands.networkMenuCommands.hideNetworkMenuCommands();
            myEditMenuCommands.demandMenuCommands.hideDemandMenuCommands();
        } else {
            // abort reloading (because "cancel button" was pressed)
            return 1;
        }
    }
    // @note. If another network has been load during this session, it might not be desirable to set useStartupOptions
    loadConfigOrNet(OptionsCont::getOptions().getString("sumo-net-file"), true, true);
    return 1;
}


long
GNEApplicationWindow::onCmdClose(FXObject*, FXSelector, void*) {
    if (continueWithUnsavedChanges("close")) {
        closeAllWindows();
        // disable save additionals and TLS menu
        disableSaveAdditionalsMenu();
        myFileMenuCommands.saveTLSPrograms->disable();
        // hide all Supermode, Network and demand commands
        mySupermodeCommands.hideSupermodeCommands();
        myEditMenuCommands.networkMenuCommands.hideNetworkMenuCommands();
        myEditMenuCommands.demandMenuCommands.hideDemandMenuCommands();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdLocate(FXObject*, FXSelector sel, void*) {
    if (myMDIClient->numChildren() > 0) {
        GNEViewParent* w = dynamic_cast<GNEViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            w->onCmdLocate(nullptr, sel, nullptr);
        }
    }
    return 1;
}

long
GNEApplicationWindow::onUpdOpen(FXObject* sender, FXSelector, void*) {
    sender->handle(this, myAmLoading ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    return 1;
}


long
GNEApplicationWindow::onCmdClearMsgWindow(FXObject*, FXSelector, void*) {
    myMessageWindow->clear();
    return 1;
}


long
GNEApplicationWindow::onCmdLoadAdditionalsInSUMOGUI(FXObject*, FXSelector, void*) {
    // write warning if netedit is running in testing mode
    WRITE_DEBUG("Toogle load additionals in sumo-gui");
    return 1;
}


long
GNEApplicationWindow::onCmdLoadDemandInSUMOGUI(FXObject*, FXSelector, void*) {
    // write warning if netedit is running in testing mode
    WRITE_DEBUG("Toogle load demand in sumo-gui");
    return 1;
}


long
GNEApplicationWindow::onCmdAbout(FXObject*, FXSelector, void*) {
    // write warning if netedit is running in testing mode
    WRITE_DEBUG("Opening about dialog");
    // create and open about dialog
    GNEAbout* about = new GNEAbout(this);
    about->create();
    about->show(PLACEMENT_OWNER);
    // write warning if netedit is running in testing mode
    WRITE_DEBUG("Closed about dialog");
    return 1;
}


long GNEApplicationWindow::onClipboardRequest(FXObject*, FXSelector, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    FXString string = GUIUserIO::clipped.c_str();
    setDNDData(FROM_CLIPBOARD, event->target, string);
    return 1;
}


long
GNEApplicationWindow::onLoadThreadEvent(FXObject*, FXSelector, void*) {
    eventOccurred();
    return 1;
}


void
GNEApplicationWindow::eventOccurred() {
    while (!myEvents.empty()) {
        // get the next event
        GUIEvent* e = myEvents.top();
        myEvents.pop();
        // process
        switch (e->getOwnType()) {
            case EVENT_SIMULATION_LOADED:
                handleEvent_NetworkLoaded(e);
                break;
            case EVENT_MESSAGE_OCCURRED:
            case EVENT_WARNING_OCCURRED:
            case EVENT_ERROR_OCCURRED:
            case EVENT_DEBUG_OCCURRED:
            case EVENT_GLDEBUG_OCCURRED:
                handleEvent_Message(e);
                break;
            default:
                break;
        }
        delete e;
    }
}


void
GNEApplicationWindow::handleEvent_NetworkLoaded(GUIEvent* e) {
    OptionsCont& oc = OptionsCont::getOptions();
    myAmLoading = false;
    GNEEvent_NetworkLoaded* ec = static_cast<GNEEvent_NetworkLoaded*>(e);
    // check whether the loading was successfull
    if (ec->myNet == nullptr) {
        // report failure
        setStatusBarText("Loading of '" + ec->myFile + "' failed!");
    } else {
        // set new Net
        myNet = ec->myNet;
        // report success
        setStatusBarText("'" + ec->myFile + "' loaded.");
        setWindowSizeAndPos();
        // build viewparent toolbar grips before creating view parent
        getToolbarsGrip().buildViewParentToolbarsGrips();
        // initialise NETEDIT View
        GNEViewParent* viewParent = new GNEViewParent(myMDIClient, myMDIMenu, "NETEDIT VIEW", this, nullptr, myNet, myUndoList, nullptr, MDI_TRACKING, 10, 10, 300, 200);
        // create it maximized
        viewParent->maximize();
        // mark it as Active child
        myMDIClient->setActiveChild(viewParent);
        // cast pointer myViewNet
        myViewNet = dynamic_cast<GNEViewNet*>(viewParent->getView());
        // set settings in view
        if (viewParent->getView() && ec->mySettingsFile != "") {
            GUISettingsHandler settings(ec->mySettingsFile, true, true);
            std::string settingsName = settings.addSettings(viewParent->getView());
            viewParent->getView()->addDecals(settings.getDecals());
            settings.applyViewport(viewParent->getView());
            settings.setSnapshots(viewParent->getView());
        }
        // set network name on the caption
        setTitle(MFXUtils::getTitleText(myTitlePrefix, ec->myFile.c_str()));
        // force supermode network
        if (myViewNet) {
            myViewNet->forceSupermodeNetwork();
        }
        if (myViewNet && ec->myViewportFromRegistry) {
            Position off;
            off.set(getApp()->reg().readRealEntry("viewport", "x"), getApp()->reg().readRealEntry("viewport", "y"), getApp()->reg().readRealEntry("viewport", "z"));
            Position p(off.x(), off.y(), 0);
            myViewNet->setViewportFromToRot(off, p, 0);
        }
    }
    getApp()->endWaitCursor();
    myMessageWindow->registerMsgHandlers();
    // check if additionals/shapes has to be loaded at start
    if (oc.isSet("additional-files") && !oc.getString("additional-files").empty() && myNet) {
        // obtain vector of additional files
        std::vector<std::string> additionalFiles = oc.getStringVector("additional-files");
        // begin undolist
        myUndoList->p_begin("Loading additionals and shapes from '" + toString(additionalFiles) + "'");
        // iterate over every additional file
        for (const auto& additionalFile : additionalFiles) {
            WRITE_MESSAGE("Loading additionals and shapes from '" + additionalFile + "'");
            GNEAdditionalHandler additionalHandler(additionalFile, myNet);
            // disable validation for additionals
            XMLSubSys::setValidation("never", "auto", "auto");
            // Run parser
            if (!XMLSubSys::runParser(additionalHandler, additionalFile, false)) {
                WRITE_ERROR("Loading of " + additionalFile + " failed.");
            }
            // disable validation for additionals
            XMLSubSys::setValidation("auto", "auto", "auto");
        }

        myUndoList->p_end();
    }
    // check if demand elements has to be loaded at start
    if (oc.isSet("route-files") && !oc.getString("route-files").empty() && myNet) {
        // obtain vector of route files
        std::vector<std::string> demandElementsFiles = oc.getStringVector("route-files");
        // begin undolist
        myUndoList->p_begin("Loading demand elements from '" + toString(demandElementsFiles) + "'");
        // iterate over every route file
        for (const auto& demandElementsFile : demandElementsFiles) {
            WRITE_MESSAGE("Loading demand elements from '" + demandElementsFile + "'");
            GNERouteHandler routeHandler(demandElementsFile, myNet);
            // disable validation for demand elements
            XMLSubSys::setValidation("never", "auto", "auto");
            if (!XMLSubSys::runParser(routeHandler, demandElementsFile, false)) {
                WRITE_ERROR("Loading of " + demandElementsFile + " failed.");
            }
            // disable validation for demand elements
            XMLSubSys::setValidation("auto", "auto", "auto");
        }

        myUndoList->p_end();
    }
    // check if data elements has to be loaded at start
    if (oc.isSet("data-files") && !oc.getString("data-files").empty() && myNet) {
        // obtain vector of data files
        std::vector<std::string> dataElementsFiles = oc.getStringVector("data-files");
        // begin undolist
        myUndoList->p_begin("Loading data elements from '" + toString(dataElementsFiles) + "'");
        // disable interval bar update
        myViewNet->getIntervalBar().disableIntervalBarUpdate();
        // iterate over every data file
        for (const auto& dataElementsFile : dataElementsFiles) {
            WRITE_MESSAGE("Loading data elements from '" + dataElementsFile + "'");
            GNEDataHandler dataHandler(dataElementsFile, myNet);
            // disable validation for data elements
            XMLSubSys::setValidation("never", "auto", "auto");
            if (!XMLSubSys::runParser(dataHandler, dataElementsFile, false)) {
                WRITE_ERROR("Loading of " + dataElementsFile + " failed.");
            }
            // disable validation for data elements
            XMLSubSys::setValidation("auto", "auto", "auto");
        }
        // enable interval bar update
        myViewNet->getIntervalBar().enableIntervalBarUpdate();
        myUndoList->p_end();
    }
    // check if additionals output must be changed
    if (oc.isSet("additionals-output")) {
        // overwrite "additional-files" with value "additionals-output"
        oc.resetWritable();
        oc.set("additional-files", oc.getString("additionals-output"));
    }
    // check if demand elements output must be changed
    if (oc.isSet("demandelements-output")) {
        // overwrite "route-files" with value "demandelements-output"
        oc.resetWritable();
        oc.set("route-files", oc.getString("demandelements-output"));
    }
    // check if data elements output must be changed
    if (oc.isSet("dataelements-output")) {
        // overwrite "data-files" with value "dataelements-output"
        oc.resetWritable();
        oc.set("data-files", oc.getString("dataelements-output"));
    }
    // after loading net shouldn't be saved
    if (myNet) {
        myNet->requireSaveNet(false);
    }
    // update app
    update();
}


void
GNEApplicationWindow::handleEvent_Message(GUIEvent* e) {
    GUIEvent_Message* ec = static_cast<GUIEvent_Message*>(e);
    myMessageWindow->appendMsg(ec->getOwnType(), ec->getMsg());
}

// ---------------------------------------------------------------------------
// private methods
// ---------------------------------------------------------------------------

void
GNEApplicationWindow::fillMenuBar() {
    // declare a FXMenuTitle needed to set height in all menu titles
    FXMenuTitle* menuTitle;
    // build file menu
    myFileMenu = new FXMenuPane(this, LAYOUT_FIX_HEIGHT);
    menuTitle = new FXMenuTitle(myToolbarsGrip.menu, "&File", nullptr, myFileMenu, LAYOUT_FIX_HEIGHT);
    menuTitle->setHeight(23);
    myFileMenuTLS = new FXMenuPane(this);
    myFileMenuAdditionals = new FXMenuPane(this);
    myFileMenuDemandElements = new FXMenuPane(this);
    myFileMenuDataElements = new FXMenuPane(this);
    myFileMenuCommands.buildFileMenuCommands(myFileMenu, myFileMenuTLS, myFileMenuAdditionals, myFileMenuDemandElements, myFileMenuDataElements);
    // build recent files
    myMenuBarFile.buildRecentFiles(myFileMenu);
    new FXMenuSeparator(myFileMenu);
    new FXMenuCommand(myFileMenu,
                      "&Quit\tCtrl+Q\tQuit the Application.",
                      nullptr, this, MID_HOTKEY_CTRL_Q_CLOSE, 0);
    // build edit menu
    myEditMenu = new FXMenuPane(this);
    menuTitle = new FXMenuTitle(myToolbarsGrip.menu, "&Edit", nullptr, myEditMenu, LAYOUT_FIX_HEIGHT);
    menuTitle->setHeight(23);
    // build undo/redo command
    myEditMenuCommands.undoLastChange = new FXMenuCommand(myEditMenu,
            "&Undo\tCtrl+Z\tUndo the last change.",
            GUIIconSubSys::getIcon(GUIIcon::UNDO), this, MID_HOTKEY_CTRL_Z_UNDO);
    myEditMenuCommands.redoLastChange = new FXMenuCommand(myEditMenu,
            "&Redo\tCtrl+Y\tRedo the last change.",
            GUIIconSubSys::getIcon(GUIIcon::REDO), this, MID_HOTKEY_CTRL_Y_REDO);
    // build separator
    new FXMenuSeparator(myEditMenu);
    // build Supermode commands and hide it
    mySupermodeCommands.buildSupermodeCommands(myEditMenu);
    mySupermodeCommands.hideSupermodeCommands();
    myEditMenuCommands.buildEditMenuCommands(myEditMenu);
    // build processing menu (trigger netbuild computations)
    myProcessingMenu = new FXMenuPane(this);
    menuTitle = new FXMenuTitle(myToolbarsGrip.menu, "&Processing", nullptr, myProcessingMenu, LAYOUT_FIX_HEIGHT);
    menuTitle->setHeight(23);
    myProcessingMenuCommands.buildProcessingMenuCommands(myProcessingMenu);
    // build locate menu
    myLocatorMenu = new FXMenuPane(this);
    menuTitle = new FXMenuTitle(myToolbarsGrip.menu, "&Locate", nullptr, myLocatorMenu, LAYOUT_FIX_HEIGHT);
    menuTitle->setHeight(23);
    myLocateMenuCommands.buildLocateMenuCommands(myLocatorMenu);
    // build windows menu
    myWindowsMenu = new FXMenuPane(this);
    menuTitle = new FXMenuTitle(myToolbarsGrip.menu, "&Windows", nullptr, myWindowsMenu, LAYOUT_FIX_HEIGHT);
    menuTitle->setHeight(23);
    myWindowsMenuCommands.buildWindowsMenuCommands(myWindowsMenu, myStatusbar, myMessageWindow);
    // build help menu
    myHelpMenu = new FXMenuPane(this);
    menuTitle = new FXMenuTitle(myToolbarsGrip.menu, "&Help", nullptr, myHelpMenu, LAYOUT_FIX_HEIGHT);
    menuTitle->setHeight(23);
    // build help menu commands
    new FXMenuCommand(myHelpMenu,
                      "&Online Documentation\tF1\tOpen Online documentation.",
                      nullptr, this, MID_HOTKEY_F1_ONLINEDOCUMENTATION);
    new FXMenuCommand(myHelpMenu,
                      "&About\tF12\tAbout netedit.",
                      GUIIconSubSys::getIcon(GUIIcon::NETEDIT_MINI), this, MID_HOTKEY_F12_ABOUT);
}


void
GNEApplicationWindow::loadConfigOrNet(const std::string file, bool isNet, bool isReload, bool useStartupOptions, bool newNet) {
    storeWindowSizeAndPos();
    getApp()->beginWaitCursor();
    myAmLoading = true;
    closeAllWindows();
    if (isReload) {
        myLoadThread->start();
        setStatusBarText("Reloading.");
    } else {
        gSchemeStorage.saveViewport(0, 0, -1, 0); // recenter view
        myLoadThread->loadConfigOrNet(file, isNet, useStartupOptions, newNet);
        setStatusBarText("Loading '" + file + "'.");
    }
    // show supermode commands menu
    mySupermodeCommands.showSupermodeCommands();
    // show Network command menus (because Network is the default supermode)
    myEditMenuCommands.networkMenuCommands.showNetworkMenuCommands();
    // update window
    update();
}


FXGLCanvas*
GNEApplicationWindow::getBuildGLCanvas() const {
    // NETEDIT uses only a single View, then return nullptr
    return nullptr;
}


SUMOTime
GNEApplicationWindow::getCurrentSimTime() const {
    return 0;
}


double
GNEApplicationWindow::getTrackerInterval() const {
    return 1;
}


GNEUndoList*
GNEApplicationWindow::getUndoList() {
    return myUndoList;
}


GNEViewNet*
GNEApplicationWindow::getViewNet() {
    return myViewNet;
}


GNEApplicationWindowHelper::ToolbarsGrip&
GNEApplicationWindow::getToolbarsGrip() {
    myToolbarsGrip.myTopDock = myTopDock;
    return myToolbarsGrip;
}


void
GNEApplicationWindow::closeAllWindows() {
    // check if view has to be saved
    if (myViewNet) {
        myViewNet->saveVisualizationSettings();
    }
    // lock tracker
    myTrackerLock.lock();
    // remove trackers and other external windows
    while (!myGLWindows.empty()) {
        delete myGLWindows.front();
    }
    myViewNet = nullptr;
    for (FXMainWindow* const window : myTrackerWindows) {
        window->destroy();
        delete window;
    }
    myTrackerWindows.clear();
    // reset the caption
    setTitle(myTitlePrefix);
    // add a separator to the log
    myMessageWindow->addSeparator();
    // unlock tracker
    myTrackerLock.unlock();
    // remove coordinate information
    myGeoCoordinate->setText("N/A");
    myCartesianCoordinate->setText("N/A");
    // check if net can be deleted
    if (myNet != nullptr) {
        delete myNet;
        myNet = nullptr;
        GeoConvHelper::resetLoaded();
    }
    myMessageWindow->unregisterMsgHandlers();
    // Reset textures
    GUITextureSubSys::resetTextures();
    // reset fonts
    GLHelper::resetFont();
    // disable saving commmand
    disableSaveAdditionalsMenu();
}


FXCursor*
GNEApplicationWindow::getDefaultCursor() {
    return getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
}


void
GNEApplicationWindow::loadOptionOnStartup() {
    OptionsCont& oc = OptionsCont::getOptions();
    // Disable normalization preserve the given network as far as possible
    oc.set("offset.disable-normalization", "true");
    loadConfigOrNet("", true, false, true, oc.getBool("new"));
}


void
GNEApplicationWindow::setStatusBarText(const std::string& statusBarText) {
    myStatusbar->getStatusLine()->setText(statusBarText.c_str());
    myStatusbar->getStatusLine()->setNormalText(statusBarText.c_str());
}


long
GNEApplicationWindow::computeJunctionWithVolatileOptions() {
    // obtain option container
    OptionsCont& oc = OptionsCont::getOptions();
    // declare variable to save FXMessageBox outputs.
    FXuint answer = 0;
    // declare string to save paths in wich additionals, shapes demand and data elements will be saved
    std::string additionalsSavePath = oc.getString("additional-files");
    std::string demandElementsSavePath = oc.getString("route-files");
    std::string dataElementsSavePath = oc.getString("data-files");
    // write warning if netedit is running in testing mode
    WRITE_DEBUG("Opening FXMessageBox 'Volatile Recomputing'");
    // open question dialog box
    answer = FXMessageBox::question(myNet->getViewNet()->getApp(), MBOX_YES_NO, "Recompute with volatile options",
                                    "Changes produced in the net due a recomputing with volatile options cannot be undone. Continue?");
    if (answer != 1) { //1:yes, 2:no, 4:esc
        // write warning if netedit is running in testing mode
        if (answer == 2) {
            WRITE_DEBUG("Closed FXMessageBox 'Volatile Recomputing' with 'No'");
        } else if (answer == 4) {
            WRITE_DEBUG("Closed FXMessageBox 'Volatile Recomputing' with 'ESC'");
        }
        // abort recompute with volatile options
        return 0;
    } else {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox 'Volatile Recomputing' with 'Yes'");
        // Check if there are additionals in our net
        if (myNet->getNumberOfAdditionals() > 0) {
            // ask user if want to save additionals if weren't saved previously
            if (oc.getString("additional-files") == "") {
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Opening FXMessageBox 'Save additionals before recomputing'");
                // open question dialog box
                answer = FXMessageBox::question(myNet->getViewNet()->getApp(), MBOX_YES_NO, "Save additionals before recomputing with volatile options",
                                                "Would you like to save additionals before recomputing?");
                if (answer != 1) { //1:yes, 2:no, 4:esc
                    // write warning if netedit is running in testing mode
                    if (answer == 2) {
                        WRITE_DEBUG("Closed FXMessageBox 'Save additionals before recomputing' with 'No'");
                    } else if (answer == 4) {
                        WRITE_DEBUG("Closed FXMessageBox 'Save additionals before recomputing' with 'ESC'");
                    }
                } else {
                    // write warning if netedit is running in testing mode
                    WRITE_DEBUG("Closed FXMessageBox 'Save additionals before recomputing' with 'Yes'");
                    // Open a dialog to set filename output
                    FXString file = MFXUtils::getFilename2Write(this,
                                    "Select name of the demand element file", ".xml",
                                    GUIIconSubSys::getIcon(GUIIcon::MODETLS),
                                    gCurrentFolder).text();
                    // add xml extension
                    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".rou.xml");
                    // check that file is valid
                    if (fileWithExtension != "") {
                        // update additional files
                        oc.resetWritable();
                        oc.set("additional-files", fileWithExtension);
                        // set obtanied filename output into additionalsSavePath (can be "")
                        additionalsSavePath = oc.getString("additional-files");
                    }
                }
            }
            // Check if additional must be saved in a temporal directory, if user didn't define a directory for additionals
            if (oc.getString("additional-files") == "") {
                // Obtain temporal directory provided by FXSystem::getCurrentDirectory()
                additionalsSavePath = FXSystem::getTempDirectory().text() + std::string("/tmpAdditionalsNetedit.xml");
            }
            // Start saving additionals
            getApp()->beginWaitCursor();
            try {
                myNet->saveAdditionals(additionalsSavePath);
            } catch (IOError& e) {
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Opening FXMessageBox 'Error saving additionals before recomputing'");
                // open error message box
                FXMessageBox::error(this, MBOX_OK, "Saving additionals in temporal folder failed!", "%s", e.what());
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Closed FXMessageBox 'Error saving additionals before recomputing' with 'OK'");
            }
            // end saving additionals
            myMessageWindow->addSeparator();
            getApp()->endWaitCursor();
        } else {
            // clear additional path
            additionalsSavePath = "";
        }
        // Check if there are demand elements in our net
        if (myNet->getNumberOfDemandElements() > 0) {
            // ask user if want to save demand elements if weren't saved previously
            if (oc.getString("route-files") == "") {
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Opening FXMessageBox 'Save demand elements before recomputing'");
                // open question dialog box
                answer = FXMessageBox::question(myNet->getViewNet()->getApp(), MBOX_YES_NO, "Save demand elements before recomputing with volatile options",
                                                "Would you like to save demand elements before recomputing?");
                if (answer != 1) { //1:yes, 2:no, 4:esc
                    // write warning if netedit is running in testing mode
                    if (answer == 2) {
                        WRITE_DEBUG("Closed FXMessageBox 'Save demand elements before recomputing' with 'No'");
                    } else if (answer == 4) {
                        WRITE_DEBUG("Closed FXMessageBox 'Save demand elements before recomputing' with 'ESC'");
                    }
                } else {
                    // write warning if netedit is running in testing mode
                    WRITE_DEBUG("Closed FXMessageBox 'Save demand elements before recomputing' with 'Yes'");
                    // Open a dialog to set filename output
                    FXString file = MFXUtils::getFilename2Write(this,
                                    "Select name of the demand element file", ".rou.xml",
                                    GUIIconSubSys::getIcon(GUIIcon::MODETLS),
                                    gCurrentFolder).text();
                    // add xml extension
                    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".rou.xml");
                    // check that file is valid
                    if (fileWithExtension != "") {
                        // update route files
                        oc.resetWritable();
                        oc.set("route-files", fileWithExtension);
                        // set obtanied filename output into demand elementSavePath (can be "")
                        demandElementsSavePath = oc.getString("route-files");
                    }
                }
            }
            // Check if demand element must be saved in a temporal directory, if user didn't define a directory for demand elements
            if (oc.getString("route-files") == "") {
                // Obtain temporal directory provided by FXSystem::getCurrentDirectory()
                demandElementsSavePath = FXSystem::getTempDirectory().text() + std::string("/tmpDemandElementsNetedit.xml");
            }
            // Start saving demand elements
            getApp()->beginWaitCursor();
            try {
                myNet->saveDemandElements(demandElementsSavePath);
            } catch (IOError& e) {
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Opening FXMessageBox 'Error saving demand elements before recomputing'");
                // open error message box
                FXMessageBox::error(this, MBOX_OK, "Saving demand elements in temporal folder failed!", "%s", e.what());
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Closed FXMessageBox 'Error saving demand elements before recomputing' with 'OK'");
            }
            // end saving demand elements
            myMessageWindow->addSeparator();
            getApp()->endWaitCursor();
        } else {
            // clear demand element path
            demandElementsSavePath = "";
        }
        // Check if there are data elements in our net
        if (myNet->getNumberOfDataSets() > 0) {
            // ask user if want to save data elements if weren't saved previously
            if (oc.getString("data-files") == "") {
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Opening FXMessageBox 'Save data elements before recomputing'");
                // open question dialog box
                answer = FXMessageBox::question(myNet->getViewNet()->getApp(), MBOX_YES_NO, "Save data elements before recomputing with volatile options",
                                                "Would you like to save data elements before recomputing?");
                if (answer != 1) { //1:yes, 2:no, 4:esc
                    // write warning if netedit is running in testing mode
                    if (answer == 2) {
                        WRITE_DEBUG("Closed FXMessageBox 'Save data elements before recomputing' with 'No'");
                    } else if (answer == 4) {
                        WRITE_DEBUG("Closed FXMessageBox 'Save data elements before recomputing' with 'ESC'");
                    }
                } else {
                    // write warning if netedit is running in testing mode
                    WRITE_DEBUG("Closed FXMessageBox 'Save data elements before recomputing' with 'Yes'");
                    // Open a dialog to set filename output
                    FXString file = MFXUtils::getFilename2Write(this,
                                    "Select name of the data element file", ".rou.xml",
                                    GUIIconSubSys::getIcon(GUIIcon::MODETLS),
                                    gCurrentFolder).text();
                    // add xml extension
                    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".rou.xml");
                    // check that file is valid
                    if (fileWithExtension != "") {
                        // update data files
                        oc.resetWritable();
                        oc.set("data-files", fileWithExtension);
                        // set obtanied filename output into data elementSavePath (can be "")
                        dataElementsSavePath = oc.getString("data-files");
                    }
                }
            }
            // Check if data element must be saved in a temporal directory, if user didn't define a directory for data elements
            if (oc.getString("data-files") == "") {
                // Obtain temporal directory provided by FXSystem::getCurrentDirectory()
                dataElementsSavePath = FXSystem::getTempDirectory().text() + std::string("/tmpDataElementsNetedit.xml");
            }
            // Start saving data elements
            getApp()->beginWaitCursor();
            try {
                myNet->saveDataElements(dataElementsSavePath);
            } catch (IOError& e) {
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Opening FXMessageBox 'Error saving data elements before recomputing'");
                // open error message box
                FXMessageBox::error(this, MBOX_OK, "Saving data elements in temporal folder failed!", "%s", e.what());
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Closed FXMessageBox 'Error saving data elements before recomputing' with 'OK'");
            }
            // end saving data elements
            myMessageWindow->addSeparator();
            getApp()->endWaitCursor();
        } else {
            // clear data element path
            dataElementsSavePath = "";
        }
        // compute with volatile options
        myNet->computeNetwork(this, true, true, additionalsSavePath, demandElementsSavePath, dataElementsSavePath);
        updateControls();
        return 1;
    }
}


void
GNEApplicationWindow::enableSaveTLSProgramsMenu() {
    myFileMenuCommands.saveTLSPrograms->enable();
}


void
GNEApplicationWindow::enableSaveAdditionalsMenu() {
    myFileMenuCommands.saveAdditionals->enable();
    myFileMenuCommands.saveAdditionalsAs->enable();
}


void
GNEApplicationWindow::disableSaveAdditionalsMenu() {
    myFileMenuCommands.saveAdditionals->disable();
    myFileMenuCommands.saveAdditionalsAs->disable();
}


void
GNEApplicationWindow::enableSaveDemandElementsMenu() {
    myFileMenuCommands.saveDemandElements->disable();
    myFileMenuCommands.saveDemandElementsAs->disable();
}


void
GNEApplicationWindow::disableSaveDemandElementsMenu() {
    myFileMenuCommands.saveDemandElements->disable();
    myFileMenuCommands.saveDemandElementsAs->disable();
}


void
GNEApplicationWindow::enableSaveDataElementsMenu() {
    myFileMenuCommands.saveDataElements->disable();
    myFileMenuCommands.saveDataElementsAs->disable();
}


void
GNEApplicationWindow::disableSaveDataElementsMenu() {
    myFileMenuCommands.saveDataElements->disable();
    myFileMenuCommands.saveDataElementsAs->disable();
}


long
GNEApplicationWindow::onCmdSetSuperMode(FXObject* sender, FXSelector sel, void* ptr) {
    // check that currently there is a View
    if (myViewNet) {
        myViewNet->onCmdSetSupermode(sender, sel, ptr);
    }
    return 1;
}


long
GNEApplicationWindow::onCmdSetMode(FXObject* sender, FXSelector sel, void* ptr) {
    // check that currently there is a View
    if (myViewNet) {
        myViewNet->onCmdSetMode(sender, sel, ptr);
    }
    return 1;
}


long
GNEApplicationWindow::onCmdProcessButton(FXObject*, FXSelector sel, void*) {
    // first check if there is a view
    if (myViewNet) {
        // process depending of supermode
        if (myViewNet->getEditModes().isCurrentSupermodeNetwork()) {
            // check what FXMenuCommand was called
            switch (FXSELID(sel)) {
                case MID_HOTKEY_F5_COMPUTE_NETWORK_DEMAND:
                    // show extra information for tests
                    WRITE_DEBUG("Key F5 (Compute) pressed");
                    myNet->computeNetwork(this, true, false);
                    updateControls();
                    break;
                case MID_HOTKEY_SHIFT_F5_COMPUTEJUNCTIONS_VOLATILE:
                    // show extra information for tests
                    WRITE_DEBUG("Keys Shift + F5 (Compute with volatile options) pressed");
                    computeJunctionWithVolatileOptions();
                    break;
                case MID_HOTKEY_F6_CLEAN_SOLITARYJUNCTIONS_UNUSEDROUTES:
                    // show extra information for tests
                    WRITE_DEBUG("Key F6 (Clean junction) pressed");
                    myNet->removeSolitaryJunctions(myUndoList);
                    break;
                case MID_HOTKEY_F7_JOIN_SELECTEDJUNCTIONS_ROUTES:
                    // show extra information for tests
                    WRITE_DEBUG("Key F7 (Join junctions) pressed");
                    myNet->joinSelectedJunctions(myUndoList);
                    break;
                case MID_HOTKEY_F8_CLEANINVALID_CROSSINGS_DEMANDELEMENTS:
                    // show extra information for tests
                    WRITE_DEBUG("Key F8 (Clean invalid crossings) pressed");
                    myNet->cleanInvalidCrossings(myUndoList);
                    break;
                default:
                    break;
            }
        } else {
            // check what FXMenuCommand was called
            switch (FXSELID(sel)) {
                case MID_HOTKEY_F5_COMPUTE_NETWORK_DEMAND:
                    // show extra information for tests
                    WRITE_DEBUG("Key F5 (Compute) pressed");
                    myNet->computeDemandElements(this);
                    updateControls();
                    break;
                case MID_HOTKEY_F6_CLEAN_SOLITARYJUNCTIONS_UNUSEDROUTES:
                    // show extra information for tests
                    WRITE_DEBUG("Key F6 (RemoveUnusedRoutes) pressed");
                    myNet->cleanUnusedRoutes(myUndoList);
                    break;
                case MID_HOTKEY_F7_JOIN_SELECTEDJUNCTIONS_ROUTES:
                    // show extra information for tests
                    WRITE_DEBUG("Key F7 (JoinRoutes) pressed");
                    myNet->joinRoutes(myUndoList);
                    break;
                case MID_HOTKEY_F8_CLEANINVALID_CROSSINGS_DEMANDELEMENTS:
                    // show extra information for tests
                    WRITE_DEBUG("Key F8 (CleanInvalidDemandElements) pressed");
                    myNet->cleanInvalidDemandElements(myUndoList);
                    break;
                default:
                    break;
            }
        }
    }
    return 1;
}


long
GNEApplicationWindow::onCmdOpenSUMOGUI(FXObject*, FXSelector, void*) {
    // check that currently there is a View
    if (myViewNet) {
        // first check if network is saved
        if (!myViewNet->getNet()->isNetSaved()) {
            // save network
            onCmdSaveNetwork(nullptr, 0, nullptr);
            if (!myViewNet->getNet()->isNetSaved()) {
                return 0;
            }
        }
        // now check if additionals must be loaded and are saved
        if ((myEditMenuCommands.loadAdditionalsInSUMOGUI->getCheck() == TRUE) &&
                (myViewNet->getNet()->getNumberOfAdditionals() > 0) &&
                (myViewNet->getNet()->isAdditionalsSaved() == false)) {
            // save additionals
            onCmdSaveAdditionals(nullptr, 0, nullptr);
            // check if additionals were sucesfully saved. If not, abort
            if (!myViewNet->getNet()->isAdditionalsSaved()) {
                return 0;
            }
        }
        // finally check if demand elements must be loaded and are saved
        if ((myEditMenuCommands.loadDemandInSUMOGUI->getCheck() == TRUE) &&
                (myViewNet->getNet()->getNumberOfDemandElements() > 0) &&
                (myViewNet->getNet()->isDemandElementsSaved() == false)) {
            // save additionals
            onCmdSaveDemandElements(nullptr, 0, nullptr);
            // check if demand elements were sucesfully saved. If not, abort
            if (!myViewNet->getNet()->isDemandElementsSaved()) {
                return 0;
            }
        }
        // obtain viewport
        FXRegistry reg("SUMO GUI", "Eclipse");
        reg.read();
        reg.writeRealEntry("viewport", "x", myViewNet->getChanger().getXPos());
        reg.writeRealEntry("viewport", "y", myViewNet->getChanger().getYPos());
        reg.writeRealEntry("viewport", "z", myViewNet->getChanger().getZPos());
        reg.write();
        std::string sumogui = "sumo-gui";
        const char* sumoPath = getenv("SUMO_HOME");
        if (sumoPath != nullptr) {
            std::string newPath = std::string(sumoPath) + "/bin/sumo-gui";
            if (FileHelpers::isReadable(newPath) || FileHelpers::isReadable(newPath + ".exe")) {
                sumogui = "\"" + newPath + "\"";
            }
        }
        std::string cmd = sumogui + " --registry-viewport" + " -n "  + "\"" + OptionsCont::getOptions().getString("output-file") + "\"";
        // obtainer options container
        OptionsCont& oc = OptionsCont::getOptions();
        // if load additionals is enabled, add it to command
        if ((myEditMenuCommands.loadAdditionalsInSUMOGUI->getCheck() == TRUE) && (oc.getString("additional-files").size() > 0)) {
            cmd += " -a \"" + oc.getString("additional-files") + "\"";
        }
        // if load demand is enabled, add it to command
        if ((myEditMenuCommands.loadDemandInSUMOGUI->getCheck() == TRUE) && (oc.getString("route-files").size() > 0)) {
            cmd += " -r \"" + oc.getString("route-files") + "\"";
        }
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
    }
    return 1;
}


long
GNEApplicationWindow::onCmdAbort(FXObject*, FXSelector, void*) {
    // check that view exists
    if (myViewNet) {
        // show extra information for tests
        WRITE_DEBUG("Key ESC (abort) pressed");
        // first check if we're selecting a subset of edges in TAZ Frame
        if (myViewNet->getViewParent()->getTAZFrame()->getTAZSelectionStatisticsModul()->getEdgeAndTAZChildrenSelected().size() > 0) {
            // show extra information for tests
            WRITE_DEBUG("Cleaning current selected edges");
            // clear current selection
            myViewNet->getViewParent()->getTAZFrame()->getTAZSelectionStatisticsModul()->clearSelectedEdges();
        } else if (myViewNet->getViewParent()->getInspectorFrame()->shown()) {
            // show extra information for tests
            WRITE_DEBUG("Cleaning inspected elements");
            // clear inspected elements
            myViewNet->getViewParent()->getInspectorFrame()->inspectSingleElement(nullptr);
        } else {
            // abort current operation
            myViewNet->abortOperation();
        }
    }
    return 1;
}


long
GNEApplicationWindow::onCmdDel(FXObject*, FXSelector, void*) {
    // check that view exists
    if (myViewNet) {
        // show extra information for tests
        WRITE_DEBUG("Key DEL (delete) pressed");
        myViewNet->hotkeyDel();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdEnter(FXObject*, FXSelector, void*) {
    // check that view exists
    if (myViewNet) {
        // show extra information for tests
        WRITE_DEBUG("Key ENTER pressed");
        myViewNet->hotkeyEnter();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdBackspace(FXObject*, FXSelector, void*) {
    // check that view exists
    if (myViewNet) {
        // show extra information for tests
        WRITE_DEBUG("Key BACKSPACE pressed");
        myViewNet->hotkeyBackSpace();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdFocusFrame(FXObject*, FXSelector, void*) {
    // check that view exists
    if (myViewNet) {
        myViewNet->hotkeyFocusFrame();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdEditViewport(FXObject*, FXSelector, void*) {
    // check that view exists
    if (myViewNet) {
        myViewNet->showViewportEditor();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdEditViewScheme(FXObject*, FXSelector, void*) {
    // check that view exists
    if (myViewNet) {
        myViewNet->showViewschemeEditor();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdToogleGrid(FXObject* obj, FXSelector sel, void* ptr) {
    // check that view exists
    if (myViewNet) {
        // Toogle getMenuCheckShowGrid of GNEViewNet
        if ((myViewNet->getNetworkViewOptions().menuCheckShowGrid->getCheck() == TRUE) ||
                (myViewNet->getDemandViewOptions().menuCheckShowGrid->getCheck() == TRUE)) {
            myViewNet->getNetworkViewOptions().menuCheckShowGrid->setCheck(FALSE);
            myViewNet->getDemandViewOptions().menuCheckShowGrid->setCheck(FALSE);
            // show extra information for tests
            WRITE_DEBUG("Disabled grid throught Ctrl+g hotkey");
        } else {
            myViewNet->getNetworkViewOptions().menuCheckShowGrid->setCheck(TRUE);
            myViewNet->getDemandViewOptions().menuCheckShowGrid->setCheck(TRUE);
            // show extra information for tests
            WRITE_DEBUG("Enabled grid throught Ctrl+g hotkey");
        }
        // Call manually show grid function
        myViewNet->onCmdToogleShowGridNetwork(obj, sel, ptr);
        myViewNet->onCmdToogleShowGridDemand(obj, sel, ptr);
    }
    return 1;
}


long
GNEApplicationWindow::onCmdSetFrontElement(FXObject* /*obj*/, FXSelector /*sel*/, void* /*ptr*/) {
    if (myViewNet) {
        if (myViewNet->getViewParent()->getInspectorFrame()->shown()) {
            // get inspected AC
            const GNEAttributeCarrier* inspectedAC = (myViewNet->getViewParent()->getInspectorFrame()->getAttributesEditor()->getEditedACs().size() == 1) ?
                    myViewNet->getViewParent()->getInspectorFrame()->getAttributesEditor()->getEditedACs().front() : nullptr;
            // set or clear front attribute
            if (myViewNet->getFrontAttributeCarrier() == inspectedAC) {
                myViewNet->setFrontAttributeCarrier(nullptr);
            } else {
                myViewNet->setFrontAttributeCarrier(inspectedAC);
            }
            myViewNet->getViewParent()->getInspectorFrame()->getNeteditAttributesEditor()->refreshNeteditAttributesEditor(true);
        } else {
            myViewNet->setFrontAttributeCarrier(nullptr);
        }
    }
    return 1;
}


long
GNEApplicationWindow::onCmdToogleEditOptions(FXObject* obj, FXSelector sel, void* /* ptr */) {
    // first check that we have a ViewNet
    if (myViewNet) {
        // first check what selector was called
        int numericalKeyPressed = sel - FXSEL(SEL_COMMAND, MID_HOTKEY_ALT_0_TOOGLEEDITOPTION) - 1;
        // check that numericalKeyPressed is valid
        if ((numericalKeyPressed < 0) || (numericalKeyPressed > 10)) {
            return 1;
        }
        // declare a vector in which save visible menu commands
        std::vector<FXMenuCheck*> visibleMenuCommands;
        // get common, network and demand visible menu commands
        myViewNet->getNetworkViewOptions().getVisibleNetworkMenuCommands(visibleMenuCommands);
        myViewNet->getDemandViewOptions().getVisibleDemandMenuCommands(visibleMenuCommands);
        myViewNet->getDataViewOptions().getVisibleDataMenuCommands(visibleMenuCommands);
        // now check that numericalKeyPressed isn't greather than visible view options
        if (numericalKeyPressed >= (int)visibleMenuCommands.size()) {
            return 1;
        }
        // toogle edit options
        if (GNEApplicationWindowHelper::toogleEditOptionsNetwork(myViewNet,
                visibleMenuCommands.at(numericalKeyPressed), numericalKeyPressed, obj, sel)) {
            return 1;
        } else if (GNEApplicationWindowHelper::toogleEditOptionsDemand(myViewNet,
                   visibleMenuCommands.at(numericalKeyPressed), numericalKeyPressed, obj, sel)) {
            return 1;
        } else if (GNEApplicationWindowHelper::toogleEditOptionsData(myViewNet,
                   visibleMenuCommands.at(numericalKeyPressed), numericalKeyPressed, obj, sel)) {
            return 1;
        }
    }
    return 1;
}


long
GNEApplicationWindow::onCmdHelp(FXObject*, FXSelector, void*) {
    FXLinkLabel::fxexecute("https://sumo.dlr.de/docs/netedit.html");
    return 1;
}


long
GNEApplicationWindow::onCmdOptions(FXObject*, FXSelector, void*) {
    GUIDialog_Options* wizard =
        new GUIDialog_Options(this, "Configure Options", getWidth(), getHeight());

    if (wizard->execute()) {
        NIFrame::checkOptions(); // needed to set projection parameters
        NBFrame::checkOptions();
        NWFrame::checkOptions();
        SystemFrame::checkOptions(); // needed to set precision
    }
    return 1;
}


long
GNEApplicationWindow::onCmdUndo(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Keys Ctrl+Z (Undo) pressed");
    // Undo needs a viewnet and a enabled undoLastChange menu command
    if (myViewNet && myEditMenuCommands.undoLastChange->isEnabled()) {
        myViewNet->getUndoList()->undo();
        // update current show frame after undo
        if (myViewNet->getViewParent()->getCurrentShownFrame()) {
            myViewNet->getViewParent()->getCurrentShownFrame()->updateFrameAfterUndoRedo();
        }
        // update manually undo/redo menu commands (see #6005)
        onUpdUndo(myEditMenuCommands.undoLastChange, 0, 0);
        onUpdRedo(myEditMenuCommands.redoLastChange, 0, 0);
        // update toolbar undo-redo buttons
        myViewNet->getViewParent()->updateUndoRedoButtons();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdRedo(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Keys Ctrl+Y (Redo) pressed");
    // redo needs a viewnet and a enabled redoLastChange menu command
    if (myViewNet && myEditMenuCommands.redoLastChange->isEnabled()) {
        myViewNet->getUndoList()->redo();
        // update current show frame after redo
        if (myViewNet->getViewParent()->getCurrentShownFrame()) {
            myViewNet->getViewParent()->getCurrentShownFrame()->updateFrameAfterUndoRedo();
        }
        // update manually undo/redo menu commands (see #6005)
        onUpdUndo(myEditMenuCommands.undoLastChange, 0, 0);
        onUpdRedo(myEditMenuCommands.redoLastChange, 0, 0);
        // update toolbar undo-redo buttons
        myViewNet->getViewParent()->updateUndoRedoButtons();
    }
    return 1;
}



long
GNEApplicationWindow::onCmdCut(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Key Ctrl+X (Cut) pressed");
    // Prepared for #6042
    return 1;
}


long
GNEApplicationWindow::onCmdCopy(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Key Ctrl+C (Copy) pressed");
    // Prepared for #6042
    return 1;
}


long
GNEApplicationWindow::onCmdPaste(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Key Ctrl+V (Paste) pressed");
    // Prepared for #6042
    return 1;
}


long
GNEApplicationWindow::onCmdSetTemplate(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Key Ctrl+F1 (Set Template) pressed");
    // first check if myViewNet exist
    if (myViewNet) {
        // call set template in inspector frame
        myViewNet->getViewParent()->getInspectorFrame()->getTemplateEditor()->setTemplate();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdCopyTemplate(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Key Ctrl+F2 (Copy Template) pressed");
    // first check if myViewNet exist
    if (myViewNet) {
        // call copy template in inspector frame
        myViewNet->getViewParent()->getInspectorFrame()->getTemplateEditor()->copyTemplate();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdClearTemplate(FXObject*, FXSelector, void*) {
    WRITE_DEBUG("Key Ctrl+F3 (Clear Template) pressed");
    // first check if myViewNet exist
    if (myViewNet) {
        // call clear template in inspector frame
        myViewNet->getViewParent()->getInspectorFrame()->getTemplateEditor()->clearTemplate();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdSaveAsNetwork(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this,
                    "Save Network as", ".net.xml",
                    GUIIconSubSys::getIcon(GUIIcon::SAVENETWORKELEMENTS),
                    gCurrentFolder);
    // add xml extension
    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".net.xml");
    // check that file with extension is valid
    if (fileWithExtension != "") {
        OptionsCont& oc = OptionsCont::getOptions();
        oc.resetWritable();
        oc.set("output-file", fileWithExtension);
        setTitle(MFXUtils::getTitleText(myTitlePrefix, fileWithExtension.c_str()));
        onCmdSaveNetwork(nullptr, 0, nullptr);
    }
    return 1;
}


long
GNEApplicationWindow::onCmdSaveAsPlainXML(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this,
                    "Select name of the plain-xml edge-file (other names will be deduced from this)", "",
                    GUIIconSubSys::getIcon(GUIIcon::SAVE),
                    gCurrentFolder);
    // check that file is valid (note: in this case we don't need to use function FileHelpers::addExtension)
    if (file != "") {
        OptionsCont& oc = OptionsCont::getOptions();
        bool wasSet = oc.isSet("plain-output-prefix");
        std::string oldPrefix = oc.getString("plain-output-prefix");
        std::string prefix = file.text();
        // if the name of an edg.xml file was given, remove the suffix
        if (StringUtils::endsWith(prefix, ".edg.xml")) {
            prefix = prefix.substr(0, prefix.size() - 8);
        }
        if (StringUtils::endsWith(prefix, ".")) {
            prefix = prefix.substr(0, prefix.size() - 1);
        }
        oc.resetWritable();
        oc.set("plain-output-prefix", prefix);
        getApp()->beginWaitCursor();
        try {
            myNet->savePlain(oc);
            myUndoList->unmark();
            myUndoList->mark();
        } catch (IOError& e) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox 'Error saving plainXML'");
            // open message box
            FXMessageBox::error(this, MBOX_OK, "Saving plain xml failed!", "%s", e.what());
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'Error saving plainXML' with 'OK'");
        }
        myMessageWindow->appendMsg(EVENT_MESSAGE_OCCURRED, "Plain XML saved with prefix '" + prefix + "'.\n");
        myMessageWindow->addSeparator();
        if (wasSet) {
            oc.resetWritable();
            oc.set("plain-output-prefix", oldPrefix);
        } else {
            oc.unSet("plain-output-prefix");
        }
        getApp()->endWaitCursor();
    }
    return 1;
}


long
GNEApplicationWindow::onCmdSaveJoined(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this,
                    "Select name of the joined-junctions file", ".nod.xml",
                    GUIIconSubSys::getIcon(GUIIcon::SAVE),
                    gCurrentFolder);
    // add xml extension
    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".xml");
    // check that file with extension is valid
    if (fileWithExtension != "") {
        OptionsCont& oc = OptionsCont::getOptions();
        bool wasSet = oc.isSet("junctions.join-output");
        std::string oldFile = oc.getString("junctions.join-output");
        oc.resetWritable();
        oc.set("junctions.join-output", fileWithExtension);
        getApp()->beginWaitCursor();
        try {
            myNet->saveJoined(oc);
        } catch (IOError& e) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox 'error saving joined'");
            // opening error message
            FXMessageBox::error(this, MBOX_OK, "Saving joined junctions failed!", "%s", e.what());
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'error saving joined' with 'OK'");
        }
        myMessageWindow->appendMsg(EVENT_MESSAGE_OCCURRED, "Joined junctions saved to '" + fileWithExtension + "'.\n");
        myMessageWindow->addSeparator();
        if (wasSet) {
            oc.resetWritable();
            oc.set("junctions.join-output", oldFile);
        } else {
            oc.unSet("junctions.join-output");
        }
        getApp()->endWaitCursor();
    }
    return 1;
}


long
GNEApplicationWindow::onUpdNeedsNetwork(FXObject* sender, FXSelector, void*) {
    // check if net exist
    if (myNet) {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    } else {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    }
    return 1;
}


long
GNEApplicationWindow::onUpdNeedsFrontElement(FXObject* sender, FXSelector, void*) {
    // check if net, viewnet and front attribute exist
    if (myNet && myViewNet && myViewNet->getFrontAttributeCarrier()) {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    } else {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    }
    return 1;
}


long
GNEApplicationWindow::onUpdReload(FXObject* sender, FXSelector, void*) {
    sender->handle(this, ((myNet == nullptr) || !OptionsCont::getOptions().isSet("sumo-net-file")) ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    return 1;
}


long
GNEApplicationWindow::onUpdSaveNetwork(FXObject* sender, FXSelector, void*) {
    sender->handle(this, ((myNet == nullptr) || myNet->isNetSaved()) ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    return 1;
}


long
GNEApplicationWindow::onUpdSaveAdditionals(FXObject* sender, FXSelector, void*) {
    sender->handle(this, ((myNet == nullptr) || myNet->isAdditionalsSaved()) ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    return 1;
}


long
GNEApplicationWindow::onUpdSaveDemandElements(FXObject* sender, FXSelector, void*) {
    sender->handle(this, ((myNet == nullptr) || myNet->isDemandElementsSaved()) ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    return 1;
}


long
GNEApplicationWindow::onUpdSaveDataElements(FXObject* sender, FXSelector, void*) {
    sender->handle(this, ((myNet == nullptr) || myNet->isDataElementsSaved()) ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    return 1;
}


long
GNEApplicationWindow::onUpdUndo(FXObject* obj, FXSelector sel, void* ptr) {
    return myUndoList->p_onUpdUndo(obj, sel, ptr);
}


long
GNEApplicationWindow::onUpdRedo(FXObject* obj, FXSelector sel, void* ptr) {
    return myUndoList->p_onUpdRedo(obj, sel, ptr);
}


long
GNEApplicationWindow::onCmdSaveNetwork(FXObject*, FXSelector, void*) {
    OptionsCont& oc = OptionsCont::getOptions();
    // function onCmdSaveAsNetwork must be executed if this is the first save
    if (oc.getString("output-file") == "" || oc.isDefault("output-file")) {
        return onCmdSaveAsNetwork(nullptr, 0, nullptr);
    } else {
        getApp()->beginWaitCursor();
        try {
            myNet->save(oc);
            myUndoList->unmark();
            myUndoList->mark();
        } catch (IOError& e) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox 'error saving network'");
            // open error message box
            FXMessageBox::error(this, MBOX_OK, "Saving Network failed!", "%s", e.what());
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'error saving network' with 'OK'");
        }
        myMessageWindow->appendMsg(EVENT_MESSAGE_OCCURRED, "Network saved in " + oc.getString("output-file") + ".\n");
        // After saveing a net sucesfully, add it into Recent Nets list.
        myMenuBarFile.myRecentNets.appendFile(oc.getString("output-file").c_str());
        myMessageWindow->addSeparator();
        getApp()->endWaitCursor();
        return 1;
    }
}


long
GNEApplicationWindow::onCmdSaveTLSPrograms(FXObject*, FXSelector, void*) {
    // obtain option container
    OptionsCont& oc = OptionsCont::getOptions();
    // check if save additional menu is enabled
    if (myFileMenuCommands.saveTLSPrograms->isEnabled()) {
        // Check if TLS Programs file was already set at start of netedit or with a previous save
        if (oc.getString("TLSPrograms-output").empty()) {
            FXString file = MFXUtils::getFilename2Write(this,
                            "Select name of the TLS file", ".xml",
                            GUIIconSubSys::getIcon(GUIIcon::MODETLS),
                            gCurrentFolder);
            // add xml extension
            std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".xml");
            // check tat file is valid
            if (file == "") {
                // None TLS Programs file was selected, then stop function
                return 0;
            } else {
                // change value of "TLSPrograms-output"
                oc.resetWritable();
                oc.set("TLSPrograms-output", fileWithExtension);
            }
        }
        // Start saving TLS Programs
        getApp()->beginWaitCursor();
        try {
            myNet->saveTLSPrograms(oc.getString("TLSPrograms-output"));
            myMessageWindow->appendMsg(EVENT_MESSAGE_OCCURRED, "TLS Programs saved in " + oc.getString("TLSPrograms-output") + ".\n");
            myFileMenuCommands.saveTLSPrograms->disable();
        } catch (IOError& e) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox 'error saving TLS Programs'");
            // open error message box
            FXMessageBox::error(this, MBOX_OK, "Saving TLS Programs failed!", "%s", e.what());
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'error saving TLS Programs' with 'OK'");
        }
        myMessageWindow->addSeparator();
        getApp()->endWaitCursor();
        return 1;
    } else {
        return 0;
    }
}


long
GNEApplicationWindow::onCmdSaveTLSProgramsAs(FXObject*, FXSelector, void*) {
    // Open window to select TLS Programs file
    FXString file = MFXUtils::getFilename2Write(this,
                    "Select name of the TLS Progarm file", ".xml",
                    GUIIconSubSys::getIcon(GUIIcon::MODETLS),
                    gCurrentFolder);
    // add xml extension
    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".xml");
    // check tat file is valid
    if (fileWithExtension != "") {
        // change value of "TLSPrograms-files"
        OptionsCont::getOptions().set("TLSPrograms-output", fileWithExtension);
        // save TLS Programs
        return onCmdSaveTLSPrograms(nullptr, 0, nullptr);
    } else {
        return 1;
    }
}


long
GNEApplicationWindow::onCmdSaveAdditionals(FXObject*, FXSelector, void*) {
    // obtain option container
    OptionsCont& oc = OptionsCont::getOptions();
    // check if save additional menu is enabled
    if (myFileMenuCommands.saveAdditionals->isEnabled()) {
        // Check if additionals file was already set at start of netedit or with a previous save
        if (oc.getString("additional-files").empty()) {
            FXString file = MFXUtils::getFilename2Write(this,
                            "Select name of the additional file", ".add.xml",
                            GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL),
                            gCurrentFolder);
            // add xml extension
            std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".add.xml");
            // check tat file is valid
            if (fileWithExtension != "") {
                // change value of "additional-files"
                oc.resetWritable();
                oc.set("additional-files", fileWithExtension);
            } else {
                // None additionals file was selected, then stop function
                return 0;
            }
        }
        // Start saving additionals
        getApp()->beginWaitCursor();
        try {
            myNet->saveAdditionals(oc.getString("additional-files"));
            myMessageWindow->appendMsg(EVENT_MESSAGE_OCCURRED, "Additionals saved in " + oc.getString("additional-files") + ".\n");
            myFileMenuCommands.saveAdditionals->disable();
        } catch (IOError& e) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox 'error saving additionals'");
            // open error message box
            FXMessageBox::error(this, MBOX_OK, "Saving additionals failed!", "%s", e.what());
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'error saving additionals' with 'OK'");
        }
        myMessageWindow->addSeparator();
        getApp()->endWaitCursor();
        return 1;
    } else {
        return 0;
    }
}


long
GNEApplicationWindow::onCmdSaveAdditionalsAs(FXObject*, FXSelector, void*) {
    // Open window to select additional file
    FXString file = MFXUtils::getFilename2Write(this,
                    "Select name of the additional file", ".add.xml",
                    GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL),
                    gCurrentFolder);
    // add xml extension
    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".add.xml");
    // check tat file is valid
    if (fileWithExtension != "") {
        // reset writtable flag
        OptionsCont::getOptions().resetWritable();
        // change value of "additional-files"
        OptionsCont::getOptions().set("additional-files", fileWithExtension);
        // change flag of menu command for save additionals
        myFileMenuCommands.saveAdditionals->enable();
        // save additionals
        return onCmdSaveAdditionals(nullptr, 0, nullptr);
    } else {
        return 1;
    }
}


long
GNEApplicationWindow::onCmdSaveDemandElements(FXObject*, FXSelector, void*) {
    // obtain option container
    OptionsCont& oc = OptionsCont::getOptions();
    // check if save demand element menu is enabled
    if (myFileMenuCommands.saveDemandElements->isEnabled()) {
        // Check if demand elements file was already set at start of netedit or with a previous save
        if (oc.getString("route-files").empty()) {
            FXString file = MFXUtils::getFilename2Write(this,
                            "Select name of the demand element file", ".rou.xml",
                            GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL),
                            gCurrentFolder);
            // add xml extension
            std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".rou.xml");
            // check tat file is valid
            if (fileWithExtension != "") {
                // change value of "route-files"
                oc.resetWritable();
                oc.set("route-files", fileWithExtension);
            } else {
                // None demand elements file was selected, then stop function
                return 0;
            }
        }
        // Start saving demand elements
        getApp()->beginWaitCursor();
        try {
            myNet->saveDemandElements(oc.getString("route-files"));
            myMessageWindow->appendMsg(EVENT_MESSAGE_OCCURRED, "Demand elements saved in " + oc.getString("route-files") + ".\n");
            myFileMenuCommands.saveDemandElements->disable();
        } catch (IOError& e) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox 'error saving demand elements'");
            // open error message box
            FXMessageBox::error(this, MBOX_OK, "Saving demand elements failed!", "%s", e.what());
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'error saving demand elements' with 'OK'");
        }
        myMessageWindow->addSeparator();
        getApp()->endWaitCursor();
        return 1;
    } else {
        return 0;
    }
}


long
GNEApplicationWindow::onCmdSaveDemandElementsAs(FXObject*, FXSelector, void*) {
    // Open window to select additionasl file
    FXString file = MFXUtils::getFilename2Write(this,
                    "Select name of the demand element file", ".rou.xml",
                    GUIIconSubSys::getIcon(GUIIcon::SUPERMODEDEMAND),
                    gCurrentFolder);
    // add xml extension
    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".rou.xml");
    // check that file is correct
    if (fileWithExtension != "") {
        // reset writtable flag
        OptionsCont::getOptions().resetWritable();
        // change value of "route-files"
        OptionsCont::getOptions().set("route-files", fileWithExtension);
        // change flag of menu command for save demand elements
        myFileMenuCommands.saveDemandElements->enable();
        // save demand elements
        return onCmdSaveDemandElements(nullptr, 0, nullptr);
    } else {
        return 1;
    }
}


long
GNEApplicationWindow::onCmdSaveDataElements(FXObject*, FXSelector, void*) {
    // obtain option container
    OptionsCont& oc = OptionsCont::getOptions();
    // check if save data element menu is enabled
    if (myFileMenuCommands.saveDataElements->isEnabled()) {
        // Check if data elements file was already set at start of netedit or with a previous save
        if (oc.getString("data-files").empty()) {
            FXString file = MFXUtils::getFilename2Write(this,
                            "Select name of the data element file", ".xml",
                            GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL),
                            gCurrentFolder);
            // add xml extension
            std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".xml");
            // check tat file is valid
            if (fileWithExtension != "") {
                // change value of "data-files"
                oc.resetWritable();
                oc.set("data-files", fileWithExtension);
            } else {
                // None data elements file was selected, then stop function
                return 0;
            }
        }
        // Start saving data elements
        getApp()->beginWaitCursor();
        try {
            myNet->saveDataElements(oc.getString("data-files"));
            myMessageWindow->appendMsg(EVENT_MESSAGE_OCCURRED, "Data elements saved in " + oc.getString("data-files") + ".\n");
            myFileMenuCommands.saveDataElements->disable();
        } catch (IOError& e) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox 'error saving data elements'");
            // open error message box
            FXMessageBox::error(this, MBOX_OK, "Saving data elements failed!", "%s", e.what());
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'error saving data elements' with 'OK'");
        }
        myMessageWindow->addSeparator();
        getApp()->endWaitCursor();
        return 1;
    } else {
        return 0;
    }
}


long
GNEApplicationWindow::onCmdSaveDataElementsAs(FXObject*, FXSelector, void*) {
    // Open window to select additionasl file
    FXString file = MFXUtils::getFilename2Write(this,
                    "Select name of the data element file", ".xml",
                    GUIIconSubSys::getIcon(GUIIcon::SUPERMODEDATA),
                    gCurrentFolder);
    // add xml extension
    std::string fileWithExtension = FileHelpers::addExtension(file.text(), ".xml");
    // check that file is correct
    if (fileWithExtension != "") {
        // reset writtable flag
        OptionsCont::getOptions().resetWritable();
        // change value of "data-files"
        OptionsCont::getOptions().set("data-files", fileWithExtension);
        // change flag of menu command for save data elements
        myFileMenuCommands.saveDataElements->enable();
        // save data elements
        return onCmdSaveDataElements(nullptr, 0, nullptr);
    } else {
        return 1;
    }
}


bool
GNEApplicationWindow::continueWithUnsavedChanges(const std::string& operation) {
    FXuint answer = 0;
    if (myViewNet && myNet && !myNet->isNetSaved()) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox 'Confirm " + operation + " network'");
        // open question box
        answer = FXMessageBox::question(getApp(), MBOX_QUIT_SAVE_CANCEL,
                                        ("Confirm " + operation + " Network").c_str(), "%s",
                                        ("You have unsaved changes in the network. Do you wish to " + operation + " and discard all changes?").c_str());
        // restore focus to view net
        myViewNet->setFocus();
        // if user close dialog box, check additionals and demand elements
        if (answer == MBOX_CLICKED_QUIT) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'Confirm " + operation + " network' with 'Quit'");
            if (continueWithUnsavedAdditionalChanges(operation) && continueWithUnsavedDemandElementChanges(operation)) {
                // clear undo list and return true to continue with closing/reload
                myUndoList->p_clear();
                return true;
            } else {
                return false;
            }
        } else if (answer == MBOX_CLICKED_SAVE) {
            // save newtork
            onCmdSaveNetwork(nullptr, 0, nullptr);
            if (!myUndoList->marked()) {
                // saving failed
                return false;
            }
            if (continueWithUnsavedAdditionalChanges(operation) && continueWithUnsavedDemandElementChanges(operation)) {
                // clear undo list and return true to continue with closing/reload
                myUndoList->p_clear();
                return true;
            } else {
                return false;
            }
        } else {
            // write warning if netedit is running in testing mode
            if (answer == 2) {
                WRITE_DEBUG("Closed FXMessageBox 'Confirm " + operation + " network' with 'No'");
            } else if (answer == 4) {
                WRITE_DEBUG("Closed FXMessageBox 'Confirm " + operation + " network' with 'ESC'");
            }
            // return false to stop closing/reloading
            return false;
        }
    } else {
        if (continueWithUnsavedAdditionalChanges(operation) && continueWithUnsavedDemandElementChanges(operation)) {
            // clear undo list and return true to continue with closing/reload
            myUndoList->p_clear(); //only ask once
            return true;
        } else {
            // return false to stop closing/reloading
            return false;
        }
    }
}


bool
GNEApplicationWindow::continueWithUnsavedAdditionalChanges(const std::string& operation) {
    // Check if there are non saved additionals
    if (myViewNet && myFileMenuCommands.saveAdditionals->isEnabled()) {
        WRITE_DEBUG("Opening FXMessageBox 'Save additionals before " + operation + "'");
        // open question box
        FXuint answer = FXMessageBox::question(getApp(), MBOX_QUIT_SAVE_CANCEL,
                                               ("Save additionals before " + operation).c_str(), "%s",
                                               ("You have unsaved additionals. Do you wish to " + operation + " and discard all changes?").c_str());
        // restore focus to view net
        myViewNet->setFocus();
        // if answer was affirmative, but there was an error during saving additionals, return false to stop closing/reloading
        if (answer == MBOX_CLICKED_QUIT) {
            WRITE_DEBUG("Closed FXMessageBox 'Save additionals before " + operation + "' with 'Quit'");
            // nothing to save, return true
            return true;
        } else if (answer == MBOX_CLICKED_SAVE) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'Save additionals before " + operation + "' with 'Yes'");
            if (onCmdSaveAdditionals(nullptr, 0, nullptr) == 1) {
                // additionals sucesfully saved
                return true;
            } else {
                // error saving additionals, abort saving
                return false;
            }
        } else {
            // write warning if netedit is running in testing mode
            if (answer == 2) {
                WRITE_DEBUG("Closed FXMessageBox 'Save additionals before " + operation + "' with 'No'");
            } else if (answer == 4) {
                WRITE_DEBUG("Closed FXMessageBox 'Save additionals before " + operation + "' with 'ESC'");
            }
            // abort saving
            return false;
        }
    } else {
        // nothing to save, return true
        return true;
    }
}


bool
GNEApplicationWindow::continueWithUnsavedDemandElementChanges(const std::string& operation) {
    // Check if there are non saved demand elements
    if (myViewNet && myFileMenuCommands.saveDemandElements->isEnabled()) {
        WRITE_DEBUG("Opening FXMessageBox 'Save demand elements before " + operation + "'");
        // open question box
        FXuint answer = FXMessageBox::question(getApp(), MBOX_QUIT_SAVE_CANCEL,
                                               ("Save demand elements before " + operation).c_str(), "%s",
                                               ("You have unsaved demand elements. Do you wish to " + operation + " and discard all changes?").c_str());
        // restore focus to view net
        myViewNet->setFocus();
        // if answer was affirmative, but there was an error during saving demand elements, return false to stop closing/reloading
        if (answer == MBOX_CLICKED_QUIT) {
            WRITE_DEBUG("Closed FXMessageBox 'Save demand elements before " + operation + "' with 'Quit'");
            // nothing to save, return true
            return true;
        } else if (answer == MBOX_CLICKED_SAVE) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'Save demand elements before " + operation + "' with 'Yes'");
            if (onCmdSaveDemandElements(nullptr, 0, nullptr) == 1) {
                // demand elements sucesfully saved
                return true;
            } else {
                // error saving demand elements, abort saving
                return false;
            }
        } else {
            // write warning if netedit is running in testing mode
            if (answer == 2) {
                WRITE_DEBUG("Closed FXMessageBox 'Save demand elements before " + operation + "' with 'No'");
            } else if (answer == 4) {
                WRITE_DEBUG("Closed FXMessageBox 'Save demand elements before " + operation + "' with 'ESC'");
            }
            // abort saving
            return false;
        }
    } else {
        // nothing to save, return true
        return true;
    }
}


void
GNEApplicationWindow::updateControls() {
    // check that view exists
    if (myViewNet) {
        myViewNet->updateControls();
    }
}


void
GNEApplicationWindow::updateSuperModeMenuCommands(const Supermode supermode) {
    if (supermode == Supermode::NETWORK) {
        // menu commands
        myEditMenuCommands.networkMenuCommands.showNetworkMenuCommands();
        myEditMenuCommands.demandMenuCommands.hideDemandMenuCommands();
        myEditMenuCommands.dataMenuCommands.hideDataMenuCommands();
        // processing
        myProcessingMenuCommands.showNetworkProcessingMenuCommands();
        myProcessingMenuCommands.hideDemandProcessingMenuCommands();
        myProcessingMenuCommands.hideDataProcessingMenuCommands();
    } else if (supermode == Supermode::DEMAND) {
        // menu commands
        myEditMenuCommands.networkMenuCommands.hideNetworkMenuCommands();
        myEditMenuCommands.demandMenuCommands.showDemandMenuCommands();
        myEditMenuCommands.dataMenuCommands.hideDataMenuCommands();
        // processing
        myProcessingMenuCommands.hideNetworkProcessingMenuCommands();
        myProcessingMenuCommands.showDemandProcessingMenuCommands();
        myProcessingMenuCommands.hideDataProcessingMenuCommands();
    } else if (supermode == Supermode::DATA) {
        // menu commands
        myEditMenuCommands.networkMenuCommands.hideNetworkMenuCommands();
        myEditMenuCommands.demandMenuCommands.hideDemandMenuCommands();
        myEditMenuCommands.dataMenuCommands.showDataMenuCommands();
        // processing
        myProcessingMenuCommands.hideNetworkProcessingMenuCommands();
        myProcessingMenuCommands.hideDemandProcessingMenuCommands();
        myProcessingMenuCommands.showDataProcessingMenuCommands();
    } else {
        // menu commands
        myEditMenuCommands.networkMenuCommands.hideNetworkMenuCommands();
        myEditMenuCommands.demandMenuCommands.hideDemandMenuCommands();
        myEditMenuCommands.dataMenuCommands.hideDataMenuCommands();
        // processing
        myProcessingMenuCommands.hideNetworkProcessingMenuCommands();
        myProcessingMenuCommands.hideDemandProcessingMenuCommands();
        myProcessingMenuCommands.hideDataProcessingMenuCommands();
    }
}


void
GNEApplicationWindow::disableUndoRedo(const std::string& reason) {
    myUndoRedoListEnabled = reason;
}


void
GNEApplicationWindow::enableUndoRedo() {
    myUndoRedoListEnabled.clear();
}


const std::string&
GNEApplicationWindow::isUndoRedoEnabled() const {
    return myUndoRedoListEnabled;
}

// ---------------------------------------------------------------------------
// GNEApplicationWindow - protected methods
// ---------------------------------------------------------------------------

GNEApplicationWindow::GNEApplicationWindow() :
    myLoadThread(nullptr),
    myAmLoading(false),
    myFileMenu(nullptr),
    myFileMenuTLS(nullptr),
    myFileMenuAdditionals(nullptr),
    myFileMenuDemandElements(nullptr),
    myFileMenuDataElements(nullptr),
    myEditMenu(nullptr),
    myProcessingMenu(nullptr),
    myLocatorMenu(nullptr),
    myWindowsMenu(nullptr),
    myHelpMenu(nullptr),
    myMessageWindow(nullptr),
    myMainSplitter(nullptr),
    hadDependentBuild(false),
    myNet(nullptr),
    myUndoList(nullptr),
    myToolbarsGrip(this),
    myMenuBarFile(this),
    myFileMenuCommands(this),
    myEditMenuCommands(this),
    myProcessingMenuCommands(this),
    myLocateMenuCommands(this),
    myWindowsMenuCommands(this),
    mySupermodeCommands(this),
    myViewNet(nullptr),
    myMDIMenu(nullptr)
{ }


long
GNEApplicationWindow::onKeyPress(FXObject* o, FXSelector sel, void* eventData) {
    const long handled = FXMainWindow::onKeyPress(o, sel, eventData);
    if (handled == 0 && myMDIClient->numChildren() > 0) {
        GNEViewParent* w = dynamic_cast<GNEViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            w->onKeyPress(nullptr, sel, eventData);
        }
    }
    return 0;
}


long
GNEApplicationWindow::onKeyRelease(FXObject* o, FXSelector sel, void* eventData) {
    const long handled = FXMainWindow::onKeyRelease(o, sel, eventData);
    if (handled == 0 && myMDIClient->numChildren() > 0) {
        GNEViewParent* w = dynamic_cast<GNEViewParent*>(myMDIClient->getActiveChild());
        if (w != nullptr) {
            w->onKeyRelease(nullptr, sel, eventData);
        }
    }
    return 0;
}


/****************************************************************************/
