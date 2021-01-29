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
/// @file    MSFrame.cpp
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Axel Wegener
/// @author  Thimor Bohn
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Sets and checks options for microsim; inits global outputs and settings
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <microsim/MSBaseVehicle.h>
#include <microsim/MSJunction.h>
#include <microsim/MSRoute.h>
#include <microsim/MSNet.h>
#include <microsim/MSGlobals.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/devices/MSDevice.h>
#include <microsim/devices/MSDevice_Vehroutes.h>
#include <microsim/output/MSStopOut.h>
#include <utils/common/RandHelper.h>
#include "MSFrame.h"
#include <utils/common/SystemFrame.h>


// ===========================================================================
// method definitions
// ===========================================================================
void
MSFrame::fillOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.addCallExample("-b 0 -e 1000 -n net.xml -r routes.xml", "start a simulation from time 0 to 1000 with given net and routes");
    oc.addCallExample("-c munich_config.cfg", "start with a configuration file");
    oc.addCallExample("--help", "print help");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    oc.addOptionSubTopic("Time");
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Routing");

    // register configuration options
    //  register input options
    oc.doRegister("net-file", 'n', new Option_FileName());
    oc.addSynonyme("net-file", "net");
    oc.addDescription("net-file", "Input", "Load road network description from FILE");
    oc.addXMLDefault("net-file", "net");

    oc.doRegister("route-files", 'r', new Option_FileName());
    oc.addSynonyme("route-files", "routes");
    oc.addDescription("route-files", "Input", "Load routes descriptions from FILE(s)");

    oc.doRegister("additional-files", 'a', new Option_FileName());
    oc.addSynonyme("additional-files", "additional");
    oc.addDescription("additional-files", "Input", "Load further descriptions from FILE(s)");

    oc.doRegister("weight-files", 'w', new Option_FileName());
    oc.addSynonyme("weight-files", "weights");
    oc.addDescription("weight-files", "Input", "Load edge/lane weights for online rerouting from FILE");
    oc.doRegister("weight-attribute", 'x', new Option_String("traveltime"));
    oc.addSynonyme("weight-attribute", "measure", true);
    oc.addDescription("weight-attribute", "Input", "Name of the xml attribute which gives the edge weight");

    oc.doRegister("load-state", new Option_FileName());//!!! check, describe
    oc.addDescription("load-state", "Input", "Loads a network state from FILE");
    oc.doRegister("load-state.offset", new Option_String("0", "TIME"));//!!! check, describe
    oc.addDescription("load-state.offset", "Input", "Shifts all times loaded from a saved state by the given offset");
    oc.doRegister("load-state.remove-vehicles", new Option_StringVector(StringVector({""})));
    oc.addDescription("load-state.remove-vehicles", "Input", "Removes vehicles with the given IDs from the loaded state");

    oc.doRegister("junction-taz", new Option_Bool(false));
    oc.addDescription("junction-taz", "Input", "Initialize a TAZ for every junction to use attributes toJunction and fromJunction");

    // need to do this here to be able to check for network and route input options
    SystemFrame::addReportOptions(oc);

    //  register output options
    oc.doRegister("netstate-dump", new Option_FileName());
    oc.addSynonyme("netstate-dump", "ndump");
    oc.addSynonyme("netstate-dump", "netstate");
    oc.addSynonyme("netstate-dump", "netstate-output");
    oc.addDescription("netstate-dump", "Output", "Save complete network states into FILE");
    oc.doRegister("netstate-dump.empty-edges", new Option_Bool(false));
    oc.addSynonyme("netstate-dump.empty-edges", "netstate.empty-edges");
    oc.addSynonyme("netstate-dump.empty-edges", "netstate-output.empty-edges");
    oc.addSynonyme("netstate-dump.empty-edges", "dump-empty-edges", true);
    oc.addDescription("netstate-dump.empty-edges", "Output", "Write also empty edges completely when dumping");
    oc.doRegister("netstate-dump.precision", new Option_Integer(2));
    oc.addSynonyme("netstate-dump.precision", "netstate.precision");
    oc.addSynonyme("netstate-dump.precision", "netstate-output.precision");
    oc.addSynonyme("netstate-dump.precision", "dump-precision", true);
    oc.addDescription("netstate-dump.precision", "Output", "Write positions and speeds with the given precision (default 2)");


    oc.doRegister("emission-output", new Option_FileName());
    oc.addDescription("emission-output", "Output", "Save the emission values of each vehicle");
    oc.doRegister("emission-output.precision", new Option_Integer(2));
    oc.addDescription("emission-output.precision", "Output", "Write emission values with the given precision (default 2)");

    oc.doRegister("battery-output", new Option_FileName());
    oc.addDescription("battery-output", "Output", "Save the battery values of each vehicle");
    oc.doRegister("battery-output.precision", new Option_Integer(2));
    oc.addDescription("battery-output.precision", "Output", "Write battery values with the given precision (default 2)");

    oc.doRegister("elechybrid-output", new Option_FileName());
    oc.addDescription("elechybrid-output", "Output", "Save the elecHybrid values of each vehicle");
    oc.doRegister("elechybrid-output.precision", new Option_Integer(2));
    oc.addDescription("elechybrid-output.precision", "Output", "Write elecHybrid values with the given precision (default 2)");
    oc.doRegister("elechybrid-output.aggregated", new Option_Bool(false));
    oc.addDescription("elechybrid-output.aggregated", "Output", "Write elecHybrid values into one aggregated file");

    oc.doRegister("chargingstations-output", new Option_FileName());
    oc.addDescription("chargingstations-output", "Output", "Write data of charging stations");

    oc.doRegister("overheadwiresegments-output", new Option_FileName());
    oc.addDescription("overheadwiresegments-output", "Output", "Write data of overhead wire segments");

    oc.doRegister("substations-output", new Option_FileName());
    oc.addDescription("substations-output", "Output", "Write data of electrical substation stations");

    oc.doRegister("fcd-output", new Option_FileName());
    oc.addDescription("fcd-output", "Output", "Save the Floating Car Data");
    oc.doRegister("fcd-output.geo", new Option_Bool(false));
    oc.addDescription("fcd-output.geo", "Output", "Save the Floating Car Data using geo-coordinates (lon/lat)");
    oc.doRegister("fcd-output.signals", new Option_Bool(false));
    oc.addDescription("fcd-output.signals", "Output", "Add the vehicle signal state to the FCD output (brake lights etc.)");
    oc.doRegister("fcd-output.distance", new Option_Bool(false));
    oc.addDescription("fcd-output.distance", "Output", "Add kilometrage to the FCD output (linear referencing)");
    oc.doRegister("fcd-output.acceleration", new Option_Bool(false));
    oc.addDescription("fcd-output.acceleration", "Output", "Add acceleration to the FCD output");
    oc.doRegister("fcd-output.params", new Option_StringVector());
    oc.addDescription("fcd-output.params", "Output", "Add generic parameter values to the FCD output");
    oc.doRegister("fcd-output.filter-edges.input-file", new Option_FileName());
    oc.addDescription("fcd-output.filter-edges.input-file", "Output", "Restrict fcd output to the edge selection from the given input file");


    oc.doRegister("full-output", new Option_FileName());
    oc.addDescription("full-output", "Output", "Save a lot of information for each timestep (very redundant)");
    oc.doRegister("queue-output", new Option_FileName());
    oc.addDescription("queue-output", "Output", "Save the vehicle queues at the junctions (experimental)");
    oc.doRegister("vtk-output", new Option_FileName());
    oc.addDescription("vtk-output", "Output", "Save complete vehicle positions inclusive speed values in the VTK Format (usage: /path/out will produce /path/out_$TIMESTEP$.vtp files)");
    oc.doRegister("amitran-output", new Option_FileName());
    oc.addDescription("amitran-output", "Output", "Save the vehicle trajectories in the Amitran format");


    oc.doRegister("summary-output", new Option_FileName());
    oc.addSynonyme("summary-output", "summary");
    oc.addDescription("summary-output", "Output", "Save aggregated vehicle departure info into FILE");

    oc.doRegister("person-summary-output", new Option_FileName());
    oc.addDescription("person-summary-output", "Output", "Save aggregated person counts into FILE");

    oc.doRegister("tripinfo-output", new Option_FileName());
    oc.addSynonyme("tripinfo-output", "tripinfo");
    oc.addDescription("tripinfo-output", "Output", "Save single vehicle trip info into FILE");

    oc.doRegister("tripinfo-output.write-unfinished", new Option_Bool(false));
    oc.addDescription("tripinfo-output.write-unfinished", "Output", "Write tripinfo output for vehicles which have not arrived at simulation end");

    oc.doRegister("vehroute-output", new Option_FileName());
    oc.addSynonyme("vehroute-output", "vehroutes");
    oc.addDescription("vehroute-output", "Output", "Save single vehicle route info into FILE");

    oc.doRegister("vehroute-output.exit-times", new Option_Bool(false));
    oc.addSynonyme("vehroute-output.exit-times", "vehroutes.exit-times");
    oc.addDescription("vehroute-output.exit-times", "Output", "Write the exit times for all edges");

    oc.doRegister("vehroute-output.last-route", new Option_Bool(false));
    oc.addSynonyme("vehroute-output.last-route", "vehroutes.last-route");
    oc.addDescription("vehroute-output.last-route", "Output", "Write the last route only");

    oc.doRegister("vehroute-output.sorted", new Option_Bool(false));
    oc.addSynonyme("vehroute-output.sorted", "vehroutes.sorted");
    oc.addDescription("vehroute-output.sorted", "Output", "Sorts the output by departure time");

    oc.doRegister("vehroute-output.dua", new Option_Bool(false));
    oc.addSynonyme("vehroute-output.dua", "vehroutes.dua");
    oc.addDescription("vehroute-output.dua", "Output", "Write the output in the duarouter alternatives style");

    oc.doRegister("vehroute-output.cost", new Option_Bool(false));
    oc.addDescription("vehroute-output.cost", "Output", "Write costs for all routes");

    oc.doRegister("vehroute-output.intended-depart", new Option_Bool(false));
    oc.addSynonyme("vehroute-output.intended-depart", "vehroutes.intended-depart");
    oc.addDescription("vehroute-output.intended-depart", "Output", "Write the output with the intended instead of the real departure time");

    oc.doRegister("vehroute-output.route-length", new Option_Bool(false));
    oc.addSynonyme("vehroute-output.route-length", "vehroutes.route-length");
    oc.addDescription("vehroute-output.route-length", "Output", "Include total route length in the output");

    oc.doRegister("vehroute-output.write-unfinished", new Option_Bool(false));
    oc.addDescription("vehroute-output.write-unfinished", "Output", "Write vehroute output for vehicles which have not arrived at simulation end");

    oc.doRegister("vehroute-output.skip-ptlines", new Option_Bool(false));
    oc.addDescription("vehroute-output.skip-ptlines", "Output", "Skip vehroute output for public transport vehicles");

    oc.doRegister("vehroute-output.incomplete", new Option_Bool(false));
    oc.addDescription("vehroute-output.incomplete", "Output", "Include invalid routes and route stubs in vehroute output");

    oc.doRegister("vehroute-output.stop-edges", new Option_Bool(false));
    oc.addDescription("vehroute-output.stop-edges", "Output", "Include information about edges between stops");

    oc.doRegister("link-output", new Option_FileName());
    oc.addDescription("link-output", "Output", "Save links states into FILE");

    oc.doRegister("railsignal-block-output", new Option_FileName());
    oc.addDescription("railsignal-block-output", "Output", "Save railsignal-blocks into FILE");

    oc.doRegister("bt-output", new Option_FileName());
    oc.addDescription("bt-output", "Output", "Save bluetooth visibilities into FILE (in conjunction with device.btreceiver and device.btsender)");

    oc.doRegister("lanechange-output", new Option_FileName());
    oc.addDescription("lanechange-output", "Output", "Record lane changes and their motivations for all vehicles into FILE");

    oc.doRegister("lanechange-output.started", new Option_Bool(false));
    oc.addDescription("lanechange-output.started", "Output", "Record start of lane change manoeuvres");

    oc.doRegister("lanechange-output.ended", new Option_Bool(false));
    oc.addDescription("lanechange-output.ended", "Output", "Record end of lane change manoeuvres");

    oc.doRegister("lanechange-output.xy", new Option_Bool(false));
    oc.addDescription("lanechange-output.xy", "Output", "Record coordinates of lane change manoeuvres");

    oc.doRegister("stop-output", new Option_FileName());
    oc.addDescription("stop-output", "Output", "Record stops and loading/unloading of passenger and containers for all vehicles into FILE");

    oc.doRegister("statistic-output", new Option_FileName());
    oc.addSynonyme("statistic-output", "statistics-output");
    oc.addDescription("statistic-output", "Output", "Write overall statistics into FILE");

#ifdef _DEBUG
    oc.doRegister("movereminder-output", new Option_FileName());
    oc.addDescription("movereminder-output", "Output", "Save movereminder states of selected vehicles into FILE");
    oc.doRegister("movereminder-output.vehicles", new Option_StringVector());
    oc.addDescription("movereminder-output.vehicles", "Output", "List of vehicle ids which shall save their movereminder states");
#endif

    oc.doRegister("save-state.times", new Option_StringVector());
    oc.addDescription("save-state.times", "Output", "Use TIME[] as times at which a network state written");
    oc.doRegister("save-state.period", new Option_String("-1", "TIME"));
    oc.addDescription("save-state.period", "Output", "save state repeatedly after TIME period");
    oc.doRegister("save-state.prefix", new Option_FileName(StringVector({ "state" })));
    oc.addDescription("save-state.prefix", "Output", "Prefix for network states");
    oc.doRegister("save-state.suffix", new Option_String(".xml.gz"));
    oc.addDescription("save-state.suffix", "Output", "Suffix for network states (.xml.gz or .xml)");
    oc.doRegister("save-state.files", new Option_FileName());//
    oc.addDescription("save-state.files", "Output", "Files for network states");
    oc.doRegister("save-state.rng", new Option_Bool(false));
    oc.addDescription("save-state.rng", "Output", "Save random number generator states");
    oc.doRegister("save-state.transportables", new Option_Bool(false));
    oc.addDescription("save-state.transportables", "Output", "Save person and container states (experimental)");

    // register the simulation settings
    oc.doRegister("begin", 'b', new Option_String("0", "TIME"));
    oc.addDescription("begin", "Time", "Defines the begin time in seconds; The simulation starts at this time");

    oc.doRegister("end", 'e', new Option_String("-1", "TIME"));
    oc.addDescription("end", "Time", "Defines the end time in seconds; The simulation ends at this time");

    oc.doRegister("step-length", new Option_String("1", "TIME"));
    oc.addDescription("step-length", "Time", "Defines the step duration in seconds");

    oc.doRegister("step-method.ballistic", new Option_Bool(false));
    oc.addDescription("step-method.ballistic", "Processing", "Whether to use ballistic method for the positional update of vehicles (default is a semi-implicit Euler method).");

    oc.doRegister("extrapolate-departpos", new Option_Bool(false));
    oc.addDescription("extrapolate-departpos", "Processing", "Whether vehicles that depart between simulation steps should extrapolate the depart position");

    oc.doRegister("threads", new Option_Integer(1));
    oc.addDescription("threads", "Processing", "Defines the number of threads for parallel simulation");

    oc.doRegister("lateral-resolution", new Option_Float(-1));
    oc.addDescription("lateral-resolution", "Processing", "Defines the resolution in m when handling lateral positioning within a lane (with -1 all vehicles drive at the center of their lane");

    // register the processing options
    oc.doRegister("route-steps", 's', new Option_String("200", "TIME"));
    oc.addDescription("route-steps", "Processing", "Load routes for the next number of seconds ahead");

    oc.doRegister("no-internal-links", new Option_Bool(false));
    oc.addDescription("no-internal-links", "Processing", "Disable (junction) internal links");

    oc.doRegister("ignore-junction-blocker", new Option_String("-1", "TIME"));
    oc.addDescription("ignore-junction-blocker", "Processing", "Ignore vehicles which block the junction after they have been standing for SECONDS (-1 means never ignore)");

    oc.doRegister("ignore-route-errors", new Option_Bool(false));
    oc.addDescription("ignore-route-errors", "Processing", "(1) Do not check whether routes are connected. (2) Allow inserting a vehicle in a situation which requires emergency braking.");

    oc.doRegister("ignore-accidents", new Option_Bool(false));
    oc.addDescription("ignore-accidents", "Processing", "Do not check whether accidents occur");

    oc.doRegister("collision.action", new Option_String("teleport"));
    oc.addDescription("collision.action", "Processing", "How to deal with collisions: [none,warn,teleport,remove]");

    oc.doRegister("collision.stoptime", new Option_String("0", "TIME"));
    oc.addDescription("collision.stoptime", "Processing", "Let vehicle stop for TIME before performing collision.action (except for action 'none')");

    oc.doRegister("collision.check-junctions", new Option_Bool(false));
    oc.addDescription("collision.check-junctions", "Processing", "Enables collisions checks on junctions");

    oc.doRegister("collision.mingap-factor", new Option_Float(-1));
    oc.addDescription("collision.mingap-factor", "Processing", "Sets the fraction of minGap that must be maintained to avoid collision detection. If a negative value is given, the carFollowModel parameter is used");

    oc.doRegister("max-num-vehicles", new Option_Integer(-1));
    oc.addDescription("max-num-vehicles", "Processing", "Delay vehicle insertion to stay within the given maximum number");

    oc.doRegister("max-num-teleports", new Option_Integer(-1));
    oc.addDescription("max-num-teleports", "Processing", "Abort the simulation if the given maximum number of teleports is exceeded");

    oc.doRegister("scale", new Option_Float(1.));
    oc.addDescription("scale", "Processing", "Scale demand by the given factor (by discarding or duplicating vehicles)");

    oc.doRegister("time-to-teleport", new Option_String("300", "TIME"));
    oc.addDescription("time-to-teleport", "Processing", "Specify how long a vehicle may wait until being teleported, defaults to 300, non-positive values disable teleporting");

    oc.doRegister("time-to-teleport.highways", new Option_String("0", "TIME"));
    oc.addDescription("time-to-teleport.highways", "Processing", "The waiting time after which vehicles on a fast road (speed > 69km/h) are teleported if they are on a non-continuing lane");

    oc.doRegister("waiting-time-memory", new Option_String("100", "TIME"));
    oc.addDescription("waiting-time-memory", "Processing", "Length of time interval, over which accumulated waiting time is taken into account (default is 100s.)");

    oc.doRegister("max-depart-delay", new Option_String("-1", "TIME"));
    oc.addDescription("max-depart-delay", "Processing", "How long vehicles wait for departure before being skipped, defaults to -1 which means vehicles are never skipped");

    oc.doRegister("sloppy-insert", new Option_Bool(false));
    oc.addDescription("sloppy-insert", "Processing", "Whether insertion on an edge shall not be repeated in same step once failed");

    oc.doRegister("eager-insert", new Option_Bool(false));
    oc.addDescription("eager-insert", "Processing", "Whether each vehicle is checked separately for insertion on an edge");

    oc.doRegister("random-depart-offset", new Option_String("0", "TIME"));
    oc.addDescription("random-depart-offset", "Processing", "Each vehicle receives a random offset to its depart value drawn uniformly from [0, TIME]");

    oc.doRegister("lanechange.duration", new Option_String("0", "TIME"));
    oc.addDescription("lanechange.duration", "Processing", "Duration of a lane change maneuver (default 0)");

    oc.doRegister("lanechange.overtake-right", new Option_Bool(false));
    oc.addDescription("lanechange.overtake-right", "Processing", "Whether overtaking on the right on motorways is permitted");

    oc.doRegister("tls.all-off", new Option_Bool(false));
    oc.addDescription("tls.all-off", "Processing", "Switches off all traffic lights.");

    oc.doRegister("tls.actuated.show-detectors", new Option_Bool(false));
    oc.addDescription("tls.actuated.show-detectors", "Processing", "Sets default visibility for actuation detectors");

    oc.doRegister("tls.delay_based.detector-range", new Option_Float(100));
    oc.addDescription("tls.delay_based.detector-range", "Processing", "Sets default range for detecting delayed vehicles");

    oc.doRegister("time-to-impatience", new Option_String("300", "TIME"));
    oc.addDescription("time-to-impatience", "Processing", "Specify how long a vehicle may wait until impatience grows from 0 to 1, defaults to 300, non-positive values disable impatience growth");

    oc.doRegister("default.action-step-length", new Option_Float(0.0));
    oc.addDescription("default.action-step-length", "Processing", "Length of the default interval length between action points for the car-following and lane-change models (in seconds). If not specified, the simulation step-length is used per default. Vehicle- or VType-specific settings override the default. Must be a multiple of the simulation step-length.");

    oc.doRegister("default.carfollowmodel", new Option_String("Krauss"));
    oc.addDescription("default.carfollowmodel", "Processing", "Select default car following model (Krauss, IDM, ...)");
    oc.addSynonyme("default.carfollowmodel", "carfollow.model", false);

    oc.doRegister("default.speeddev", new Option_Float(-1));
    oc.addDescription("default.speeddev", "Processing", "Select default speed deviation. A negative value implies vClass specific defaults (0.1 for the default passenger class");

    oc.doRegister("default.emergencydecel", new Option_String("default"));
    oc.addDescription("default.emergencydecel", "Processing", "Select default emergencyDecel value among ('decel', 'default', FLOAT) which sets the value either to the same as the deceleration value, a vClass-class specific default or the given FLOAT in m/s^2");

    oc.doRegister("overhead-wire-solver", new Option_Bool(true));
    oc.addDescription("overhead-wire-solver", "Processing", "Use Kirchhoff's laws for solving overhead wire circuit");

    oc.doRegister("emergencydecel.warning-threshold", new Option_Float(1));
    oc.addDescription("emergencydecel.warning-threshold", "Processing", "Sets the fraction of emergency decel capability that must be used to trigger a warning.");

    oc.doRegister("parking.maneuver", new Option_Bool(false));
    oc.addDescription("parking.maneuver", "Processing", "Whether parking simulation includes manoeuvering time and associated lane blocking");

    // pedestrian model
    oc.doRegister("pedestrian.model", new Option_String("striping"));
    oc.addDescription("pedestrian.model", "Processing", "Select among pedestrian models ['nonInteracting', 'striping', 'remote']");

    oc.doRegister("pedestrian.striping.stripe-width", new Option_Float(0.64));
    oc.addDescription("pedestrian.striping.stripe-width", "Processing", "Width of parallel stripes for segmenting a sidewalk (meters) for use with model 'striping'");

    oc.doRegister("pedestrian.striping.dawdling", new Option_Float(0.2));
    oc.addDescription("pedestrian.striping.dawdling", "Processing", "Factor for random slow-downs [0,1] for use with model 'striping'");

    oc.doRegister("pedestrian.striping.jamtime", new Option_String("300", "TIME"));
    oc.addDescription("pedestrian.striping.jamtime", "Processing", "Time in seconds after which pedestrians start squeezing through a jam when using model 'striping' (non-positive values disable squeezing)");
    oc.doRegister("pedestrian.striping.jamtime.crossing", new Option_String("10", "TIME"));
    oc.addDescription("pedestrian.striping.jamtime.crossing", "Processing", "Time in seconds after which pedestrians start squeezing through a jam while on a pedestrian crossing when using model 'striping' (non-positive values disable squeezing)");

    oc.doRegister("pedestrian.striping.reserve-oncoming", new Option_Float(0.0));
    oc.addDescription("pedestrian.striping.reserve-oncoming", "Processing", "Fraction of stripes to reserve for oncoming pedestrians");

    oc.doRegister("pedestrian.striping.reserve-oncoming.junctions", new Option_Float(0.34));
    oc.addDescription("pedestrian.striping.reserve-oncoming.junctions", "Processing", "Fraction of stripes to reserve for oncoming pedestrians on crossings and walkingareas");

    oc.doRegister("pedestrian.remote.address", new Option_String("localhost:9000"));
    oc.addDescription("pedestrian.remote.address", "Processing", "The address (host:port) of the external simulation");

    oc.doRegister("ride.stop-tolerance", new Option_Float(10.));
    oc.addDescription("ride.stop-tolerance", "Processing", "Tolerance to apply when matching pedestrian and vehicle positions on boarding at individual stops");

    // generic routing options
    oc.doRegister("routing-algorithm", new Option_String("dijkstra"));
    oc.addDescription("routing-algorithm", "Routing",
                      "Select among routing algorithms ['dijkstra', 'astar', 'CH', 'CHWrapper']");

    oc.doRegister("weights.random-factor", new Option_Float(1.));
    oc.addDescription("weights.random-factor", "Routing", "Edge weights for routing are dynamically disturbed by a random factor drawn uniformly from [1,FLOAT)");

    oc.doRegister("weights.minor-penalty", new Option_Float(1.5));
    oc.addDescription("weights.minor-penalty", "Routing", "Apply the given time penalty when computing minimum routing costs for minor-link internal lanes");

    oc.doRegister("weights.priority-factor", new Option_Float(0));
    oc.addDescription("weights.priority-factor", "Routing", "Consider edge priorities in addition to travel times, weighted by factor");

    oc.doRegister("astar.all-distances", new Option_FileName());
    oc.addDescription("astar.all-distances", "Routing", "Initialize lookup table for astar from the given file (generated by marouter --all-pairs-output)");

    oc.doRegister("astar.landmark-distances", new Option_FileName());
    oc.addDescription("astar.landmark-distances", "Routing", "Initialize lookup table for astar ALT-variant from the given file");

    oc.doRegister("persontrip.walkfactor", new Option_Float(double(0.75)));
    oc.addDescription("persontrip.walkfactor", "Routing", "Use FLOAT as a factor on pedestrian maximum speed during intermodal routing");

    oc.doRegister("persontrip.transfer.car-walk", new Option_StringVector(StringVector({ "parkingAreas" })));
    oc.addDescription("persontrip.transfer.car-walk", "Routing",
                      "Where are mode changes from car to walking allowed (possible values: 'parkingAreas', 'ptStops', 'allJunctions' and combinations)");

    oc.doRegister("persontrip.transfer.taxi-walk", new Option_StringVector());
    oc.addDescription("persontrip.transfer.taxi-walk", "Routing", "Where taxis can drop off customers ('allJunctions, 'ptStops')");

    oc.doRegister("persontrip.transfer.walk-taxi", new Option_StringVector());
    oc.addDescription("persontrip.transfer.walk-taxi", "Routing", "Where taxis can pick up customers ('allJunctions, 'ptStops')");

    oc.doRegister("railway.max-train-length", new Option_Float(5000.0));
    oc.addDescription("railway.max-train-length", "Routing", "Use FLOAT as a maximum train length when initializing the railway router");

    // devices
    oc.addOptionSubTopic("Emissions");
    oc.doRegister("phemlight-path", new Option_FileName(StringVector({ "./PHEMlight/" })));
    oc.addDescription("phemlight-path", "Emissions", "Determines where to load PHEMlight definitions from.");

    oc.addOptionSubTopic("Communication");
    oc.addOptionSubTopic("Battery");
    MSDevice::insertOptions(oc);

    // register report options
    oc.doRegister("duration-log.disable", new Option_Bool(false));
    oc.addSynonyme("duration-log.disable", "no-duration-log", false);
    oc.addDescription("duration-log.disable", "Report", "Disable performance reports for individual simulation steps");

    oc.doRegister("duration-log.statistics", 't', new Option_Bool(false));
    oc.addDescription("duration-log.statistics", "Report", "Enable statistics on vehicle trips");

    oc.doRegister("no-step-log", new Option_Bool(false));
    oc.addDescription("no-step-log", "Report", "Disable console output of current simulation step");

    oc.doRegister("step-log.period", new Option_Integer(100));
    oc.addDescription("step-log.period", "Report", "Number of simulation steps between step-log outputs");

    //remote port 0 if not used
    oc.addOptionSubTopic("TraCI Server");
    oc.doRegister("remote-port", new Option_Integer(0));
    oc.addDescription("remote-port", "TraCI Server", "Enables TraCI Server if set");
    oc.doRegister("num-clients", new Option_Integer(1));
    oc.addDescription("num-clients", "TraCI Server", "Expected number of connecting clients");

    oc.addOptionSubTopic("Mesoscopic");
    oc.doRegister("mesosim", new Option_Bool(false));
    oc.addDescription("mesosim", "Mesoscopic", "Enables mesoscopic simulation");
    oc.doRegister("meso-edgelength", new Option_Float(98.0f));
    oc.addDescription("meso-edgelength", "Mesoscopic", "Length of an edge segment in mesoscopic simulation");
    oc.doRegister("meso-tauff", new Option_String("1.13", "TIME"));
    oc.addDescription("meso-tauff", "Mesoscopic", "Factor for calculating the net free-free headway time");
    oc.doRegister("meso-taufj", new Option_String("1.13", "TIME"));
    oc.addDescription("meso-taufj", "Mesoscopic", "Factor for calculating the net free-jam headway time");
    oc.doRegister("meso-taujf", new Option_String("1.73", "TIME"));
    oc.addDescription("meso-taujf", "Mesoscopic", "Factor for calculating the jam-free headway time");
    oc.doRegister("meso-taujj", new Option_String("1.4", "TIME"));
    oc.addDescription("meso-taujj", "Mesoscopic", "Factor for calculating the jam-jam headway time");
    oc.doRegister("meso-jam-threshold", new Option_Float(-1));
    oc.addDescription("meso-jam-threshold", "Mesoscopic",
                      "Minimum percentage of occupied space to consider a segment jammed. A negative argument causes thresholds to be computed based on edge speed and tauff (default)");
    oc.doRegister("meso-multi-queue", new Option_Bool(true));
    oc.addDescription("meso-multi-queue", "Mesoscopic", "Enable multiple queues at edge ends");
    oc.doRegister("meso-lane-queue", new Option_Bool(false));
    oc.addDescription("meso-lane-queue", "Mesoscopic", "Enable separate queues for every lane");
    oc.doRegister("meso-junction-control", new Option_Bool(false));
    oc.addDescription("meso-junction-control", "Mesoscopic", "Enable mesoscopic traffic light and priority junction handling");
    oc.doRegister("meso-junction-control.limited", new Option_Bool(false));
    oc.addDescription("meso-junction-control.limited", "Mesoscopic",
                      "Enable mesoscopic traffic light and priority junction handling for saturated links. This prevents faulty traffic lights from hindering flow in low-traffic situations");
    oc.doRegister("meso-tls-penalty", new Option_Float(0));
    oc.addDescription("meso-tls-penalty", "Mesoscopic",
                      "Apply scaled travel time penalties when driving across tls controlled junctions based on green split instead of checking actual phases");
    oc.doRegister("meso-tls-flow-penalty", new Option_Float(0));
    oc.addDescription("meso-tls-flow-penalty", "Mesoscopic",
                      "Apply scaled headway penalties when driving across tls controlled junctions based on green split instead of checking actual phases");
    oc.doRegister("meso-minor-penalty", new Option_String("0", "TIME"));
    oc.addDescription("meso-minor-penalty", "Mesoscopic",
                      "Apply fixed time penalty when driving across a minor link. When using --meso-junction-control.limited, the penalty is not applied whenever limited control is active.");
    oc.doRegister("meso-overtaking", new Option_Bool(false));
    oc.addDescription("meso-overtaking", "Mesoscopic", "Enable mesoscopic overtaking");
    oc.doRegister("meso-recheck", new Option_String("0", "TIME"));
    oc.addDescription("meso-recheck", "Mesoscopic", "Time interval for rechecking insertion into the next segment after failure");

    // add rand options
    RandHelper::insertRandOptions();
    oc.doRegister("thread-rngs", new Option_Integer(64));
    oc.addDescription("thread-rngs", "Random Number",
                      "Number of pre-allocated random number generators to ensure repeatable multi-threaded simulations (should be at least the number of threads for repeatable simulations).");

    // add GUI options
    // the reason that we include them in vanilla sumo as well is to make reusing config files easy
    oc.addOptionSubTopic("GUI Only");
    oc.doRegister("gui-settings-file", 'g', new Option_FileName());
    oc.addDescription("gui-settings-file", "GUI Only", "Load visualisation settings from FILE");

    oc.doRegister("quit-on-end", 'Q', new Option_Bool(false));
    oc.addDescription("quit-on-end", "GUI Only", "Quits the GUI when the simulation stops");

    oc.doRegister("game", 'G', new Option_Bool(false));
    oc.addDescription("game", "GUI Only", "Start the GUI in gaming mode");

    oc.doRegister("game.mode", new Option_String("tls"));
    oc.addDescription("game.mode", "GUI Only", "Select the game type ('tls', 'drt')");

    oc.doRegister("start", 'S', new Option_Bool(false));
    oc.addDescription("start", "GUI Only", "Start the simulation after loading");

    oc.doRegister("breakpoints", 'B', new Option_StringVector());
    oc.addDescription("breakpoints", "GUI Only", "Use TIME[] as times when the simulation should halt");

    oc.doRegister("edgedata-files", new Option_FileName());
    oc.addDescription("edgedata-files", "GUI Only", "Load edge/lane weights for visualization from FILE");

    oc.doRegister("demo", 'D', new Option_Bool(false));
    oc.addDescription("demo", "GUI Only", "Restart the simulation after ending (demo mode)");

    oc.doRegister("disable-textures", 'T', new Option_Bool(false));
    oc.addDescription("disable-textures", "GUI Only", "Do not load background pictures");

    oc.doRegister("registry-viewport", new Option_Bool(false));
    oc.addDescription("registry-viewport", "GUI Only", "Load current viewport from registry");

    oc.doRegister("window-size", new Option_StringVector());
    oc.addDescription("window-size", "GUI Only", "Create initial window with the given x,y size");

    oc.doRegister("window-pos", new Option_StringVector());
    oc.addDescription("window-pos", "GUI Only", "Create initial window at the given x,y position");

    oc.doRegister("tracker-interval", new Option_Float(1.0));
    oc.addDescription("tracker-interval", "GUI Only", "The aggregation period for value tracker windows");

#ifdef HAVE_OSG
    oc.doRegister("osg-view", new Option_Bool(false));
    oc.addDescription("osg-view", "GUI Only", "Start with an OpenSceneGraph view instead of the regular 2D view");
#endif

    // gui testing
    oc.doRegister("gui-testing", new Option_Bool(false));
    oc.addDescription("gui-testing", "GUI Only", "Enable ovelay for screen recognition");

    // gui testing - debug
    oc.doRegister("gui-testing-debug", new Option_Bool(false));
    oc.addDescription("gui-testing-debug", "GUI Only", "Enable output messages during GUI-Testing");

    // gui testing - settings output
    oc.doRegister("gui-testing.setting-output", new Option_FileName());
    oc.addDescription("gui-testing.setting-output", "GUI Only", "Save gui settings in the given settings output file");
}


void
MSFrame::buildStreams() {
    // standard outputs
    OutputDevice::createDeviceByOption("netstate-dump", "netstate", "netstate_file.xsd");
    OutputDevice::createDeviceByOption("summary-output", "summary", "summary_file.xsd");
    OutputDevice::createDeviceByOption("person-summary-output", "personSummary", "person_summary_file.xsd");
    OutputDevice::createDeviceByOption("tripinfo-output", "tripinfos", "tripinfo_file.xsd");

    //extended
    OutputDevice::createDeviceByOption("fcd-output", "fcd-export", "fcd_file.xsd");
    OutputDevice::createDeviceByOption("emission-output", "emission-export", "emission_file.xsd");
    OutputDevice::createDeviceByOption("battery-output", "battery-export");
    if (OptionsCont::getOptions().getBool("elechybrid-output.aggregated")) {
        OutputDevice::createDeviceByOption("elechybrid-output", "elecHybrid-export-aggregated");
    }
    //OutputDevice::createDeviceByOption("elecHybrid-output", "elecHybrid-export");
    OutputDevice::createDeviceByOption("chargingstations-output", "chargingstations-export");
    OutputDevice::createDeviceByOption("overheadwiresegments-output", "overheadWireSegments-export");
    OutputDevice::createDeviceByOption("substations-output", "substations-export");
    OutputDevice::createDeviceByOption("full-output", "full-export", "full_file.xsd");
    OutputDevice::createDeviceByOption("queue-output", "queue-export", "queue_file.xsd");
    OutputDevice::createDeviceByOption("amitran-output", "trajectories", "amitran/trajectories.xsd\" timeStepSize=\"" + toString(STEPS2MS(DELTA_T)));

    //OutputDevice::createDeviceByOption("vtk-output", "vtk-export");
    OutputDevice::createDeviceByOption("link-output", "link-output");
    OutputDevice::createDeviceByOption("railsignal-block-output", "railsignal-block-output");
    OutputDevice::createDeviceByOption("bt-output", "bt-output");
    OutputDevice::createDeviceByOption("lanechange-output", "lanechanges");
    OutputDevice::createDeviceByOption("stop-output", "stops", "stopinfo_file.xsd");
    OutputDevice::createDeviceByOption("statistic-output", "statistics", "statistic_file.xsd");

#ifdef _DEBUG
    OutputDevice::createDeviceByOption("movereminder-output", "movereminder-output");
#endif

    MSDevice_Vehroutes::init();
    MSStopOut::init();
}


bool
MSFrame::checkOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    bool ok = true;
    if (!oc.isSet("net-file")) {
        WRITE_ERROR("No network file (-n) specified.");
        ok = false;
    }
    if (oc.getFloat("scale") < 0.) {
        WRITE_ERROR("Invalid scaling factor.");
        ok = false;
    }
    if (oc.getBool("vehroute-output.exit-times") && !oc.isSet("vehroute-output")) {
        WRITE_ERROR("A vehroute-output file is needed for exit times.");
        ok = false;
    }
    if (oc.isSet("gui-settings-file") &&
            oc.getString("gui-settings-file") != "" &&
            !oc.isUsableFileList("gui-settings-file")) {
        ok = false;
    }
    if (oc.getBool("demo") && oc.isDefault("start")) {
        oc.set("start", "true");
    }
    if (oc.getBool("demo") && oc.getBool("quit-on-end")) {
        WRITE_ERROR("You can either restart or quit on end.");
        ok = false;
    }
    if (oc.getBool("meso-junction-control.limited") && !oc.getBool("meso-junction-control")) {
        oc.set("meso-junction-control", "true");
    }
    if (oc.getBool("mesosim")) {
        if (oc.isDefault("pedestrian.model")) {
            oc.set("pedestrian.model", "nonInteracting");
        }
    }
    const SUMOTime begin = string2time(oc.getString("begin"));
    const SUMOTime end = string2time(oc.getString("end"));
    if (begin < 0) {
        WRITE_ERROR("The begin time should not be negative.");
        ok = false;
    }
    // DELTA_T not yet initialized
    const SUMOTime deltaT = MAX2((SUMOTime)1, string2time(oc.getString("step-length")));
    if (begin < TIME2STEPS(1)) {
        checkStepLengthMultiple(begin, " for begin", deltaT);
    }
    if (end != string2time("-1")) {
        if (end < begin) {
            WRITE_ERROR("The end time should be after the begin time.");
            ok = false;
        }
    }
    if (string2time(oc.getString("step-length")) <= 0) {
        WRITE_ERROR("the minimum step-length is 0.001");
        ok = false;
    }
    const SUMOTime period = string2time(oc.getString("device.fcd.period"));
    if (period > 0) {
        checkStepLengthMultiple(period, " for device.fcd.period", deltaT);
    }
    const SUMOTime statePeriod = string2time(oc.getString("save-state.period"));
    if (statePeriod > 0) {
        checkStepLengthMultiple(statePeriod, " for save-state.period", deltaT);
    }
    for (const std::string& timeStr : oc.getStringVector("save-state.times")) {
        try {
            const SUMOTime saveT = string2time(timeStr);
            if (end > 0 && saveT >= end) {
                WRITE_WARNING("The save-state.time " + timeStr + " will not be used before simulation end at " + time2string(end));
            } else {
                checkStepLengthMultiple(saveT, " for save-state.times", deltaT);
            }
        } catch (ProcessError& e) {
            WRITE_ERROR("Invalid time '" + timeStr + "' for option 'save-state.times'. " + e.what());
            ok = false;
        }
    }

#ifdef _DEBUG
    if (oc.isSet("movereminder-output.vehicles") && !oc.isSet("movereminder-output")) {
        WRITE_ERROR("option movereminder-output.vehicles requires option movereminder-output to be set");
        ok = false;
    }
#endif
    if (oc.getBool("sloppy-insert")) {
        WRITE_WARNING("The option 'sloppy-insert' is deprecated, because it is now activated by default, see the new option 'eager-insert'.");
    }
    if (string2time(oc.getString("lanechange.duration")) > 0 && oc.getFloat("lateral-resolution") > 0) {
        WRITE_ERROR("Only one of the options 'lanechange.duration' or 'lateral-resolution' may be given.");
        ok = false;
    }
    if (oc.getBool("mesosim") && (oc.getFloat("lateral-resolution") > 0 || string2time(oc.getString("lanechange.duration")) > 0)) {
        WRITE_ERROR("Sublane dynamics are not supported by mesoscopic simulation");
        ok = false;
    }
    if (oc.getBool("ignore-accidents")) {
        WRITE_WARNING("The option 'ignore-accidents' is deprecated. Use 'collision.action none' instead.");
    }
    if (oc.getBool("duration-log.statistics") && oc.isDefault("verbose")) {
        oc.set("verbose", "true");
    }
    if (oc.isDefault("precision") && string2time(oc.getString("step-length")) < 10) {
        oc.set("precision", "3");
    }
    if (oc.isDefault("tracker-interval") && !oc.isDefault("step-length")) {
        oc.set("tracker-interval", oc.getString("step-length"));
    }
    if (oc.getInt("precision") > 2) {
        if (oc.isDefault("netstate-dump.precision")) {
            oc.set("netstate-dump.precision", toString(oc.getInt("precision")));
        }
        if (oc.isDefault("emission-output.precision")) {
            oc.set("emission-output.precision", toString(oc.getInt("precision")));
        }
        if (oc.isDefault("battery-output.precision")) {
            oc.set("battery-output.precision", toString(oc.getInt("precision")));
        }
        if (oc.isDefault("elechybrid-output.precision")) {
            oc.set("elechybrid-output.precision", toString(oc.getInt("precision")));
        }
    }
    if (!SUMOXMLDefinitions::CarFollowModels.hasString(oc.getString("carfollow.model"))) {
        WRITE_ERROR("Unknown model '" + oc.getString("carfollow.model")  + "' for option 'carfollow.model'.");
        ok = false;
    }
    if (oc.isSet("default.emergencydecel")) {
        const std::string val = oc.getString("default.emergencydecel");
        if (val != "default" && val != "decel") {
            try {
                StringUtils::toDouble(val);
            } catch (NumberFormatException&) {
                WRITE_ERROR("Invalid value '" + val + "' for option 'default.emergencydecel'. Must be a FLOAT or 'default' or 'decel'");
                ok = false;
            }
        }
    }
    for (const std::string& val : oc.getStringVector("breakpoints")) {
        try {
            string2time(val);
        } catch (ProcessError& e) {
            WRITE_ERROR("Invalid time '" + val + "' for option 'breakpoints'. " + e.what());
            ok = false;
        }
    }
#ifndef HAVE_FOX
    if (oc.getInt("threads") > 1) {
        WRITE_ERROR("Parallel simulation is only possible when compiled with Fox.");
        ok = false;
    }
#endif
    if (oc.getInt("threads") > oc.getInt("thread-rngs")) {
        WRITE_WARNING("Number of threads exceeds number of thread-rngs. Simulation runs with the same seed may produce different results");
    }
    if (oc.getString("game.mode") != "tls" && oc.getString("game.mode") != "drt") {
        WRITE_ERROR("game.mode must be one of ['tls', 'drt']");
    }

    if (oc.isSet("persontrip.transfer.car-walk")) {
        for (const std::string& opt : OptionsCont::getOptions().getStringVector("persontrip.transfer.car-walk")) {
            if (opt != "parkingAreas" && opt != "ptStops" && opt != "allJunctions") {
                WRITE_ERROR("Invalid transfer option '" + opt + "'. Must be one of 'parkingAreas', 'ptStops' and 'allJunctions'");
                ok = false;
            }
        }
    }

    ok &= MSDevice::checkOptions(oc);
    ok &= SystemFrame::checkOptions();

    return ok;
}


void
MSFrame::setMSGlobals(OptionsCont& oc) {
    // pre-initialise the network
    // set whether empty edges shall be printed on dump
    MSGlobals::gOmitEmptyEdgesOnDump = !oc.getBool("netstate-dump.empty-edges");
    // set whether internal lanes shall be used
    MSGlobals::gUsingInternalLanes = !oc.getBool("no-internal-links");
    MSGlobals::gIgnoreJunctionBlocker = string2time(oc.getString("ignore-junction-blocker")) < 0 ?
                                        std::numeric_limits<SUMOTime>::max() : string2time(oc.getString("ignore-junction-blocker"));
    // set the grid lock time
    MSGlobals::gTimeToGridlock = string2time(oc.getString("time-to-teleport")) < 0 ? 0 : string2time(oc.getString("time-to-teleport"));
    MSGlobals::gTimeToImpatience = string2time(oc.getString("time-to-impatience"));
    MSGlobals::gTimeToGridlockHighways = string2time(oc.getString("time-to-teleport.highways")) < 0 ? 0 : string2time(oc.getString("time-to-teleport.highways"));
    MSGlobals::gCheck4Accidents = !oc.getBool("ignore-accidents");
    MSGlobals::gCheckRoutes = !oc.getBool("ignore-route-errors");
    MSGlobals::gLaneChangeDuration = string2time(oc.getString("lanechange.duration"));
    MSGlobals::gLateralResolution = oc.getFloat("lateral-resolution");
    MSGlobals::gSublane = (MSGlobals::gLaneChangeDuration > 0 || MSGlobals::gLateralResolution > 0);
    MSGlobals::gStateLoaded = oc.isSet("load-state");
    MSGlobals::gUseMesoSim = oc.getBool("mesosim");
    MSGlobals::gMesoLimitedJunctionControl = oc.getBool("meso-junction-control.limited");
    MSGlobals::gMesoOvertaking = oc.getBool("meso-overtaking");
    MSGlobals::gMesoTLSPenalty = oc.getFloat("meso-tls-penalty");
    MSGlobals::gMesoTLSFlowPenalty = oc.getFloat("meso-tls-flow-penalty");
    MSGlobals::gMesoMinorPenalty = string2time(oc.getString("meso-minor-penalty"));
    if (MSGlobals::gUseMesoSim) {
        MSGlobals::gUsingInternalLanes = false;
    }
    MSGlobals::gWaitingTimeMemory = string2time(oc.getString("waiting-time-memory"));
    MSAbstractLaneChangeModel::initGlobalOptions(oc);
    MSGlobals::gOverheadWireSolver = oc.getBool("overhead-wire-solver");

    MSLane::initCollisionOptions(oc);

    DELTA_T = string2time(oc.getString("step-length"));


    bool integrationMethodSet = !oc.isDefault("step-method.ballistic");
    bool actionStepLengthSet  = !oc.isDefault("default.action-step-length");
    MSGlobals::gSemiImplicitEulerUpdate = !oc.getBool("step-method.ballistic");
    // Init default value for gActionStepLength
    if (MSGlobals::gSemiImplicitEulerUpdate && actionStepLengthSet && !integrationMethodSet) {
        WRITE_MESSAGE("Integration method was set to 'ballistic', since a default action step length was specified.");
        MSGlobals::gSemiImplicitEulerUpdate = false;
    }
    double givenDefaultActionStepLength = oc.getFloat("default.action-step-length");
    MSGlobals::gActionStepLength = SUMOVehicleParserHelper::processActionStepLength(givenDefaultActionStepLength);

    const std::string defaultEmergencyDecelOption = OptionsCont::getOptions().getString("default.emergencydecel");
    if (defaultEmergencyDecelOption == "default") {
        MSGlobals::gDefaultEmergencyDecel = VTYPEPARS_DEFAULT_EMERGENCYDECEL_DEFAULT;
    } else if (defaultEmergencyDecelOption == "decel") {
        MSGlobals::gDefaultEmergencyDecel = VTYPEPARS_DEFAULT_EMERGENCYDECEL_DECEL;
    } else {
        // value already checked in checkOptions()
        MSGlobals::gDefaultEmergencyDecel = StringUtils::toDouble(defaultEmergencyDecelOption);
    }
    MSGlobals::gNumSimThreads = oc.getInt("threads");
    MSGlobals::gNumThreads = MAX2(MSGlobals::gNumSimThreads, oc.getInt("device.rerouting.threads"));

    MSGlobals::gEmergencyDecelWarningThreshold = oc.getFloat("emergencydecel.warning-threshold");
    MSGlobals::gMinorPenalty = oc.getFloat("weights.minor-penalty");

    MSGlobals::gModelParkingManoeuver = oc.getBool("parking.maneuver");

    MSGlobals::gStopTolerance = oc.getFloat("ride.stop-tolerance");

#ifdef _DEBUG
    if (oc.isSet("movereminder-output")) {
        MSBaseVehicle::initMoveReminderOutput(oc);
    }
#endif
}


/****************************************************************************/
