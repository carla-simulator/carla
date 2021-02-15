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
/// @file    GUILoadThread.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Class describing the thread that performs the loading of a simulation
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <ctime>
#include <utils/common/RandHelper.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/MsgRetrievingFunction.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsIO.h>
#include <utils/foxtools/FXSynchQue.h>
#include <utils/gui/events/GUIEvent_Message.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/xml/XMLSubSys.h>
#include <guisim/GUINet.h>
#include <guisim/GUIEventControl.h>
#include <guisim/GUIVehicleControl.h>
#include <netload/NLBuilder.h>
#include <netload/NLHandler.h>
#include <netload/NLJunctionControlBuilder.h>
#include <guinetload/GUIEdgeControlBuilder.h>
#include <guinetload/GUIDetectorBuilder.h>
#include <guinetload/GUITriggerBuilder.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/devices/MSDevice.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSFrame.h>
#include <microsim/MSRouteHandler.h>
#include <mesogui/GUIMEVehicleControl.h>
#include <traci-server/TraCIServer.h>
#include "TraCIServerAPI_GUI.h"
#include "GUIApplicationWindow.h"
#include "GUILoadThread.h"
#include "GUIGlobals.h"
#include "GUIEvent_SimulationLoaded.h"


// ===========================================================================
// member method definitions
// ===========================================================================
GUILoadThread::GUILoadThread(FXApp* app, GUIApplicationWindow* mw,
                             FXSynchQue<GUIEvent*>& eq, FXEX::FXThreadEvent& ev)
    : FXSingleEventThread(app, mw), myParent(mw), myEventQue(eq),
      myEventThrow(ev) {
    myErrorRetriever = new MsgRetrievingFunction<GUILoadThread>(this, &GUILoadThread::retrieveMessage, MsgHandler::MT_ERROR);
    myMessageRetriever = new MsgRetrievingFunction<GUILoadThread>(this, &GUILoadThread::retrieveMessage, MsgHandler::MT_MESSAGE);
    myWarningRetriever = new MsgRetrievingFunction<GUILoadThread>(this, &GUILoadThread::retrieveMessage, MsgHandler::MT_WARNING);
    MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
}


GUILoadThread::~GUILoadThread() {
    delete myErrorRetriever;
    delete myMessageRetriever;
    delete myWarningRetriever;
}


FXint
GUILoadThread::run() {
    // register message callbacks
    MsgHandler::getMessageInstance()->addRetriever(myMessageRetriever);
    MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
    MsgHandler::getWarningInstance()->addRetriever(myWarningRetriever);

    // try to load the given configuration
    OptionsCont& oc = OptionsCont::getOptions();
    try {
        if (myFile != "") {
            // triggered by menu option or reload
            oc.clear();
            MSFrame::fillOptions();
            oc.setByRootElement(OptionsIO::getRoot(myFile), myFile);
            oc.resetWritable(); // there may be command line options
            OptionsIO::getOptions();
        } else {
            // triggered at application start
            OptionsIO::loadConfiguration();
            if (oc.isSet("configuration-file")) {
                myFile = oc.getString("configuration-file");
            } else if (oc.isSet("net-file")) {
                myFile = oc.getString("net-file");
            }
            myEventQue.push_back(new GUIEvent_Message("Loading '" + myFile + "'."));
            myEventThrow.signal();
            myParent->addRecentFile(FXPath::absolute(myFile.c_str()));
        }
        myTitle = myFile;
        // within gui-based applications, nothing is reported to the console
        MsgHandler::getMessageInstance()->removeRetriever(&OutputDevice::getDevice("stdout"));
        MsgHandler::getWarningInstance()->removeRetriever(&OutputDevice::getDevice("stderr"));
        MsgHandler::getErrorInstance()->removeRetriever(&OutputDevice::getDevice("stderr"));
        // do this once again to get parsed options
        if (oc.getBool("duration-log.statistics") && oc.isDefault("verbose")) {
            // must be done before calling initOutputOptions (which checks option "verbose")
            // but initOutputOptions must come before checkOptions (so that warnings are printed)
            oc.set("verbose", "true");
        }
        MsgHandler::initOutputOptions();
        if (!MSFrame::checkOptions()) {
            throw ProcessError();
        }
        XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"), oc.getString("xml-validation.routes"));
        GUIGlobals::gRunAfterLoad = oc.getBool("start");
        GUIGlobals::gQuitOnEnd = oc.getBool("quit-on-end");
        GUIGlobals::gDemoAutoReload = oc.getBool("demo");
        GUIGlobals::gTrackerInterval = oc.getFloat("tracker-interval");
    } catch (ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        // the options are not valid but maybe we want to quit
        GUIGlobals::gQuitOnEnd = oc.getBool("quit-on-end");
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        submitEndAndCleanup(nullptr, 0, 0);
        return 0;
    }

    // initialise global settings
    NLBuilder::initRandomness();
    MSFrame::setMSGlobals(oc);
    GUITexturesHelper::allowTextures(!oc.getBool("disable-textures"));

    MSVehicleControl* vehControl = nullptr;
    GUIVisualizationSettings::UseMesoSim = MSGlobals::gUseMesoSim;
    if (MSGlobals::gUseMesoSim) {
        vehControl = new GUIMEVehicleControl();
    } else {
        vehControl = new GUIVehicleControl();
    }

    GUINet* net = nullptr;
    SUMOTime simStartTime = 0;
    SUMOTime simEndTime = 0;
    std::vector<std::string> guiSettingsFiles;
    bool osgView = false;
    GUIEdgeControlBuilder* eb = nullptr;
    try {
        net = new GUINet(
            vehControl,
            new GUIEventControl(),
            new GUIEventControl(),
            new GUIEventControl());
        // need to init TraCI-Server before loading routes to catch VEHICLE_STATE_BUILT
        std::map<int, TraCIServer::CmdExecutor> execs;
        execs[libsumo::CMD_GET_GUI_VARIABLE] = &TraCIServerAPI_GUI::processGet;
        execs[libsumo::CMD_SET_GUI_VARIABLE] = &TraCIServerAPI_GUI::processSet;
        TraCIServer::openSocket(execs);

        eb = new GUIEdgeControlBuilder();
        GUIDetectorBuilder db(*net);
        NLJunctionControlBuilder jb(*net, db);
        GUITriggerBuilder tb;
        NLHandler handler("", *net, db, tb, *eb, jb);
        tb.setHandler(&handler);
        NLBuilder builder(oc, *net, *eb, jb, db, handler);
        MsgHandler::getErrorInstance()->clear();
        MsgHandler::getWarningInstance()->clear();
        MsgHandler::getMessageInstance()->clear();
        if (!builder.build()) {
            throw ProcessError();
        } else {
            net->initGUIStructures();
            simStartTime = string2time(oc.getString("begin"));
            simEndTime = string2time(oc.getString("end"));
            guiSettingsFiles = oc.getStringVector("gui-settings-file");
#ifdef HAVE_OSG
            osgView = oc.getBool("osg-view");
#endif
            if (oc.isSet("edgedata-files")) {
                if (!oc.isUsableFileList("edgedata-files")) {
                    WRITE_ERROR("Could not load edgedata-files '" + oc.getString("edgedata-files") + "'");
                } else {
                    for (const std::string& file : oc.getStringVector("edgedata-files")) {
                        net->loadEdgeData(file);
                    }
                }
            }
        }
    } catch (ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        delete net;
        net = nullptr;
#ifndef _DEBUG
    } catch (std::exception& e) {
        WRITE_ERROR(e.what());
        delete net;
        net = nullptr;
#endif
    }
    if (net == nullptr) {
        MSNet::clearAll();
    }
    delete eb;
    submitEndAndCleanup(net, simStartTime, simEndTime, guiSettingsFiles, osgView,
                        oc.getBool("registry-viewport"));
    return 0;
}


void
GUILoadThread::submitEndAndCleanup(GUINet* net,
                                   const SUMOTime simStartTime,
                                   const SUMOTime simEndTime,
                                   const std::vector<std::string>& guiSettingsFiles,
                                   const bool osgView,
                                   const bool viewportFromRegistry) {
    // remove message callbacks
    MsgHandler::getErrorInstance()->removeRetriever(myErrorRetriever);
    MsgHandler::getWarningInstance()->removeRetriever(myWarningRetriever);
    MsgHandler::getMessageInstance()->removeRetriever(myMessageRetriever);
    // inform parent about the process
    GUIEvent* e = new GUIEvent_SimulationLoaded(net, simStartTime, simEndTime, myTitle, guiSettingsFiles, osgView, viewportFromRegistry);
    myEventQue.push_back(e);
    myEventThrow.signal();
}


void
GUILoadThread::loadConfigOrNet(const std::string& file) {
    myFile = file;
    if (myFile != "") {
        OptionsIO::setArgs(0, nullptr);
    }
    start();
}


void
GUILoadThread::retrieveMessage(const MsgHandler::MsgType type, const std::string& msg) {
    GUIEvent* e = new GUIEvent_Message(type, msg);
    myEventQue.push_back(e);
    myEventThrow.signal();
}


const std::string&
GUILoadThread::getFileName() const {
    return myFile;
}


/****************************************************************************/
