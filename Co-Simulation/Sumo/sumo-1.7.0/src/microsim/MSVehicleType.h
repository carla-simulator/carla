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
/// @file    MSVehicleType.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 12 Mar 2001
///
// The car-following model and parameter
/****************************************************************************/
#pragma once
#include <config.h>

#include <cassert>
#include <map>
#include <string>
#include <microsim/cfmodels/MSCFModel.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/StdDefs.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/RandHelper.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <utils/common/RGBColor.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class MSCFModel;
class SUMOVTypeParameter;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVehicleType
 * @brief The car-following model and parameter
 *
 * MSVehicleType stores the parameter of a single vehicle type and methods
 *  that use these for computing the vehicle's car-following behavior
 *
 * It is assumed that within the simulation many vehicles are using the same
 *  vehicle type, quite common is using only one vehicle type for all vehicles.
 *
 * You can think of it like of having a vehicle type for each VW Golf or
 *  Ford Mustang in your simulation while the car instances just refer to it.
 */
class MSVehicleType {
public:
    /** @brief Constructor.
     *
     * @param[in] parameter The vehicle type's parameter
     */
    MSVehicleType(const SUMOVTypeParameter& parameter);


    /// @brief Destructor
    virtual ~MSVehicleType();


    /** @brief Returns whether the given parameter was set
     * @param[in] what The parameter which one asks for
     * @return Whether the given parameter was set
     */
    bool wasSet(int what) const {
        return (myParameter.parametersSet & what) != 0;
    }


    /// @name Atomar getter for simulation
    /// @{

    /** @brief Returns the name of the vehicle type
     * @return This type's id
     */
    const std::string& getID() const {
        return myParameter.id;
    }


    /** @brief Returns the running index of the vehicle type
     * @return This type's numerical id
     */
    int getNumericalID() const {
        return myIndex;
    }


    /** @brief Get vehicle's length [m]
     * @return The length vehicles of this type have in m
     */
    double getLength() const {
        return myParameter.length;
    }


    /** @brief Get vehicle's length including the minimum gap [m]
     * @return The length vehicles of this type have (including the minimum gap in m
     */
    double getLengthWithGap() const {
        return myParameter.length + myParameter.minGap;
    }


    /** @brief Get the free space in front of vehicles of this class
     * @return The place before the vehicle
     */
    double getMinGap() const {
        return myParameter.minGap;
    }

    /** @brief Get the minimum lateral gap that vehicles of this type maintain
     * @return The place before the vehicle
     */
    double getMinGapLat() const {
        return myParameter.minGapLat;
    }


    /** @brief Returns the vehicle type's car following model definition (const version)
     * @return The vehicle type's car following model definition
     */
    inline const MSCFModel& getCarFollowModel() const {
        return *myCarFollowModel;
    }


    /** @brief Returns the vehicle type's car following model definition (non-const version)
     * @return The vehicle type's car following model definition
     */
    inline MSCFModel& getCarFollowModel() {
        return *myCarFollowModel;
    }


    inline LaneChangeModel getLaneChangeModel() const {
        return myParameter.lcModel;
    }


    /** @brief Get vehicle's maximum speed [m/s].
     * @return The maximum speed (in m/s) of vehicles of this class
     */
    double getMaxSpeed() const {
        return myParameter.maxSpeed;
    }


    /** @brief Computes and returns the speed deviation
     * @return A new, random speed deviation
     */
    double computeChosenSpeedDeviation(std::mt19937* rng, const double minDev = -1.) const;


    /** @brief Get the default probability of this vehicle type
     * @return The probability to use this type
     */
    double getDefaultProbability() const {
        return myParameter.defaultProbability;
    }


    /** @brief Get this vehicle type's vehicle class
     * @return The class of this vehicle type
     * @see SUMOVehicleClass
     */
    SUMOVehicleClass getVehicleClass() const {
        return myParameter.vehicleClass;
    }


    /** @brief Get this vehicle type's emission class
     * @return The emission class of this vehicle type
     * @see SUMOEmissionClass
     */
    SUMOEmissionClass getEmissionClass() const {
        return myParameter.emissionClass;
    }


    /** @brief Returns this type's color
     * @return The color of this type
     */
    const RGBColor& getColor() const {
        return myParameter.color;
    }


    /** @brief Returns this type's speed factor
     * @return The speed factor of this type
     */
    const Distribution_Parameterized& getSpeedFactor() const {
        return myParameter.speedFactor;
    }


    /** @brief Returns this type's default action step length
     * @return The default action step length of this type (in ms.)
     */
    SUMOTime getActionStepLength() const {
        return myParameter.actionStepLength;
    }


    /** @brief Returns this type's default action step length in seconds
     * @return The default action step length of this type (in s.)
     */
    double getActionStepLengthSecs() const {
        return myCachedActionStepLengthSecs;
    }


    /** @brief Returns this type's impatience
     * @return The impatience of this type
     */
    double getImpatience() const {
        return myParameter.impatience;
    }
    /// @}



    /// @name Atomar getter for visualization
    /// @{

    /** @brief Get the width which vehicles of this class shall have when being drawn
     * @return The width of this type's vehicles
     */
    double getWidth() const {
        return myParameter.width;
    }

    /** @brief Get the height which vehicles of this class shall have when being drawn
     * @return The height of this type's vehicles
     */
    double getHeight() const {
        return myParameter.height;
    }

    /** @brief Get this vehicle type's shape
     * @return The shape of this vehicle type
     * @see SUMOVehicleShape
     */
    SUMOVehicleShape getGuiShape() const {
        return myParameter.shape;
    }

    /** @brief Get this vehicle type's 3D model file name
     * @return The model file name of this vehicle type
     */
    std::string getOSGFile() const {
        return myParameter.osgFile;
    }


    /** @brief Get this vehicle type's raster model file name
     * @return The raster file name of this vehicle type
     */
    std::string getImgFile() const {
        return myParameter.imgFile;
    }


    /** @brief Get this vehicle type's person capacity
     * @return The person capacity of this vehicle type
     */
    int getPersonCapacity() const {
        return myParameter.personCapacity;
    }


    /** @brief Get this vehicle type's container capacity
     * @return The container capacity of this vehicle type
     */
    int getContainerCapacity() const {
        return myParameter.containerCapacity;
    }

    /** @brief Get this vehicle type's boarding duration
     * @return The time a person needs to board a vehicle of this type
     */
    SUMOTime getBoardingDuration() const {
        return myParameter.boardingDuration;
    }

    /** @brief Get this vehicle type's loading duration
     * @return The time a container needs to get laoded on a vehicle of this type
     */
    SUMOTime getLoadingDuration() const {
        return myParameter.loadingDuration;
    }

    /** @brief Get vehicle's maximum lateral speed [m/s].
     * @return The maximum lateral speed (in m/s) of vehicles of this class
     */
    double getMaxSpeedLat() const {
        return myParameter.maxSpeedLat;
    }

    /** @brief Get vehicle's preferred lateral alignment
     * @return The vehicle's preferred lateral alignment
     */
    LateralAlignment getPreferredLateralAlignment() const {
        return myParameter.latAlignment;
    }
    /// @}


    /// @name Setter methods
    /// @{

    /** @brief Set a new value for this type's acceleration.
     * @param[in] accel The new acceleration of this type
     */
    void setAccel(double accel);

    /** @brief Set a new value for this type's deceleration.
     * @param[in] decel The new deceleration of this type
     */
    void setDecel(double decel);

    /** @brief Set a new value for this type's emergency deceleration.
     * @param[in] emergencyDecel The new emergency deceleration of this type
     */
    void setEmergencyDecel(double emergencyDecel);

    /** @brief Set a new value for this type's apparent deceleration.
     * @param[in] apparentDecel The new apparent deceleration of this type
     */
    void setApparentDecel(double apparentDecel);

    /** @brief Set a new value for this type's imperfection.
     * @param[in] imperfection The new imperfection of this type
     */
    void setImperfection(double imperfection);

    /** @brief Set a new value for this type's headway.
     * @param[in] tau The new headway of this type
     */
    void setTau(double tau);

    /** @brief Set a new value for this type's length
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] length The new length of this type
     */
    void setLength(const double& length);


    /** @brief Set a new value for this type's height
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] height The new height of this type
     */
    void setHeight(const double& height);


    /** @brief Set a new value for this type's minimum gap
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] minGap The new minimum gap of this type
     */
    void setMinGap(const double& minGap);


    /** @brief Set a new value for this type's minimum lataral gap
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] minGapLat The new minimum lateral gap of this type
     */
    void setMinGapLat(const double& minGapLat);

    /** @brief Set a new value for this type's maximum speed
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] maxSpeed The new maximum speed of this type
     */
    void setMaxSpeed(const double& maxSpeed);

    /** @brief Set a new value for this type's maximum lateral speed
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] maxSpeedLat The new maximum lateral speed of this type
     */
    void setMaxSpeedLat(const double& maxSpeedLat);

    /** @brief Set a new value for this type's vehicle class
     * @param[in] vclass The new vehicle class of this type
     */
    void setVClass(SUMOVehicleClass vclass);


    /** @brief Set a new value for this type's default probability
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] prob The new default probability of this type
     */
    void setDefaultProbability(const double& prob);


    /** @brief Set a new value for this type's speed factor
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] factor The new speed factor of this type
     */
    void setSpeedFactor(const double& factor);


    /** @brief Set a new value for this type's speed deviation
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] dev The new speed deviation of this type
     */
    void setSpeedDeviation(const double& dev);


    /** @brief Set a new value for this type's action step length
     *
     * @param[in] actionStepLength The new action step length of this type (in ms.)
     * @param[in] resetActionOffset If True (default), the next action point is
     *            scheduled immediately. if If resetActionOffset == False, the interval
     *            between the last and the next action point is updated to match the given
     *            value for all vehicles of this type, or if the latter is smaller than the
     *            time since the last action point, the next action follows immediately.
     *
     * @note: Singular vtypes do not update the state of the corresponding vehicle, because
     *        the global lookup would be too expensive. The caller is responsible to
     *        perform the actionOffsetReset operation at caller context, where the vehicle is known.
     */
    void setActionStepLength(const SUMOTime actionStepLength, bool resetActionOffset);


    /** @brief Set a new value for this type's emission class
     * @param[in] eclass The new emission class of this type
     */
    void setEmissionClass(SUMOEmissionClass eclass);


    /** @brief Set a new value for this type's color
     * @param[in] color The new color of this type
     */
    void setColor(const RGBColor& color);


    /** @brief Set a new value for this type's width
     *
     * If the given value<0 then the one from the original type will
     *  be used.
     *
     * @param[in] width The new width of this type
     */
    void setWidth(const double& width);


    /** @brief Set a new value for this type's shape
     * @param[in] shape The new shape of this type
     */
    void setShape(SUMOVehicleShape shape);

    /** @brief Set a new value for this type's impatience
     * @param[in] impatience The new impatience of this type
     */
    void setImpatience(const double impatience);

    /** @brief Set vehicle's preferred lateral alignment
     */
    void setPreferredLateralAlignment(LateralAlignment latAlignment);
    /// @}



    /// @name methods for building vehicle types
    /// @{

    /** @brief Builds the microsim vehicle type described by the given parameter
     * @param[in] from The vehicle type description
     * @return The built vehicle type
     * @exception ProcessError on false values (not et used)
     */
    static MSVehicleType* build(SUMOVTypeParameter& from);

    /// @brief   Accessor function for parameter equivalent returning entry time for a specific manoeuver angle
    SUMOTime getEntryManoeuvreTime(const int angle) const;
    /// @brief   Accessor function for parameter equivalent returning exit time for a specific manoeuver angle
    SUMOTime getExitManoeuvreTime(const int angle) const;


    /** @brief Duplicates the microsim vehicle type giving the newly created type the given id,
     *         marking it as vehicle specific
     * @param[in] id The new id of the type
     * @return The built vehicle type
     * @note This method is used in case that a vType is meant to be used only for a specific vehicle
     *       The created vType will be removed with the vehicle or if it is assigned a new type.
     */
    MSVehicleType* buildSingularType(const std::string& id) const;


    /** @brief Duplicates the microsim vehicle type giving the newly created type the given id.
     *
     * @param[in] id The new id of the type
     * @param[in] persistent If true the created vType will be persistent and can be used by several vehicles,
     *            otherwise it may be removed before simulation end, @see buildSingularType()
     * @return The built vehicle type
     */
    MSVehicleType* duplicateType(const std::string& id, bool persistent) const;
    /// @}


    /** @brief Returns whether this type belongs to a single vehicle only (was modified)
     * @return Whether this vehicle type is based on a different one, and belongs to one vehicle only
     */
    bool isVehicleSpecific() const {
        return myOriginalType != nullptr;
    }


    /** @brief Returns the id of the original vehicle type if this is a vehicle specific type, the id otherwise
     * @return the original vehicle type id
     */
    const std::string& getOriginalID() const {
        return myOriginalType != nullptr ? myOriginalType->getID() : getID();
    }


    const SUMOVTypeParameter& getParameter() const {
        return myParameter;
    }

    /** @brief Checks whether vehicle type parameters may be problematic
     *         (Currently, only the value for the action step length is
     *         compared with the value for the desired headway time.)
     */
    void check();

protected:
    /// @brief init Rail Visualization Parameters
    void initRailVisualizationParameters();

private:
    /// @brief the parameter container
    SUMOVTypeParameter myParameter;

    /// @brief the vtypes actionsStepLength in seconds (cached because needed very often)
    double myCachedActionStepLengthSecs;

    /// @brief Indicator whether the user was already warned once about an action step length
    ///        larger than the desired time headway.
    bool myWarnedActionStepLengthTauOnce;
    bool myWarnedActionStepLengthBallisticOnce;

    /// @brief the running index
    const int myIndex;

    /// @brief instance of the car following model.
    MSCFModel* myCarFollowModel;

    /// @brief The original type
    const MSVehicleType* myOriginalType;

    /// @brief next value for the running index
    static int myNextIndex;


private:
    /// @brief Invalidated copy constructor
    MSVehicleType(const MSVehicleType&) = delete;

    /// @brief Invalidated assignment operator
    MSVehicleType& operator=(const MSVehicleType&) = delete;
};
