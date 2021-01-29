/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
/// @file    activitygen_main.cpp
/// @author  Piotr Woznica
/// @author  Walter Bamberger
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Jul 2010
///
// Main object of the ActivityGen application
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <exception>
#include <typeinfo>
#include <router/RONet.h>
#include <router/ROLoader.h>
#include <router/RONetHandler.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SystemFrame.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/iodevices/OutputDevice.h>
//ActivityGen
#include "AGFrame.h"
#include "AGActivityGen.h"
#include "city/AGTime.h"


// ===========================================================================
// method definitions
// ===========================================================================

/// Loads the network
void
loadNet(RONet& toFill, ROAbstractEdgeBuilder& eb) {
    OptionsCont& oc = OptionsCont::getOptions();
    std::string file = oc.getString("net-file");
    if (file == "") {
        throw ProcessError("Missing definition of network to load!");
    }
    if (!FileHelpers::isReadable(file)) {
        throw ProcessError("The network file '" + file + "' could not be accessed.");
    }
    PROGRESS_BEGIN_MESSAGE("Loading net");
    RONetHandler handler(toFill, eb, true, 0);
    handler.setFileName(file);
    if (!XMLSubSys::runParser(handler, file, true)) {
        PROGRESS_FAILED_MESSAGE();
        throw ProcessError();
    } else {
        PROGRESS_DONE_MESSAGE();
    }
    if (!deprecatedVehicleClassesSeen.empty()) {
        WRITE_WARNING("Deprecated vehicle classes '" + toString(deprecatedVehicleClassesSeen) + "' in input network.");
        deprecatedVehicleClassesSeen.clear();
    }
}


int
main(int argc, char* argv[]) {
    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription(
        "Generates trips of persons throughout a day for the microscopic, multi-modal traffic simulation SUMO.");
    oc.setApplicationName("activitygen", "Eclipse SUMO activitygen Version " VERSION_STRING);
    oc.addCopyrightNotice("Copyright (C) 2010-2012 Technische Universitaet Muenchen");
    int ret = 0;
    RONet* net = nullptr;
    try {
        // Initialise subsystems and process options
        XMLSubSys::init();
        AGFrame::fillOptions();
        OptionsIO::setArgs(argc, argv);
        OptionsIO::getOptions();
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"), "never");
        MsgHandler::initOutputOptions();
        RandHelper::initRandGlobal();
        SystemFrame::checkOptions();

        // Load network
        net = new RONet();
        AGStreet::Builder builder;
        loadNet(*net, builder);
        WRITE_MESSAGE("Loaded " + toString(net->getEdgeNumber()) + " edges.");
        if (oc.getBool("debug")) {
            WRITE_MESSAGE("\n\t ---- begin AcitivtyGen ----\n");
        }

        std::string statFile = oc.getString("stat-file");
        OutputDevice::createDeviceByOption("output-file", "routes", "routes_file.xsd");
        AGTime duration(oc.getInt("duration-d"), 0, 0);
        AGTime begin(oc.getInt("begin") % 86400);
        AGTime end(oc.getInt("end") % 86400);
        AGActivityGen actiGen(statFile, OutputDevice::getDevice(oc.getString("output-file")), net);
        actiGen.importInfoCity();
        actiGen.makeActivityTrips(duration.getDay(), begin.getTime(), end.getTime());

        if (oc.getBool("debug")) {
            WRITE_MESSAGE("\n\t ---- end of ActivityGen ----\n");
        }
        ret = 0;
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
    SystemFrame::close();
    if (ret == 0) {
        std::cout << "Success." << std::endl;
    }
    return ret;
}


/****************************************************************************/
