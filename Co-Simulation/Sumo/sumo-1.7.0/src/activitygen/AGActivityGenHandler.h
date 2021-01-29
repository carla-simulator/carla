/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
/// @file    AGActivityGenHandler.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Walter Bamberger
/// @date    July 2010
///
// The handler for parsing the statistics file.
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>
#include "city/AGCity.h"
#include "city/AGBusLine.h"


// ===========================================================================
// class declarations
// ===========================================================================
class RONet;


// ===========================================================================
// class definitions
// ===========================================================================
class AGActivityGenHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     *
     * @param[in] net The City instance to fill
     */
    AGActivityGenHandler(AGCity& city, RONet* net);


    /// @brief Destructor
    virtual ~AGActivityGenHandler();


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
    //@}

protected:
    /// @name called from myStartElement
    //@{

    /**
     * Parses attributes from an "general"-element in city file.
     */
    void parseGeneralCityInfo(const SUMOSAXAttributes& attrs);
    /*void parseInhabitants(const SUMOSAXAttributes &attrs);
    void parseHouseholds(const SUMOSAXAttributes &attrs);
    void parseChildrenAgeLimit(const SUMOSAXAttributes &attrs);
    void parseRetirementAgeLimit(const SUMOSAXAttributes &attrs);
    void parseCarRate(const SUMOSAXAttributes &attrs);
    void parseUnemployment(const SUMOSAXAttributes &attrs);
    void parseFootDistanceLimit(const SUMOSAXAttributes &attrs);
    void parseIncomingTraffic(const SUMOSAXAttributes &attrs);
    void parseOutgoingTraffic(const SUMOSAXAttributes &attrs);*/

    /**
     * parse parameters, all are optional, default values are given.
     */
    void parseParameters(const SUMOSAXAttributes& attrs);

    /**
     * streets are extracted: it fills the streets list of the City object
     * an additional computation is needed to normalize the number of inhabitants (and workpositions) in each street
     */
    void parseStreets(const SUMOSAXAttributes& attrs);

    /**
     * parse city entrances: the gates of the city, used for incoming and outgoing traffic
     */
    void parseCityGates(const SUMOSAXAttributes& attrs);

    /**
     * extraction of work opening and closing hours
     */
    void parseWorkHours();
    void parseOpeningHour(const SUMOSAXAttributes& attrs);
    void parseClosingHour(const SUMOSAXAttributes& attrs);

    /**
     * School extraction
     */
    void parseSchools();
    void parseSchool(const SUMOSAXAttributes& attrs);

    /**
     * Bus stations and Bus extraction
     * an additional computation have to be done to create buses' names and buses in the return way
     */
    void parseBusStation(const SUMOSAXAttributes& attrs);
    void parseBusLine(const SUMOSAXAttributes& attrs);
    void parseStations();
    void parseRevStations();
    void parseStation(const SUMOSAXAttributes& attrs);
    void parseFrequency(const SUMOSAXAttributes& attrs);

    /**
     * population and children accompaniment bracket extraction
     * then households and all population data will be computable
     */
    void parsePopulation();
    //void parseChildrenAccompaniment();
    void parseBracket(const SUMOSAXAttributes& attrs);


protected:
    /// @brief The city to store the information into
    AGCity& myCity;

    /// @brief The name of the object that is currently processed
    std::string myCurrentObject;
    AGBusLine* currentBusLine;

    /// @brief indicator whether the current station (in bus line context) is a reverse station or not.
    bool isRevStation;

    /// @brief The loaded network
    RONet* net;

    /** @brief An indicator whether the next edge shall be read (internal edges are not read by now) */
    bool myProcess;


private:
    /// @brief Invalidated copy constructor
    AGActivityGenHandler(const AGActivityGenHandler& src);

    /// @brief Invalidated assignment operator
    AGActivityGenHandler& operator=(const AGActivityGenHandler& src);

};
