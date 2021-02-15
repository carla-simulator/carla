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
/// @file    MSDevice_ElecHybrid.h
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-11-25
///
// A device which stands as an implementation ElecHybrid and which outputs movereminder calls
/****************************************************************************/
#pragma once
#include <config.h>

#include <microsim/devices/MSVehicleDevice.h>
#include <microsim/MSVehicle.h>
#include <microsim/trigger/MSOverheadWire.h>
#include <utils/common/SUMOTime.h>

// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
* @class MSDevice_ElecHybrid
* @brief A device which collects info on the vehicle trip (mainly on departure and arrival)
*
* Each device collects departure time, lane and speed and the same for arrival.
*
* @see MSDevice
*/
class MSDevice_ElecHybrid : public MSVehicleDevice {
public:
    /** @brief Inserts MSDevice_ElecHybrid-options
    * @param[filled] oc The options container to add the options to
    */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
    *
    * The options are read and evaluated whether a ElecHybrid-device shall be built
    *  for the given vehicle.
    *
    * The built device is stored in the given vector.
    *
    * @param[in] v The vehicle for which a device may be built
    * @param[filled] into The vector to store the built device in
    */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);

    /// @brief Destructor.
    ~MSDevice_ElecHybrid();



    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Checks for waiting steps when the vehicle moves
    *
    * @param[in] veh Vehicle that asks this reminder.
    * @param[in] oldPos Position before move.
    * @param[in] newPos Position after move with newSpeed.
    * @param[in] newSpeed Moving speed.
    *
    * @return True (always).
    */
    bool notifyMove(SUMOTrafficObject& tObject, double oldPos, double newPos, double newSpeed);

    /** @brief Saves departure info on insertion
    *
    * @param[in] veh The entering vehicle.
    * @param[in] reason how the vehicle enters the lane
    * @return Always true
    * @see MSMoveReminder::notifyEnter
    * @see MSMoveReminder::Notification
    */
    bool notifyEnter(SUMOTrafficObject& tObject, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Saves arrival info
    *
    * @param[in] veh The leaving vehicle.
    * @param[in] lastPos Position on the lane when leaving.
    * @param[in] isArrival whether the vehicle arrived at its destination
    * @param[in] isLaneChange whether the vehicle changed from the lane
    * @return True if it did not leave the net.
    */
    bool notifyLeave(SUMOTrafficObject& tObject, double lastPos, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);

    /** @brief Internal notification about the vehicle moves
     *  @see MSMoveReminder::notifyMoveInternal()
     */
    virtual void notifyMoveInternal(
        const SUMOTrafficObject& tObject,
        const double frontOnLane,
        const double timeOnLane,
        const double meanSpeedFrontOnLane,
        const double meanSpeedVehicleOnLane,
        const double travelledDistanceFrontOnLane,
        const double travelledDistanceVehicleOnLane,
        const double meanLengthOnLane);
    /// @}

    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "elecHybrid";
    }

    /// @brief try to retrieve the given parameter from this device. Throw exception for unsupported key
    std::string getParameter(const std::string& key) const;

    double getParameterDouble(const std::string& key) const;

    /// @brief try to set the given parameter for this device. Throw exception for unsupported key
    void setParameter(const std::string& key, const std::string& value);

    /** @brief Called on writing tripinfo output
     *
     * @param[in] tripinfoOut The output device to write the information into
     * @exception IOError not yet implemented
     * @see MSDevice::tripInfoOutput
     */
    void generateOutput(OutputDevice* tripinfoOut) const;

    /// @brief Get the actual vehicle's Battery Capacity in kWh
    double getActualBatteryCapacity() const;

    /// @brief Get the total vehicle's Battery Capacity in kWh
    double getMaximumBatteryCapacity() const;

    /// @brief Get actual overhead wire segment ID
    std::string getOverheadWireSegmentID() const;

    /// @brief Get actual traction substationn ID
    std::string getTractionSubstationID() const;

    /// @brief Get charged energy
    double getEnergyCharged() const;

    void setEnergyCharged(double energyCharged);

    double getCircuitAlpha() const;

    double getPowerWanted() const;

    /// @brief Get actual current in the overhead wire segment
    double getCurrentFromOverheadWire() const;

    void setCurrentFromOverheadWire(double current);

    /// @brief Get actual voltage on the overhead wire segment
    double getVoltageOfOverheadWire() const;

    void setVoltageOfOverheadWire(double voltage);

    /// @brief Get consum
    double getConsum() const;

    double getDistance() const {
        return myDistance;
    }

    /// @brief Get consum
    bool isBatteryDischarged() const;

    /// @brief Set actual vehicle's Battery Capacity in kWh
    void setActualBatteryCapacity(const double actualBatteryCapacity);

    void setConsum(const double consumption);

    double acceleration(SUMOVehicle& veh, double power, double oldSpeed);
    double consumption(SUMOVehicle& veh, double a, double newSpeed);

    MSOverheadWire* getActOverheadWireSegment() {
        return myActOverheadWireSegment;
    };

    Element* getVehElem() {
        return veh_elem;
    }

private:
    /** @brief Constructor
    *
    * @param[in] holder The vehicle that holds this device
    * @param[in] id The ID of the device
    */
    MSDevice_ElecHybrid(SUMOVehicle& holder, const std::string& id,
                        const double actualBatteryCapacity, const double maximumBatteryCapacity, const double overheadWireChargingPower, const std::map<int, double>& param);

    void checkParam(const SumoXMLAttr paramKey, const double lower = 0., const double upper = std::numeric_limits<double>::infinity());

protected:
    /// @brief Parameter, The actual vehicles's Battery Capacity in Wh, [myActualBatteryCapacity <= myMaximumBatteryCapacity]
    double myActualBatteryCapacity;

    /// @brief Parameter, The total vehicles's Battery Capacity in Wh, [myMaximumBatteryCapacity >= 0]
    double myMaximumBatteryCapacity;

    /// @brief Parameter, overhead wire charging power to battery, if the battery SoC is not full (in Watt)
    double myOverheadWireChargingPower;

    /// @brief Parameter collection
    std::map<int, double> myParam;

    /// @brief Parameter, Vehicle's last angle
    double myLastAngle;

    /// @brief Parameter, Vehicle consumption during a time step (by default is 0.)
    double myConsum;

    /// @brief Parameter, Flag: Battery of Vehicle is fully discharged (by default is false)
    bool myBatteryDischargedLogic;

    /// @brief Parameter, Flag: Vehicle is charging (by default is false)
    bool myCharging;

    /// @brief Energy flowing into (+) or from (-) the battery pack in the given timestep
    double myEnergyCharged;

    /// @brief Parameter, Current wanted at overhead wire in next timestep
    double myCircuitCurrent;

    double myCircuitVoltage;

    /// @name Tripinfo statistics
    /// @{
    double myMaxBatteryPower;
    double myMinBatteryPower;
    double myTotalPowerConsumed;
    double myTotalPowerRegenerated;

    /// @brief Energy that could not be stored back to the battery or traction station
    /// and was wasted on resistors. This is approximate, we ignore the use of classical
    /// brakes in lower speeds.
    double myTotalPowerWasted;
    /// @}

    /// @name Power management parameters
    /// @{
    /// @brief Minimal SOC of the battery pack, below this value the battery is assumed discharged
    double mySOCMin;
    /// @brief Maximal SOC of the battery pack, battery will not be charged above this level.
    /// (But the buffer may still be used for regenerative braking).
    double mySOCMax;
    /// @}

    /// @brief Parameter, Pointer to the actual overhead wire segment in which vehicle is placed (by default is nullptr)
    MSOverheadWire* myActOverheadWireSegment;

    /// @brief Parameter, Pointer to the act overhead wire segment in previous step  (by default is nullptr), i.e. auxiliar pointer for disabling charging vehicle from previous (not current) overherad wire segment
    MSOverheadWire* myPreviousOverheadWireSegment;

    double myDistance;

    //circuit element of elecHybrid device
    //  ----|veh_pos_tail_elem|---------|pos_veh_node|--------
    //										  |
    //										  |
    //									  |veh_elem|
    //										  |
    //										  |
    //  ----------------------------------|ground|------------
    Element* veh_elem;
    Element* veh_pos_tail_elem;
    Node* pos_veh_node;

    void deleteVehicleFromCircuit(SUMOVehicle& veh);


private:
    /// @brief Invalidated copy constructor.
    MSDevice_ElecHybrid(const MSDevice_ElecHybrid&);

    /// @brief Invalidated assignment operator.
    MSDevice_ElecHybrid& operator=(const MSDevice_ElecHybrid&);


};
