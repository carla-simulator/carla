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
/// @file    ROLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Christian Roessel
/// @date    Sept 2002
///
// Loader for networks and route imports
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <string>
#include <iomanip>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <utils/options/OptionsCont.h>
#include <utils/common/ToString.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/xml/SAXWeightsHandler.h>
#include <utils/vehicle/SUMORouteLoader.h>
#include <utils/vehicle/SUMORouteLoaderControl.h>
#include "RONet.h"
#include "RONetHandler.h"
#include "ROLoader.h"
#include "ROLane.h"
#include "ROEdge.h"
#include "RORouteHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// ROLoader::EdgeFloatTimeLineRetriever_EdgeTravelTime - methods
// ---------------------------------------------------------------------------
void
ROLoader::EdgeFloatTimeLineRetriever_EdgeTravelTime::addEdgeWeight(const std::string& id,
        double val, double beg, double end) const {
    ROEdge* e = myNet.getEdge(id);
    if (e != nullptr) {
        e->addTravelTime(val, beg, end);
    } else {
        if (id[0] != ':') {
            if (OptionsCont::getOptions().getBool("ignore-errors")) {
                WRITE_WARNING("Trying to set a weight for the unknown edge '" + id + "'.");
            } else {
                WRITE_ERROR("Trying to set a weight for the unknown edge '" + id + "'.");
            }
        }
    }
}


// ---------------------------------------------------------------------------
// ROLoader::EdgeFloatTimeLineRetriever_EdgeWeight - methods
// ---------------------------------------------------------------------------
void
ROLoader::EdgeFloatTimeLineRetriever_EdgeWeight::addEdgeWeight(const std::string& id,
        double val, double beg, double end) const {
    ROEdge* e = myNet.getEdge(id);
    if (e != nullptr) {
        e->addEffort(val, beg, end);
    } else {
        if (id[0] != ':') {
            if (OptionsCont::getOptions().getBool("ignore-errors")) {
                WRITE_WARNING("Trying to set a weight for the unknown edge '" + id + "'.");
            } else {
                WRITE_ERROR("Trying to set a weight for the unknown edge '" + id + "'.");
            }
        }
    }
}


// ---------------------------------------------------------------------------
// ROLoader - methods
// ---------------------------------------------------------------------------
ROLoader::ROLoader(OptionsCont& oc, const bool emptyDestinationsAllowed, const bool logSteps) :
    myOptions(oc),
    myEmptyDestinationsAllowed(emptyDestinationsAllowed),
    myLogSteps(logSteps),
    myLoaders(oc.exists("unsorted-input") && oc.getBool("unsorted-input") ? 0 : DELTA_T) {
}


ROLoader::~ROLoader() {
}


void
ROLoader::loadNet(RONet& toFill, ROAbstractEdgeBuilder& eb) {
    std::string file = myOptions.getString("net-file");
    if (file == "") {
        throw ProcessError("Missing definition of network to load!");
    }
    if (!FileHelpers::isReadable(file)) {
        throw ProcessError("The network file '" + file + "' is not accessible.");
    }
    PROGRESS_BEGIN_MESSAGE("Loading net");
    RONetHandler handler(toFill, eb, !myOptions.exists("no-internal-links") || myOptions.getBool("no-internal-links"),
                         myOptions.exists("weights.minor-penalty") ? myOptions.getFloat("weights.minor-penalty") : 0);
    handler.setFileName(file);
    if (!XMLSubSys::runParser(handler, file, true)) {
        PROGRESS_FAILED_MESSAGE();
        throw ProcessError();
    } else {
        PROGRESS_DONE_MESSAGE();
    }
    if (myOptions.exists("restriction-params") && myOptions.isSet("restriction-params")) {
        const std::vector<std::string> paramKeys = myOptions.getStringVector("restriction-params");
        for (auto& edgeIt : toFill.getEdgeMap()) {
            edgeIt.second->cacheParamRestrictions(paramKeys);
        }
    }
    if (!deprecatedVehicleClassesSeen.empty()) {
        WRITE_WARNING("Deprecated vehicle classes '" + toString(deprecatedVehicleClassesSeen) + "' in input network.");
        deprecatedVehicleClassesSeen.clear();
    }
    if (myOptions.isSet("additional-files", false)) { // dfrouter does not register this option
        std::vector<std::string> files = myOptions.getStringVector("additional-files");
        for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
            if (!FileHelpers::isReadable(*fileIt)) {
                throw ProcessError("The additional file '" + *fileIt + "' is not accessible.");
            }
            PROGRESS_BEGIN_MESSAGE("Loading additional file '" + *fileIt + "' ");
            handler.setFileName(*fileIt);
            if (!XMLSubSys::runParser(handler, *fileIt)) {
                PROGRESS_FAILED_MESSAGE();
                throw ProcessError();
            } else {
                PROGRESS_DONE_MESSAGE();
            }
        }
    }
    if (myOptions.exists("junction-taz") && myOptions.getBool("junction-taz")) {
        // create a TAZ for every junction
        toFill.addJunctionTaz(eb);
    }
    toFill.setBidiEdges(handler.getBidiMap());
}


void
ROLoader::openRoutes(RONet& net) {
    // build loader
    // load relevant elements from additional file
    bool ok = openTypedRoutes("additional-files", net, true);
    // load sumo routes, trips, and flows
    ok &= openTypedRoutes("route-files", net);
    // check
    if (ok) {
        myLoaders.loadNext(string2time(myOptions.getString("begin")));
        if (!net.furtherStored()) {
            if (MsgHandler::getErrorInstance()->wasInformed()) {
                throw ProcessError();
            } else {
                const std::string error = "No route input specified or all routes were invalid.";
                if (myOptions.getBool("ignore-errors")) {
                    WRITE_WARNING(error);
                } else {
                    throw ProcessError(error);
                }
            }
        }
        // skip routes prior to the begin time
        if (!myOptions.getBool("unsorted-input")) {
            WRITE_MESSAGE("Skipped until: " + time2string(myLoaders.getFirstLoadTime()));
        }
    }
}


void
ROLoader::processRoutes(const SUMOTime start, const SUMOTime end, const SUMOTime increment,
                        RONet& net, const RORouterProvider& provider) {
    const SUMOTime absNo = end - start;
    const bool endGiven = !OptionsCont::getOptions().isDefault("end");
    // skip routes that begin before the simulation's begin
    // loop till the end
    const SUMOTime firstStep = myLoaders.getFirstLoadTime();
    SUMOTime lastStep = firstStep;
    SUMOTime time = MIN2(firstStep, end);
    while (time <= end) {
        writeStats(time, start, absNo, endGiven);
        myLoaders.loadNext(time);
        if (!net.furtherStored() || MsgHandler::getErrorInstance()->wasInformed()) {
            break;
        }
        lastStep = net.saveAndRemoveRoutesUntil(myOptions, provider, time);
        if ((!net.furtherStored() && myLoaders.haveAllLoaded()) || MsgHandler::getErrorInstance()->wasInformed()) {
            break;
        }
        if (time < end && time + increment > end) {
            time = end;
        } else {
            time += increment;
        }
    }
    if (myLogSteps) {
        WRITE_MESSAGE("Routes found between time steps " + time2string(firstStep) + " and " + time2string(lastStep) + ".");
    }
}


bool
ROLoader::openTypedRoutes(const std::string& optionName,
                          RONet& net, const bool readAll) {
    // check whether the current loader is wished
    //  and the file(s) can be used
    if (!myOptions.isUsableFileList(optionName)) {
        return !myOptions.isSet(optionName);
    }
    for (const std::string& fileIt : myOptions.getStringVector(optionName)) {
        try {
            RORouteHandler* handler = new RORouteHandler(net, fileIt, myOptions.getBool("repair"), myEmptyDestinationsAllowed, myOptions.getBool("ignore-errors"), !readAll);
            if (readAll) {
                if (!XMLSubSys::runParser(*handler, fileIt)) {
                    WRITE_ERROR("Loading of " + fileIt + " failed.");
                    return false;
                }
                delete handler;
            } else {
                myLoaders.add(new SUMORouteLoader(handler));
            }
        } catch (ProcessError& e) {
            WRITE_ERROR("The loader for " + optionName + " from file '" + fileIt + "' could not be initialised (" + e.what() + ").");
            return false;
        }
    }
    return true;
}


bool
ROLoader::loadWeights(RONet& net, const std::string& optionName,
                      const std::string& measure, const bool useLanes, const bool boundariesOverride) {
    // check whether the file exists
    if (!myOptions.isUsableFileList(optionName)) {
        return false;
    }
    // build and prepare the weights handler
    std::vector<SAXWeightsHandler::ToRetrieveDefinition*> retrieverDefs;
    //  travel time, first (always used)
    EdgeFloatTimeLineRetriever_EdgeTravelTime ttRetriever(net);
    retrieverDefs.push_back(new SAXWeightsHandler::ToRetrieveDefinition("traveltime", !useLanes, ttRetriever));
    //  the measure to use, then
    EdgeFloatTimeLineRetriever_EdgeWeight eRetriever(net);
    if (measure != "traveltime") {
        std::string umeasure = measure;
        if (measure == "CO" || measure == "CO2" || measure == "HC" || measure == "PMx" || measure == "NOx" || measure == "fuel" || measure == "electricity") {
            umeasure = measure + "_perVeh";
        }
        retrieverDefs.push_back(new SAXWeightsHandler::ToRetrieveDefinition(umeasure, !useLanes, eRetriever));
    }
    //  set up handler
    SAXWeightsHandler handler(retrieverDefs, "");
    // go through files
    std::vector<std::string> files = myOptions.getStringVector(optionName);
    for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
        PROGRESS_BEGIN_MESSAGE("Loading precomputed net weights from '" + *fileIt + "'");
        if (XMLSubSys::runParser(handler, *fileIt)) {
            PROGRESS_DONE_MESSAGE();
        } else {
            WRITE_MESSAGE("failed.");
            return false;
        }
    }
    // build edge-internal time lines
    for (const auto& i : net.getEdgeMap()) {
        i.second->buildTimeLines(measure, boundariesOverride);
    }
    return true;
}


void
ROLoader::writeStats(const SUMOTime time, const SUMOTime start, const SUMOTime absNo, bool endGiven) {
    if (myLogSteps) {
        if (endGiven) {
            const double perc = (double)(time - start) / (double) absNo;
            std::cout << "Reading up to time step: " + time2string(time) + "  (" + time2string(time - start) + "/" + time2string(absNo) + " = " + toString(perc * 100) + "% done)       \r";
        } else {
            std::cout << "Reading up to time step: " + time2string(time) + "\r";
        }
    }
}


/****************************************************************************/
