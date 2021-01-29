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
/// @file    guisim_main.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Felix Brack
/// @date    Tue, 20 Nov 2001
///
// Main for GUISIM
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <ctime>
#include <signal.h>
#include <iostream>
#include <microsim/MSFrame.h>
#include <microsim/MSNet.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SystemFrame.h>
#include <utils/foxtools/MsgHandlerSynchronized.h>
#include <utils/xml/XMLSubSys.h>
#include <gui/GUIApplicationWindow.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <traci-server/TraCIServer.h>


// ===========================================================================
// main function
// ===========================================================================
int
main(int argc, char** argv) {
    // make the output aware of threading
    MsgHandler::setFactory(&MsgHandlerSynchronized::create);
    // get the options
    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("GUI version of the microscopic, multi-modal traffic simulation SUMO.");
    oc.setApplicationName("sumo-gui", "Eclipse SUMO GUI Version " VERSION_STRING);
    gSimulation = true;
    int ret = 0;
    try {
        // initialise subsystems
        XMLSubSys::init();
        MSFrame::fillOptions();
        OptionsIO::setArgs(argc, argv);
        OptionsIO::getOptions(true);
        if (oc.processMetaOptions(false)) {
            SystemFrame::close();
            return 0;
        }
        // Make application
        FXApp application("SUMO GUI", "Eclipse");
        // Open display
        application.init(argc, argv);
        int minor, major;
        if (!FXGLVisual::supported(&application, major, minor)) {
            throw ProcessError("This system has no OpenGL support. Exiting.");
        }

        // build the main window
        GUIApplicationWindow* window =
            new GUIApplicationWindow(&application, "*.sumo.cfg,*.sumocfg");
        gSchemeStorage.init(&application);
        window->dependentBuild();
        // Create app
        application.addSignal(SIGINT, window, MID_HOTKEY_CTRL_Q_CLOSE);
        application.create();
        // Load configuration given on command line
        if (argc > 1) {
            window->loadOnStartup();
        }
        // Run
        ret = application.run();
    } catch (const ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
#ifndef _DEBUG
    } catch (const std::exception& e) {
        if (std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on unknown error).", false);
        ret = 1;
#endif
    }
    TraCIServer::close();
    SystemFrame::close();
    return ret;
}


/****************************************************************************/
