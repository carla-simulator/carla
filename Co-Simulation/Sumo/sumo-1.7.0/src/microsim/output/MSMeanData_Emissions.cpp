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
/// @file    MSMeanData_Emissions.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 10.05.2004
///
// Emission data collector for edges/lanes that
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include "MSMeanData_Emissions.h"
#include <utils/emissions/PollutantsInterface.h>
#include <limits>


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// MSMeanData_Emissions::MSLaneMeanDataValues - methods
// ---------------------------------------------------------------------------
MSMeanData_Emissions::MSLaneMeanDataValues::MSLaneMeanDataValues(MSLane* const lane,
        const double length, const bool doAdd,
        const MSMeanData_Emissions* parent)
    : MSMeanData::MeanDataValues(lane, length, doAdd, parent),
      myEmissions() {}


MSMeanData_Emissions::MSLaneMeanDataValues::~MSLaneMeanDataValues() {
}


void
MSMeanData_Emissions::MSLaneMeanDataValues::reset(bool) {
    sampleSeconds = 0.;
    travelledDistance = 0.;
    myEmissions = PollutantsInterface::Emissions();
}


void
MSMeanData_Emissions::MSLaneMeanDataValues::addTo(MSMeanData::MeanDataValues& val) const {
    MSLaneMeanDataValues& v = (MSLaneMeanDataValues&) val;
    v.sampleSeconds += sampleSeconds;
    v.travelledDistance += travelledDistance;
    v.myEmissions.addScaled(myEmissions);
}


void
MSMeanData_Emissions::MSLaneMeanDataValues::notifyMoveInternal(const SUMOTrafficObject& veh, const double /* frontOnLane */, const double timeOnLane, const double /*meanSpeedFrontOnLane*/, const double meanSpeedVehicleOnLane, const double /*travelledDistanceFrontOnLane*/, const double travelledDistanceVehicleOnLane, const double /* meanLengthOnLane */) {
    if (myParent != nullptr && !myParent->vehicleApplies(veh)) {
        return;
    }
    if (veh.isVehicle()) {
        sampleSeconds += timeOnLane;
        travelledDistance += travelledDistanceVehicleOnLane;
        const double a = veh.getAcceleration();
        myEmissions.addScaled(PollutantsInterface::computeAll(veh.getVehicleType().getEmissionClass(),
                              // XXX: recheck, which value to use here for the speed. (Leo) Refs. #2579
                              meanSpeedVehicleOnLane, a, veh.getSlope(),
                              static_cast<const SUMOVehicle&>(veh).getEmissionParameters()), timeOnLane);
    }
}

bool
MSMeanData_Emissions::MSLaneMeanDataValues::notifyIdle(SUMOTrafficObject& veh) {
    if (veh.isVehicle()) {
        myEmissions.addScaled(PollutantsInterface::computeAll(veh.getVehicleType().getEmissionClass(),
                              0., 0., 0.,
                              static_cast<const SUMOVehicle&>(veh).getEmissionParameters()), TS);
    }
    return true;
}


void
MSMeanData_Emissions::MSLaneMeanDataValues::write(OutputDevice& dev, long long int attributeMask, const SUMOTime period,
        const double /*numLanes*/, const double defaultTravelTime, const int /*numVehicles*/) const {
    const double normFactor = double(3600. / STEPS2TIME(period) / myLaneLength);
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO_ABS,          OutputDevice::realString(myEmissions.CO, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO2_ABS,         OutputDevice::realString(myEmissions.CO2, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_HC_ABS,          OutputDevice::realString(myEmissions.HC, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_PMX_ABS,         OutputDevice::realString(myEmissions.PMx, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_NOX_ABS,         OutputDevice::realString(myEmissions.NOx, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_FUEL_ABS,        OutputDevice::realString(myEmissions.fuel, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_ELECTRICITY_ABS, OutputDevice::realString(myEmissions.electricity, 6));
    if (attributeMask == 0) {
        dev << "\n           ";
    }
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO_NORMED,          OutputDevice::realString(normFactor * myEmissions.CO, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO2_NORMED,         OutputDevice::realString(normFactor * myEmissions.CO2, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_HC_NORMED,          OutputDevice::realString(normFactor * myEmissions.HC, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_PMX_NORMED,         OutputDevice::realString(normFactor * myEmissions.PMx, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_NOX_NORMED,         OutputDevice::realString(normFactor * myEmissions.NOx, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_FUEL_NORMED,        OutputDevice::realString(normFactor * myEmissions.fuel, 6));
    checkWriteAttribute(dev, attributeMask, SUMO_ATTR_ELECTRICITY_NORMED, OutputDevice::realString(normFactor * myEmissions.electricity, 6));

    if (sampleSeconds > myParent->getMinSamples()) {
        double vehFactor = myParent->getMaxTravelTime() / sampleSeconds;
        double traveltime = myParent->getMaxTravelTime();
        if (travelledDistance > 0.f) {
            vehFactor = MIN2(vehFactor, myLaneLength / travelledDistance);
            traveltime = MIN2(traveltime, myLaneLength * sampleSeconds / travelledDistance);
        }
        if (attributeMask == 0) {
            dev << "\n           ";
        }
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_TRAVELTIME,         OutputDevice::realString(traveltime));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO_PERVEH,          OutputDevice::realString(vehFactor * myEmissions.CO, 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO2_PERVEH,         OutputDevice::realString(vehFactor * myEmissions.CO2, 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_HC_PERVEH,          OutputDevice::realString(vehFactor * myEmissions.HC, 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_PMX_PERVEH,         OutputDevice::realString(vehFactor * myEmissions.PMx, 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_NOX_PERVEH,         OutputDevice::realString(vehFactor * myEmissions.NOx, 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_FUEL_PERVEH,        OutputDevice::realString(vehFactor * myEmissions.fuel, 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_ELECTRICITY_PERVEH, OutputDevice::realString(vehFactor * myEmissions.electricity, 6));
    } else if (defaultTravelTime >= 0.) {
        const MSVehicleType* t = MSNet::getInstance()->getVehicleControl().getVType();
        const double speed = MIN2(myLaneLength / defaultTravelTime, t->getMaxSpeed());

        if (attributeMask == 0) {
            dev << "\n           ";
        }
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_TRAVELTIME,         OutputDevice::realString(defaultTravelTime));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO_PERVEH,          OutputDevice::realString(PollutantsInterface::computeDefault(t->getEmissionClass(), PollutantsInterface::CO,   speed, t->getCarFollowModel().getMaxAccel(), 0, defaultTravelTime), 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_CO2_PERVEH,         OutputDevice::realString(PollutantsInterface::computeDefault(t->getEmissionClass(), PollutantsInterface::CO2,  speed, t->getCarFollowModel().getMaxAccel(), 0, defaultTravelTime), 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_HC_PERVEH,          OutputDevice::realString(PollutantsInterface::computeDefault(t->getEmissionClass(), PollutantsInterface::HC,   speed, t->getCarFollowModel().getMaxAccel(), 0, defaultTravelTime), 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_PMX_PERVEH,         OutputDevice::realString(PollutantsInterface::computeDefault(t->getEmissionClass(), PollutantsInterface::PM_X, speed, t->getCarFollowModel().getMaxAccel(), 0, defaultTravelTime), 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_NOX_PERVEH,         OutputDevice::realString(PollutantsInterface::computeDefault(t->getEmissionClass(), PollutantsInterface::NO_X, speed, t->getCarFollowModel().getMaxAccel(), 0, defaultTravelTime), 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_FUEL_PERVEH,        OutputDevice::realString(PollutantsInterface::computeDefault(t->getEmissionClass(), PollutantsInterface::FUEL, speed, t->getCarFollowModel().getMaxAccel(), 0, defaultTravelTime), 6));
        checkWriteAttribute(dev, attributeMask, SUMO_ATTR_ELECTRICITY_PERVEH, OutputDevice::realString(PollutantsInterface::computeDefault(t->getEmissionClass(), PollutantsInterface::ELEC, speed, t->getCarFollowModel().getMaxAccel(), 0, defaultTravelTime), 6));
    }
    dev.closeTag();
}



// ---------------------------------------------------------------------------
// MSMeanData_Emissions - methods
// ---------------------------------------------------------------------------
MSMeanData_Emissions::MSMeanData_Emissions(const std::string& id,
        const SUMOTime dumpBegin,
        const SUMOTime dumpEnd,
        const bool useLanes, const bool withEmpty,
        const bool printDefaults,
        const bool withInternal,
        const bool trackVehicles,
        const double maxTravelTime,
        const double minSamples,
        const std::string& vTypes,
        const std::string& writeAttributes) :
    MSMeanData(id, dumpBegin, dumpEnd, useLanes, withEmpty, printDefaults,
               withInternal, trackVehicles, 0, maxTravelTime, minSamples, vTypes, writeAttributes)
{ }


MSMeanData_Emissions::~MSMeanData_Emissions() {}


MSMeanData::MeanDataValues*
MSMeanData_Emissions::createValues(MSLane* const lane, const double length, const bool doAdd) const {
    return new MSLaneMeanDataValues(lane, length, doAdd, this);
}


/****************************************************************************/
