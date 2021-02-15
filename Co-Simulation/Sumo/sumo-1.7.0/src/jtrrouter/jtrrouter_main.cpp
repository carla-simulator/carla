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
/// @file    jtrrouter_main.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Jan 2004
///
// Main for JTRROUTER
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <string>
#include <limits.h>
#include <ctime>
#include <set>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/ToString.h>
#include <utils/common/RandHelper.h>
#include <utils/common/StringTokenizer.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/xml/XMLSubSys.h>
#include <router/ROFrame.h>
#include <router/ROLoader.h>
#include <router/RONet.h>
#include <router/RORouteDef.h>
#include "ROJTREdgeBuilder.h"
#include "ROJTRRouter.h"
#include "ROJTREdge.h"
#include "ROJTRTurnDefLoader.h"
#include "ROJTRFrame.h"


// ===========================================================================
// functions
// ===========================================================================
/* -------------------------------------------------------------------------
 * data processing methods
 * ----------------------------------------------------------------------- */
/**
 * loads the net
 * The net is in this meaning made up by the net itself and the dynamic
 * weights which may be supplied in a separate file
 */
void
initNet(RONet& net, ROLoader& loader,
        const std::vector<double>& turnDefs) {
    // load the net
    ROJTREdgeBuilder builder;
    loader.loadNet(net, builder);
    // set the turn defaults
    for (const auto& i : net.getEdgeMap()) {
        static_cast<ROJTREdge*>(i.second)->setTurnDefaults(turnDefs);
    }
}

std::vector<double>
getTurningDefaults(OptionsCont& oc) {
    std::vector<double> ret;
    std::vector<std::string> defs = oc.getStringVector("turn-defaults");
    if (defs.size() < 2) {
        throw ProcessError("The defaults for turnings must be a tuple of at least two numbers divided by ','.");
    }
    for (std::vector<std::string>::const_iterator i = defs.begin(); i != defs.end(); ++i) {
        try {
            double val = StringUtils::toDouble(*i);
            ret.push_back(val);
        } catch (NumberFormatException&) {
            throw ProcessError("A turn default is not numeric.");
        }
    }
    return ret;
}


void
loadJTRDefinitions(RONet& net, OptionsCont& oc) {
    // load the turning definitions (and possible sink definition)
    if (oc.isSet("turn-ratio-files")) {
        ROJTRTurnDefLoader loader(net);
        std::vector<std::string> ratio_files = oc.getStringVector("turn-ratio-files");
        for (std::vector<std::string>::const_iterator i = ratio_files.begin(); i != ratio_files.end(); ++i) {
            if (!XMLSubSys::runParser(loader, *i)) {
                throw ProcessError();
            }
        }
    }
    if (oc.getBool("sources-are-sinks") || oc.getBool("discount-sources")) {
        // load all route-files and additional files to discover sink edges and flow discount values
        ROJTRTurnDefLoader loader(net);
        for (std::string fileOption : {
                    "route-files", "additional-files"
                }) {
            for (std::string file : oc.getStringVector(fileOption)) {
                if (!XMLSubSys::runParser(loader, file)) {
                    throw ProcessError();
                }
            }
        }
    }

    if (MsgHandler::getErrorInstance()->wasInformed() && oc.getBool("ignore-errors")) {
        MsgHandler::getErrorInstance()->clear();
    }
    // parse sink edges specified at the input/within the configuration
    if (oc.isSet("sink-edges")) {
        std::vector<std::string> edges = oc.getStringVector("sink-edges");
        for (std::vector<std::string>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            ROJTREdge* edge = static_cast<ROJTREdge*>(net.getEdge(*i));
            if (edge == nullptr) {
                throw ProcessError("The edge '" + *i + "' declared as a sink is not known.");
            }
            edge->setSink();
        }
    }
}


/**
 * Computes the routes saving them
 */
void
computeRoutes(RONet& net, ROLoader& loader, OptionsCont& oc) {
    // initialise the loader
    loader.openRoutes(net);
    // prepare the output
    net.openOutput(oc);
    // build the router
    ROJTRRouter* router = new ROJTRRouter(oc.getBool("ignore-errors"), oc.getBool("accept-all-destinations"),
                                          (int)(((double) net.getEdgeNumber()) * OptionsCont::getOptions().getFloat("max-edges-factor")),
                                          oc.getBool("ignore-vclasses"),
                                          oc.getBool("allow-loops"),
                                          oc.getBool("discount-sources"));
    RORouteDef::setUsingJTRR();
    RORouterProvider provider(router, new PedestrianRouter<ROEdge, ROLane, RONode, ROVehicle>(),
                              new ROIntermodalRouter(RONet::adaptIntermodalRouter, 0, "dijkstra"), nullptr);
    loader.processRoutes(string2time(oc.getString("begin")), string2time(oc.getString("end")),
                         string2time(oc.getString("route-steps")), net, provider);
    net.cleanup();
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char** argv) {
    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("Router for the microscopic, multi-modal traffic simulation SUMO based on junction turning ratios.");
    oc.setApplicationName("jtrrouter", "Eclipse SUMO jtrrouter Version " VERSION_STRING);
    int ret = 0;
    RONet* net = nullptr;
    try {
        // initialise the application system (messaging, xml, options)
        XMLSubSys::init();
        ROJTRFrame::fillOptions();
        OptionsIO::setArgs(argc, argv);
        OptionsIO::getOptions();
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        SystemFrame::checkOptions();
        XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"), oc.getString("xml-validation.routes"));
        MsgHandler::initOutputOptions();
        if (!ROJTRFrame::checkOptions()) {
            throw ProcessError();
        }
        RandHelper::initRandGlobal();
        std::vector<double> defs = getTurningDefaults(oc);
        // load data
        ROLoader loader(oc, true, !oc.getBool("no-step-log"));
        net = new RONet();
        initNet(*net, loader, defs);
        try {
            // parse and set the turn defaults first
            loadJTRDefinitions(*net, oc);
            // build routes
            computeRoutes(*net, loader, oc);
        } catch (XERCES_CPP_NAMESPACE::SAXParseException& e) {
            WRITE_ERROR(toString(e.getLineNumber()));
            ret = 1;
        } catch (XERCES_CPP_NAMESPACE::SAXException& e) {
            WRITE_ERROR(StringUtils::transcode(e.getMessage()));
            ret = 1;
        }
        if (MsgHandler::getErrorInstance()->wasInformed()) {
            throw ProcessError();
        }
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
    delete net;
    SystemFrame::close();
    if (ret == 0) {
        std::cout << "Success." << std::endl;
    }
    return ret;
}


/****************************************************************************/
