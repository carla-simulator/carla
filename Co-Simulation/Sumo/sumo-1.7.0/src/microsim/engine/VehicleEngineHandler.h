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
/// @file    VehicleEngineHandler.h
/// @author  Michele Segata
/// @date    4 Feb 2015
///
/****************************************************************************/

#pragma once

#include <string>
#include <map>
#include <stack>
#include <sstream>
#include <vector>
#include <iostream>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include "EngineParameters.h"

//definition of tag names of the xml file
#define ENGINE_TAG_VEHICLES               "vehicles"
#define ENGINE_TAG_VEHICLE                "vehicle"
#define ENGINE_TAG_VEHICLE_ID             "id"
#define ENGINE_TAG_VEHICLE_DESCRIPTION    "description"
#define ENGINE_TAG_GEARS                  "gears"
#define ENGINE_TAG_GEAR                   "gear"
#define ENGINE_TAG_GEAR_N                 "n"
#define ENGINE_TAG_GEAR_RATIO             "ratio"
#define ENGINE_TAG_GEAR_DIFFERENTIAL      "differential"
#define ENGINE_TAG_MASS                   "mass"
#define ENGINE_TAG_MASS_MASS              "mass"
#define ENGINE_TAG_MASS_FACTOR            "massFactor"
#define ENGINE_TAG_WHEELS                 "wheels"
#define ENGINE_TAG_WHEELS_DIAMETER        "diameter"
#define ENGINE_TAG_WHEELS_FRICTION        "friction"
#define ENGINE_TAG_WHEELS_CR1             "cr1"
#define ENGINE_TAG_WHEELS_CR2             "cr2"
#define ENGINE_TAG_DRAG                   "drag"
#define ENGINE_TAG_DRAG_CAIR              "cAir"
#define ENGINE_TAG_DRAG_SECTION           "section"
#define ENGINE_TAG_ENGINE                 "engine"
#define ENGINE_TAG_ENGINE_TYPE            "type"
#define ENGINE_TAG_ENGINE_EFFICIENCY      "efficiency"
#define ENGINE_TAG_ENGINE_CYLINDERS       "cylinders"
#define ENGINE_TAG_ENGINE_MINRPM          "minRpm"
#define ENGINE_TAG_ENGINE_MAXRPM          "maxRpm"
#define ENGINE_TAG_ENGINE_TAU_EX          "tauEx"
#define ENGINE_TAG_ENGINE_TAU_BURN        "tauBurn"
#define ENGINE_TAG_ENGINE_POWER           "power"
#define ENGINE_TAG_ENGINE_POWER_RPM       "rpm"
#define ENGINE_TAG_ENGINE_POWER_HP        "hp"
#define ENGINE_TAG_ENGINE_POWER_KW        "kw"
#define ENGINE_TAG_ENGINE_POWER_SLOPE     "slope"
#define ENGINE_TAG_ENGINE_POWER_INTERCEPT "intercept"
#define ENGINE_TAG_SHIFTING               "shifting"
#define ENGINE_TAG_SHIFTING_RPM           "rpm"
#define ENGINE_TAG_SHIFTING_DELTARPM      "deltaRpm"
#define ENGINE_TAG_BRAKES                 "brakes"
#define ENGINE_TAG_BRAKES_TAU             "tau"

#define TAG_VEHICLES 0
#define TAG_VEHICLE  1
#define TAG_GEARS    2
#define TAG_ENGINE   3

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * SAX handler used to parse engine parameters
 */
class VehicleEngineHandler : public XERCES_CPP_NAMESPACE::DefaultHandler {

public:

    /**
     * Constructor
     *
     * @param[in] toLoad id of the vehicle to be loaded
     */
    VehicleEngineHandler(const std::string& toLoad);


    /** @brief Destructor */
    virtual ~VehicleEngineHandler();

    void startElement(const XMLCh* const uri, const XMLCh* const localname,
                      const XMLCh* const qname, const XERCES_CPP_NAMESPACE::Attributes& attrs);

    void endElement(const XMLCh* const uri, const XMLCh* const localname,
                    const XMLCh* const qname);

    void endDocument();

    const EngineParameters& getEngineParameters();

protected:

    /**
     * Loads mass information, i.e., mass in kg and mass factor which takes
     * into account rotational parts of the engine
     */
    void loadMassData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    \
    /**
     * Load air drag related data such as drag coefficient and maximum vehicle
     * section
     */
    void loadDragData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Load data about vehicle's wheels, such as diameter and friction
     * coefficient
     */
    void loadWheelsData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Load data about the engine, such as efficiency factor and cylinders
     */
    void loadEngineData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Load gear ratios
     */
    void loadGearData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Load final drive ratio
     */
    void loadDifferentialData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Load the mapping between engine rpm and output power in terms of linear
     * function, i.e., slope and intercept
     */
    void loadEngineModelData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Load the gear shifting rules
     */
    void loadShiftingData(const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Load data about brakes
     */
    void loadBrakesData(const XERCES_CPP_NAMESPACE::Attributes& attrs);

    /**
     * Checks whether an attribute exists
     *
     * @return the index, if the attribute exists, -1 otherwise
     */
    int existsAttribute(std::string tag, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Search and returns a string attribute if existing. If not, an error is
     * printed to stderr and the simulation is stopped
     */
    std::string parseStringAttribute(std::string tag, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Search and returns an integer attribute if existing. If not, an error is
     * printed to stderr and the simulation is stopped. The simulation is
     * stopped if the integer cannot be parsed as well
     */
    int parseIntAttribute(std::string tag, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Search and returns a double attribute if existing. If not, an error is
     * printed to stderr and the simulation is stopped. The simulation is
     * stopped if the double cannot be parsed as well
     */
    double parseDoubleAttribute(std::string tag, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Search for the x_i coefficient in the list of attributes. If not found,
     * an error is printed to stderr and the simulation is stopped. The
     * simulation is stopped if the value cannot be parsed as well
     */
    double parsePolynomialCoefficient(int index, const XERCES_CPP_NAMESPACE::Attributes& attrs);
    /**
     * Writes a missing attribute error to stderr and terminates the simulation
     */
    void raiseMissingAttributeError(std::string tag, std::string attribute);
    /**
     * Writes an unknown tag error to stderr and terminates the simulation
     */
    void raiseUnknownTagError(std::string tag);


private:

    //current tag we're into
    int currentTag;
    //vehicle type to load
    std::string vehicleToLoad;
    //skip loading of current vehicle data
    bool skip;
    //current loaded gear
    int currentGear;
    //where to store loaded data
    EngineParameters engineParameters;
    //vector of gear ratios
    std::vector<double> gearRatios;

private:
    /// @brief invalidated copy constructor
    VehicleEngineHandler(const VehicleEngineHandler& s);

    /// @brief invalidated assignment operator
    const VehicleEngineHandler& operator=(const VehicleEngineHandler& s);

};
