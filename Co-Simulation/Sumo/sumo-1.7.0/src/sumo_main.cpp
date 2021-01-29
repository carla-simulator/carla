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
/// @file    sumo_main.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Axel Wegener
/// @author  Thimor Bohn
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Main for SUMO
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <ctime>
#include <csignal>
#include <string>
#include <iostream>
#include <netload/NLBuilder.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <utils/common/StdDefs.h>
#include <utils/xml/XMLSubSys.h>
#include <traci-server/TraCIServer.h>


// ===========================================================================
// functions
// ===========================================================================
void
signalHandler(int signum) {
    if (MSNet::hasInstance()) {
        switch (signum) {
            case SIGINT:
            case SIGTERM:
                if (MSNet::getInstance()->isInterrupted()) {
                    std::cout << "Another interrupt signal received, hard exit." << std::endl;
                    exit(signum);
                }
                std::cout << "Interrupt signal received, trying to exit gracefully." << std::endl;
                MSNet::getInstance()->interrupt();
                break;
#ifndef WIN32
            case SIGUSR1:
                std::cout << "Step #" << SIMSTEP << std::endl;
                std::cout << MSNet::getInstance()->generateStatistics(string2time(OptionsCont::getOptions().getString("begin"))) << std::endl;
                break;
            case SIGUSR2:
                //TODO reload sim
                break;
#endif
            default:
                break;
        }
    }
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
#ifndef WIN32
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
#endif

    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("A microscopic, multi-modal traffic simulation.");
    oc.setApplicationName("sumo", "Eclipse SUMO sumo Version " VERSION_STRING);
    gSimulation = true;
    int ret = 0;
    MSNet* net = nullptr;
    try {
        // initialise subsystems
        XMLSubSys::init();
        OptionsIO::setArgs(argc, argv);
        // load the net
        MSNet::SimulationState state = MSNet::SIMSTATE_LOADING;
        while (state == MSNet::SIMSTATE_LOADING) {
            net = NLBuilder::init();
            if (net != nullptr) {
                state = net->simulate(string2time(oc.getString("begin")), string2time(oc.getString("end")));
                delete net;
            } else {
                break;
            }
        }
    } catch (const ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        // we need to delete the network explicitly to trigger the cleanup in the correct order
        delete net;
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
