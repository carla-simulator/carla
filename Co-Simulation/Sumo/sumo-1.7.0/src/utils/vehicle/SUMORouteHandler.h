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
/// @file    SUMORouteHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Parser for routes during their loading
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/IDSupplier.h>
#include <utils/common/SUMOTime.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MsgHandler;
class SUMOVTypeParameter;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class SUMORouteHandler
 * @brief Parser for routes during their loading
 *
 * SUMORouteHandler is the abstract super class for routers
 * and simulation loading routes.
 */
class SUMORouteHandler : public SUMOSAXHandler {
public:
    /// @brief enum for stops
    enum StopPos {
        STOPPOS_VALID,
        STOPPOS_INVALID_STARTPOS,
        STOPPOS_INVALID_ENDPOS,
        STOPPOS_INVALID_LANELENGTH
    };

    /// @brief standard constructor
    SUMORouteHandler(const std::string& file, const std::string& expectedRoot, const bool hardFail);

    /// @brief standard destructor
    virtual ~SUMORouteHandler();

    /**@brief check start and end position of a stop
     * @brief return */
    static StopPos checkStopPos(double& startPos, double& endPos, const double laneLength, const double minLength, const bool friendlyPos);

    /// @brief check if start and end position of a stop is valid
    static bool isStopPosValid(const double startPos, const double endPos, const double laneLength, const double minLength, const bool friendlyPos);

    /// @brief returns the first departure time that was ever read
    SUMOTime getFirstDepart() const;

    /// @brief Returns the last loaded depart time
    SUMOTime getLastDepart() const;

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    virtual void myStartElement(int element,
                                const SUMOSAXAttributes& attrs);

    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    virtual void myEndElement(int element);
    //@}

    /// @name open element functions
    //@{

    /// @brief opens a type distribution for reading
    virtual void openVehicleTypeDistribution(const SUMOSAXAttributes& attrs) = 0;

    /// @brief closes (ends) the building of a distribution
    virtual void closeVehicleTypeDistribution() = 0;

    /// @brief opens a route for reading
    virtual void openRoute(const SUMOSAXAttributes& attrs) = 0;

    /// @brief opens a flow for reading
    virtual void openFlow(const SUMOSAXAttributes& attrs) = 0;

    /// @brief opens a route flow for reading
    virtual void openRouteFlow(const SUMOSAXAttributes& attrs) = 0;

    /// @brief opens a trip for reading
    virtual void openTrip(const SUMOSAXAttributes& attrs) = 0;
    //@}

    /// @name close element functions
    //@{

    /**closes (ends) the building of a route.
     * Afterwards no edges may be added to it;
     * this method may throw exceptions when
     * a) the route is empty or
     * b) another route with the same id already exists
     */
    virtual void closeRoute(const bool mayBeDisconnected = false) = 0;

    /// @brief opens a route distribution for reading
    virtual void openRouteDistribution(const SUMOSAXAttributes& attrs) = 0;

    /// @brief closes (ends) the building of a distribution
    virtual void closeRouteDistribution() = 0;

    /// @brief Ends the processing of a vehicle
    virtual void closeVehicle() = 0;

    /// @brief Ends the processing of a vehicle type
    virtual void closeVType() = 0;

    /// @brief Ends the processing of a person
    virtual void closePerson() = 0;

    /// @brief Ends the processing of a person
    virtual void closePersonFlow() = 0;

    /// @brief Ends the processing of a container
    virtual void closeContainer() = 0;

    /// @brief Ends the processing of a flow
    virtual void closeFlow() = 0;

    /// @brief Ends the processing of a trip
    virtual void closeTrip() = 0;
    //@}

    /// @name add element functions
    //@{

    /// @brief Processing of a stop
    virtual void addStop(const SUMOSAXAttributes& attrs) = 0;

    /// @brief add a routing request for a walking or intermodal person
    virtual void addPersonTrip(const SUMOSAXAttributes& attrs) = 0;

    /// @brief add a fully specified walk
    virtual void addWalk(const SUMOSAXAttributes& attrs) = 0;

    /// @brief Processing of a person
    virtual void addPerson(const SUMOSAXAttributes& attrs) = 0;

    /// @brief Processing of a container
    virtual void addContainer(const SUMOSAXAttributes& attrs) = 0;

    /// @brief Processing of a ride
    virtual void addRide(const SUMOSAXAttributes& attrs) = 0;

    /// @brief Processing of a transport
    virtual void addTransport(const SUMOSAXAttributes& attrs) = 0;

    /// @brief Processing of a tranship
    virtual void addTranship(const SUMOSAXAttributes& attrs) = 0;

    //@}

    /// @brief Checks whether the route file is sorted by departure time if needed
    bool checkLastDepart();

    /// @brief save last depart (only to be used if vehicle is not discarded)
    void registerLastDepart();

    /// @brief assign arbitrary vehicle parameters
    void addParam(const SUMOSAXAttributes& attrs);

    /// @brief parses attributes common to all stops
    bool parseStop(SUMOVehicleParameter::Stop& stop, const SUMOSAXAttributes& attrs, std::string errorSuffix, MsgHandler* const errorOutput);

protected:
    /// @brief flag to enable or disable hard fails
    const bool myHardFail;

    /// @brief Parameter of the current vehicle, trip, person, container or flow
    SUMOVehicleParameter* myVehicleParameter;

    /// @brief The insertion time of the vehicle read last
    SUMOTime myLastDepart;

    /// @brief The id of the current route
    std::string myActiveRouteID;

    /// @brief The id of the route the current route references to
    std::string myActiveRouteRefID;

    /// @brief The probability of the current route
    double myActiveRouteProbability;

    /// @brief The currently parsed route's color
    const RGBColor* myActiveRouteColor;

    /// @brief The currently parsed route costs
    double myCurrentCosts;

    /// @brief List of the stops on the parsed route
    std::vector<SUMOVehicleParameter::Stop> myActiveRouteStops;

    /// @brief The currently parsed vehicle type
    SUMOVTypeParameter* myCurrentVType;

    /// @brief Parameterised used for saving loaded generic parameters that aren't saved in Vehicles or Vehicle Types
    Parameterised myLoadedParameterised;

    /// @brief generates numerical ids
    IDSupplier myIdSupplier;

    /// @brief The default value for flow begins
    SUMOTime myBeginDefault;

    /// @brief The default value for flow ends
    SUMOTime myEndDefault;

    /// @brief the first read departure time
    SUMOTime myFirstDepart;

    /// @brief where stop edges can be inserted into the current route (-1 means no insertion)
    int myInsertStopEdgesAt;

private:
    /// @brief Invalidated copy constructor
    SUMORouteHandler(const SUMORouteHandler& s) = delete;

    /// @brief Invalidated assignment operator
    SUMORouteHandler& operator=(const SUMORouteHandler& s) = delete;
};
