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
/// @file    GNELoadThread.h
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// The thread that performs the loading of a Netedit-net (adapted from
// GUILoadThread)
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/foxtools/FXSingleEventThread.h>
#include <utils/foxtools/FXSynchQue.h>
#include <utils/foxtools/MFXInterThreadEventClient.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GNENet;
class GUIEvent;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNELoadThread
 */
class GNELoadThread : public FXSingleEventThread {
public:
    /// @brief constructor
    GNELoadThread(FXApp* app, MFXInterThreadEventClient* mw, FXSynchQue<GUIEvent*>& eq,
                  FXEX::FXThreadEvent& ev);

    /// @brief destructor
    virtual ~GNELoadThread();

    /// @brief starts the thread. The thread ends after the net has been loaded
    FXint run();

    /**@brief begins the loading of a netconvert configuration or a a network
     * @param[in] file The network or configuration-file to be loaded
     * @param[in] isNet whether file is a network file
     * @param[in] useStartupOptions whether the initial startup options shall be used
     */
    void loadConfigOrNet(const std::string& file, bool isNet, bool useStartupOptions, bool newNet = false);

    /// @brief Retrieves messages from the loading module
    void retrieveMessage(const MsgHandler::MsgType type, const std::string& msg);

    /// @brief clears and initializes the OptionsCont
    static void fillOptions(OptionsCont& oc);

    /// @brief sets required options for proper functioning
    static void setDefaultOptions(OptionsCont& oc);

protected:
    /// @brief init options
    bool initOptions();

    /**@brief Closes the loading process
     *
     * This method is called both on success and failure.
     * All message callbacks to this instance are removed and the parent
     * application is informed about the loading */
    void submitEndAndCleanup(GNENet* net, const std::string& guiSettingsFile = "", const bool viewportFromRegistry = false);

protected:
    /// @brief the parent window to inform about the loading
    MFXInterThreadEventClient* myParent;

    /// @brief the path to load the network from
    std::string myFile;

    /// @brief @brief The instances of message retriever encapsulations Needed to be deleted from the handler later on
    OutputDevice* myErrorRetriever, *myMessageRetriever, *myWarningRetriever, *myDebugRetriever, *myGLDebugRetriever;

    /// @brief event Queue
    FXSynchQue<GUIEvent*>& myEventQue;

    /// @brief event throw
    FXEX::FXThreadEvent& myEventThrow;

    /// @brief Information whether only the network shall be loaded
    bool myLoadNet;

    /// @brief if true, a new net is created
    bool myNewNet;
};
