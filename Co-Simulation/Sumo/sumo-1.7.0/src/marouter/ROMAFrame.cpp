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
/// @file    ROMAFrame.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Sets and checks options for ma-routing
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
#include "ROMAFrame.h"
#include <router/ROFrame.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SystemFrame.h>
#include <utils/vehicle/SUMOVehicleParameter.h>


// ===========================================================================
// method definitions
// ===========================================================================
void
ROMAFrame::fillOptions() {
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
    addImportOptions();
    addAssignmentOptions();
    // add rand options
    RandHelper::insertRandOptions();
}


void
ROMAFrame::addImportOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    // register import options
    oc.doRegister("output-file", 'o', new Option_FileName());
    oc.addSynonyme("output-file", "output");
    oc.addDescription("output-file", "Output", "Write flow definitions with route distributions to FILE");

    oc.doRegister("vtype-output", new Option_FileName());
    oc.addDescription("vtype-output", "Output", "Write used vehicle types into separate FILE");

    oc.doRegister("ignore-vehicle-type", new Option_Bool(false));
    oc.addSynonyme("ignore-vehicle-type", "no-vtype", true);
    oc.addDescription("ignore-vehicle-type", "Output", "Does not save vtype information");

    oc.doRegister("netload-output", new Option_FileName());
    oc.addDescription("netload-output", "Output", "Writes edge loads and final costs into FILE");

    oc.doRegister("all-pairs-output", new Option_FileName());
    oc.addDescription("all-pairs-output", "Output", "Writes complete distance matrix into FILE");

    oc.doRegister("net-file", 'n', new Option_FileName());
    oc.addSynonyme("net-file", "net");
    oc.addDescription("net-file", "Input", "Use FILE as SUMO-network to route on");

    oc.doRegister("additional-files", 'd', new Option_FileName());
    oc.addSynonyme("additional-files", "additional");
    oc.addSynonyme("additional-files", "taz-files");
    oc.addSynonyme("additional-files", "districts", true);
    oc.addDescription("additional-files", "Input", "Read additional network data (districts, bus stops) from FILE");

    oc.doRegister("od-matrix-files", 'm', new Option_FileName());
    oc.addSynonyme("od-matrix-files", "od-files");
    oc.addDescription("od-matrix-files", "Input", "Loads O/D-files from FILE(s)");

    oc.doRegister("od-amitran-files", new Option_FileName());
    oc.addSynonyme("od-amitran-files", "amitran-files");
    oc.addSynonyme("od-amitran-files", "amitran");
    oc.addDescription("od-amitran-files", "Input", "Loads O/D-matrix in Amitran format from FILE(s)");

    oc.doRegister("route-files", 'r', new Option_FileName());
    oc.addSynonyme("route-files", "routes");
    oc.addSynonyme("route-files", "trips");
    oc.addSynonyme("route-files", "trip-files");
    oc.addDescription("route-files", "Input", "Read sumo-routes or trips from FILE(s)");

    oc.doRegister("weight-files", 'w', new Option_FileName());
    oc.addSynonyme("weight-files", "weights");
    oc.addDescription("weight-files", "Input", "Read network weights from FILE(s)");

    oc.doRegister("lane-weight-files", new Option_FileName());
    oc.addDescription("lane-weight-files", "Input", "Read lane-based network weights from FILE(s)");

    oc.doRegister("weight-attribute", 'x', new Option_String("traveltime"));
    oc.addSynonyme("weight-attribute", "measure", true);
    oc.addDescription("weight-attribute", "Input", "Name of the xml attribute which gives the edge weight");

    oc.doRegister("weight-adaption", new Option_Float(0.));
    oc.addDescription("weight-adaption", "Input", "The travel time influence of prior intervals");

    oc.doRegister("taz-param", new Option_StringVector());
    oc.addDescription("taz-param", "Input", "Parameter key(s) defining source (and sink) taz");

    oc.doRegister("junction-taz", new Option_Bool(false));
    oc.addDescription("junction-taz", "Input", "Initialize a TAZ for every junction to use attributes toJunction and fromJunction");

    // need to do this here to be able to check for network and route input options
    SystemFrame::addReportOptions(oc);

    // register the time settings
    oc.doRegister("begin", 'b', new Option_String("0", "TIME"));
    oc.addDescription("begin", "Time", "Defines the begin time; Previous trips will be discarded");

    oc.doRegister("end", 'e', new Option_String(SUMOTIME_MAXSTRING, "TIME"));
    oc.addDescription("end", "Time", "Defines the end time; Later trips will be discarded; Defaults to the maximum time that SUMO can represent");

    // register the processing options
    oc.doRegister("aggregation-interval", new Option_String("3600", "TIME"));
    oc.addDescription("aggregation-interval", "Processing", "Defines the time interval when aggregating single vehicle input; Defaults to one hour");

    oc.doRegister("ignore-errors", new Option_Bool(false));
    oc.addSynonyme("ignore-errors", "continue-on-unbuild", true);
    oc.addSynonyme("ignore-errors", "dismiss-loading-errors", true);
    oc.addDescription("ignore-errors", "Report", "Continue if a route could not be build");

    oc.doRegister("max-alternatives", new Option_Integer(5));
    oc.addDescription("max-alternatives", "Processing", "Prune the number of alternatives to INT");

    oc.doRegister("weights.interpolate", new Option_Bool(false));
    oc.addSynonyme("weights.interpolate", "interpolate", true);
    oc.addDescription("weights.interpolate", "Processing", "Interpolate edge weights at interval boundaries");

    oc.doRegister("weights.expand", new Option_Bool(false));
    oc.addSynonyme("weights.expand", "expand-weights", true);
    oc.addDescription("weights.expand", "Processing", "Expand weights behind the simulation's end");

    oc.doRegister("weights.priority-factor", new Option_Float(0));
    oc.addDescription("weights.priority-factor", "Processing", "Consider edge priorities in addition to travel times, weighted by factor");

    oc.doRegister("routing-algorithm", new Option_String("dijkstra"));
    oc.addDescription("routing-algorithm", "Processing", "Select among routing algorithms ['dijkstra', 'astar', 'CH', 'CHWrapper']");

    oc.doRegister("bulk-routing.vtypes", new Option_Bool(false));
    oc.addDescription("bulk-routing.vtypes", "Processing", "Aggregate routing queries with the same origin for different vehicle types");

    oc.doRegister("routing-threads", new Option_Integer(0));
    oc.addDescription("routing-threads", "Processing", "The number of parallel execution threads used for routing");

    oc.doRegister("weight-period", new Option_String("3600", "TIME"));
    oc.addDescription("weight-period", "Processing", "Aggregation period for the given weight files; triggers rebuilding of Contraction Hierarchy");

    // register defaults options
    oc.doRegister("flow-output.departlane", new Option_String("free"));
    oc.addSynonyme("flow-output.departlane", "departlane");
    oc.addDescription("flow-output.departlane", "Defaults", "Assigns a default depart lane");

    oc.doRegister("flow-output.departpos", new Option_String());
    oc.addSynonyme("flow-output.departpos", "departpos");
    oc.addDescription("flow-output.departpos", "Defaults", "Assigns a default depart position");

    oc.doRegister("flow-output.departspeed", new Option_String("max"));
    oc.addSynonyme("flow-output.departspeed", "departspeed");
    oc.addDescription("flow-output.departspeed", "Defaults", "Assigns a default depart speed");

    oc.doRegister("flow-output.arrivallane", new Option_String());
    oc.addSynonyme("flow-output.arrivallane", "arrivallane");
    oc.addDescription("flow-output.arrivallane", "Defaults", "Assigns a default arrival lane");

    oc.doRegister("flow-output.arrivalpos", new Option_String());
    oc.addSynonyme("flow-output.arrivalpos", "arrivalpos");
    oc.addDescription("flow-output.arrivalpos", "Defaults", "Assigns a default arrival position");

    oc.doRegister("flow-output.arrivalspeed", new Option_String());
    oc.addSynonyme("flow-output.arrivalspeed", "arrivalspeed");
    oc.addDescription("flow-output.arrivalspeed", "Defaults", "Assigns a default arrival speed");

}


void
ROMAFrame::addAssignmentOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    // register the data processing options
    oc.doRegister("scale", 's', new Option_Float(1));
    oc.addDescription("scale", "Processing", "Scales the loaded flows by FLOAT");

    oc.doRegister("vtype", new Option_String(""));
    oc.addDescription("vtype", "Processing", "Defines the name of the vehicle type to use");

    oc.doRegister("prefix", new Option_String(""));
    oc.addDescription("prefix", "Processing", "Defines the prefix for vehicle flow names");

    oc.doRegister("timeline", new Option_StringVector());
    oc.addDescription("timeline", "Processing", "Uses STR[] as a timeline definition");

    oc.doRegister("timeline.day-in-hours", new Option_Bool(false));
    oc.addDescription("timeline.day-in-hours", "Processing", "Uses STR as a 24h-timeline definition");

    oc.doRegister("additive-traffic", new Option_Bool(false));
    oc.addDescription("additive-traffic", "Processing", "Keep traffic flows of all time slots in the net");

    // register macroscopic SUE-settings
    oc.doRegister("assignment-method", new Option_String("incremental"));
    oc.addDescription("assignment-method", "Processing", "Choose a assignment method: incremental, UE or SUE");

    oc.doRegister("tolerance", new Option_Float(double(0.001)));
    oc.addDescription("tolerance", "Processing", "Use FLOAT as tolerance when checking for SUE stability");

    oc.doRegister("left-turn-penalty", new Option_Float(0.));
    oc.addDescription("left-turn-penalty", "Processing", "Use left-turn penalty FLOAT to calculate link travel time when searching routes");

    oc.doRegister("paths",  new Option_Integer(1));
    oc.addDescription("paths", "Processing", "Use INTEGER as the number of paths needed to be searched for each OD pair at each iteration");

    oc.doRegister("paths.penalty", new Option_Float(double(1)));
    oc.addDescription("paths.penalty", "Processing", "Penalize existing routes with FLOAT to find secondary routes");

    oc.doRegister("upperbound", new Option_Float(double(0.5)));
    oc.addSynonyme("upperbound", "upper", true);
    oc.addDescription("upperbound", "Processing", "Use FLOAT as the upper bound to determine auxiliary link cost");

    oc.doRegister("lowerbound", new Option_Float(double(0.15)));
    oc.addSynonyme("lowerbound", "lower", true);
    oc.addDescription("lowerbound", "Processing", "Use FLOAT as the lower bound to determine auxiliary link cost");

    oc.doRegister("max-iterations", 'i', new Option_Integer(20));
    oc.addDescription("max-iterations", "Processing", "maximal number of iterations for new route searching in incremental and stochastic user assignment");

    oc.doRegister("max-inner-iterations", new Option_Integer(1000));
    oc.addDescription("max-inner-iterations", "Processing", "maximal number of inner iterations for user equilibrium calculation in the stochastic user assignment");

    // register route choice settings
    oc.doRegister("route-choice-method", new Option_String("logit"));
    oc.addDescription("route-choice-method", "Processing", "Choose a route choice method: gawron, logit, or lohse");

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

    oc.doRegister("logit.beta", new Option_Float(double(0.15)));  // check: remove the default?
    oc.addSynonyme("logit.beta", "lBeta", true);
    oc.addDescription("logit.beta", "Processing", "Use FLOAT as (c-)logit's beta for the commonality factor");

    oc.doRegister("logit.gamma", new Option_Float(double(1)));
    oc.addSynonyme("logit.gamma", "lGamma", true);
    oc.addDescription("logit.gamma", "Processing", "Use FLOAT as (c-)logit's gamma for the commonality factor");

    oc.doRegister("logit.theta", new Option_Float(double(0.01)));
    oc.addSynonyme("logit.theta", "lTheta", true);
    oc.addDescription("logit.theta", "Processing", "Use FLOAT as (c-)logit's theta");
}


bool
ROMAFrame::checkOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    if (oc.isSet("assignment-method") && oc.getString("assignment-method") != "incremental" && oc.getString("assignment-method") != "UE" && oc.getString("assignment-method") != "SUE") {
        WRITE_ERROR("invalid assignment method");
        return false;
    }
    if (oc.getString("route-choice-method") != "gawron" && oc.getString("route-choice-method") != "logit" && oc.getString("route-choice-method") != "lohse") {
        WRITE_ERROR("invalid route choice method");
        return false;
    }
    return true;
}


/****************************************************************************/
