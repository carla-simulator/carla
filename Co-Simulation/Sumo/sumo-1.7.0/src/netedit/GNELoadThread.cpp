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
/// @file    GNELoadThread.cpp
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// The thread that performs the loading of a Netedit-net (adapted from
// GUILoadThread)
/****************************************************************************/
#include <netbuild/NBFrame.h>
#include <netbuild/NBNetBuilder.h>
#include <netimport/NIFrame.h>
#include <netimport/NILoader.h>
#include <netwrite/NWFrame.h>
#include <utils/common/MsgRetrievingFunction.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SystemFrame.h>
#include <utils/gui/events/GUIEvent_Message.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/xml/XMLSubSys.h>

#include "GNEEvent_NetworkLoaded.h"
#include "GNELoadThread.h"
#include "GNENet.h"


// ===========================================================================
// member method definitions
// ===========================================================================
GNELoadThread::GNELoadThread(FXApp* app, MFXInterThreadEventClient* mw, FXSynchQue<GUIEvent*>& eq, FXEX::FXThreadEvent& ev) :
    FXSingleEventThread(app, mw), myParent(mw), myEventQue(eq),
    myEventThrow(ev) {
    myDebugRetriever = new MsgRetrievingFunction<GNELoadThread>(this, &GNELoadThread::retrieveMessage, MsgHandler::MT_DEBUG);
    myGLDebugRetriever = new MsgRetrievingFunction<GNELoadThread>(this, &GNELoadThread::retrieveMessage, MsgHandler::MT_GLDEBUG);
    myErrorRetriever = new MsgRetrievingFunction<GNELoadThread>(this, &GNELoadThread::retrieveMessage, MsgHandler::MT_ERROR);
    myMessageRetriever = new MsgRetrievingFunction<GNELoadThread>(this, &GNELoadThread::retrieveMessage, MsgHandler::MT_MESSAGE);
    myWarningRetriever = new MsgRetrievingFunction<GNELoadThread>(this, &GNELoadThread::retrieveMessage, MsgHandler::MT_WARNING);
    MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
}


GNELoadThread::~GNELoadThread() {
    delete myDebugRetriever;
    delete myGLDebugRetriever;
    delete myErrorRetriever;
    delete myMessageRetriever;
    delete myWarningRetriever;
}


FXint
GNELoadThread::run() {
    // register message callbacks
    MsgHandler::getMessageInstance()->addRetriever(myMessageRetriever);
    MsgHandler::getDebugInstance()->addRetriever(myDebugRetriever);
    MsgHandler::getGLDebugInstance()->addRetriever(myGLDebugRetriever);
    MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
    MsgHandler::getWarningInstance()->addRetriever(myWarningRetriever);

    GNENet* net = nullptr;

    // try to load the given configuration
    OptionsCont& oc = OptionsCont::getOptions();
    if (myFile != "" || oc.getString("sumo-net-file") != "") {
        oc.clear();
        if (!initOptions()) {
            submitEndAndCleanup(net);
            return 0;
        }
    }
    if (oc.isDefault("aggregate-warnings")) {
        oc.set("aggregate-warnings", "5");
    }
    MsgHandler::initOutputOptions();
    if (!(NIFrame::checkOptions() &&
            NBFrame::checkOptions() &&
            NWFrame::checkOptions() &&
            SystemFrame::checkOptions())) {
        // options are not valid
        WRITE_ERROR("Invalid Options. Nothing loaded");
        submitEndAndCleanup(net);
        return 0;
    }
    MsgHandler::getGLDebugInstance()->clear();
    MsgHandler::getDebugInstance()->clear();
    MsgHandler::getErrorInstance()->clear();
    MsgHandler::getWarningInstance()->clear();
    MsgHandler::getMessageInstance()->clear();

    RandHelper::initRandGlobal();
    if (!GeoConvHelper::init(oc)) {
        WRITE_ERROR("Could not build projection!");
        submitEndAndCleanup(net);
        return 0;
    }
    XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"), oc.getString("xml-validation.routes"));
    // check if Debug has to be enabled
    MsgHandler::enableDebugMessages(oc.getBool("gui-testing-debug"));
    // check if GL Debug has to be enabled
    MsgHandler::enableDebugGLMessages(oc.getBool("gui-testing-debug-gl"));
    // this netbuilder instance becomes the responsibility of the GNENet
    NBNetBuilder* netBuilder = new NBNetBuilder();

    netBuilder->applyOptions(oc);

    if (myNewNet) {
        // create new network
        net = new GNENet(netBuilder);
    } else {
        NILoader nl(*netBuilder);
        try {
            nl.load(oc);

            if (!myLoadNet) {
                WRITE_MESSAGE("Performing initial computation ...\n");
                // perform one-time processing (i.e. edge removal)
                netBuilder->compute(oc);
                // @todo remove one-time processing options!
            } else {
                // make coordinate conversion usable before first netBuilder->compute()
                GeoConvHelper::computeFinal();
            }

            if (oc.getBool("ignore-errors")) {
                MsgHandler::getErrorInstance()->clear();
            }

            // check whether any errors occurred
            if (MsgHandler::getErrorInstance()->wasInformed()) {
                throw ProcessError();
            } else {
                net = new GNENet(netBuilder);
                if (oc.getBool("lefthand")) {
                    // force initial geometry computation without volatile options because the net will look strange otherwise
                    net->computeAndUpdate(oc, false);
                }
            }
            if (myFile == "") {
                if (oc.isSet("configuration-file")) {
                    myFile = oc.getString("configuration-file");
                } else if (oc.isSet("sumo-net-file")) {
                    myFile = oc.getString("sumo-net-file");
                }
            }

        } catch (ProcessError& e) {
            if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
                WRITE_ERROR(e.what());
            }
            WRITE_ERROR("Failed to build network.");
            delete net;
            delete netBuilder;
            net = nullptr;
        } catch (std::exception& e) {
            WRITE_ERROR(e.what());
#ifdef _DEBUG
            throw;
#endif
            delete net;
            delete netBuilder;
            net = nullptr;
        }
    }
    // only a single setting file is supported
    submitEndAndCleanup(net, oc.getString("gui-settings-file"), oc.getBool("registry-viewport"));
    return 0;
}



void
GNELoadThread::submitEndAndCleanup(GNENet* net, const std::string& guiSettingsFile, const bool viewportFromRegistry) {
    // remove message callbacks
    MsgHandler::getDebugInstance()->removeRetriever(myDebugRetriever);
    MsgHandler::getGLDebugInstance()->removeRetriever(myGLDebugRetriever);
    MsgHandler::getErrorInstance()->removeRetriever(myErrorRetriever);
    MsgHandler::getWarningInstance()->removeRetriever(myWarningRetriever);
    MsgHandler::getMessageInstance()->removeRetriever(myMessageRetriever);
    // inform parent about the process
    GUIEvent* e = new GNEEvent_NetworkLoaded(net, myFile, guiSettingsFile, viewportFromRegistry);
    myEventQue.push_back(e);
    myEventThrow.signal();
}


void
GNELoadThread::fillOptions(OptionsCont& oc) {
    oc.clear();
    oc.addCallExample("--new", "start plain GUI with empty net");
    oc.addCallExample("-s <SUMO_NET>", "edit SUMO network");
    oc.addCallExample("-c <CONFIGURATION>", "edit net with options read from file");

    SystemFrame::addConfigurationOptions(oc); // this subtopic is filled here, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    GeoConvHelper::addProjectionOptions(oc);
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Building Defaults");
    oc.addOptionSubTopic("TLS Building");
    oc.addOptionSubTopic("Ramp Guessing");
    oc.addOptionSubTopic("Edge Removal");
    oc.addOptionSubTopic("Unregulated Nodes");
    oc.addOptionSubTopic("Junctions");
    oc.addOptionSubTopic("Pedestrian");
    oc.addOptionSubTopic("Bicycle");
    oc.addOptionSubTopic("Railway");
    oc.addOptionSubTopic("Formats");
    oc.addOptionSubTopic("Netedit");
    oc.addOptionSubTopic("Visualisation");
    oc.addOptionSubTopic("Time");

    oc.doRegister("new", new Option_Bool(false)); // !!!
    oc.addDescription("new", "Input", "Start with a new network");

    oc.doRegister("additional-files", 'a', new Option_FileName());
    oc.addSynonyme("additional-files", "additional");
    oc.addDescription("additional-files", "Netedit", "Load additional and shapes descriptions from FILE(s)");

    oc.doRegister("additionals-output", new Option_String());
    oc.addDescription("additionals-output", "Netedit", "file in which additionals must be saved");

    oc.doRegister("route-files", 'r', new Option_FileName());
    oc.addSynonyme("route-files", "routes");
    oc.addDescription("route-files", "Netedit", "Load demand elements descriptions from FILE(s)");

    oc.doRegister("demandelements-output", new Option_String());
    oc.addDescription("demandelements-output", "Netedit", "file in which demand elements must be saved");

    oc.doRegister("data-files", 'd', new Option_FileName());
    oc.addSynonyme("data-files", "data");
    oc.addDescription("data-files", "Netedit", "Load data elements descriptions from FILE(s)");

    oc.doRegister("dataelements-output", new Option_String());
    oc.addDescription("dataelements-output", "Netedit", "file in which data elements must be saved");

    oc.doRegister("TLSPrograms-output", new Option_String());
    oc.addDescription("TLSPrograms-output", "Netedit", "file in which TLS Programs must be saved");

    oc.doRegister("disable-laneIcons", new Option_Bool(false));
    oc.addDescription("disable-laneIcons", "Visualisation", "Disable icons of special lanes");

    oc.doRegister("disable-textures", 'T', new Option_Bool(false)); // !!!
    oc.addDescription("disable-textures", "Visualisation", "");

    oc.doRegister("gui-settings-file", 'g', new Option_FileName());
    oc.addDescription("gui-settings-file", "Visualisation", "Load visualisation settings from FILE");

    oc.doRegister("registry-viewport", new Option_Bool(false));
    oc.addDescription("registry-viewport", "Visualisation", "Load current viewport from registry");

    oc.doRegister("window-size", new Option_StringVector());
    oc.addDescription("window-size", "Visualisation", "Create initial window with the given x,y size");

    oc.doRegister("window-pos", new Option_StringVector());
    oc.addDescription("window-pos", "Visualisation", "Create initial window at the given x,y position");

    // testing

    oc.doRegister("gui-testing", new Option_Bool(false));
    oc.addDescription("gui-testing", "Visualisation", "Enable overlay for screen recognition");

    oc.doRegister("gui-testing-debug", new Option_Bool(false));
    oc.addDescription("gui-testing-debug", "Visualisation", "Enable output messages during GUI-Testing");

    oc.doRegister("gui-testing-debug-gl", new Option_Bool(false));
    oc.addDescription("gui-testing-debug-gl", "Visualisation", "Enable output messages during GUI-Testing specific of gl functions");

    oc.doRegister("gui-testing.setting-output", new Option_FileName());
    oc.addDescription("gui-testing.setting-output", "Visualisation", "Save gui settings in the given settings-output file");

    // register the simulation settings (needed for GNERouteHandler)
    oc.doRegister("begin", new Option_String("0", "TIME"));
    oc.addDescription("begin", "Time", "Defines the begin time in seconds; The simulation starts at this time");

    oc.doRegister("end", new Option_String("-1", "TIME"));
    oc.addDescription("end", "Time", "Defines the end time in seconds; The simulation ends at this time");

    oc.doRegister("default.action-step-length", new Option_Float(0.0));
    oc.addDescription("default.action-step-length", "Processing", "Length of the default interval length between action points for the car-following and lane-change models (in seconds). If not specified, the simulation step-length is used per default. Vehicle- or VType-specific settings override the default. Must be a multiple of the simulation step-length.");

    NIFrame::fillOptions(true);
    NBFrame::fillOptions(false);
    NWFrame::fillOptions(false);
    RandHelper::insertRandOptions();
}


void
GNELoadThread::setDefaultOptions(OptionsCont& oc) {
    oc.resetWritable();
    oc.set("offset.disable-normalization", "true"); // preserve the given network as far as possible
    oc.set("no-turnarounds", "true"); // otherwise it is impossible to manually removed turn-arounds
}


bool
GNELoadThread::initOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    // fill all optiones
    fillOptions(oc);
    // set manually the net file
    if (myFile != "") {
        if (myLoadNet) {
            oc.set("sumo-net-file", myFile);
        } else {
            oc.set("configuration-file", myFile);
        }
    }
    // set default options defined in GNELoadThread::setDefaultOptions(...)
    setDefaultOptions(oc);
    try {
        // set all values writables, because certain attributes already setted can be updated throught console
        oc.resetWritable();
        // load options from console
        OptionsIO::getOptions();
        // if output file wasn't defined in the command line manually, set value of "sumo-net-file"
        if (!oc.isSet("output-file")) {
            oc.set("output-file", oc.getString("sumo-net-file"));
        }
        return true;
    } catch (ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        WRITE_ERROR("Failed to parse options.");
        return false;
    }
}


void
GNELoadThread::loadConfigOrNet(const std::string& file, bool isNet, bool useStartupOptions, bool newNet) {
    myFile = file;
    myLoadNet = isNet;
    if (myFile != "" && !useStartupOptions) {
        OptionsIO::setArgs(0, nullptr);
    }
    myNewNet = newNet;
    start();
}


void
GNELoadThread::retrieveMessage(const MsgHandler::MsgType type, const std::string& msg) {
    GUIEvent* e = new GUIEvent_Message(type, msg);
    myEventQue.push_back(e);
    myEventThrow.signal();
}


/****************************************************************************/
