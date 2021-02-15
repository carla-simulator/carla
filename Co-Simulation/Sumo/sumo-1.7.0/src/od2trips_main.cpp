/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    od2trips_main.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Yun-Pang Floetteroed
/// @date    Thu, 12 September 2002
///
// Main for OD2TRIPS
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/RandHelper.h>
#include <utils/common/ToString.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/StringUtils.h>
#include <od/ODDistrictCont.h>
#include <od/ODDistrictHandler.h>
#include <od/ODMatrix.h>
#include <utils/common/StringUtils.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/FileHelpers.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/importio/LineReader.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// functions
// ===========================================================================
void
fillOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.addCallExample("-c <CONFIGURATION>", "run with configuration file");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    oc.addOptionSubTopic("Time");
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Defaults");
    SystemFrame::addReportOptions(oc); // fill this subtopic, too


    // register the file input options
    oc.doRegister("taz-files", 'n', new Option_FileName());
    oc.addSynonyme("taz-files", "taz");
    oc.addSynonyme("taz-files", "net-file");
    oc.addSynonyme("taz-files", "net");
    oc.addDescription("taz-files", "Input", "Loads TAZ (districts; also from networks) from FILE(s)");

    oc.doRegister("od-matrix-files", 'd', new Option_FileName());
    oc.addSynonyme("od-matrix-files", "od-files");
    oc.addSynonyme("od-matrix-files", "od");
    oc.addDescription("od-matrix-files", "Input", "Loads O/D-files from FILE(s)");

    oc.doRegister("od-amitran-files", new Option_FileName());
    oc.addSynonyme("od-amitran-files", "amitran-files");
    oc.addSynonyme("od-amitran-files", "amitran");
    oc.addDescription("od-amitran-files", "Input", "Loads O/D-matrix in Amitran format from FILE(s)");


    // register the file output options
    oc.doRegister("output-file", 'o', new Option_FileName());
    oc.addSynonyme("output-file", "output", true);
    oc.addDescription("output-file", "Output", "Writes trip definitions into FILE");

    oc.doRegister("flow-output", new Option_FileName());
    oc.addDescription("flow-output", "Output", "Writes flow definitions into FILE");

    oc.doRegister("flow-output.probability", new Option_Bool(false));
    oc.addDescription("flow-output.probability", "Output", "Writes probabilistic flow instead of evenly spaced flow");

    oc.doRegister("pedestrians", new Option_Bool(false));
    oc.addDescription("pedestrians", "Output", "Writes pedestrians instead of vehicles");

    oc.doRegister("persontrips", new Option_Bool(false));
    oc.addDescription("persontrips", "Output", "Writes persontrips instead of vehicles");

    oc.doRegister("persontrips.modes", new Option_StringVector());
    oc.addDescription("persontrips.modes", "Output", "Add modes attribute to personTrips");

    oc.doRegister("ignore-vehicle-type", new Option_Bool(false));
    oc.addSynonyme("ignore-vehicle-type", "no-vtype", true);
    oc.addDescription("ignore-vehicle-type", "Output", "Does not save vtype information");


    // register the time settings
    oc.doRegister("begin", 'b', new Option_String("0", "TIME"));
    oc.addDescription("begin", "Time", "Defines the begin time; Previous trips will be discarded");

    oc.doRegister("end", 'e', new Option_String(SUMOTIME_MAXSTRING, "TIME"));
    oc.addDescription("end", "Time", "Defines the end time; Later trips will be discarded; Defaults to the maximum time that SUMO can represent");


    // register the data processing options
    oc.doRegister("scale", 's', new Option_Float(1));
    oc.addDescription("scale", "Processing", "Scales the loaded flows by FLOAT");

    oc.doRegister("spread.uniform", new Option_Bool(false));
    oc.addDescription("spread.uniform", "Processing", "Spreads trips uniformly over each time period");

    oc.doRegister("different-source-sink", new Option_Bool(false));
    oc.addDescription("different-source-sink", "Processing", "Always choose source and sink edge which are not identical");

    oc.doRegister("vtype", new Option_String(""));
    oc.addDescription("vtype", "Processing", "Defines the name of the vehicle type to use");

    oc.doRegister("prefix", new Option_String(""));
    oc.addDescription("prefix", "Processing", "Defines the prefix for vehicle names");

    oc.doRegister("timeline", new Option_StringVector());
    oc.addDescription("timeline", "Processing", "Uses STR[] as a timeline definition");

    oc.doRegister("timeline.day-in-hours", new Option_Bool(false));
    oc.addDescription("timeline.day-in-hours", "Processing", "Uses STR as a 24h-timeline definition");

    oc.doRegister("ignore-errors", new Option_Bool(false)); // !!! describe, document
    oc.addSynonyme("ignore-errors", "dismiss-loading-errors", true);
    oc.addDescription("ignore-errors", "Report", "Continue on broken input");

    oc.doRegister("no-step-log", new Option_Bool(false));
    oc.addDescription("no-step-log", "Processing", "Disable console output of current time step");


    // register defaults options
    oc.doRegister("departlane", new Option_String("free"));
    oc.addDescription("departlane", "Defaults", "Assigns a default depart lane");

    oc.doRegister("departpos", new Option_String());
    oc.addDescription("departpos", "Defaults", "Assigns a default depart position");

    oc.doRegister("departspeed", new Option_String("max"));
    oc.addDescription("departspeed", "Defaults", "Assigns a default depart speed");

    oc.doRegister("arrivallane", new Option_String());
    oc.addDescription("arrivallane", "Defaults", "Assigns a default arrival lane");

    oc.doRegister("arrivalpos", new Option_String());
    oc.addDescription("arrivalpos", "Defaults", "Assigns a default arrival position");

    oc.doRegister("arrivalspeed", new Option_String());
    oc.addDescription("arrivalspeed", "Defaults", "Assigns a default arrival speed");

    // add rand options
    RandHelper::insertRandOptions();
}

bool
checkOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    bool ok = true;
    if (!oc.isSet("taz-files")) {
        WRITE_ERROR("No TAZ input file (-n) specified.");
        ok = false;
    }
    if (!oc.isSet("od-matrix-files") && !oc.isSet("od-amitran-files")) {
        WRITE_ERROR("No input specified.");
        ok = false;
    }
    if (!oc.isSet("output-file") && !oc.isSet("flow-output")) {
        WRITE_ERROR("No trip table output file (-o) or flow-output is specified.");
        ok = false;
    }
    if (oc.getBool("pedestrians") && oc.getBool("persontrips")) {
        WRITE_ERROR("Only one of the the options 'pedestrians' and 'persontrips' may be set.");
        ok = false;
    }
    //
    SUMOVehicleParameter p;
    std::string error;
    if (oc.isSet("departlane") && !SUMOVehicleParameter::parseDepartLane(oc.getString("departlane"), "option", "departlane", p.departLane, p.departLaneProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("departpos") && !SUMOVehicleParameter::parseDepartPos(oc.getString("departpos"), "option", "departpos", p.departPos, p.departPosProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("departspeed") && !SUMOVehicleParameter::parseDepartSpeed(oc.getString("departspeed"), "option", "departspeed", p.departSpeed, p.departSpeedProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("arrivallane") && !SUMOVehicleParameter::parseArrivalLane(oc.getString("arrivallane"), "option", "arrivallane", p.arrivalLane, p.arrivalLaneProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("arrivalpos") && !SUMOVehicleParameter::parseArrivalPos(oc.getString("arrivalpos"), "option", "arrivalpos", p.arrivalPos, p.arrivalPosProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("arrivalspeed") && !SUMOVehicleParameter::parseArrivalSpeed(oc.getString("arrivalspeed"), "option", "arrivalspeed", p.arrivalSpeed, p.arrivalSpeedProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    ok &= SystemFrame::checkOptions();
    return ok;
}




/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char** argv) {
    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("Importer of O/D-matrices for the microscopic, multi-modal traffic simulation SUMO.");
    oc.setApplicationName("od2trips", "Eclipse SUMO od2trips Version " VERSION_STRING);
    int ret = 0;
    try {
        // initialise subsystems
        XMLSubSys::init();
        fillOptions();
        OptionsIO::setArgs(argc, argv);
        OptionsIO::getOptions();
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        XMLSubSys::setValidation(oc.getString("xml-validation"), "never", "never");
        MsgHandler::initOutputOptions();
        if (!checkOptions()) {
            throw ProcessError();
        }
        RandHelper::initRandGlobal();
        // load the districts
        // check whether the user gave a net filename
        if (!oc.isSet("taz-files")) {
            throw ProcessError("You must supply a TAZ, network or districts file ('-n').");
        }
        // get the file name and set it
        ODDistrictCont districts;
        districts.loadDistricts(oc.getStringVector("taz-files"));
        if (districts.size() == 0) {
            throw ProcessError("No districts loaded.");
        }
        // load the matrix
        ODMatrix matrix(districts);
        matrix.loadMatrix(oc);
        if (matrix.getNumLoaded() == 0) {
            throw ProcessError("No vehicles loaded.");
        }
        if (MsgHandler::getErrorInstance()->wasInformed() && !oc.getBool("ignore-errors")) {
            throw ProcessError("Loading failed.");
        }
        WRITE_MESSAGE(toString(matrix.getNumLoaded()) + " vehicles loaded.");
        // apply a curve if wished
        if (oc.isSet("timeline")) {
            matrix.applyCurve(matrix.parseTimeLine(oc.getStringVector("timeline"), oc.getBool("timeline.day-in-hours")));
        }
        const std::string modes = toString(oc.getStringVector("persontrips.modes"));
        // write
        bool haveOutput = false;
        if (OutputDevice::createDeviceByOption("output-file", "routes", "routes_file.xsd")) {
            matrix.write(string2time(oc.getString("begin")), string2time(oc.getString("end")),
                         OutputDevice::getDeviceByOption("output-file"),
                         oc.getBool("spread.uniform"), oc.getBool("different-source-sink"),
                         oc.getBool("ignore-vehicle-type"),
                         oc.getString("prefix"), !oc.getBool("no-step-log"),
                         oc.getBool("pedestrians"),
                         oc.getBool("persontrips"), modes);
            haveOutput = true;
        }
        if (OutputDevice::createDeviceByOption("flow-output", "routes", "routes_file.xsd")) {
            matrix.writeFlows(string2time(oc.getString("begin")), string2time(oc.getString("end")),
                              OutputDevice::getDeviceByOption("flow-output"),
                              oc.getBool("ignore-vehicle-type"), oc.getString("prefix"),
                              oc.getBool("flow-output.probability"), oc.getBool("pedestrians"),
                              oc.getBool("persontrips"), modes);
            haveOutput = true;
        }
        if (!haveOutput) {
            throw ProcessError("No output file given.");
        }
        WRITE_MESSAGE(toString(matrix.getNumDiscarded()) + " vehicles discarded.");
        WRITE_MESSAGE(toString(matrix.getNumWritten()) + " vehicles written.");
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
