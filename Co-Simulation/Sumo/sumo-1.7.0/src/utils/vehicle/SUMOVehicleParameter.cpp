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
/// @file    SUMOVehicleParameter.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 31.03.2009
///
// Structure representing possible vehicle parameter
/****************************************************************************/
#include <config.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/options/OptionsCont.h>

#include "SUMOVehicleParameter.h"

// ===========================================================================
// member method definitions
// ===========================================================================

SUMOVehicleParameter::SUMOVehicleParameter()
    : tag(SUMO_TAG_NOTHING), vtypeid(DEFAULT_VTYPE_ID), color(RGBColor::DEFAULT_COLOR),
      depart(-1), departProcedure(DEPART_GIVEN),
      departLane(0), departLaneProcedure(DepartLaneDefinition::DEFAULT),
      departPos(0), departPosProcedure(DepartPosDefinition::DEFAULT),
      departPosLat(0), departPosLatProcedure(DepartPosLatDefinition::DEFAULT),
      departSpeed(-1), departSpeedProcedure(DepartSpeedDefinition::DEFAULT),
      arrivalLane(0), arrivalLaneProcedure(ArrivalLaneDefinition::DEFAULT),
      arrivalPos(0), arrivalPosProcedure(ArrivalPosDefinition::DEFAULT),
      arrivalPosLat(0), arrivalPosLatProcedure(ArrivalPosLatDefinition::DEFAULT),
      arrivalSpeed(-1), arrivalSpeedProcedure(ArrivalSpeedDefinition::DEFAULT),
      repetitionNumber(-1), repetitionsDone(-1), repetitionOffset(-1), repetitionProbability(-1), repetitionEnd(-1),
      line(), fromTaz(), toTaz(), personNumber(0), containerNumber(0),
      speedFactor(-1),
      parametersSet(0)
{ }


SUMOVehicleParameter::~SUMOVehicleParameter() {
}


bool
SUMOVehicleParameter::defaultOptionOverrides(const OptionsCont& oc, const std::string& optionName) const {
    return oc.exists(optionName) && oc.isSet(optionName) && oc.getBool("defaults-override");
}


void
SUMOVehicleParameter::write(OutputDevice& dev, const OptionsCont& oc, const SumoXMLTag tag, const std::string& typeID) const {
    dev.openTag(tag).writeAttr(SUMO_ATTR_ID, id);
    if (typeID == "") {
        if (wasSet(VEHPARS_VTYPE_SET)) {
            dev.writeAttr(SUMO_ATTR_TYPE, vtypeid);
        }
    } else {
        dev.writeAttr(SUMO_ATTR_TYPE, typeID);
    }
    // write depart depending of tag
    if ((tag == SUMO_TAG_FLOW) || (tag == SUMO_TAG_PERSONFLOW) ||
            (tag == GNE_TAG_FLOW_ROUTE) || (tag == GNE_TAG_FLOW_WITHROUTE)) {
        dev.writeAttr(SUMO_ATTR_BEGIN, getDepart());
    } else {
        dev.writeAttr(SUMO_ATTR_DEPART, getDepart());
    }
    // optional parameter
    //  departlane
    if (wasSet(VEHPARS_DEPARTLANE_SET) && !defaultOptionOverrides(oc, "departlane")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_DEPARTLANE, getDepartLane());
    } else if (oc.exists("departlane") && oc.isSet("departlane")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_DEPARTLANE, oc.getString("departlane"));
    }
    //  departpos
    if (wasSet(VEHPARS_DEPARTPOS_SET) && !defaultOptionOverrides(oc, "departpos")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_DEPARTPOS, getDepartPos());
    } else if (oc.exists("departpos") && oc.isSet("departpos")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_DEPARTPOS, oc.getString("departpos"));
    }
    //  departPosLat
    if (wasSet(VEHPARS_DEPARTPOSLAT_SET)) {
        dev.writeNonEmptyAttr(SUMO_ATTR_DEPARTPOS_LAT, getDepartPosLat());
    }
    //  departspeed
    if (wasSet(VEHPARS_DEPARTSPEED_SET) && !defaultOptionOverrides(oc, "departspeed")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_DEPARTSPEED, getDepartSpeed());
    } else if (oc.exists("departspeed") && oc.isSet("departspeed")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_DEPARTSPEED, oc.getString("departspeed"));
    }
    //  arrivallane
    if (wasSet(VEHPARS_ARRIVALLANE_SET) && !defaultOptionOverrides(oc, "arrivallane")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_ARRIVALLANE, getArrivalLane());
    } else if (oc.exists("arrivallane") && oc.isSet("arrivallane")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_ARRIVALLANE, oc.getString("arrivallane"));
    }
    //  arrivalpos
    if (wasSet(VEHPARS_ARRIVALPOS_SET) && !defaultOptionOverrides(oc, "arrivalpos")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_ARRIVALPOS, getArrivalPos());
    } else if (oc.exists("arrivalpos") && oc.isSet("arrivalpos")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_ARRIVALPOS, oc.getString("arrivalpos"));
    }
    //  arrivalPosLat
    if (wasSet(VEHPARS_ARRIVALPOSLAT_SET)) {
        dev.writeNonEmptyAttr(SUMO_ATTR_ARRIVALPOS_LAT, getArrivalPosLat());
    }
    //  arrivalspeed
    if (wasSet(VEHPARS_ARRIVALSPEED_SET) && !defaultOptionOverrides(oc, "arrivalspeed")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_ARRIVALSPEED, getArrivalSpeed());
    } else if (oc.exists("arrivalspeed") && oc.isSet("arrivalspeed")) {
        dev.writeNonEmptyAttr(SUMO_ATTR_ARRIVALSPEED, oc.getString("arrivalspeed"));
    }
    // color
    if (wasSet(VEHPARS_COLOR_SET)) {
        dev.writeAttr(SUMO_ATTR_COLOR, color);
    }
    // line
    if (wasSet(VEHPARS_LINE_SET)) {
        dev.writeAttr(SUMO_ATTR_LINE, line);
    }
    // from TAZ
    if (wasSet(VEHPARS_FROM_TAZ_SET)) {
        dev.writeAttr(SUMO_ATTR_FROM_TAZ, fromTaz);
    }
    // to TAZ
    if (wasSet(VEHPARS_TO_TAZ_SET)) {
        dev.writeAttr(SUMO_ATTR_TO_TAZ, toTaz);
    }
    // person number
    if (wasSet(VEHPARS_PERSON_NUMBER_SET)) {
        dev.writeAttr(SUMO_ATTR_PERSON_NUMBER, personNumber);
    }
    // container number
    if (wasSet(VEHPARS_CONTAINER_NUMBER_SET)) {
        dev.writeAttr(SUMO_ATTR_CONTAINER_NUMBER, containerNumber);
    }
    // individual speedFactor
    if (wasSet(VEHPARS_SPEEDFACTOR_SET)) {
        dev.writeAttr(SUMO_ATTR_SPEEDFACTOR, speedFactor);
    }
}


void
SUMOVehicleParameter::Stop::write(OutputDevice& dev, bool close) const {
    dev.openTag(SUMO_TAG_STOP);
    if (busstop != "") {
        dev.writeAttr(SUMO_ATTR_BUS_STOP, busstop);
    }
    if (containerstop != "") {
        dev.writeAttr(SUMO_ATTR_CONTAINER_STOP, containerstop);
    }
    if (chargingStation != "") {
        dev.writeAttr(SUMO_ATTR_CHARGING_STATION, chargingStation);
    }
    if (parkingarea != "") {
        dev.writeAttr(SUMO_ATTR_PARKING_AREA, parkingarea);
    }
    if ((busstop == "") && (containerstop == "") && (parkingarea == "") && (chargingStation == "")) {
        if (edge.size() > 0) {
            dev.writeAttr(SUMO_ATTR_LANE, edge + "_0");
        } else {
            dev.writeAttr(SUMO_ATTR_LANE, lane);
        }
        if ((parametersSet & STOP_START_SET) != 0) {
            dev.writeAttr(SUMO_ATTR_STARTPOS, startPos);
        }
        if ((parametersSet & STOP_END_SET) != 0) {
            dev.writeAttr(SUMO_ATTR_ENDPOS, endPos);
        }
    }
    if ((parametersSet & STOP_ARRIVAL_SET) && (arrival >= 0)) {
        dev.writeAttr(SUMO_ATTR_ARRIVAL, time2string(arrival));
    }
    if ((parametersSet & STOP_DURATION_SET) && (duration >= 0)) {
        dev.writeAttr(SUMO_ATTR_DURATION, time2string(duration));
    }
    if ((parametersSet & STOP_UNTIL_SET) && (until >= 0)) {
        dev.writeAttr(SUMO_ATTR_UNTIL, time2string(until));
    }
    if ((parametersSet & STOP_EXTENSION_SET) && (extension >= 0)) {
        dev.writeAttr(SUMO_ATTR_EXTENSION, time2string(extension));
    }
    writeTriggers(dev);
    if ((parametersSet & STOP_PARKING_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_PARKING, parking);
    }
    if ((parametersSet & STOP_EXPECTED_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_EXPECTED, awaitedPersons);
    }
    if ((parametersSet & STOP_EXPECTED_CONTAINERS_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_EXPECTED_CONTAINERS, awaitedContainers);
    }
    if ((parametersSet & STOP_TRIP_ID_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_TRIP_ID, tripId);
    }
    if ((parametersSet & STOP_LINE_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_LINE, line);
    }
    if ((parametersSet & STOP_SPLIT_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_SPLIT, split);
    }
    if ((parametersSet & STOP_JOIN_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_JOIN, join);
    }
    if ((parametersSet & STOP_SPEED_SET) != 0) {
        dev.writeAttr(SUMO_ATTR_SPEED, speed);
    }
    // only write friendly position if is true
    if (friendlyPos == true) {
        dev.writeAttr(SUMO_ATTR_FRIENDLY_POS, friendlyPos);
    }
    // only write act type if isn't empty
    if (!actType.empty()) {
        dev.writeAttr(SUMO_ATTR_ACTTYPE, actType);
    }
    if (close) {
        dev.closeTag();
    }
}


bool
SUMOVehicleParameter::parseDepart(const std::string& val, const std::string& element, const std::string& id,
                                  SUMOTime& depart, DepartDefinition& dd, std::string& error) {
    if (val == "triggered") {
        dd = DEPART_TRIGGERED;
    } else if (val == "containerTriggered") {
        dd = DEPART_CONTAINER_TRIGGERED;
    } else if (val == "split") {
        dd = DEPART_SPLIT;
    } else if (val == "now") {
        // only used via TraCI. depart must be set by the calling code
        dd = DEPART_NOW;
    } else {
        try {
            depart = string2time(val);
            dd = DEPART_GIVEN;
            if (depart < 0) {
                error = "Negative departure time in the definition of '" + id + "'.";
                return false;
            }
        } catch (...) {
            if (id.empty()) {
                error = "Invalid departure time for " + element + ". Must be one of (\"triggered\", \"containerTriggered\", \"now\", or a float >= 0)";
            } else {
                error = "Invalid departure time for " + element + " '" + id + "';\n must be one of (\"triggered\", \"containerTriggered\", \"now\", or a float >= 0)";
            }
            return false;
        }
    }
    return true;
}


bool
SUMOVehicleParameter::parseDepartLane(const std::string& val, const std::string& element, const std::string& id,
                                      int& lane, DepartLaneDefinition& dld, std::string& error) {
    bool ok = true;
    lane = 0;
    dld = DepartLaneDefinition::GIVEN;
    if (val == "random") {
        dld = DepartLaneDefinition::RANDOM;
    } else if (val == "free") {
        dld = DepartLaneDefinition::FREE;
    } else if (val == "allowed") {
        dld = DepartLaneDefinition::ALLOWED_FREE;
    } else if (val == "best") {
        dld = DepartLaneDefinition::BEST_FREE;
    } else if (val == "first") {
        dld = DepartLaneDefinition::FIRST_ALLOWED;
    } else {
        try {
            lane = StringUtils::toInt(val);
            if (lane < 0) {
                ok = false;
            }
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid departLane definition for " + element + ". Must be one of (\"random\", \"free\", \"allowed\", \"best\", \"first\", or an int>=0)";
        } else {
            error = "Invalid departLane definition for " + element + " '" + id + "';\n must be one of (\"random\", \"free\", \"allowed\", \"best\", \"first\", or an int>=0)";
        }
    }
    return ok;
}


bool
SUMOVehicleParameter::parseDepartPos(const std::string& val, const std::string& element, const std::string& id,
                                     double& pos, DepartPosDefinition& dpd, std::string& error) {
    bool ok = true;
    pos = 0.;
    dpd = DepartPosDefinition::GIVEN;
    if (val == "random") {
        dpd = DepartPosDefinition::RANDOM;
    } else if (val == "random_free") {
        dpd = DepartPosDefinition::RANDOM_FREE;
    } else if (val == "free") {
        dpd = DepartPosDefinition::FREE;
    } else if (val == "base") {
        dpd = DepartPosDefinition::BASE;
    } else if (val == "last") {
        dpd = DepartPosDefinition::LAST;
    } else if (val == "stop") {
        dpd = DepartPosDefinition::STOP;
    } else {
        try {
            pos = StringUtils::toDouble(val);
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid departPos definition for " + element + ". Must be one of (\"random\", \"random_free\", \"free\", \"base\", \"last\" or a float)";
        } else {
            error = "Invalid departPos definition for " + element + " '" + id + "';\n must be one of (\"random\", \"random_free\", \"free\", \"base\", \"last\" or a float)";
        }
    }
    return ok;
}


bool
SUMOVehicleParameter::parseDepartPosLat(const std::string& val, const std::string& element, const std::string& id,
                                        double& pos, DepartPosLatDefinition& dpd, std::string& error) {
    bool ok = true;
    pos = 0.;
    dpd = DepartPosLatDefinition::GIVEN;
    if (val == "random") {
        dpd = DepartPosLatDefinition::RANDOM;
    } else if (val == "random_free") {
        dpd = DepartPosLatDefinition::RANDOM_FREE;
    } else if (val == "free") {
        dpd = DepartPosLatDefinition::FREE;
    } else if (val == "right") {
        dpd = DepartPosLatDefinition::RIGHT;
    } else if (val == "center") {
        dpd = DepartPosLatDefinition::CENTER;
    } else if (val == "left") {
        dpd = DepartPosLatDefinition::LEFT;
    } else {
        try {
            pos = StringUtils::toDouble(val);
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid departPosLat definition for " + element + ". Must be one of (\"random\", \"random_free\", \"free\", \"right\", \"center\", \"left\", or a float)";
        } else {
            error = "Invalid departPosLat definition for " + element + " '" + id + "';\n must be one of (\"random\", \"random_free\", \"free\", \"right\", \"center\", \"left\", or a float)";
        }
    }
    return ok;
}


bool
SUMOVehicleParameter::parseDepartSpeed(const std::string& val, const std::string& element, const std::string& id,
                                       double& speed, DepartSpeedDefinition& dsd, std::string& error) {
    bool ok = true;
    speed = -1.;
    dsd = DepartSpeedDefinition::GIVEN;
    if (val == "random") {
        dsd = DepartSpeedDefinition::RANDOM;
    } else if (val == "max") {
        dsd = DepartSpeedDefinition::MAX;
    } else if (val == "desired") {
        dsd = DepartSpeedDefinition::DESIRED;
    } else if (val == "speedLimit") {
        dsd = DepartSpeedDefinition::LIMIT;
    } else {
        try {
            speed = StringUtils::toDouble(val);
            if (speed < 0) {
                ok = false;
            }
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid departSpeed definition for " + element + ". Must be one of (\"random\", \"max\", or a float>=0)";
        } else {
            error = "Invalid departSpeed definition for " + element + " '" + id + "';\n must be one of (\"random\", \"max\", or a float>=0)";
        }
    }
    return ok;
}


bool
SUMOVehicleParameter::parseArrivalLane(const std::string& val, const std::string& element, const std::string& id,
                                       int& lane, ArrivalLaneDefinition& ald, std::string& error) {
    bool ok = true;
    lane = 0;
    ald = ArrivalLaneDefinition::GIVEN;
    if (val == "current") {
        ald = ArrivalLaneDefinition::CURRENT;
    } else {
        try {
            lane = StringUtils::toInt(val);
            if (lane < 0) {
                ok = false;
            }
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid arrivalLane definition for " + element + ". Must be one of (\"current\", or an int>=0)";
        } else {
            error = "Invalid arrivalLane definition for " + element + " '" + id + "';\n must be one of (\"current\", or an int>=0)";
        }
    }
    return ok;
}


bool
SUMOVehicleParameter::parseArrivalPos(const std::string& val, const std::string& element, const std::string& id,
                                      double& pos, ArrivalPosDefinition& apd, std::string& error) {
    bool ok = true;
    pos = 0.;
    apd = ArrivalPosDefinition::GIVEN;
    if (val == "random") {
        apd = ArrivalPosDefinition::RANDOM;
    } else if (val == "center") {
        apd = ArrivalPosDefinition::CENTER;
    } else if (val == "max") {
        apd = ArrivalPosDefinition::MAX;
    } else {
        try {
            pos = StringUtils::toDouble(val);
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid arrivalPos definition for " + element + ". Must be one of (\"random\", \"max\", or a float)";
        } else {
            error = "Invalid arrivalPos definition for " + element + " '" + id + "';\n must be one of (\"random\", \"max\", or a float)";
        }
    }
    return ok;
}


bool
SUMOVehicleParameter::parseArrivalPosLat(const std::string& val, const std::string& element, const std::string& id,
        double& pos, ArrivalPosLatDefinition& apd, std::string& error) {
    bool ok = true;
    pos = 0.;
    apd = ArrivalPosLatDefinition::GIVEN;
    if (val == "right") {
        apd = ArrivalPosLatDefinition::RIGHT;
    } else if (val == "center") {
        apd = ArrivalPosLatDefinition::CENTER;
    } else if (val == "left") {
        apd = ArrivalPosLatDefinition::LEFT;
    } else {
        try {
            pos = StringUtils::toDouble(val);
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid arrivalPosLat definition for " + element + ". Must be one of (\"right\", \"center\", \"left\", or a float)";
        } else {
            error = "Invalid arrivalPosLat definition for " + element + " '" + id + "';\n must be one of (\"right\", \"center\", \"left\", or a float)";
        }
    }
    return ok;
}


bool
SUMOVehicleParameter::parseArrivalSpeed(const std::string& val, const std::string& element, const std::string& id,
                                        double& speed, ArrivalSpeedDefinition& asd, std::string& error) {
    bool ok = true;
    speed = -1.;
    asd = ArrivalSpeedDefinition::GIVEN;
    if (val == "current") {
        asd = ArrivalSpeedDefinition::CURRENT;
    } else {
        try {
            speed = StringUtils::toDouble(val);
            if (speed < 0) {
                ok = false;
            }
        } catch (...) {
            ok = false;
        }
    }
    if (!ok) {
        if (id.empty()) {
            error = "Invalid arrivalSpeed definition for " + element + ". Must be one of (\"current\", or a float>=0)";
        } else {
            error = "Invalid arrivalSpeed definition for " + element + " '" + id + "';\n must be one of (\"current\", or a float>=0)";
        }
    }
    return ok;
}


double
SUMOVehicleParameter::interpretEdgePos(double pos, double maximumValue, SumoXMLAttr attr, const std::string& id, bool silent) {
    if (pos < 0) {
        pos = maximumValue + pos;
    }
    if (pos > maximumValue && pos != std::numeric_limits<double>::infinity()) {
        if (!silent) {
            WRITE_WARNING("Invalid " + toString(attr) + " " + toString(pos) + " given for " + id + ". Using edge end instead.");
        }
        pos = maximumValue;
    }
    return pos;
}


bool
SUMOVehicleParameter::parsePersonModes(const std::string& modes, const std::string& element, const std::string& id, SVCPermissions& modeSet, std::string& error) {
    // separte modes in different strings, and check if modes are valid
    for (StringTokenizer st(modes); st.hasNext();) {
        const std::string mode = st.next();
        if (mode == "car") {
            modeSet |= SVC_PASSENGER;
        } else if (mode == "taxi") {
            modeSet |= SVC_TAXI;
        } else if (mode == "bicycle") {
            modeSet |= SVC_BICYCLE;
        } else if (mode == "public") {
            modeSet |= SVC_BUS;
        } else {
            if (id.empty()) {
                error = "Unknown person mode '" + mode + "'. Must be a combination of (\"car\", \"bicycle\" or \"public\")";
            } else {
                error = "Unknown person mode '" + mode + "' for " + element + " '" + id + "';\n must be a combination of (\"car\", \"bicycle\" or \"public\")";
            }
            return false;
        }
    }
    return true;
}


void
SUMOVehicleParameter::parseStopTriggers(const std::vector<std::string>& triggers, bool expectTrigger, Stop& stop) {
    if (triggers.size() == 0 && expectTrigger) {
        stop.triggered = true;
    }
    for (std::string val : triggers) {
        if (val == toString(SUMO_TAG_PERSON)) {
            stop.triggered = true;
        } else if (val == toString(SUMO_TAG_CONTAINER)) {
            stop.containerTriggered = true;
        } else if (val == toString(SUMO_ATTR_JOIN)) {
            stop.joinTriggered = true;
        } else {
            try {
                stop.triggered = StringUtils::toBool(val);
            } catch (BoolFormatException&) {
                WRITE_ERROR("Value of stop attribute 'trigger' must be 'person', 'container', 'join' or a boolean");
            }
        }
    }
}


void
SUMOVehicleParameter::Stop::writeTriggers(OutputDevice& dev) const {
    if ((parametersSet & STOP_TRIGGER_SET) != 0) {
        std::vector<std::string> triggers;
        if (triggered) {
            triggers.push_back(toString(SUMO_TAG_PERSON));
        }
        if (containerTriggered) {
            triggers.push_back(toString(SUMO_TAG_CONTAINER));
        }
        if (joinTriggered) {
            triggers.push_back(toString(SUMO_ATTR_JOIN));
        }
        dev.writeAttr(SUMO_ATTR_TRIGGERED, triggers);
    }
}


std::string
SUMOVehicleParameter::getDepart() const {
    if (departProcedure == DEPART_TRIGGERED) {
        return "triggered";
    } else if (departProcedure == DEPART_CONTAINER_TRIGGERED) {
        return "containerTriggered";
    } else if (departProcedure == DEPART_SPLIT) {
        return "split";
    } else {
        return time2string(depart);
    }
}


std::string
SUMOVehicleParameter::getDepartLane() const {
    std::string val;
    switch (departLaneProcedure) {
        case DepartLaneDefinition::GIVEN:
            val = toString(departLane);
            break;
        case DepartLaneDefinition::RANDOM:
            val = "random";
            break;
        case DepartLaneDefinition::FREE:
            val = "free";
            break;
        case DepartLaneDefinition::ALLOWED_FREE:
            val = "allowed";
            break;
        case DepartLaneDefinition::BEST_FREE:
            val = "best";
            break;
        case DepartLaneDefinition::FIRST_ALLOWED:
            val = "first";
            break;
        case DepartLaneDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


std::string
SUMOVehicleParameter::getDepartPos() const {
    std::string val;
    switch (departPosProcedure) {
        case DepartPosDefinition::GIVEN:
            val = toString(departPos);
            break;
        case DepartPosDefinition::RANDOM:
            val = "random";
            break;
        case DepartPosDefinition::RANDOM_FREE:
            val = "random_free";
            break;
        case DepartPosDefinition::FREE:
            val = "free";
            break;
        case DepartPosDefinition::LAST:
            val = "last";
            break;
        case DepartPosDefinition::BASE:
            val = "base";
            break;
        case DepartPosDefinition::STOP:
            val = "stop";
            break;
        case DepartPosDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


std::string
SUMOVehicleParameter::getDepartPosLat() const {
    std::string val;
    switch (departPosLatProcedure) {
        case DepartPosLatDefinition::GIVEN:
            val = toString(departPos);
            break;
        case DepartPosLatDefinition::RANDOM:
            val = "random";
            break;
        case DepartPosLatDefinition::RANDOM_FREE:
            val = "random_free";
            break;
        case DepartPosLatDefinition::FREE:
            val = "free";
            break;
        case DepartPosLatDefinition::RIGHT:
            val = "right";
            break;
        case DepartPosLatDefinition::CENTER:
            val = "center";
            break;
        case DepartPosLatDefinition::LEFT:
            val = "left";
            break;
        case DepartPosLatDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


std::string
SUMOVehicleParameter::getDepartSpeed() const {
    std::string val;
    switch (departSpeedProcedure) {
        case DepartSpeedDefinition::GIVEN:
            val = toString(departSpeed);
            break;
        case DepartSpeedDefinition::RANDOM:
            val = "random";
            break;
        case DepartSpeedDefinition::MAX:
            val = "max";
            break;
        case DepartSpeedDefinition::DESIRED:
            val = "desired";
            break;
        case DepartSpeedDefinition::LIMIT:
            val = "speedLimit";
            break;
        case DepartSpeedDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


std::string
SUMOVehicleParameter::getArrivalLane() const {
    std::string val;
    switch (arrivalLaneProcedure) {
        case ArrivalLaneDefinition::GIVEN:
            val = toString(arrivalLane);
            break;
        case ArrivalLaneDefinition::CURRENT:
            val = "current";
            break;
        case ArrivalLaneDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


std::string
SUMOVehicleParameter::getArrivalPos() const {
    std::string val;
    switch (arrivalPosProcedure) {
        case ArrivalPosDefinition::GIVEN:
            val = toString(arrivalPos);
            break;
        case ArrivalPosDefinition::RANDOM:
            val = "random";
            break;
        case ArrivalPosDefinition::CENTER:
            val = "center";
            break;
        case ArrivalPosDefinition::MAX:
            val = "max";
            break;
        case ArrivalPosDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


std::string
SUMOVehicleParameter::getArrivalPosLat() const {
    std::string val;
    switch (arrivalPosLatProcedure) {
        case ArrivalPosLatDefinition::GIVEN:
            val = toString(arrivalPos);
            break;
        case ArrivalPosLatDefinition::RIGHT:
            val = "right";
            break;
        case ArrivalPosLatDefinition::CENTER:
            val = "center";
            break;
        case ArrivalPosLatDefinition::LEFT:
            val = "left";
            break;
        case ArrivalPosLatDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


std::string
SUMOVehicleParameter::getArrivalSpeed() const {
    std::string val;
    switch (arrivalSpeedProcedure) {
        case ArrivalSpeedDefinition::GIVEN:
            val = toString(arrivalSpeed);
            break;
        case ArrivalSpeedDefinition::CURRENT:
            val = "current";
            break;
        case ArrivalSpeedDefinition::DEFAULT:
        default:
            break;
    }
    return val;
}


/****************************************************************************/
