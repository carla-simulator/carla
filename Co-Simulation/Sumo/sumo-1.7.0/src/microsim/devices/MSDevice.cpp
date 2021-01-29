/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    14.08.2013
///
// Abstract in-vehicle device
/****************************************************************************/
#include <config.h>

#include <utils/options/OptionsCont.h>
#include <utils/common/StringUtils.h>
#include <microsim/MSVehicle.h>
#include <microsim/transportables/MSTransportable.h>
#include <microsim/MSVehicleControl.h>
#include "MSDevice.h"
#include "MSDevice_Vehroutes.h"
#include "MSDevice_Tripinfo.h"
#include "MSDevice_Routing.h"
#include "MSDevice_Emissions.h"
#include "MSDevice_BTreceiver.h"
#include "MSDevice_BTsender.h"
#include "MSDevice_Example.h"
#include "MSDevice_Battery.h"
#include "MSDevice_SSM.h"
#include "MSDevice_ToC.h"
#include "MSDevice_DriverState.h"
#include "MSDevice_Bluelight.h"
#include "MSDevice_FCD.h"
#include "MSDevice_Taxi.h"
#include "MSDevice_ElecHybrid.h"
#include "MSTransportableDevice_Routing.h"
#include "MSTransportableDevice_FCD.h"
#include "MSRoutingEngine.h"


// ===========================================================================
// static member variables
// ===========================================================================
std::map<std::string, std::set<std::string> > MSDevice::myExplicitIDs;
std::mt19937 MSDevice::myEquipmentRNG;

// ===========================================================================
// debug flags
// ===========================================================================
//#define DEBUG_DEVICE_PARAMS


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice::insertOptions(OptionsCont& oc) {
    MSDevice_Routing::insertOptions(oc);
    MSDevice_Emissions::insertOptions(oc);
    MSDevice_BTreceiver::insertOptions(oc);
    MSDevice_BTsender::insertOptions(oc);
    MSDevice_Example::insertOptions(oc);
    MSDevice_Battery::insertOptions(oc);
    MSDevice_SSM::insertOptions(oc);
    MSDevice_ToC::insertOptions(oc);
    MSDevice_DriverState::insertOptions(oc);
    MSDevice_Bluelight::insertOptions(oc);
    MSDevice_FCD::insertOptions(oc);
    MSDevice_ElecHybrid::insertOptions(oc);
    MSDevice_Taxi::insertOptions(oc);
    MSDevice_Tripinfo::insertOptions(oc);

    MSTransportableDevice_Routing::insertOptions(oc);
    MSTransportableDevice_FCD::insertOptions(oc);
}


bool
MSDevice::checkOptions(OptionsCont& oc) {
    bool ok = true;
    ok &= MSDevice_Routing::checkOptions(oc);
    return ok;
}


void
MSDevice::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into) {
    MSDevice_Vehroutes::buildVehicleDevices(v, into);
    MSDevice_Tripinfo::buildVehicleDevices(v, into);
    MSDevice_Routing::buildVehicleDevices(v, into);
    MSDevice_Emissions::buildVehicleDevices(v, into);
    MSDevice_BTreceiver::buildVehicleDevices(v, into);
    MSDevice_BTsender::buildVehicleDevices(v, into);
    MSDevice_Example::buildVehicleDevices(v, into);
    MSDevice_Battery::buildVehicleDevices(v, into);
    MSDevice_SSM::buildVehicleDevices(v, into);
    MSDevice_ToC::buildVehicleDevices(v, into);
    MSDevice_DriverState::buildVehicleDevices(v, into);
    MSDevice_Bluelight::buildVehicleDevices(v, into);
    MSDevice_FCD::buildVehicleDevices(v, into);
    MSDevice_ElecHybrid::buildVehicleDevices(v, into);
    MSDevice_Taxi::buildVehicleDevices(v, into);
}


void
MSDevice::buildTransportableDevices(MSTransportable& p, std::vector<MSTransportableDevice*>& into) {
    MSTransportableDevice_Routing::buildDevices(p, into);
    MSTransportableDevice_FCD::buildDevices(p, into);
}


void
MSDevice::cleanupAll() {
    MSRoutingEngine::cleanup();
    MSDevice_Tripinfo::cleanup();
    MSDevice_FCD::cleanup();
    MSDevice_Taxi::cleanup();
}

void
MSDevice::insertDefaultAssignmentOptions(const std::string& deviceName, const std::string& optionsTopic, OptionsCont& oc, const bool isPerson) {
    const std::string prefix = (isPerson ? "person-device." : "device.") + deviceName;
    const std::string object = isPerson ? "person" : "vehicle";
    oc.doRegister(prefix + ".probability", new Option_Float(-1.0));// (default: no need to call RNG)
    oc.addDescription(prefix + ".probability", optionsTopic, "The probability for a " + object + " to have a '" + deviceName + "' device");

    oc.doRegister(prefix + ".explicit", new Option_StringVector());
    oc.addSynonyme(prefix + ".explicit", prefix + ".knownveh", true);
    oc.addDescription(prefix + ".explicit", optionsTopic, "Assign a '" + deviceName + "' device to named " + object + "s");

    oc.doRegister(prefix + ".deterministic", new Option_Bool(false));
    oc.addDescription(prefix + ".deterministic", optionsTopic, "The '" + deviceName + "' devices are set deterministic using a fraction of 1000");
}


void
MSDevice::saveState(OutputDevice& /* out */) const {
    WRITE_WARNING("Device '" + getID() + "' cannot save state");
}


void
MSDevice::loadState(const SUMOSAXAttributes& /* attrs */) {
}


std::string
MSDevice::getStringParam(const SUMOVehicle& v, const OptionsCont& oc, std::string paramName, std::string deflt, bool required) {
    const std::string key = "device." + paramName;
    if (v.getParameter().knowsParameter(key)) {
        return v.getParameter().getParameter(key, "");
    } else if (v.getVehicleType().getParameter().knowsParameter(key)) {
        return v.getVehicleType().getParameter().getParameter(key, "");
    } else {
        if (oc.exists(key) && oc.isSet(key)) {
            return oc.getValueString(key);
        } else {
            if (required) {
                throw ProcessError("Missing parameter '" + key + "' for vehicle '" + v.getID());
            } else {
#ifdef DEBUG_DEVICE_PARAMS
                std::cout << "vehicle '" << v.getID() << "' does not supply vehicle parameter '" + key + "'. Using default of '" << result << "'\n";
#endif
                return deflt;
            }
        }
    }
}


double
MSDevice::getFloatParam(const SUMOVehicle& v, const OptionsCont& oc, std::string paramName, double deflt, bool required) {
    const std::string key = "device." + paramName;
    std::string val = getStringParam(v, oc, paramName, toString(deflt), required);
    try {
        return StringUtils::toDouble(val);
    } catch (...) {
        WRITE_ERROR("Invalid float value '" + val + "'for parameter '" + key + "'");
        return deflt;
    }
}


bool
MSDevice::getBoolParam(const SUMOVehicle& v, const OptionsCont& oc, std::string paramName, bool deflt, bool required) {
    const std::string key = "device." + paramName;
    std::string val = getStringParam(v, oc, paramName, toString(deflt), required);
    try {
        return StringUtils::toBool(val);
    } catch (...) {
        WRITE_ERROR("Invalid bool value '" + val + "'for parameter '" + key + "'");
        return deflt;
    }
}


/****************************************************************************/
