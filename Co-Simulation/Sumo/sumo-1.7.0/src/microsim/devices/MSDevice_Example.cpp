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
/// @file    MSDevice_Example.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    11.06.2013
///
// A device which stands as an implementation example and which outputs movereminder calls
/****************************************************************************/
#include <config.h>

#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicle.h>
#include "MSDevice_Tripinfo.h"
#include "MSDevice_Example.h"


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_Example::insertOptions(OptionsCont& oc) {
    oc.addOptionSubTopic("Example Device");
    insertDefaultAssignmentOptions("example", "Example Device", oc);

    oc.doRegister("device.example.parameter", new Option_Float(0.0));
    oc.addDescription("device.example.parameter", "Example Device", "An exemplary parameter which can be used by all instances of the example device");
}


void
MSDevice_Example::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into) {
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "example", v, false)) {
        // build the device
        // get custom vehicle parameter
        double customParameter2 = -1;
        if (v.getParameter().knowsParameter("example")) {
            try {
                customParameter2 = StringUtils::toDouble(v.getParameter().getParameter("example", "-1"));
            } catch (...) {
                WRITE_WARNING("Invalid value '" + v.getParameter().getParameter("example", "-1") + "'for vehicle parameter 'example'");
            }

        } else {
            std::cout << "vehicle '" << v.getID() << "' does not supply vehicle parameter 'example'. Using default of " << customParameter2 << "\n";
        }
        // get custom vType parameter
        double customParameter3 = -1;
        if (v.getVehicleType().getParameter().knowsParameter("example")) {
            try {
                customParameter3 = StringUtils::toDouble(v.getVehicleType().getParameter().getParameter("example", "-1"));
            } catch (...) {
                WRITE_WARNING("Invalid value '" + v.getVehicleType().getParameter().getParameter("example", "-1") + "'for vType parameter 'example'");
            }

        } else {
            std::cout << "vehicle '" << v.getID() << "' does not supply vType parameter 'example'. Using default of " << customParameter3 << "\n";
        }
        MSDevice_Example* device = new MSDevice_Example(v, "example_" + v.getID(),
                oc.getFloat("device.example.parameter"),
                customParameter2,
                customParameter3);
        into.push_back(device);
    }
}

void
MSDevice_Example::cleanup() {
    // cleaning up global state (if any)
}

// ---------------------------------------------------------------------------
// MSDevice_Example-methods
// ---------------------------------------------------------------------------
MSDevice_Example::MSDevice_Example(SUMOVehicle& holder, const std::string& id,
                                   double customValue1, double customValue2, double customValue3) :
    MSVehicleDevice(holder, id),
    myCustomValue1(customValue1),
    myCustomValue2(customValue2),
    myCustomValue3(customValue3) {
    std::cout << "initialized device '" << id << "' with myCustomValue1=" << myCustomValue1 << ", myCustomValue2=" << myCustomValue2 << ", myCustomValue3=" << myCustomValue3 << "\n";
}


MSDevice_Example::~MSDevice_Example() {
}


bool
MSDevice_Example::notifyMove(SUMOTrafficObject& tObject, double /* oldPos */,
                             double /* newPos */, double newSpeed) {
    std::cout << "device '" << getID() << "' notifyMove: newSpeed=" << newSpeed << "\n";
    if (tObject.isVehicle()) {
        SUMOVehicle& veh = static_cast<SUMOVehicle&>(tObject);
        // check whether another device is present on the vehicle:
        MSDevice_Tripinfo* otherDevice = static_cast<MSDevice_Tripinfo*>(veh.getDevice(typeid(MSDevice_Tripinfo)));
        if (otherDevice != nullptr) {
            std::cout << "  veh '" << veh.getID() << " has device '" << otherDevice->getID() << "'\n";
        }
    }
    return true; // keep the device
}


bool
MSDevice_Example::notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    std::cout << "device '" << getID() << "' notifyEnter: reason=" << reason << " currentEdge=" << veh.getEdge()->getID() << "\n";
    return true; // keep the device
}


bool
MSDevice_Example::notifyLeave(SUMOTrafficObject& veh, double /*lastPos*/, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    std::cout << "device '" << getID() << "' notifyLeave: reason=" << reason << " currentEdge=" << veh.getEdge()->getID() << "\n";
    return true; // keep the device
}


void
MSDevice_Example::generateOutput(OutputDevice* tripinfoOut) const {
    if (tripinfoOut != nullptr) {
        tripinfoOut->openTag("example_device");
        tripinfoOut->writeAttr("customValue1", toString(myCustomValue1));
        tripinfoOut->writeAttr("customValue2", toString(myCustomValue2));
        tripinfoOut->closeTag();
    }
}

std::string
MSDevice_Example::getParameter(const std::string& key) const {
    if (key == "customValue1") {
        return toString(myCustomValue1);
    } else if (key == "customValue2") {
        return toString(myCustomValue2);
    } else if (key == "meaningOfLife") {
        return "42";
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


void
MSDevice_Example::setParameter(const std::string& key, const std::string& value) {
    double doubleValue;
    try {
        doubleValue = StringUtils::toDouble(value);
    } catch (NumberFormatException&) {
        throw InvalidArgument("Setting parameter '" + key + "' requires a number for device of type '" + deviceName() + "'");
    }
    if (key == "customValue1") {
        myCustomValue1 = doubleValue;
    } else {
        throw InvalidArgument("Setting parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
    }
}


/****************************************************************************/
