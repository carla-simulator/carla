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
/// @file    AGActivityGenHandler.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    July 2010
///
// The handler for parsing the statistics file.
/****************************************************************************/
#include <config.h>

#include "AGActivityGenHandler.h"
#include <iostream>
#include <utility>
#include <map>
#include <string>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <router/RONet.h>
#include "city/AGCity.h"
#include "city/AGSchool.h"
#include "city/AGPosition.h"
#include "city/AGBusLine.h"


// ===========================================================================
// method definitions
// ===========================================================================
AGActivityGenHandler::AGActivityGenHandler(AGCity& city, RONet* net)
    : SUMOSAXHandler("sumo-stat"),
      myCity(city), net(net) {}


AGActivityGenHandler::~AGActivityGenHandler() {}


void
AGActivityGenHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    try {
        switch (element) {
            case AGEN_TAG_GENERAL:
                parseGeneralCityInfo(attrs);
                break;
            case AGEN_TAG_STREET:
                parseStreets(attrs);
                break;
            case AGEN_TAG_WORKHOURS:
                parseWorkHours();
                break;
            case AGEN_TAG_OPENING:
                parseOpeningHour(attrs);
                break;
            case AGEN_TAG_CLOSING:
                parseClosingHour(attrs);
                break;
            case AGEN_TAG_SCHOOLS:
                parseSchools();
                break;
            case AGEN_TAG_SCHOOL:
                parseSchool(attrs);
                break;
            case AGEN_TAG_BUSSTATION:
                parseBusStation(attrs);
                break;
            case AGEN_TAG_BUSLINE:
                parseBusLine(attrs);
                break;
            case AGEN_TAG_STATIONS:
                parseStations();
                break;
            case AGEN_TAG_REV_STATIONS:
                parseRevStations();
                break;
            case AGEN_TAG_STATION:
                parseStation(attrs);
                break;
            case AGEN_TAG_FREQUENCY:
                parseFrequency(attrs);
                break;
            case AGEN_TAG_POPULATION:
                parsePopulation();
                break;
            /*case AGEN_TAG_CHILD_ACOMP:
                parseChildrenAccompaniment();
                break;*/
            case AGEN_TAG_BRACKET:
                parseBracket(attrs);
                break;
            case AGEN_TAG_PARAM:
                parseParameters(attrs);
                break;
            case AGEN_TAG_ENTRANCE:
                parseCityGates(attrs);
                break;
            default:
                break;
        }
    } catch (const std::exception& e) {
        throw ProcessError(e.what());
    }
}


void
AGActivityGenHandler::parseGeneralCityInfo(const SUMOSAXAttributes& attrs) {
    try {
        bool ok;
        myCity.statData.inhabitants = attrs.getInt(AGEN_ATTR_INHABITANTS);
        myCity.statData.households = attrs.getInt(AGEN_ATTR_HOUSEHOLDS);
        myCity.statData.limitAgeChildren = attrs.getOpt<int>(AGEN_ATTR_CHILDREN, nullptr, ok, 18);
        myCity.statData.limitAgeRetirement = attrs.getOpt<int>(AGEN_ATTR_RETIREMENT, nullptr, ok, 63);
        myCity.statData.carRate = attrs.getOpt<double>(AGEN_ATTR_CARS, nullptr, ok, 0.58);
        myCity.statData.unemployement = attrs.getOpt<double>(AGEN_ATTR_UNEMPLOYEMENT, nullptr, ok, 0.06);
        myCity.statData.laborDemand = attrs.getOpt<double>(AGEN_ATTR_LABORDEMAND, nullptr, ok, 1.05);
        myCity.statData.maxFootDistance = attrs.getOpt<double>(AGEN_ATTR_MAX_FOOT_DIST, nullptr, ok, 300.0);
        myCity.statData.incomingTraffic = attrs.getOpt<int>(AGEN_ATTR_IN_TRAFFIC, nullptr, ok, 0);
        myCity.statData.outgoingTraffic = attrs.getOpt<int>(AGEN_ATTR_OUT_TRAFFIC, nullptr, ok, 0);
    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_GENERAL) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseParameters(const SUMOSAXAttributes& attrs) {
    try {
        bool ok;
        myCity.statData.carPreference = attrs.getOpt<double>(AGEN_ATTR_CARPREF, nullptr, ok, 0.0);
        myCity.statData.speedTimePerKm = attrs.getOpt<double>(AGEN_ATTR_CITYSPEED, nullptr, ok, 360.0);
        myCity.statData.freeTimeActivityRate = attrs.getOpt<double>(AGEN_ATTR_FREETIMERATE, nullptr, ok, 0.15);
        myCity.statData.uniformRandomTrafficRate = attrs.getOpt<double>(AGEN_ATTR_UNI_RAND_TRAFFIC, nullptr, ok, 0.0);
        myCity.statData.departureVariation = attrs.getOpt<double>(AGEN_ATTR_DEP_VARIATION, nullptr, ok, 0.0);
    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_PARAM) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseStreets(const SUMOSAXAttributes& attrs) {
    try {
        double pop = 0;
        double work = 0;

        if (attrs.hasAttribute(AGEN_ATTR_POPULATION)) {
            pop = attrs.getFloat(AGEN_ATTR_POPULATION);
        }
        if (attrs.hasAttribute(AGEN_ATTR_OUT_WORKPOSITION)) {
            work = attrs.getFloat(AGEN_ATTR_OUT_WORKPOSITION);
        }
        std::string eid = attrs.getString(SUMO_ATTR_EDGE);
        AGStreet* street = dynamic_cast<AGStreet*>(net->getEdge(eid));
        if (street == nullptr) {
            WRITE_ERROR("Edge '" + eid + "' is not known.");
            return;
        }
        street->setPopulation(pop * street->getLength());
        street->setWorkplaceNumber(work * street->getLength());
        myCity.streets.push_back(street);
    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_STREET) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseCityGates(const SUMOSAXAttributes& attrs) {
    try {
        std::string edge = attrs.getString(SUMO_ATTR_EDGE);
        double positionOnEdge = attrs.getFloat(SUMO_ATTR_POSITION);
        AGPosition posi(myCity.getStreet(edge), positionOnEdge);
        myCity.statData.incoming[(int)myCity.cityGates.size()] = attrs.getFloat(AGEN_ATTR_INCOMING);
        myCity.statData.outgoing[(int)myCity.cityGates.size()] = attrs.getFloat(AGEN_ATTR_OUTGOING);
        myCity.cityGates.push_back(posi);

    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_CITYGATES) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseWorkHours() {
    myCurrentObject = "workHours";
}

void
AGActivityGenHandler::parseOpeningHour(const SUMOSAXAttributes& attrs) {
    if (myCurrentObject == "workHours") {
        try {
            myCity.statData.beginWorkHours[attrs.getInt(AGEN_ATTR_HOUR)] = attrs.getFloat(AGEN_ATTR_PROP);

        } catch (const std::exception& e) {
            WRITE_ERROR("Error while parsing the element " +
                        SUMOXMLDefinitions::Tags.getString(AGEN_TAG_OPENING) + ": "
                        + e.what());
            throw ProcessError();
        }
    }
}

void
AGActivityGenHandler::parseClosingHour(const SUMOSAXAttributes& attrs) {
    if (myCurrentObject == "workHours") {
        try {
            myCity.statData.endWorkHours[attrs.getInt(AGEN_ATTR_HOUR)] = attrs.getFloat(AGEN_ATTR_PROP);

        } catch (const std::exception& e) {
            WRITE_ERROR("Error while parsing the element " +
                        SUMOXMLDefinitions::Tags.getString(AGEN_TAG_CLOSING) + ": "
                        + e.what());
            throw ProcessError();
        }
    }
}

void
AGActivityGenHandler::parseSchools() {
    myCurrentObject = "schools";
}

void
AGActivityGenHandler::parseSchool(const SUMOSAXAttributes& attrs) {
    try {
        std::string edge = attrs.getString(SUMO_ATTR_EDGE);
        double positionOnEdge = attrs.getFloat(SUMO_ATTR_POSITION);
        AGPosition posi(myCity.getStreet(edge), positionOnEdge);
        int beginAge = attrs.getInt(AGEN_ATTR_BEGINAGE);
        int endAge = attrs.getInt(AGEN_ATTR_ENDAGE);
        int capacity = attrs.getInt(AGEN_ATTR_CAPACITY);
        int openingHour = attrs.getInt(AGEN_ATTR_OPENING);
        int closingHour = attrs.getInt(AGEN_ATTR_CLOSING);
        AGSchool sch(capacity, posi, beginAge, endAge, openingHour, closingHour);
        myCity.schools.push_back(sch);

    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_SCHOOL) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseBusStation(const SUMOSAXAttributes& attrs) {
    try {
        std::string edge = attrs.getString(SUMO_ATTR_EDGE);
        double positionOnEdge = attrs.getFloat(SUMO_ATTR_POSITION);
        int id = attrs.getInt(SUMO_ATTR_ID);
        AGPosition posi(myCity.getStreet(edge), positionOnEdge);
        myCity.statData.busStations.insert(std::pair<int, AGPosition>(id, posi));

    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_BUSSTATION) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseBusLine(const SUMOSAXAttributes& attrs) {
    try {
        myCurrentObject = "busLine";
        AGBusLine busL(attrs.getString(SUMO_ATTR_ID));
        busL.setMaxTripTime(attrs.getInt(AGEN_ATTR_MAX_TRIP_DURATION));
        myCity.busLines.push_front(busL);
        currentBusLine = &*myCity.busLines.begin();

    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_BUSLINE) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseStations() {
    isRevStation = false;
}

void
AGActivityGenHandler::parseRevStations() {
    isRevStation = true;
}

void
AGActivityGenHandler::parseStation(const SUMOSAXAttributes& attrs) {
    if (myCurrentObject != "busLine") {
        return;
    }

    try {
        bool ok = true;
        int refID = attrs.get<int>(SUMO_ATTR_REFID, myCurrentObject.c_str(), ok);
        if (!ok) {
            throw ProcessError();
        }
        if (myCity.statData.busStations.count(refID) == 0) {
            throw ProcessError("Unknown bus station " + toString(refID));
        }
        if (!isRevStation) {
            currentBusLine->locateStation(myCity.statData.busStations.find(refID)->second);
        } else {
            currentBusLine->locateRevStation(myCity.statData.busStations.find(refID)->second);
        }

    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_STATION) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parseFrequency(const SUMOSAXAttributes& attrs) {
    if (myCurrentObject != "busLine") {
        return;
    }

    try {
        int beginB = attrs.getInt(SUMO_ATTR_BEGIN);
        int endB = attrs.getInt(SUMO_ATTR_END);
        int rateB = attrs.getInt(AGEN_ATTR_RATE);
        currentBusLine->generateBuses(beginB, endB, rateB);

    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_FREQUENCY) + ": " +
                    e.what());
        throw ProcessError();
    }
}

void
AGActivityGenHandler::parsePopulation() {
    myCurrentObject = "population";
}

void
AGActivityGenHandler::parseBracket(const SUMOSAXAttributes& attrs) {
    try {
//TODO beginAge needs to be evaluated
//        int beginAge = attrs.getInt(AGEN_ATTR_BEGINAGE); //included in the bracket
        int endAge = attrs.getInt(AGEN_ATTR_ENDAGE); //NOT included in the bracket
        if (myCurrentObject == "population") {
            myCity.statData.population[endAge] = attrs.getInt(AGEN_ATTR_PEOPLENBR);
        }

    } catch (const std::exception& e) {
        WRITE_ERROR("Error while parsing the element " +
                    SUMOXMLDefinitions::Tags.getString(AGEN_TAG_BRACKET) + ": " +
                    e.what());
        throw ProcessError();
    }
}


/****************************************************************************/
