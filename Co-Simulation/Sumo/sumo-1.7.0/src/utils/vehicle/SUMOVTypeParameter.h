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
/// @file    SUMOVTypeParameter.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    10.09.2009
///
// Structure representing possible vehicle parameter
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/common/Parameterised.h>
#include <utils/common/RGBColor.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/distribution/Distribution_Parameterized.h>

// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class OptionsCont;


// ===========================================================================
// value definitions
// ===========================================================================
const int VTYPEPARS_LENGTH_SET = 1;
const int VTYPEPARS_MINGAP_SET = 1 << 1;
const int VTYPEPARS_MAXSPEED_SET = 1 << 2;
const int VTYPEPARS_PROBABILITY_SET = 1 << 3;
const int VTYPEPARS_SPEEDFACTOR_SET = 1 << 4;
const int VTYPEPARS_EMISSIONCLASS_SET = 1 << 5;
const int VTYPEPARS_COLOR_SET = 1 << 6;
const int VTYPEPARS_VEHICLECLASS_SET = 1 << 7;
const int VTYPEPARS_WIDTH_SET = 1 << 8;
const int VTYPEPARS_HEIGHT_SET = 1 << 9;
const int VTYPEPARS_SHAPE_SET = 1 << 10;
const int VTYPEPARS_OSGFILE_SET = 1 << 11;
const int VTYPEPARS_IMGFILE_SET = 1 << 12;
const int VTYPEPARS_IMPATIENCE_SET = 1 << 13;
const int VTYPEPARS_LANE_CHANGE_MODEL_SET = 1 << 14;
const int VTYPEPARS_PERSON_CAPACITY = 1 << 15;
const int VTYPEPARS_BOARDING_DURATION = 1 << 16;
const int VTYPEPARS_CONTAINER_CAPACITY = 1 << 17;
const int VTYPEPARS_LOADING_DURATION = 1 << 18;
const int VTYPEPARS_CAR_FOLLOW_MODEL = 1 << 19;
const int VTYPEPARS_MAXSPEED_LAT_SET = 1 << 20;
const int VTYPEPARS_LATALIGNMENT_SET = 1 << 21;
const int VTYPEPARS_MINGAP_LAT_SET = 1 << 22;
const int VTYPEPARS_ACTIONSTEPLENGTH_SET = 1 << 23;

const int VTYPEPARS_CARRIAGE_LENGTH_SET = 1 << 25;
const int VTYPEPARS_LOCOMOTIVE_LENGTH_SET = 1 << 26;
const int VTYPEPARS_CARRIAGE_GAP_SET = 1 << 27;
const int VTYPEPARS_MANEUVER_ANGLE_TIMES_SET = 1 << 28;


const int VTYPEPARS_DEFAULT_EMERGENCYDECEL_DEFAULT = -1;
const int VTYPEPARS_DEFAULT_EMERGENCYDECEL_DECEL = -2;

// ===========================================================================
// struct definitions
// ===========================================================================
/**
 * @class SUMOVTypeParameter
 * @brief Structure representing possible vehicle parameter
 */
class SUMOVTypeParameter : public Parameterised {
public:
    /// @brief struct for default values that depend of VClass
    struct VClassDefaultValues {
        /// @brief parameter constructor
        VClassDefaultValues(SUMOVehicleClass vClass);

        /// @brief The physical vehicle length
        double length;

        /// @brief This class' free space in front of the vehicle itself
        double minGap;

        /// @brief The vehicle type's maximum speed [m/s]
        double maxSpeed;

        /// @brief This class' width
        double width;

        /// @brief This class' height
        double height;

        /// @brief This class' shape
        SUMOVehicleShape shape;

        /// @brief The emission class of this vehicle
        SUMOEmissionClass emissionClass;

        /// @brief The factor by which the maximum speed may deviate from the allowed max speed on the street
        Distribution_Parameterized speedFactor;

        /// @brief The person capacity of the vehicle
        int personCapacity;

        /// @brief The container capacity of the vehicle
        int containerCapacity;

        /// @brief 3D model file for this class
        std::string osgFile;

        /// @brief the length of train carriages
        double carriageLength;

        /// @brief the length of train locomotive
        double locomotiveLength;

    private:
        /// @brief default constructor
        VClassDefaultValues();
    };

    /** @brief Constructor
     *
     * Initialises the structure with default values
     */
    SUMOVTypeParameter(const std::string& vtid, const SUMOVehicleClass vc = SVC_IGNORING);

    /// @brief virtual destructor
    virtual ~SUMOVTypeParameter() {};

    /** @brief Returns whether the given parameter was set
     * @param[in] what The parameter which one asks for
     * @return Whether the given parameter was set
     */
    bool wasSet(int what) const {
        return (parametersSet & what) != 0;
    }

    /** @brief Writes the vtype
     *
     * @param[in, out] dev The device to write into
     * @exception IOError not yet implemented
     */
    void write(OutputDevice& dev) const;

    /** @brief Returns the named value from the map, or the default if it is not contained there
     * @param[in] attr The corresponding xml attribute
     * @param[in] defaultValue The value to return if the given map does not contain the named variable
     * @return The named value from the map or the default if it does not exist there
     */
    double getCFParam(const SumoXMLAttr attr, const double defaultValue) const;

    /** @brief Returns the named value from the map, or the default if it is not contained there
    * @param[in] attr The corresponding xml attribute
    * @param[in] defaultValue The value to return if the given map does not contain the named variable
    * @return The named value from the map or the default if it does not exist there
    */
    std::string getCFParamString(const SumoXMLAttr attr, const std::string defaultValue) const;

    /** @brief Returns the named value from the map, or the default if it is not contained there
     * @param[in] attr The corresponding xml attribute
     * @param[in] defaultValue The value to return if the given map does not contain the named variable
     * @return The named value from the map or the default if it does not exist there
     */
    double getLCParam(const SumoXMLAttr attr, const double defaultValue) const;

    /** @brief Returns the named value from the map, or the default if it is not contained there
     * @param[in] attr The corresponding xml attribute
     * @param[in] defaultValue The value to return if the given map does not contain the named variable
     * @return The named value from the map or the default if it does not exist there
     */
    std::string getLCParamString(const SumoXMLAttr attr, const std::string& defaultValue) const;

    /// @brief sub-model parameters
    typedef std::map<SumoXMLAttr, std::string> SubParams;

    /// @brief Returns the LC parameter
    const SubParams& getLCParams() const;

    /** @brief Returns the named value from the map, or the default if it is not contained there
     * @param[in] attr The corresponding xml attribute
     * @param[in] defaultValue The value to return if the given map does not contain the named variable
     * @return The named value from the map or the default if it does not exist there
     */
    double getJMParam(const SumoXMLAttr attr, const double defaultValue) const;

    /** @brief Returns the named value from the map, or the default if it is not contained there
     * @param[in] attr The corresponding xml attribute
     * @param[in] defaultValue The value to return if the given map does not contain the named variable
     * @return The named value from the map or the default if it does not exist there
     */
    std::string getJMParamString(const SumoXMLAttr attr, const std::string defaultValue) const;

    void cacheParamRestrictions(const std::vector<std::string>& restrictionKeys);

    /// @brief The vehicle type's id
    std::string id;

    /// @brief The physical vehicle length
    double length;

    /// @brief This class' free space in front of the vehicle itself
    double minGap;

    /// @brief The vehicle type's maximum speed [m/s]
    double maxSpeed;

    /// @brief The vehicle type's default actionStepLength [ms], i.e. the interval between two control actions.
    ///        The default value of 0ms. induces the value to be traced from MSGlobals::gActionStepLength
    SUMOTime actionStepLength;

    /// @brief The probability when being added to a distribution without an explicit probability
    double defaultProbability;

    /// @brief The factor by which the maximum speed may deviate from the allowed max speed on the street
    Distribution_Parameterized speedFactor;

    /// @brief The emission class of this vehicle
    SUMOEmissionClass emissionClass;

    /// @brief The color
    RGBColor color;

    /// @brief The vehicle's class
    SUMOVehicleClass vehicleClass;

    /// @brief The vehicle's impatience (willingness to obstruct others)
    double impatience;

    /// @brief The person capacity of the vehicle
    int personCapacity;

    /// @brief The container capacity of the vehicle
    int containerCapacity;

    /// @brief The time a person needs to board the vehicle
    SUMOTime boardingDuration;

    /// @brief The time a container needs to get loaded on the vehicle
    SUMOTime loadingDuration;

    /// @name Values for drawing this class' vehicles
    /// @{

    /// @brief This class' width
    double width;

    /// @brief This class' height
    double height;

    /// @brief This class' shape
    SUMOVehicleShape shape;

    /// @brief 3D model file for this class
    std::string osgFile;

    /// @brief Image file for this class
    std::string imgFile;
    /// @}


    /// @brief The enum-representation of the car-following model to use
    SumoXMLTag cfModel;

    /// @brief Car-following parameter
    SubParams cfParameter;

    /// @brief Lane-changing parameter
    SubParams lcParameter;

    /// @brief Junction-model parameter
    SubParams jmParameter;

    /// @brief The lane-change model to use
    LaneChangeModel lcModel;

    /// @brief The vehicle type's maximum lateral speed [m/s]
    double maxSpeedLat;

    /// @brief The vehicles desired lateral alignment
    LateralAlignment latAlignment;

    /// @brief The vehicle type's minimum lateral gap [m]
    double minGapLat;

    /// @brief the length of train carriages and locomotive
    double carriageLength;
    double locomotiveLength;
    double carriageGap;

    /// @brief Information for the router which parameter were set
    int parametersSet;

    /// @brief Information whether this type was already saved (needed by routers)
    mutable bool saved;

    /// @brief Information whether this is a type-stub, being only referenced but not defined (needed by routers)
    bool onlyReferenced;

    /// @brief cached value of parameters which may restrict access to certain edges
    std::vector<double> paramRestrictions;

    /// @brief satisfy vType / router template requirements
    inline double getLength() const {
        return length;
    }

    /** @brief Returns the default acceleration for the given vehicle class
     * This needs to be a function because the actual value is stored in the car following model
     * @param[in] vc the vehicle class
     * @return the acceleration in m/s^2
     */
    static double getDefaultAccel(const SUMOVehicleClass vc = SVC_IGNORING);

    /** @brief Returns the default deceleration for the given vehicle class
     * This needs to be a function because the actual value is stored in the car following model
     * @param[in] vc the vehicle class
     * @return the deceleration in m/s^2
     */
    static double getDefaultDecel(const SUMOVehicleClass vc = SVC_IGNORING);

    /** @brief Returns the default emergency deceleration for the given vehicle class
     * This needs to be a function because the actual value is stored in the car following model
     * @param[in] vc the vehicle class
     * @param[in] decel the deceleration of the vehicle type
     * @return the emergency deceleration in m/s^2
     */
    static double getDefaultEmergencyDecel(const SUMOVehicleClass vc, double decel, double defaultOption);

    /** @brief Returns the default driver's imperfection (sigma or epsilon in Krauss' model) for the given vehicle class
     * This needs to be a function because the actual value is stored in the car following model
     * @param[in] vc the vehicle class
     * @return the imperfection as a value between 0 and 1
     */
    static double getDefaultImperfection(const SUMOVehicleClass vc = SVC_IGNORING);

    /// @brief return the default parameters, this is a function due to the http://www.parashift.com/c++-faq/static-init-order.html
    static const SUMOVTypeParameter& getDefault();

    /// @brief Map of manoeuver angles versus the times (entry, exit) to execute the manoeuver
    std::map<int, std::pair<SUMOTime, SUMOTime>>  myManoeuverAngleTimes;

    /** @brief Initialise the default mapping between manoeuver angle and times dependant on vehicle class
     *  @param[in] vclass The vehicle class
     *  @note  These default values were 'informed' by a paper by Purnawan, and Yousif:
     *  @note    usir.salford.ac.uk/id/eprint/9729/3/Paper_Kassel_%28Seminar%29.pdf (no reverse park values in paper)
     *  @note    truck values were simply doubled - all are modifiable in the vehicle type definition and there is no limit to the no of triplets
     *    TODO:
     *        optionality for 90 degree bay entry (forwards or reverse) not implemented - probably should be a driver propensity
     *        the defaults assume reverse entry - a reverse manoeuvre has to happen and there will be a small difference in timings depending whether its reverse in or out
     */
    void setManoeuverAngleTimes(const SUMOVehicleClass vclass);

    /** @brief Returns the time that will be needed for the vehicle type to execute the (entry) manoeuvre (and be blocking the lane)
     * @param[in] angle The angle, in degrees through which the vehicle needs to manoeuver (0-180 degrees)
     * @return The SUMOTime value
     */
    SUMOTime getEntryManoeuvreTime(const int angle) const;

    /** @brief Returns the time that will be needed for the vehicle type to execute the (exit) manoeuvre (and be blocking the lane)
      * @param[in] angle The angle, in degrees through which the vehicle needs to manoeuver (0-180 degrees)
      * @return The SUMOTime value
      */
    SUMOTime getExitManoeuvreTime(const int angle) const;

    /** @brief Returns myManoeuverAngleTimes as a string for xml output
     *  @return A string of , separated triplets (angle entry-time exit-time)
     */
    std::string getManoeuverAngleTimesS() const;
};
