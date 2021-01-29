/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_Routing.cpp
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Christoph Sommer
/// @author  Jakob Erdmann
/// @date    Tue, 04 Dec 2007
///
// A device that performs vehicle rerouting based on current edge speeds
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSVehicleControl.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/WrappingCommand.h>
#include <utils/common/StaticCommand.h>
#include <utils/common/StringUtils.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include "MSRoutingEngine.h"
#include "MSDevice_Routing.h"


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_Routing::insertOptions(OptionsCont& oc) {
    insertDefaultAssignmentOptions("rerouting", "Routing", oc);

    oc.doRegister("device.rerouting.period", new Option_String("0", "TIME"));
    oc.addSynonyme("device.rerouting.period", "device.routing.period", true);
    oc.addDescription("device.rerouting.period", "Routing", "The period with which the vehicle shall be rerouted");

    oc.doRegister("device.rerouting.pre-period", new Option_String("60", "TIME"));
    oc.addSynonyme("device.rerouting.pre-period", "device.routing.pre-period", true);
    oc.addDescription("device.rerouting.pre-period", "Routing", "The rerouting period before depart");

    oc.doRegister("device.rerouting.adaptation-weight", new Option_Float(0));
    oc.addSynonyme("device.rerouting.adaptation-weight", "device.routing.adaptation-weight", true);
    oc.addDescription("device.rerouting.adaptation-weight", "Routing", "The weight of prior edge weights for exponential moving average");

    oc.doRegister("device.rerouting.adaptation-steps", new Option_Integer(180));
    oc.addSynonyme("device.rerouting.adaptation-steps", "device.routing.adaptation-steps", true);
    oc.addDescription("device.rerouting.adaptation-steps", "Routing", "The number of steps for moving average weight of prior edge weights");

    oc.doRegister("device.rerouting.adaptation-interval", new Option_String("1", "TIME"));
    oc.addSynonyme("device.rerouting.adaptation-interval", "device.routing.adaptation-interval", true);
    oc.addDescription("device.rerouting.adaptation-interval", "Routing", "The interval for updating the edge weights");

    oc.doRegister("device.rerouting.with-taz", new Option_Bool(false));
    oc.addSynonyme("device.rerouting.with-taz", "device.routing.with-taz", true);
    oc.addSynonyme("device.rerouting.with-taz", "with-taz");
    oc.addDescription("device.rerouting.with-taz", "Routing", "Use zones (districts) as routing start- and endpoints");

    oc.doRegister("device.rerouting.init-with-loaded-weights", new Option_Bool(false));
    oc.addDescription("device.rerouting.init-with-loaded-weights", "Routing", "Use weight files given with option --weight-files for initializing edge weights");

    oc.doRegister("device.rerouting.threads", new Option_Integer(0));
    oc.addDescription("device.rerouting.threads", "Routing", "The number of parallel execution threads used for rerouting");

    oc.doRegister("device.rerouting.synchronize", new Option_Bool(false));
    oc.addDescription("device.rerouting.synchronize", "Routing", "Let rerouting happen at the same time for all vehicles");

    oc.doRegister("device.rerouting.railsignal", new Option_Bool(true));
    oc.addDescription("device.rerouting.railsignal", "Routing", "Allow rerouting triggered by rail signals.");

    oc.doRegister("device.rerouting.bike-speeds", new Option_Bool(false));
    oc.addDescription("device.rerouting.bike-speeds", "Routing", "Compute separate average speeds for bicycles");

    oc.doRegister("device.rerouting.output", new Option_FileName());
    oc.addDescription("device.rerouting.output", "Routing", "Save adapting weights to FILE");
}


bool
MSDevice_Routing::checkOptions(OptionsCont& oc) {
    bool ok = true;
    if (!oc.isDefault("device.rerouting.adaptation-steps") && !oc.isDefault("device.rerouting.adaptation-weight")) {
        WRITE_ERROR("Only one of the options 'device.rerouting.adaptation-steps' or 'device.rerouting.adaptation-weight' may be given.");
        ok = false;
    }
    if (oc.getFloat("weights.random-factor") < 1) {
        WRITE_ERROR("weights.random-factor cannot be less than 1");
        ok = false;
    }
    if (string2time(oc.getString("device.rerouting.adaptation-interval")) < 0) {
        WRITE_ERROR("Negative value for device.rerouting.adaptation-interval!");
        ok = false;
    }
    if (oc.getFloat("device.rerouting.adaptation-weight") < 0.  ||
            oc.getFloat("device.rerouting.adaptation-weight") > 1.) {
        WRITE_ERROR("The value for device.rerouting.adaptation-weight must be between 0 and 1!");
        ok = false;
    }
#ifndef HAVE_FOX
    if (oc.getInt("device.rerouting.threads") > 1) {
        WRITE_ERROR("Parallel routing is only possible when compiled with Fox.");
        ok = false;
    }
#endif
    if (oc.getInt("threads") > 1 && oc.getInt("device.rerouting.threads") > 1 && oc.getInt("threads") != oc.getInt("device.rerouting.threads")) {
        WRITE_WARNING("Adapting number of routing threads to number of simulation threads.");
    }
    return ok;
}


void
MSDevice_Routing::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into) {
    const OptionsCont& oc = OptionsCont::getOptions();
    const bool equip = equippedByDefaultAssignmentOptions(oc, "rerouting", v, false);
    if (v.getParameter().wasSet(VEHPARS_FORCE_REROUTE) || equip) {
        // route computation is enabled
        // for implicitly equipped vehicles (trips, flows), option probability
        // can still be used to disable periodic rerouting after insertion for
        // parts of the fleet
        const SUMOTime period = equip || oc.isDefault("device.rerouting.probability") ? string2time(oc.getString("device.rerouting.period")) : 0;
        const SUMOTime prePeriod = MAX2((SUMOTime)0, string2time(oc.getString("device.rerouting.pre-period")));
        MSRoutingEngine::initWeightUpdate();
        // build the device
        into.push_back(new MSDevice_Routing(v, "routing_" + v.getID(), period, prePeriod));
    }
}


// ---------------------------------------------------------------------------
// MSDevice_Routing-methods
// ---------------------------------------------------------------------------
MSDevice_Routing::MSDevice_Routing(SUMOVehicle& holder, const std::string& id,
                                   SUMOTime period, SUMOTime preInsertionPeriod) :
    MSVehicleDevice(holder, id),
    myPeriod(period),
    myPreInsertionPeriod(preInsertionPeriod),
    myLastRouting(-1),
    mySkipRouting(-1),
    myRerouteCommand(nullptr),
    myRerouteRailSignal(getBoolParam(holder, OptionsCont::getOptions(), "rerouting.railsignal", true, true)) {
    if (myPreInsertionPeriod > 0 || holder.getParameter().wasSet(VEHPARS_FORCE_REROUTE)) {
        // we do always a pre insertion reroute for trips to fill the best lanes of the vehicle with somehow meaningful values (especially for deaprtLane="best")
        myRerouteCommand = new WrappingCommand<MSDevice_Routing>(this, &MSDevice_Routing::preInsertionReroute);
        // if we don't update the edge weights, we might as well reroute now and hopefully use our threads better
        const SUMOTime execTime = MSRoutingEngine::hasEdgeUpdates() ? holder.getParameter().depart : -1;
        MSNet::getInstance()->getInsertionEvents()->addEvent(myRerouteCommand, execTime);
    }
}


MSDevice_Routing::~MSDevice_Routing() {
    // make the rerouting command invalid if there is one
    if (myRerouteCommand != nullptr) {
        myRerouteCommand->deschedule();
    }
}


bool
MSDevice_Routing::notifyEnter(SUMOTrafficObject& /*veh*/, MSMoveReminder::Notification reason, const MSLane* enteredLane) {
    if (reason == MSMoveReminder::NOTIFICATION_DEPARTED) {
        // clean up pre depart rerouting
        if (myRerouteCommand != nullptr) {
            myRerouteCommand->deschedule();
        } else if (myPreInsertionPeriod > 0 && myHolder.getDepartDelay() > myPreInsertionPeriod && enteredLane != nullptr) {
            // pre-insertion rerouting was disabled. Reroute once if insertion was delayed
            // this is happening in the run thread (not inbeginOfTimestepEvents) so we cannot safely use the threadPool
            myHolder.reroute(MSNet::getInstance()->getCurrentTimeStep(), "device.rerouting",
                             MSRoutingEngine::getRouterTT(myHolder.getRNGIndex(), myHolder.getVClass()),
                             false, MSRoutingEngine::withTaz(), false);
        }
        myRerouteCommand = nullptr;
        // build repetition trigger if routing shall be done more often
        if (myPeriod > 0) {
            myRerouteCommand = new WrappingCommand<MSDevice_Routing>(this, &MSDevice_Routing::wrappedRerouteCommandExecute);
            SUMOTime start = MSNet::getInstance()->getCurrentTimeStep();
            if (OptionsCont::getOptions().getBool("device.rerouting.synchronize")) {
                start -= start % myPeriod;
            }
            MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(myRerouteCommand, myPeriod + start);
        }
    }
    return false;
}


SUMOTime
MSDevice_Routing::preInsertionReroute(const SUMOTime currentTime) {
    if (mySkipRouting == currentTime) {
        return DELTA_T;
    }
    if (myPreInsertionPeriod == 0) {
        // the event will deschedule and destroy itself so it does not need to be stored
        myRerouteCommand = nullptr;
    }
    const MSEdge* source = *myHolder.getRoute().begin();
    const MSEdge* dest = myHolder.getRoute().getLastEdge();
    if (source->isTazConnector() && dest->isTazConnector()) {
        const MSRoute* cached = MSRoutingEngine::getCachedRoute(std::make_pair(source, dest));
        if (cached != nullptr && cached->size() > 2) {
            myHolder.replaceRoute(cached, "device.rerouting", true);
            return myPreInsertionPeriod;
        }
    }
    try {
        std::string msg;
        if (myHolder.hasValidRouteStart(msg)) {
            reroute(currentTime, true);
        }
    } catch (ProcessError&) {
        myRerouteCommand = nullptr;
        throw;
    }
    // avoid repeated pre-insertion rerouting when the departure edge is fix and
    // the departure lane does not depend on the route
    if (myPreInsertionPeriod > 0 && !source->isTazConnector() && myHolder.getParameter().departLaneProcedure != DepartLaneDefinition::BEST_FREE) {
        myRerouteCommand = nullptr;
        return 0;
    }
    return myPreInsertionPeriod;
}


SUMOTime
MSDevice_Routing::wrappedRerouteCommandExecute(SUMOTime currentTime) {
    reroute(currentTime);
    return myPeriod;
}


void
MSDevice_Routing::reroute(const SUMOTime currentTime, const bool onInit) {
    MSRoutingEngine::initEdgeWeights(myHolder.getVClass());
    //check whether the weights did change since the last reroute
    if (myLastRouting >= MSRoutingEngine::getLastAdaptation()) {
        return;
    }
    myLastRouting = currentTime;
    MSRoutingEngine::reroute(myHolder, currentTime, "device.rerouting", onInit);
}


std::string
MSDevice_Routing::getParameter(const std::string& key) const {
    if (StringUtils::startsWith(key, "edge:")) {
        const std::string edgeID = key.substr(5);
        const MSEdge* edge = MSEdge::dictionary(edgeID);
        if (edge == nullptr) {
            throw InvalidArgument("Edge '" + edgeID + "' is invalid for parameter retrieval of '" + deviceName() + "'");
        }
        return toString(MSRoutingEngine::getEffort(edge, &myHolder, 0));
    } else if (key == "period") {
        return time2string(myPeriod);
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


void
MSDevice_Routing::setParameter(const std::string& key, const std::string& value) {
    double doubleValue;
    try {
        doubleValue = StringUtils::toDouble(value);
    } catch (NumberFormatException&) {
        throw InvalidArgument("Setting parameter '" + key + "' requires a number for device of type '" + deviceName() + "'");
    }
    if (StringUtils::startsWith(key, "edge:")) {
        const std::string edgeID = key.substr(5);
        const MSEdge* edge = MSEdge::dictionary(edgeID);
        if (edge == nullptr) {
            throw InvalidArgument("Edge '" + edgeID + "' is invalid for parameter setting of '" + deviceName() + "'");
        }
        MSRoutingEngine::setEdgeTravelTime(edge, doubleValue);
    } else if (key == "period") {
        myPeriod = TIME2STEPS(doubleValue);
        // re-schedule routing command
        notifyEnter(myHolder, MSMoveReminder::NOTIFICATION_DEPARTED, nullptr);
    } else {
        throw InvalidArgument("Setting parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
    }
}


void
MSDevice_Routing::saveState(OutputDevice& out) const {
    out.openTag(SUMO_TAG_DEVICE);
    out.writeAttr(SUMO_ATTR_ID, getID());
    std::vector<std::string> internals;
    internals.push_back(toString(myPeriod));
    out.writeAttr(SUMO_ATTR_STATE, toString(internals));
    out.closeTag();
}


void
MSDevice_Routing::loadState(const SUMOSAXAttributes& attrs) {
    std::istringstream bis(attrs.getString(SUMO_ATTR_STATE));
    bis >> myPeriod;
}


/****************************************************************************/
