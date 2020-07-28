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
/// @file    SUMOVehicleParameter.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Axel Wegener
/// @author  Michael Behrisch
/// @date    2006-01-24
///
// Structure representing possible vehicle parameter
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/common/Parameterised.h>
#include <utils/common/RGBColor.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOVehicleClass.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class OptionsCont;


// ===========================================================================
// value definitions
// ===========================================================================
const int VEHPARS_COLOR_SET = 1;
const int VEHPARS_VTYPE_SET = 2;
const int VEHPARS_DEPARTLANE_SET = 2 << 1;
const int VEHPARS_DEPARTPOS_SET = 2 << 2;
const int VEHPARS_DEPARTSPEED_SET = 2 << 3;
const int VEHPARS_END_SET = 2 << 4;
const int VEHPARS_NUMBER_SET = 2 << 5;
const int VEHPARS_PERIOD_SET = 2 << 6;
const int VEHPARS_VPH_SET = 2 << 7;
const int VEHPARS_PROB_SET = 2 << 8;
const int VEHPARS_ROUTE_SET = 2 << 9;
const int VEHPARS_ARRIVALLANE_SET = 2 << 10;
const int VEHPARS_ARRIVALPOS_SET = 2 << 11;
const int VEHPARS_ARRIVALSPEED_SET = 2 << 12;
const int VEHPARS_LINE_SET = 2 << 13;
const int VEHPARS_FROM_TAZ_SET = 2 << 14;
const int VEHPARS_TO_TAZ_SET = 2 << 15;
const int VEHPARS_FORCE_REROUTE = 2 << 16;
const int VEHPARS_PERSON_CAPACITY_SET = 2 << 17;
const int VEHPARS_PERSON_NUMBER_SET = 2 << 18;
const int VEHPARS_CONTAINER_NUMBER_SET = 2 << 19;
const int VEHPARS_DEPARTPOSLAT_SET = 2 << 20;
const int VEHPARS_ARRIVALPOSLAT_SET = 2 << 21;
const int VEHPARS_VIA_SET = 2 << 22;
const int VEHPARS_SPEEDFACTOR_SET = 2 << 23;

const int STOP_INDEX_END = -1;
const int STOP_INDEX_FIT = -2;

const int STOP_START_SET = 1;
const int STOP_END_SET = 2;
const int STOP_DURATION_SET = 2 << 1;
const int STOP_UNTIL_SET = 2 << 2;
const int STOP_EXTENSION_SET = 2 << 3;
const int STOP_TRIGGER_SET = 2 << 4;
const int STOP_PARKING_SET = 2 << 5;
const int STOP_EXPECTED_SET = 2 << 6;
const int STOP_CONTAINER_TRIGGER_SET = 2 << 7;
const int STOP_EXPECTED_CONTAINERS_SET = 2 << 8;
const int STOP_TRIP_ID_SET = 2 << 9;
const int STOP_LINE_SET = 2 << 10;
const int STOP_SPEED_SET = 2 << 11;
const int STOP_SPLIT_SET = 2 << 12;
const int STOP_JOIN_SET = 2 << 13;
const int STOP_ARRIVAL_SET = 2 << 14;

const double MIN_STOP_LENGTH = 2 * POSITION_EPS;


// ===========================================================================
// enum definitions
// ===========================================================================
/**
 * @enum DepartDefinition
 * @brief Possible ways to depart
 */
enum DepartDefinition {
    /// @brief The time is given
    DEPART_GIVEN,
    /// @brief The departure is person triggered
    DEPART_TRIGGERED,
    /// @brief The departure is container triggered
    DEPART_CONTAINER_TRIGGERED,
    /// @brief The vehicle is discarded if emission fails (not fully implemented yet)
    DEPART_NOW,
    /// @brief The departure is triggered by a train split
    DEPART_SPLIT,
    /// @brief Tag for the last element in the enum for safe int casting
    DEPART_DEF_MAX
};


/**
 * @enum DepartLaneDefinition
 * @brief Possible ways to choose a lane on depart
 */
enum class DepartLaneDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The lane is given
    GIVEN,
    /// @brief The lane is chosen randomly
    RANDOM,
    /// @brief The least occupied lane is used
    FREE,
    /// @brief The least occupied lane from lanes which allow the continuation
    ALLOWED_FREE,
    /// @brief The least occupied lane from best lanes
    BEST_FREE,
    /// @brief The rightmost lane the vehicle may use
    FIRST_ALLOWED
};


/**
 * @enum DepartPosDefinition
 * @brief Possible ways to choose the departure position
 */
enum class DepartPosDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The position is given
    GIVEN,
    /// @brief The position is chosen randomly
    RANDOM,
    /// @brief A free position is chosen
    FREE,
    /// @brief Back-at-zero position
    BASE,
    /// @brief Insert behind the last vehicle as close as possible to still allow the specified departSpeed. Fallback to DepartPosDefinition::BASE if there is no vehicle on the departLane yet.
    LAST,
    /// @brief If a fixed number of random choices fails, a free position is chosen
    RANDOM_FREE,
    /// @brief depart position is endPos of first stop
    STOP
};


/**
 * @enum DepartPosDefinition
 * @brief Possible ways to choose the departure position
 */
enum class DepartPosLatDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The position is given
    GIVEN,
    /// @brief At the rightmost side of the lane
    RIGHT,
    /// @brief At the center of the lane
    CENTER,
    /// @brief At the leftmost side of the lane
    LEFT,
    /// @brief The lateral position is chosen randomly
    RANDOM,
    /// @brief A free lateral position is chosen
    FREE,
    /// @brief If a fixed number of random choices fails, a free lateral position is chosen
    RANDOM_FREE
};


/**
 * @enum DepartSpeedDefinition
 * @brief Possible ways to choose the departure speed
 */
enum class DepartSpeedDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The speed is given
    GIVEN,
    /// @brief The speed is chosen randomly
    RANDOM,
    /// @brief The maximum safe speed is used
    MAX,
    /// @brief The maximum lane speed is used (speedLimit * speedFactor)
    DESIRED,
    /// @brief The maximum lane speed is used (speedLimit)
    LIMIT
};


/**
 * @enum ArrivalLaneDefinition
 * @brief Possible ways to choose the arrival lane
 */
enum class ArrivalLaneDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The arrival lane is given
    GIVEN,
    /// @brief The current lane shall be used
    CURRENT
};


/**
 * @enum ArrivalPosDefinition
 * @brief Possible ways to choose the arrival position
 */
enum class ArrivalPosDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The arrival position is given
    GIVEN,
    /// @brief The arrival position is chosen randomly
    RANDOM,
    /// @brief Half the road length
    CENTER,
    /// @brief The maximum arrival position is used
    MAX
};


/**
 * @enum ArrivalPosLatDefinition
 * @brief Possible ways to choose the departure position
 */
enum class ArrivalPosLatDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The position is given
    GIVEN,
    /// @brief At the rightmost side of the lane
    RIGHT,
    /// @brief At the center of the lane
    CENTER,
    /// @brief At the leftmost side of the lane
    LEFT
};


/**
 * @enum ArrivalSpeedDefinition
 * @brief Possible ways to choose the arrival speed
 */
enum class ArrivalSpeedDefinition {
    /// @brief No information given; use default
    DEFAULT,
    /// @brief The speed is given
    GIVEN,
    /// @brief The current speed is used
    CURRENT
};


// ===========================================================================
// struct definitions
// ===========================================================================
/**
 * @class SUMOVehicleParameter
 * @brief Structure representing possible vehicle parameter
 *
 * When used within a vehicle, parameter are usually const except for selected items
 *  adaptable via TraCI which are flagged as mutable below
 * The fields yielding with "Procedure" describe whether the according value
 *  shall be used or another procedure is used to choose the value.
 * @see DepartLaneDefinition
 * @see DepartPosDefinition
 * @see DepartSpeedDefinition
 */
class SUMOVehicleParameter : public Parameterised {
public:
    /** @brief Constructor
     *
     * Initialises the structure with default values
     */
    SUMOVehicleParameter();

    /// @brief Destructor
    virtual ~SUMOVehicleParameter();

    /** @struct Stop
     * @brief Definition of vehicle stop (position and duration)
     */
    class Stop : public Parameterised {

    public:
        /// @brief constructor
        Stop();

        /** @brief Writes the stop as XML
         *
         * @param[in, out] dev The device to write into
         * @exception IOError not yet implemented
         */
        void write(OutputDevice& dev, bool close = true) const;

        /// @brief write trigger attribute
        void writeTriggers(OutputDevice& dev) const;

        /// @brief The edge to stop at (used only in NETEDIT)
        std::string edge;

        /// @brief The lane to stop at
        std::string lane;

        /// @brief (Optional) bus stop if one is assigned to the stop
        std::string busstop;

        /// @brief (Optional) container stop if one is assigned to the stop
        std::string containerstop;

        /// @brief (Optional) parking area if one is assigned to the stop
        std::string parkingarea;

        /// @brief (Optional) charging station if one is assigned to the stop
        std::string chargingStation;

        /// @brief (Optional) overhead line segment if one is assigned to the stop
        std::string overheadWireSegment;

        /// @brief The stopping position start
        double startPos;

        /// @brief The stopping position end
        double endPos;

        /// @brief The (expected) time at which the vehicle reaches the stop
        SUMOTime arrival;

        /// @brief The stopping duration
        SUMOTime duration;

        /// @brief The time at which the vehicle may continue its journey
        SUMOTime until;

        /// @brief The maximum time extension for boarding / loading
        SUMOTime extension;

        /// @brief whether an arriving person lets the vehicle continue
        bool triggered;

        /// @brief whether an arriving container lets the vehicle continue
        bool containerTriggered;

        /// @brief whether an joined vehicle lets this vehicle continue
        bool joinTriggered;

        /// @brief whether the vehicle is removed from the net while stopping
        bool parking;

        /// @brief IDs of persons the vehicle has to wait for until departing
        std::set<std::string> awaitedPersons;

        /// @brief IDs of containers the vehicle has to wait for until departing
        std::set<std::string> awaitedContainers;

        /// @brief enable or disable friendly position (used by NETEDIT)
        bool friendlyPos;

        /// @brief act Type (only used by Persons) (used by NETEDIT)
        std::string actType;

        /// @brief id of the trip within a cyclical public transport route
        std::string tripId;

        /// @brief the new line id of the trip within a cyclical public transport route
        std::string line;

        /// @brief the id of the vehicle (train portion) that splits of upon reaching this stop
        std::string split;

        /// @brief the id of the vehicle (train portion) to which this vehicle shall be joined
        std::string join;

        /// @brief the speed at which this stop counts as reached (waypoint mode)
        double speed;

        /// @brief lanes and positions connected to this stop (only used by duarouter where Stop is used to store stopping places)
        std::vector<std::tuple<std::string, double, double> > accessPos;

        /// @brief at which position in the stops list
        int index;

        /// @brief Information for the output which parameter were set
        int parametersSet = 0;

    };


    /** @brief Returns whether the given parameter was set
     * @param[in] what The parameter which one asks for
     * @return Whether the given parameter was set
     */
    bool wasSet(int what) const {
        return (parametersSet & what) != 0;
    }

    /** @brief Writes the parameters as a beginning element
     *
     * @param[in, out] dev The device to write into
     * @param[in] oc The options to get defaults from
     * @param[in] tag The "root" tag to write (defaults to vehicle)
     * @param[in] tag The typeID to write (defaults to member vtypeid)
     * @exception IOError not yet implemented
     */
    void write(OutputDevice& dev, const OptionsCont& oc, const SumoXMLTag tag = SUMO_TAG_VEHICLE, const std::string& typeID = "") const;

    /** @brief Returns whether the defaults shall be used
     * @param[in] oc The options to get the options from
     * @param[in] optionName The name of the option to determine whether its value shall be used
     * @return Whether the option is set and --defaults-override was set
     */
    bool defaultOptionOverrides(const OptionsCont& oc, const std::string& optionName) const;

    /// @name Depart/arrival-attributes verification
    /// @{
    /** @brief Validates a given depart value
     * @param[in] val The depart value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] depart The parsed depart time, if given
     * @param[out] dd The parsed departProcedure definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid depart definition
     */
    static bool parseDepart(const std::string& val, const std::string& element, const std::string& id,
                            SUMOTime& depart, DepartDefinition& dd, std::string& error);

    /** @brief Validates a given departLane value
     * @param[in] val The departLane value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] lane The parsed lane, if given
     * @param[out] dld The parsed departLane definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid departLane definition
     */
    static bool parseDepartLane(const std::string& val, const std::string& element, const std::string& id,
                                int& lane, DepartLaneDefinition& dld, std::string& error);

    /** @brief Validates a given departPos value
     * @param[in] val The departPos value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] pos The parsed position, if given
     * @param[out] dpd The parsed departPos definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid departPos definition
     */
    static bool parseDepartPos(const std::string& val, const std::string& element, const std::string& id,
                               double& pos, DepartPosDefinition& dpd, std::string& error);

    /** @brief Validates a given departPosLat value
     * @param[in] val The departPosLat value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] pos The parsed position, if given
     * @param[out] dpd The parsed departPos definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid departPos definition
     */
    static bool parseDepartPosLat(const std::string& val, const std::string& element, const std::string& id,
                                  double& pos, DepartPosLatDefinition& dpd, std::string& error);

    /** @brief Validates a given departSpeed value
     * @param[in] val The departSpeed value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] speed The parsed speed, if given
     * @param[out] dsd The parsed departSpeed definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid departSpeed definition
     */
    static bool parseDepartSpeed(const std::string& val, const std::string& element, const std::string& id,
                                 double& speed, DepartSpeedDefinition& dsd, std::string& error);

    /** @brief Validates a given arrivalLane value
     * @param[in] val The arrivalLane value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] lane The parsed lane, if given
     * @param[out] ald The parsed arrivalLane definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid arrivalLane definition
     */
    static bool parseArrivalLane(const std::string& val, const std::string& element, const std::string& id,
                                 int& lane, ArrivalLaneDefinition& ald, std::string& error);

    /** @brief Validates a given arrivalPos value
     * @param[in] val The arrivalPos value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] pos The parsed position, if given
     * @param[out] apd The parsed arrivalPos definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid arrivalPos definition
     */
    static bool parseArrivalPos(const std::string& val, const std::string& element, const std::string& id,
                                double& pos, ArrivalPosDefinition& apd, std::string& error);

    /** @brief Validates a given arrivalPosLat value
     * @param[in] val The arrivalPosLat value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] pos The parsed position, if given
     * @param[out] apd The parsed arrivalPos definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid arrivalPos definition
     */
    static bool parseArrivalPosLat(const std::string& val, const std::string& element, const std::string& id,
                                   double& pos, ArrivalPosLatDefinition& apd, std::string& error);


    /** @brief Validates a given arrivalSpeed value
     * @param[in] val The arrivalSpeed value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] speed The parsed speed, if given
     * @param[out] asd The parsed arrivalSpeed definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid arrivalSpeed definition
     */
    static bool parseArrivalSpeed(const std::string& val, const std::string& element, const std::string& id,
                                  double& speed, ArrivalSpeedDefinition& asd, std::string& error);
    /// @}

    /** @brief Interprets negative edge positions and fits them onto a given edge
     * @param[in] pos The position to be interpreted
     * @param[in] maximumValue The maximum allowed value (edge length)
     * @param[in] attr The attribute from which the value originated
     * @param[in] id The id of the object to which this attribute belongs
     * @return Whether the interpreted position
     */
    static double interpretEdgePos(double pos, double maximumValue, SumoXMLAttr attr, const std::string& id);

    /** @brief Validates a given person modes value
     * @param[in] modes The modes value to parse
     * @param[in] element The name of the type of the parsed element, for building the error message
     * @param[in] id The id of the parsed element, for building the error message
     * @param[out] modeSet The parsed modes definition
     * @param[out] error Error message, if an error occures
     * @return Whether the given value is a valid arrivalSpeed definition
     */
    static bool parsePersonModes(const std::string& modes, const std::string& element, const std::string& id, SVCPermissions& modeSet, std::string& error);

    /// @brief parses stop trigger values
    static void parseStopTriggers(const std::vector<std::string>& triggers, bool expectTrigger, Stop& stop);

    /// @brief The vehicle tag
    SumoXMLTag tag;

    /// @brief The vehicle's id
    std::string id;

    /// @brief The vehicle's route id
    std::string routeid;

    /// @brief The vehicle's type id
    std::string vtypeid;

    /// @brief The vehicle's color, TraCI may change this
    mutable RGBColor color;

    /// @name Departure definition
    /// @{
    /// @brief The vehicle's departure time
    SUMOTime depart;

    /// @brief Information how the vehicle shall choose the depart time
    DepartDefinition departProcedure;

    /// @brief (optional) The lane the vehicle shall depart from (index in edge)
    int departLane;

    /// @brief Information how the vehicle shall choose the lane to depart from
    DepartLaneDefinition departLaneProcedure;

    /// @brief (optional) The position the vehicle shall depart from
    double departPos;

    /// @brief Information how the vehicle shall choose the departure position
    DepartPosDefinition departPosProcedure;

    /// @brief (optional) The lateral position the vehicle shall depart from
    double departPosLat;

    /// @brief Information how the vehicle shall choose the lateral departure position
    DepartPosLatDefinition departPosLatProcedure;

    /// @brief (optional) The initial speed of the vehicle
    double departSpeed;

    /// @brief Information how the vehicle's initial speed shall be chosen
    DepartSpeedDefinition departSpeedProcedure;

    /// @}

    /// @name Arrival definition
    /// @{
    /// @brief (optional) The lane the vehicle shall arrive on (not used yet)
    int arrivalLane;

    /// @brief Information how the vehicle shall choose the lane to arrive on
    ArrivalLaneDefinition arrivalLaneProcedure;

    /// @brief (optional) The position the vehicle shall arrive on
    double arrivalPos;

    /// @brief Information how the vehicle shall choose the arrival position
    ArrivalPosDefinition arrivalPosProcedure;

    /// @brief (optional) The lateral position the vehicle shall arrive on
    double arrivalPosLat;

    /// @brief Information how the vehicle shall choose the lateral arrival position
    ArrivalPosLatDefinition arrivalPosLatProcedure;

    /// @brief (optional) The final speed of the vehicle (not used yet)
    double arrivalSpeed;

    /// @brief Information how the vehicle's end speed shall be chosen
    ArrivalSpeedDefinition arrivalSpeedProcedure;

    /// @}

    /// @name Repetition definition
    /// @{
    /// @brief The number of times the vehicle shall be repeatedly inserted
    int repetitionNumber;

    /// @brief The number of times the vehicle was already inserted
    int repetitionsDone;

    /// @brief The time offset between vehicle reinsertions
    SUMOTime repetitionOffset;

    /// @brief The probability for emitting a vehicle per second
    double repetitionProbability;

    /// @brief The time at which the flow ends (only needed when using repetitionProbability)
    SUMOTime repetitionEnd;

    /// @}

    /// @brief The vehicle's line (mainly for public transport)
    mutable std::string line;

    /// @brief The vehicle's origin zone (district)
    std::string fromTaz;

    /// @brief The vehicle's destination zone (district)
    std::string toTaz;

    /// @brief List of the stops the vehicle will make, TraCI may add entries here
    mutable std::vector<Stop> stops;

    /// @brief List of the via-edges the vehicle must visit
    mutable std::vector<std::string> via;

    /// @brief The static number of persons in the vehicle when it departs (not including boarding persons)
    int personNumber;

    /// @brief The static number of containers in the vehicle when it departs
    int containerNumber;

    /// @brief individual speedFactor (overriding distribution from vType)
    double speedFactor;

    /// @brief Information for the router which parameter were set, TraCI may modify this (when changing color)
    mutable int parametersSet;

protected:
    /// @brief obtain depart parameter in string format
    std::string getDepart() const;

    /// @brief obtain depart lane parameter in string format
    std::string getDepartLane() const;

    /// @brief obtain depart pos parameter in string format
    std::string getDepartPos() const;

    /// @brief obtain depart pos lat parameter in string format
    std::string getDepartPosLat() const;

    /// @brief obtain depart speed parameter in string format
    std::string getDepartSpeed() const;

    /// @brief obtain arrival lane parameter in string format
    std::string getArrivalLane() const;

    /// @brief obtain arrival pos parameter in string format
    std::string getArrivalPos() const;

    /// @brief obtain arrival pos lat parameter in string format
    std::string getArrivalPosLat() const;

    /// @brief obtain arrival speed parameter in string format
    std::string getArrivalSpeed() const;
};
