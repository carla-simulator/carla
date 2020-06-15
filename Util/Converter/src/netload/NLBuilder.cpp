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
/// @file    NLBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// The main interface for loading a microsim
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/SystemFrame.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/StringUtils.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/SysUtils.h>
#include <utils/common/ToString.h>
#include <utils/vehicle/SUMORouteLoaderControl.h>
#include <utils/vehicle/SUMORouteLoader.h>
#include <utils/xml/XMLSubSys.h>
#ifdef HAVE_FOX
#include <utils/foxtools/MsgHandlerSynchronized.h>
#endif
#include <mesosim/MEVehicleControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSVehicleTransfer.h>
#include <microsim/MSNet.h>
#include <microsim/devices/MSDevice.h>
#include <microsim/devices/MSDevice_ToC.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSGlobals.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/MSFrame.h>
#include <microsim/MSEdgeWeightsStorage.h>
#include <microsim/MSStateHandler.h>
#include <microsim/MSDriverState.h>
#include <traci-server/TraCIServer.h>

#include "NLHandler.h"
#include "NLEdgeControlBuilder.h"
#include "NLJunctionControlBuilder.h"
#include "NLDetectorBuilder.h"
#include "NLTriggerBuilder.h"
#include "NLBuilder.h"


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// NLBuilder::EdgeFloatTimeLineRetriever_EdgeWeight - methods
// ---------------------------------------------------------------------------
void
NLBuilder::EdgeFloatTimeLineRetriever_EdgeEffort::addEdgeWeight(const std::string& id,
        double value, double begTime, double endTime) const {
    MSEdge* edge = MSEdge::dictionary(id);
    if (edge != nullptr) {
        myNet.getWeightsStorage().addEffort(edge, begTime, endTime, value);
    } else {
        WRITE_ERROR("Trying to set the effort for the unknown edge '" + id + "'.");
    }
}


// ---------------------------------------------------------------------------
// NLBuilder::EdgeFloatTimeLineRetriever_EdgeTravelTime - methods
// ---------------------------------------------------------------------------
void
NLBuilder::EdgeFloatTimeLineRetriever_EdgeTravelTime::addEdgeWeight(const std::string& id,
        double value, double begTime, double endTime) const {
    MSEdge* edge = MSEdge::dictionary(id);
    if (edge != nullptr) {
        myNet.getWeightsStorage().addTravelTime(edge, begTime, endTime, value);
    } else {
        WRITE_ERROR("Trying to set the travel time for the unknown edge '" + id + "'.");
    }
}


// ---------------------------------------------------------------------------
// NLBuilder - methods
// ---------------------------------------------------------------------------
NLBuilder::NLBuilder(OptionsCont& oc,
                     MSNet& net,
                     NLEdgeControlBuilder& eb,
                     NLJunctionControlBuilder& jb,
                     NLDetectorBuilder& db,
                     NLHandler& xmlHandler)
    : myOptions(oc), myEdgeBuilder(eb), myJunctionBuilder(jb),
      myDetectorBuilder(db),
      myNet(net), myXMLHandler(xmlHandler) {}


NLBuilder::~NLBuilder() {}


bool
NLBuilder::build() {
    // try to build the net
    if (!load("net-file", true)) {
        return false;
    }
    if (myXMLHandler.networkVersion() == 0.) {
        throw ProcessError("Invalid network, no network version declared.");
    }
    // check whether the loaded net agrees with the simulation options
    if (myOptions.getBool("no-internal-links") && myXMLHandler.haveSeenInternalEdge() && myXMLHandler.haveSeenDefaultLength()) {
        WRITE_WARNING("Network contains internal links but option --no-internal-links is set. Vehicles will 'jump' across junctions and thus underestimate route lengths and travel times.");
    }
    buildNet();
    // @note on loading order constraints:
    // - additional-files before route-files and state-files due to referencing
    // - additional-files before weight-files since the latter might contain intermodal edge data and the intermodal net depends on the stops and public transport from the additionals

    // load additional net elements (sources, detectors, ...)
    if (myOptions.isSet("additional-files")) {
        if (!load("additional-files")) {
            return false;
        }
        // load shapes with separate handler
        NLShapeHandler sh("", myNet.getShapeContainer());
        if (!ShapeHandler::loadFiles(myOptions.getStringVector("additional-files"), sh)) {
            return false;
        }
        if (myXMLHandler.haveSeenAdditionalSpeedRestrictions()) {
            myNet.getEdgeControl().setAdditionalRestrictions();
        }
    }
    if (myOptions.getBool("junction-taz")) {
        // create a TAZ for every junction
        const MSJunctionControl& junctions = myNet.getJunctionControl();
        for (auto it = junctions.begin(); it != junctions.end(); it++) {
            const std::string sinkID = it->first + "-sink";
            const std::string sourceID = it->first + "-source";
            if (MSEdge::dictionary(sinkID) == nullptr && MSEdge::dictionary(sourceID) == nullptr) {
                // sink must be built and addd before source
                MSEdge* sink = myEdgeBuilder.buildEdge(sinkID, SumoXMLEdgeFunc::CONNECTOR, "", "", -1, 0);
                MSEdge* source = myEdgeBuilder.buildEdge(sourceID, SumoXMLEdgeFunc::CONNECTOR, "", "", -1, 0);
                sink->setOtherTazConnector(source);
                source->setOtherTazConnector(sink);
                MSEdge::dictionary(sinkID, sink);
                MSEdge::dictionary(sourceID, source);
                sink->initialize(new std::vector<MSLane*>());
                source->initialize(new std::vector<MSLane*>());
                const MSJunction* junction = it->second;
                for (const MSEdge* edge : junction->getIncoming()) {
                    if (!edge->isInternal()) {
                        const_cast<MSEdge*>(edge)->addSuccessor(sink);
                    }
                }
                for (const MSEdge* edge : junction->getOutgoing()) {
                    if (!edge->isInternal()) {
                        source->addSuccessor(const_cast<MSEdge*>(edge));
                    }
                }
            } else {
                WRITE_WARNINGF("A TAZ with id '%' already exists. Not building junction TAZ.", it->first)
            }
        }
    }
    // load weights if wished
    if (myOptions.isSet("weight-files")) {
        if (!myOptions.isUsableFileList("weight-files")) {
            return false;
        }
        // build and prepare the weights handler
        std::vector<SAXWeightsHandler::ToRetrieveDefinition*> retrieverDefs;
        //  travel time, first (always used)
        EdgeFloatTimeLineRetriever_EdgeTravelTime ttRetriever(myNet);
        retrieverDefs.push_back(new SAXWeightsHandler::ToRetrieveDefinition("traveltime", true, ttRetriever));
        //  the measure to use, then
        EdgeFloatTimeLineRetriever_EdgeEffort eRetriever(myNet);
        std::string measure = myOptions.getString("weight-attribute");
        if (!myOptions.isDefault("weight-attribute")) {
            if (measure == "CO" || measure == "CO2" || measure == "HC" || measure == "PMx" || measure == "NOx" || measure == "fuel" || measure == "electricity") {
                measure += "_perVeh";
            }
            retrieverDefs.push_back(new SAXWeightsHandler::ToRetrieveDefinition(measure, true, eRetriever));
        }
        //  set up handler
        SAXWeightsHandler handler(retrieverDefs, "");
        // start parsing; for each file in the list
        std::vector<std::string> files = myOptions.getStringVector("weight-files");
        for (std::vector<std::string>::iterator i = files.begin(); i != files.end(); ++i) {
            // report about loading when wished
            WRITE_MESSAGE("Loading weights from '" + *i + "'...");
            // parse the file
            if (!XMLSubSys::runParser(handler, *i)) {
                return false;
            }
        }
    }
    // load the previous state if wished
    if (myOptions.isSet("load-state")) {
        const std::string& f = myOptions.getString("load-state");
        long before = PROGRESS_BEGIN_TIME_MESSAGE("Loading state from '" + f + "'");
        MSStateHandler h(f, string2time(myOptions.getString("load-state.offset")));
        XMLSubSys::runParser(h, f);
        if (myOptions.isDefault("begin")) {
            myOptions.set("begin", time2string(h.getTime()));
            if (TraCIServer::getInstance() != nullptr) {
                TraCIServer::getInstance()->setTargetTime(h.getTime());
            }
        }
        if (MsgHandler::getErrorInstance()->wasInformed()) {
            return false;
        }
        if (h.getTime() != string2time(myOptions.getString("begin"))) {
            WRITE_WARNING("State was written at a different time " + time2string(h.getTime()) + " than the begin time " + myOptions.getString("begin") + "!");
        }
        PROGRESS_TIME_MESSAGE(before);
    }
    // load routes
    if (myOptions.isSet("route-files") && string2time(myOptions.getString("route-steps")) <= 0) {
        if (!load("route-files")) {
            return false;
        }
    }
    // optionally switch off traffic lights
    if (myOptions.getBool("tls.all-off")) {
        myNet.getTLSControl().switchOffAll();
    }
    WRITE_MESSAGE("Loading done.");
    return true;
}


MSNet*
NLBuilder::init(const bool isLibsumo) {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.clear();
    MSFrame::fillOptions();
    OptionsIO::getOptions();
    if (oc.processMetaOptions(OptionsIO::getArgC() < 2)) {
        SystemFrame::close();
        return nullptr;
    }
    XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"));
    if (!MSFrame::checkOptions()) {
        throw ProcessError();
    }
#ifdef HAVE_FOX
    if (oc.getInt("threads") > 1) {
        // make the output aware of threading
        MsgHandler::setFactory(&MsgHandlerSynchronized::create);
    }
#endif
    MsgHandler::initOutputOptions();
    initRandomness();
    MSFrame::setMSGlobals(oc);
    MSVehicleControl* vc = nullptr;
    if (MSGlobals::gUseMesoSim) {
        vc = new MEVehicleControl();
    } else {
        vc = new MSVehicleControl();
    }
    MSNet* net = new MSNet(vc, new MSEventControl(), new MSEventControl(), new MSEventControl());
    // need to init TraCI-Server before loading routes to catch VEHICLE_STATE_BUILT
    TraCIServer::openSocket(std::map<int, TraCIServer::CmdExecutor>());
    if (isLibsumo) {
        libsumo::Helper::registerVehicleStateListener();
    }

    NLEdgeControlBuilder eb;
    NLDetectorBuilder db(*net);
    NLJunctionControlBuilder jb(*net, db);
    NLTriggerBuilder tb;
    NLHandler handler("", *net, db, tb, eb, jb);
    tb.setHandler(&handler);
    NLBuilder builder(oc, *net, eb, jb, db, handler);
    MsgHandler::getErrorInstance()->clear();
    MsgHandler::getWarningInstance()->clear();
    MsgHandler::getMessageInstance()->clear();
    if (builder.build()) {
        // preload the routes especially for TraCI
        net->loadRoutes();
        return net;
    }
    delete net;
    throw ProcessError();
}

void
NLBuilder::initRandomness() {
    RandHelper::initRandGlobal();
    RandHelper::initRandGlobal(MSRouteHandler::getParsingRNG());
    RandHelper::initRandGlobal(MSDevice::getEquipmentRNG());
    RandHelper::initRandGlobal(OUProcess::getRNG());
    RandHelper::initRandGlobal(MSDevice_ToC::getResponseTimeRNG());
    MSLane::initRNGs(OptionsCont::getOptions());
}

void
NLBuilder::buildNet() {
    MSEdgeControl* edges = nullptr;
    MSJunctionControl* junctions = nullptr;
    SUMORouteLoaderControl* routeLoaders = nullptr;
    MSTLLogicControl* tlc = nullptr;
    std::vector<SUMOTime> stateDumpTimes;
    std::vector<std::string> stateDumpFiles;
    try {
        MSFrame::buildStreams(); // ensure streams are ready for output during building
        edges = myEdgeBuilder.build(myXMLHandler.networkVersion());
        junctions = myJunctionBuilder.build();
        junctions->postloadInitContainer();
        routeLoaders = buildRouteLoaderControl(myOptions);
        tlc = myJunctionBuilder.buildTLLogics();
        for (std::string timeStr : myOptions.getStringVector("save-state.times")) {
            stateDumpTimes.push_back(string2time(timeStr));
        }
        if (myOptions.isSet("save-state.files")) {
            stateDumpFiles = myOptions.getStringVector("save-state.files");
            if (stateDumpFiles.size() != stateDumpTimes.size()) {
                throw ProcessError("Wrong number of state file names!");
            }
        } else {
            const std::string prefix = myOptions.getString("save-state.prefix");
            const std::string suffix = myOptions.getString("save-state.suffix");
            for (std::vector<SUMOTime>::iterator i = stateDumpTimes.begin(); i != stateDumpTimes.end(); ++i) {
                stateDumpFiles.push_back(prefix + "_" + time2string(*i) + suffix);
            }
        }
    } catch (IOError& e) {
        delete edges;
        delete junctions;
        delete routeLoaders;
        delete tlc;
        throw ProcessError(e.what());
    } catch (ProcessError&) {
        delete edges;
        delete junctions;
        delete routeLoaders;
        delete tlc;
        throw;
    }
    // if anthing goes wrong after this point, the net is responsible for cleaning up
    myNet.closeBuilding(myOptions, edges, junctions, routeLoaders, tlc, stateDumpTimes, stateDumpFiles,
                        myXMLHandler.haveSeenInternalEdge(),
                        myXMLHandler.haveSeenNeighs(),
                        myXMLHandler.networkVersion());
}


bool
NLBuilder::load(const std::string& mmlWhat, const bool isNet) {
    if (!myOptions.isUsableFileList(mmlWhat)) {
        return false;
    }
    std::vector<std::string> files = myOptions.getStringVector(mmlWhat);
    for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
        const long before = PROGRESS_BEGIN_TIME_MESSAGE("Loading " + mmlWhat + " from '" + *fileIt + "'");
        if (!XMLSubSys::runParser(myXMLHandler, *fileIt, isNet)) {
            WRITE_MESSAGE("Loading of " + mmlWhat + " failed.");
            return false;
        }
        PROGRESS_TIME_MESSAGE(before);
    }
    return true;
}


SUMORouteLoaderControl*
NLBuilder::buildRouteLoaderControl(const OptionsCont& oc) {
    // build the loaders
    SUMORouteLoaderControl* loaders =  new SUMORouteLoaderControl(string2time(oc.getString("route-steps")));
    // check whether a list is existing
    if (oc.isSet("route-files") && string2time(oc.getString("route-steps")) > 0) {
        std::vector<std::string> files = oc.getStringVector("route-files");
        for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
            if (!FileHelpers::isReadable(*fileIt)) {
                throw ProcessError("The route file '" + *fileIt + "' is not accessible.");
            }
        }
        // open files for reading
        for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
            loaders->add(new SUMORouteLoader(new MSRouteHandler(*fileIt, false)));
        }
    }
    return loaders;
}


/****************************************************************************/
