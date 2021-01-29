//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include <string>
#include <fstream>
#include <list>
#include <stdexcept>

#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/FindModule.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/VehicleSignal.h"
#include "veins/base/utils/Heading.h"

namespace veins {

/**
 * @brief
 * Used in modules created by the TraCIScenarioManager.
 *
 * This module relies on the TraCIScenarioManager for state updates
 * and can not be used on its own.
 *
 * TraCI server implementations do not differentiate between the orientation and direction of a vehicle.
 * Thus, TraCIMobility::updatePosition sets the BaseMobility's orientation and direction to the same value.
 * Said value is equivalent to the heading of the vehicle.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer, David Eckhoff, Luca Bedogni, Bastian Halmos, Stefan Joerer
 *
 * @see TraCIScenarioManager
 * @see TraCIScenarioManagerLaunchd
 *
 * @ingroup mobility
 */
class VEINS_API TraCIMobility : public BaseMobility {
public:
    class VEINS_API Statistics {
    public:
        double firstRoadNumber; /**< for statistics: number of first road we encountered (if road id can be expressed as a number) */
        simtime_t startTime; /**< for statistics: start time */
        simtime_t totalTime; /**< for statistics: total time travelled */
        simtime_t stopTime; /**< for statistics: stop time */
        double minSpeed; /**< for statistics: minimum value of currentSpeed */
        double maxSpeed; /**< for statistics: maximum value of currentSpeed */
        double totalDistance; /**< for statistics: total distance travelled */
        double totalCO2Emission; /**< for statistics: total CO2 emission */

        void initialize();
        void watch(cSimpleModule& module);
        void recordScalars(cSimpleModule& module);
    };

    const static simsignal_t parkingStateChangedSignal;

    TraCIMobility()
        : BaseMobility()
        , isPreInitialized(false)
        , manager(nullptr)
        , commandInterface(nullptr)
        , vehicleCommandInterface(nullptr)
    {
    }
    ~TraCIMobility() override
    {
        delete vehicleCommandInterface;
    }
    void initialize(int) override;
    void finish() override;

    void handleSelfMsg(cMessage* msg) override;
    virtual void preInitialize(std::string external_id, const Coord& position, std::string road_id = "", double speed = -1, Heading heading = Heading::nan);
    virtual void nextPosition(const Coord& position, std::string road_id = "", double speed = -1, Heading heading = Heading::nan, VehicleSignalSet signals = {VehicleSignal::undefined});
    virtual void changePosition();
    virtual void changeParkingState(bool);
    virtual void setExternalId(std::string external_id)
    {
        this->external_id = external_id;
    }
    virtual std::string getExternalId() const
    {
        if (external_id == "") throw cRuntimeError("TraCIMobility::getExternalId called with no external_id set yet");
        return external_id;
    }
    virtual double getHostPositionOffset() const
    {
        return hostPositionOffset;
    }
    virtual bool getParkingState() const
    {
        return isParking;
    }
    virtual std::string getRoadId() const
    {
        if (road_id == "") throw cRuntimeError("TraCIMobility::getRoadId called with no road_id set yet");
        return road_id;
    }
    virtual double getSpeed() const
    {
        if (speed == -1) throw cRuntimeError("TraCIMobility::getSpeed called with no speed set yet");
        return speed;
    }
    virtual VehicleSignalSet getSignals() const
    {
        if (signals.test(VehicleSignal::undefined)) throw cRuntimeError("TraCIMobility::getSignals called with no signals set yet");
        return signals;
    }
    /**
     * returns heading.
     */
    virtual Heading getHeading() const
    {
        if (heading.isNan()) throw cRuntimeError("TraCIMobility::getHeading called with no heading set yet");
        return heading;
    }
    virtual TraCIScenarioManager* getManager() const
    {
        if (!manager) manager = TraCIScenarioManagerAccess().get();
        return manager;
    }
    virtual TraCICommandInterface* getCommandInterface() const
    {
        if (!commandInterface) commandInterface = getManager()->getCommandInterface();
        return commandInterface;
    }
    virtual TraCICommandInterface::Vehicle* getVehicleCommandInterface() const
    {
        if (!vehicleCommandInterface) vehicleCommandInterface = new TraCICommandInterface::Vehicle(getCommandInterface()->vehicle(getExternalId()));
        return vehicleCommandInterface;
    }

    /**
     * Returns the speed of the host (likely 0 if setHostSpeed==false)
     */
    Coord getHostSpeed() const
    {
        return BaseMobility::getCurrentSpeed();
    }

protected:
    int accidentCount; /**< number of accidents */

    cOutVector currentPosXVec; /**< vector plotting posx */
    cOutVector currentPosYVec; /**< vector plotting posy */
    cOutVector currentSpeedVec; /**< vector plotting speed */
    cOutVector currentAccelerationVec; /**< vector plotting acceleration */
    cOutVector currentCO2EmissionVec; /**< vector plotting current CO2 emission */

    Statistics statistics; /**< everything statistics-related */

    bool isPreInitialized; /**< true if preInitialize() has been called immediately before initialize() */

    std::string external_id; /**< updated by setExternalId() */
    double hostPositionOffset; /**< front offset for the antenna on this car */
    bool setHostSpeed; /**< whether to update the speed of the host (along with its position)  */

    simtime_t lastUpdate; /**< updated by nextPosition() */
    Coord roadPosition; /**< position of front bumper, updated by nextPosition() */
    std::string road_id; /**< updated by nextPosition() */
    double speed; /**< updated by nextPosition() */
    Heading heading; /**< updated by nextPosition() */
    VehicleSignalSet signals; /**<updated by nextPosition() */

    cMessage* startAccidentMsg;
    cMessage* stopAccidentMsg;
    mutable TraCIScenarioManager* manager;
    mutable TraCICommandInterface* commandInterface;
    mutable TraCICommandInterface::Vehicle* vehicleCommandInterface;
    double last_speed;

    bool isParking;

    void fixIfHostGetsOutside() override; /**< called after each read to check for (and handle) invalid positions */

    /**
     * Returns the amount of CO2 emissions in grams/second, calculated for an average Car
     * @param v speed in m/s
     * @param a acceleration in m/s^2
     * @returns emission in g/s
     */
    double calculateCO2emission(double v, double a) const;

    /**
     * Calculates where the OMNeT++ module position of this car should be, given its front bumper position
     */
    Coord calculateHostPosition(const Coord& vehiclePos) const;

    /**
     * Calling this method on pointers of type TraCIMobility is deprecated in favor of calling either getHostSpeed or getSpeed.
     */
    Coord getCurrentSpeed() const override
    {
        return BaseMobility::getCurrentSpeed();
    }
};

class VEINS_API TraCIMobilityAccess {
public:
    TraCIMobility* get(cModule* host)
    {
        TraCIMobility* traci = FindModule<TraCIMobility*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};

} // namespace veins
