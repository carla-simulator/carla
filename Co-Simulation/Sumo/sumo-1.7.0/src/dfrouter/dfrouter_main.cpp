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
/// @file    dfrouter_main.cpp
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Thu, 16.03.2006
///
// Main for the DFROUTER
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <string>
#include <limits.h>
#include <ctime>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/ToString.h>
#include <utils/common/FileHelpers.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/xml/XMLSubSys.h>
#include <router/ROLoader.h>
#include <router/RONet.h>
#include "RODFEdgeBuilder.h"
#include "RODFFrame.h"
#include "RODFNet.h"
#include "RODFEdge.h"
#include "RODFDetector.h"
#include "RODFDetectorHandler.h"
#include "RODFRouteCont.h"
#include "RODFDetectorFlow.h"
#include "RODFDetFlowLoader.h"


// ===========================================================================
// functions
// ===========================================================================
/* -------------------------------------------------------------------------
 * data processing methods
 * ----------------------------------------------------------------------- */
void
readDetectors(RODFDetectorCon& detectors, OptionsCont& oc, RODFNet* optNet) {
    if (!oc.isSet("detector-files")) {
        throw ProcessError("No detector file given (use --detector-files <FILE>).");
    }
    // read definitions stored in XML-format
    std::vector<std::string> files = oc.getStringVector("detector-files");
    for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
        if (!FileHelpers::isReadable(*fileIt)) {
            throw ProcessError("Could not open detector file '" + *fileIt + "'");
        }
        PROGRESS_BEGIN_MESSAGE("Loading detector definitions from '" + *fileIt + "'");
        RODFDetectorHandler handler(optNet, oc.getBool("ignore-invalid-detectors"), detectors, *fileIt);
        if (XMLSubSys::runParser(handler, *fileIt)) {
            PROGRESS_DONE_MESSAGE();
        } else {
            PROGRESS_FAILED_MESSAGE();
            throw ProcessError();
        }
    }
    if (detectors.getDetectors().empty()) {
        throw ProcessError("No detectors found.");
    }
}


void
readDetectorFlows(RODFDetectorFlows& flows, OptionsCont& oc, RODFDetectorCon& dc) {
    if (!oc.isSet("measure-files")) {
        // ok, not given, return an empty container
        return;
    }
    // check whether the file exists
    std::vector<std::string> files = oc.getStringVector("measure-files");
    for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
        if (!FileHelpers::isReadable(*fileIt)) {
            throw ProcessError("The measure-file '" + *fileIt + "' can not be opened.");
        }
        // parse
        PROGRESS_BEGIN_MESSAGE("Loading flows from '" + *fileIt + "'");
        RODFDetFlowLoader dfl(dc, flows, string2time(oc.getString("begin")), string2time(oc.getString("end")),
                              string2time(oc.getString("time-offset")), string2time(oc.getString("time-factor")));
        dfl.read(*fileIt);
        PROGRESS_DONE_MESSAGE();
    }
}


void
startComputation(RODFNet* optNet, RODFDetectorFlows& flows, RODFDetectorCon& detectors, OptionsCont& oc) {
    if (oc.getBool("print-absolute-flows")) {
        flows.printAbsolute();
    }

    // if a network was loaded... (mode1)
    if (optNet != nullptr) {
        if (oc.getBool("remove-empty-detectors")) {
            PROGRESS_BEGIN_MESSAGE("Removing empty detectors");
            optNet->removeEmptyDetectors(detectors, flows);
            PROGRESS_DONE_MESSAGE();
        } else  if (oc.getBool("report-empty-detectors")) {
            PROGRESS_BEGIN_MESSAGE("Scanning for empty detectors");
            optNet->reportEmptyDetectors(detectors, flows);
            PROGRESS_DONE_MESSAGE();
        }
        // compute the detector types (optionally)
        if (!detectors.detectorsHaveCompleteTypes() || oc.getBool("revalidate-detectors")) {
            optNet->computeTypes(detectors, oc.getBool("strict-sources"));
        }
        std::vector<RODFDetector*>::const_iterator i = detectors.getDetectors().begin();
        for (; i != detectors.getDetectors().end(); ++i) {
            if ((*i)->getType() == SOURCE_DETECTOR) {
                break;
            }
        }
        if (i == detectors.getDetectors().end() && !oc.getBool("routes-for-all")) {
            throw ProcessError("No source detectors found.");
        }
        // compute routes between the detectors (optionally)
        if (!detectors.detectorsHaveRoutes() || oc.getBool("revalidate-routes") || oc.getBool("guess-empty-flows")) {
            PROGRESS_BEGIN_MESSAGE("Computing routes");
            optNet->buildRoutes(detectors,
                                oc.getBool("keep-unfinished-routes"), oc.getBool("routes-for-all"),
                                !oc.getBool("keep-longer-routes"), oc.getInt("max-search-depth"));
            PROGRESS_DONE_MESSAGE();
        }
    }

    // check
    // whether the detectors are valid
    if (!detectors.detectorsHaveCompleteTypes()) {
        throw ProcessError("The detector types are not defined; use in combination with a network");
    }
    // whether the detectors have routes
    if (!detectors.detectorsHaveRoutes()) {
        throw ProcessError("The emitters have no routes; use in combination with a network");
    }

    // save the detectors if wished
    if (oc.isSet("detector-output")) {
        detectors.save(oc.getString("detector-output"));
    }
    // save their positions as POIs if wished
    if (oc.isSet("detectors-poi-output")) {
        detectors.saveAsPOIs(oc.getString("detectors-poi-output"));
    }

    // save the routes file if it was changed or it's wished
    if (detectors.detectorsHaveRoutes() && oc.isSet("routes-output")) {
        detectors.saveRoutes(oc.getString("routes-output"));
    }

    // guess flows if wished
    if (oc.getBool("guess-empty-flows")) {
        optNet->buildDetectorDependencies(detectors);
        detectors.guessEmptyFlows(flows);
    }

    const SUMOTime begin = string2time(oc.getString("begin"));
    const SUMOTime end = string2time(oc.getString("end"));
    const SUMOTime step = string2time(oc.getString("time-step"));

    // save emitters if wished
    if (oc.isSet("emitters-output") || oc.isSet("emitters-poi-output")) {
        optNet->buildEdgeFlowMap(flows, detectors, begin, end, step); // !!!
        if (oc.getBool("revalidate-flows")) {
            PROGRESS_BEGIN_MESSAGE("Rechecking loaded flows");
            optNet->revalidateFlows(detectors, flows, begin, end, step);
            PROGRESS_DONE_MESSAGE();
        }
        if (oc.isSet("emitters-output")) {
            PROGRESS_BEGIN_MESSAGE("Writing emitters");
            detectors.writeEmitters(oc.getString("emitters-output"), flows,
                                    begin, end, step,
                                    *optNet,
                                    oc.getBool("calibrator-output"),
                                    oc.getBool("include-unused-routes"),
                                    oc.getFloat("scale"),
//                                    oc.getInt("max-search-depth"),
                                    oc.getBool("emissions-only"));
            PROGRESS_DONE_MESSAGE();
        }
        if (oc.isSet("emitters-poi-output")) {
            PROGRESS_BEGIN_MESSAGE("Writing emitter pois");
            detectors.writeEmitterPOIs(oc.getString("emitters-poi-output"), flows);
            PROGRESS_DONE_MESSAGE();
        }
    }
    // save end speed trigger if wished
    if (oc.isSet("variable-speed-sign-output")) {
        PROGRESS_BEGIN_MESSAGE("Writing speed triggers");
        detectors.writeSpeedTrigger(optNet, oc.getString("variable-speed-sign-output"), flows,
                                    begin, end, step);
        PROGRESS_DONE_MESSAGE();
    }
    // save checking detectors if wished
    if (oc.isSet("validation-output")) {
        PROGRESS_BEGIN_MESSAGE("Writing validation detectors");
        detectors.writeValidationDetectors(oc.getString("validation-output"),
                                           oc.getBool("validation-output.add-sources"), true, true); // !!!
        PROGRESS_DONE_MESSAGE();
    }
    // build global rerouter on end if wished
    if (oc.isSet("end-reroute-output")) {
        PROGRESS_BEGIN_MESSAGE("Writing highway end rerouter");
        detectors.writeEndRerouterDetectors(oc.getString("end-reroute-output")); // !!!
        PROGRESS_DONE_MESSAGE();
    }
    /*
       // save the insertion definitions
       if(oc.isSet("flow-definitions")) {
           buildVehicleEmissions(oc.getString("flow-definitions"));
       }
    */
    //
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char** argv) {
    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("Builds vehicle routes for SUMO using detector values.");
    oc.setApplicationName("dfrouter", "Eclipse SUMO dfrouter Version " VERSION_STRING);
    int ret = 0;
    RODFNet* net = nullptr;
    RODFDetectorCon* detectors = nullptr;
    RODFDetectorFlows* flows = nullptr;
    try {
        // initialise the application system (messaging, xml, options)
        XMLSubSys::init();
        RODFFrame::fillOptions();
        OptionsIO::setArgs(argc, argv);
        OptionsIO::getOptions();
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        SystemFrame::checkOptions();
        XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"), "never");
        MsgHandler::initOutputOptions();
        if (!RODFFrame::checkOptions()) {
            throw ProcessError();
        }
        RandHelper::initRandGlobal();
        // load data
        ROLoader loader(oc, false, !oc.getBool("no-step-log"));
        net = new RODFNet(oc.getBool("highway-mode"));
        RODFEdgeBuilder builder;
        loader.loadNet(*net, builder);
        net->buildApproachList();
        // load detectors
        detectors = new RODFDetectorCon();
        readDetectors(*detectors, oc, net);
        // load detector values
        flows = new RODFDetectorFlows(string2time(oc.getString("begin")), string2time(oc.getString("end")),
                                      string2time(oc.getString("time-step")));
        readDetectorFlows(*flows, oc, *detectors);
        // build routes
        startComputation(net, *flows, *detectors, oc);
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
    delete flows;
    delete detectors;
    SystemFrame::close();
    if (ret == 0) {
        std::cout << "Success." << std::endl;
    }
    return ret;
}


/****************************************************************************/
