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
/// @file    RODUAFrame.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Sets and checks options for dua-routing
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <fstream>
#include <ctime>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include "RODUAFrame.h"
#include <router/ROFrame.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SystemFrame.h>
#include <utils/vehicle/SUMOVehicleParameter.h>


// ===========================================================================
// method definitions
// ===========================================================================
void
RODUAFrame::fillOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.addCallExample("-c <CONFIGURATION>", "run routing with options from file");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Defaults");
    oc.addOptionSubTopic("Time");

    // insert options
    ROFrame::fillOptions(oc);
    // to make the transition from --trip-files easier, but has a conflict with jtrrouter
    oc.addSynonyme("route-files", "t", true);
    addImportOptions();
    addDUAOptions();
    // add rand options
    RandHelper::insertRandOptions();
}


void
RODUAFrame::addImportOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.doRegister("alternatives-output", new Option_FileName());
    oc.addSynonyme("alternatives-output", "alternatives");
    oc.addDescription("alternatives-output", "Output", "Write generated route alternatives to FILE");

    oc.doRegister("intermodal-network-output", new Option_FileName());
    oc.addDescription("intermodal-network-output", "Output", "Write edge splits and connectivity to FILE");

    oc.doRegister("intermodal-weight-output", new Option_FileName());
    oc.addDescription("intermodal-weight-output", "Output", "Write intermodal edges with lengths and travel times to FILE");

    oc.doRegister("write-trips", new Option_Bool(false));
    oc.addDescription("write-trips", "Output", "Write trips instead of vehicles (for validating trip input)");

    oc.doRegister("write-trips.geo", new Option_Bool(false));
    oc.addDescription("write-trips.geo", "Output", "Write trips with geo-coordinates");

    oc.doRegister("write-trips.junctions", new Option_Bool(false));
    oc.addDescription("write-trips.junctions", "Output", "Write trips with fromJunction and toJunction");

    // register import options
    oc.doRegister("weight-files", 'w', new Option_FileName());
    oc.addSynonyme("weight-files", "weights");
    oc.addDescription("weight-files", "Input", "Read network weights from FILE(s)");

    oc.doRegister("lane-weight-files", new Option_FileName());
    oc.addDescription("lane-weight-files", "Input", "Read lane-based network weights from FILE(s)");

    oc.doRegister("weight-attribute", 'x', new Option_String("traveltime"));
    oc.addSynonyme("weight-attribute", "measure", true);
    oc.addDescription("weight-attribute", "Input", "Name of the xml attribute which gives the edge weight");

    // register further processing options
    // ! The subtopic "Processing" must be initialised earlier !
    oc.doRegister("weights.expand", new Option_Bool(false));
    oc.addSynonyme("weights.expand", "expand-weights", true);
    oc.addDescription("weights.expand", "Processing", "Expand weights behind the simulation's end");

    oc.doRegister("weights.random-factor", new Option_Float(1.));
    oc.addDescription("weights.random-factor", "Processing", "Edge weights for routing are dynamically disturbed by a random factor drawn uniformly from [1,FLOAT)");

    oc.doRegister("routing-algorithm", new Option_String("dijkstra"));
    oc.addDescription("routing-algorithm", "Processing", "Select among routing algorithms ['dijkstra', 'astar', 'CH', 'CHWrapper']");

    oc.doRegister("weight-period", new Option_String("3600", "TIME"));
    oc.addDescription("weight-period", "Processing", "Aggregation period for the given weight files; triggers rebuilding of Contraction Hierarchy");

    oc.doRegister("weights.priority-factor", new Option_Float(0));
    oc.addDescription("weights.priority-factor", "Processing", "Consider edge priorities in addition to travel times, weighted by factor");

    oc.doRegister("astar.all-distances", new Option_FileName());
    oc.addDescription("astar.all-distances", "Processing", "Initialize lookup table for astar from the given file (generated by marouter --all-pairs-output)");

    oc.doRegister("astar.landmark-distances", new Option_FileName());
    oc.addDescription("astar.landmark-distances", "Processing", "Initialize lookup table for astar ALT-variant from the given file");

    oc.doRegister("astar.save-landmark-distances", new Option_FileName());
    oc.addDescription("astar.save-landmark-distances", "Processing", "Save lookup table for astar ALT-variant to the given file");
}


void
RODUAFrame::addDUAOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    // register Gawron's DUE-settings
    oc.doRegister("gawron.beta", new Option_Float(double(0.3)));
    oc.addSynonyme("gawron.beta", "gBeta", true);
    oc.addDescription("gawron.beta", "Processing", "Use FLOAT as Gawron's beta");

    oc.doRegister("gawron.a", new Option_Float(double(0.05)));
    oc.addSynonyme("gawron.a", "gA", true);
    oc.addDescription("gawron.a", "Processing", "Use FLOAT as Gawron's a");

    oc.doRegister("exit-times", new Option_Bool(false));
    oc.addDescription("exit-times", "Output", "Write exit times (weights) for each edge");

    oc.doRegister("keep-all-routes", new Option_Bool(false));
    oc.addDescription("keep-all-routes", "Processing", "Save routes with near zero probability");

    oc.doRegister("skip-new-routes", new Option_Bool(false));
    oc.addDescription("skip-new-routes", "Processing", "Only reuse routes from input, do not calculate new ones");

    oc.doRegister("ptline-routing", new Option_Bool(false));
    oc.addDescription("ptline-routing", "Processing", "Route all public transport input");

    oc.doRegister("logit", new Option_Bool(false)); // deprecated
    oc.addDescription("logit", "Processing", "Use c-logit model (deprecated in favor of --route-choice-method logit)");

    oc.doRegister("route-choice-method", new Option_String("gawron"));
    oc.addDescription("route-choice-method", "Processing", "Choose a route choice method: gawron, logit, or lohse");

    oc.doRegister("logit.beta", new Option_Float(double(-1)));
    oc.addSynonyme("logit.beta", "lBeta", true);
    oc.addDescription("logit.beta", "Processing", "Use FLOAT as logit's beta");

    oc.doRegister("logit.gamma", new Option_Float(double(1)));
    oc.addSynonyme("logit.gamma", "lGamma", true);
    oc.addDescription("logit.gamma", "Processing", "Use FLOAT as logit's gamma");

    oc.doRegister("logit.theta", new Option_Float(double(-1)));
    oc.addSynonyme("logit.theta", "lTheta", true);
    oc.addDescription("logit.theta", "Processing", "Use FLOAT as logit's theta (negative values mean auto-estimation)");

    oc.doRegister("persontrip.walkfactor", new Option_Float(double(0.75)));
    oc.addDescription("persontrip.walkfactor", "Processing", "Use FLOAT as a factor on pedestrian maximum speed during intermodal routing");

    oc.doRegister("persontrip.transfer.car-walk", new Option_StringVector(StringVector({ "parkingAreas" })));
    oc.addDescription("persontrip.transfer.car-walk", "Processing",
                      "Where are mode changes from car to walking allowed (possible values: 'parkingAreas', 'ptStops', 'allJunctions' and combinations)");

    oc.doRegister("persontrip.transfer.taxi-walk", new Option_StringVector());
    oc.addDescription("persontrip.transfer.taxi-walk", "Processing", "Where taxis can drop off customers ('allJunctions, 'ptStops')");

    oc.doRegister("persontrip.transfer.walk-taxi", new Option_StringVector());
    oc.addDescription("persontrip.transfer.walk-taxi", "Processing", "Where taxis can pick up customers ('allJunctions, 'ptStops')");

    oc.doRegister("railway.max-train-length", new Option_Float(5000.0));
    oc.addDescription("railway.max-train-length", "Processing", "Use FLOAT as a maximum train length when initializing the railway router");
}


bool
RODUAFrame::checkOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    bool ok = ROFrame::checkOptions(oc);

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

    if (oc.getString("routing-algorithm") != "dijkstra" && oc.getString("weight-attribute") != "traveltime") {
        WRITE_ERROR("Routing algorithm '" + oc.getString("routing-algorithm") + "' does not support weight-attribute '" + oc.getString("weight-attribute") + "'.");
        return false;
    }
    if (oc.getBool("bulk-routing") && (oc.getString("routing-algorithm") == "CH" || oc.getString("routing-algorithm") == "CHWrapper")) {
        WRITE_ERROR("Routing algorithm '" + oc.getString("routing-algorithm") + "' does not support bulk routing.");
        return false;
    }
    if (oc.isDefault("routing-algorithm") && (oc.isSet("astar.all-distances") || oc.isSet("astar.landmark-distances") || oc.isSet("astar.save-landmark-distances"))) {
        oc.set("routing-algorithm", "astar");
    }

    if (oc.getString("route-choice-method") != "gawron" && oc.getString("route-choice-method") != "logit") {
        WRITE_ERROR("Invalid route choice method '" + oc.getString("route-choice-method") + "'.");
        return false;
    }
    if (oc.getBool("logit")) {
        WRITE_WARNING("The --logit option is deprecated, please use --route-choice-method logit.");
        oc.set("route-choice-method", "logit");
    }

    if (oc.isSet("output-file") && !oc.isSet("alternatives-output")) {
        const std::string& filename = oc.getString("output-file");
        const int len = (int)filename.length();
        if (len > 4 && filename.substr(len - 4) == ".xml") {
            oc.set("alternatives-output", filename.substr(0, len - 4) + ".alt.xml");
        } else if (len > 4 && filename.substr(len - 3) == ".gz") {
            oc.set("alternatives-output", filename.substr(0, len - 3) + ".alt.gz");
        } else {
            WRITE_WARNING("Cannot derive file name for alternatives output, skipping it.");
        }
    }
    if (oc.getBool("write-trips.junctions")) {
        if (oc.isDefault("write-trips")) {
            oc.set("write-trips", "true");
        } else if (!oc.getBool("write-trips")) {
            WRITE_WARNING("Option --write-trips.junctions takes no affect when --write-trips is disabled.");
        }
    }
    return ok;
}


/****************************************************************************/
