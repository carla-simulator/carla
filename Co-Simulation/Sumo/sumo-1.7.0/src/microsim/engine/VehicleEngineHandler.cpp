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
/// @file    VehicleEngineHandler.cpp
/// @author  Michele Segata
/// @date    4 Feb 2015
///
/****************************************************************************/
#include <config.h>

#include <utils/common/StringUtils.h>
#include "VehicleEngineHandler.h"


// ===========================================================================
// class definitions
// ===========================================================================
VehicleEngineHandler::VehicleEngineHandler(const std::string& toLoad)
    : currentTag(TAG_VEHICLES), skip(false), currentGear(1) {
    vehicleToLoad = toLoad;
}


VehicleEngineHandler::~VehicleEngineHandler() {}

std::string transcode(const XMLCh* const qname) {
    return std::string(XERCES_CPP_NAMESPACE::XMLString::transcode(qname));
}

const XMLCh* transcode(const char* name) {
    return XERCES_CPP_NAMESPACE::XMLString::transcode(name);
}

std::string getAttributeValue(const char* attributeName, const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    return transcode(attrs.getValue(transcode(attributeName)));
}

void
VehicleEngineHandler::startElement(const XMLCh* const /*uri*/,
                                   const XMLCh* const /*localname*/,
                                   const XMLCh* const qname,
                                   const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    std::string tag = XERCES_CPP_NAMESPACE::XMLString::transcode(qname);

    switch (currentTag) {

        case TAG_VEHICLES:

            //we are already inside the root. just ignore this
            if (tag == ENGINE_TAG_VEHICLES) {
                break;
            }

            //this is a new vehicle definition. is this the one we should load?
            if (tag == ENGINE_TAG_VEHICLE) {
                if (getAttributeValue(ENGINE_TAG_VEHICLE_ID, attrs) != vehicleToLoad) {
                    skip = true;
                } else {
                    engineParameters.id = vehicleToLoad;
                }
                currentTag = TAG_VEHICLE;
            }

            break;

        case TAG_VEHICLE:

            //we are not interested in this vehicle
            if (skip) {
                break;
            }

            //definition of gear ratios
            if (tag == ENGINE_TAG_GEARS) {
                currentTag = TAG_GEARS;
            }
            //definition of masses
            else if (tag == ENGINE_TAG_MASS) {
                loadMassData(attrs);
            }
            //definition of air drag
            else if (tag == ENGINE_TAG_DRAG) {
                loadDragData(attrs);
            }
            //definition of wheels
            else if (tag == ENGINE_TAG_WHEELS) {
                loadWheelsData(attrs);
            }
            //definition of engine
            else if (tag == ENGINE_TAG_ENGINE) {
                loadEngineData(attrs);
                currentTag = TAG_ENGINE;
            }
            //definition of the shifting rule
            else if (tag == ENGINE_TAG_SHIFTING) {
                loadShiftingData(attrs);
            }
            //definition of brakes
            else if (tag == ENGINE_TAG_BRAKES) {
                loadBrakesData(attrs);
            } else {
                raiseUnknownTagError(tag);
            }

            break;

        case TAG_GEARS:

            if (skip) {
                break;
            }

            if (tag == ENGINE_TAG_GEAR) {
                //definition of the ratio for a single gear
                loadGearData(attrs);
            } else if (tag == ENGINE_TAG_GEAR_DIFFERENTIAL) {
                //definition of the ratio for the final drive
                loadDifferentialData(attrs);
            } else {
                raiseUnknownTagError(tag);
            }

            break;

        case TAG_ENGINE:

            if (skip) {
                break;
            }

            if (tag == ENGINE_TAG_ENGINE_POWER) {
                loadEngineModelData(attrs);
            } else {
                raiseUnknownTagError(tag);
            }

            break;

        default:

            break;

    }

}


void
VehicleEngineHandler::endElement(const XMLCh* const /*uri*/,
                                 const XMLCh* const /*localname*/,
                                 const XMLCh* const qname) {
    std::string tag = XERCES_CPP_NAMESPACE::XMLString::transcode(qname);

    switch (currentTag) {

        case TAG_VEHICLES:
            break;

        case TAG_VEHICLE:
            if (tag == ENGINE_TAG_VEHICLE) {
                skip = false;
                currentTag = TAG_VEHICLES;
            }
            break;

        case TAG_GEARS:
            if (tag == ENGINE_TAG_GEARS) {
                currentTag = TAG_VEHICLE;
                currentGear = 0;

                delete [] engineParameters.gearRatios;
                engineParameters.gearRatios = new double[gearRatios.size()];
                for (int i = 0; i < (int)gearRatios.size(); i++) {
                    engineParameters.gearRatios[i] = gearRatios[i];
                }
                engineParameters.nGears = (int)gearRatios.size();
            }

            break;

        case TAG_ENGINE:
            if (tag == ENGINE_TAG_ENGINE) {
                currentTag = TAG_VEHICLE;
            }
            break;

        default:

            break;

    }

}



void
VehicleEngineHandler::endDocument() {
    engineParameters.computeCoefficients();
}

const EngineParameters& VehicleEngineHandler::getEngineParameters() {
    return engineParameters;
}



void
VehicleEngineHandler::loadMassData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    engineParameters.mass_kg = parseDoubleAttribute(ENGINE_TAG_MASS, ENGINE_TAG_MASS_MASS, attrs);
    engineParameters.massFactor = parseDoubleAttribute(ENGINE_TAG_MASS, ENGINE_TAG_MASS_FACTOR, attrs);
}


void
VehicleEngineHandler::loadDragData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    engineParameters.cAir = parseDoubleAttribute(ENGINE_TAG_DRAG, ENGINE_TAG_DRAG_CAIR, attrs);
    engineParameters.a_m2 = parseDoubleAttribute(ENGINE_TAG_DRAG, ENGINE_TAG_DRAG_SECTION, attrs);
}


void
VehicleEngineHandler::loadWheelsData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    engineParameters.wheelDiameter_m = parseDoubleAttribute(ENGINE_TAG_WHEELS, ENGINE_TAG_WHEELS_DIAMETER, attrs);
    engineParameters.tiresFrictionCoefficient = parseDoubleAttribute(ENGINE_TAG_WHEELS, ENGINE_TAG_WHEELS_FRICTION, attrs);
    engineParameters.cr1 = parseDoubleAttribute(ENGINE_TAG_WHEELS, ENGINE_TAG_WHEELS_CR1, attrs);
    engineParameters.cr2 = parseDoubleAttribute(ENGINE_TAG_WHEELS, ENGINE_TAG_WHEELS_CR2, attrs);
}


void
VehicleEngineHandler::loadEngineData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    engineParameters.engineEfficiency = parseDoubleAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_EFFICIENCY, attrs);
    engineParameters.cylinders = parseIntAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_CYLINDERS, attrs);
    engineParameters.minRpm = parseIntAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_MINRPM, attrs);
    engineParameters.maxRpm = parseIntAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_MAXRPM, attrs);
    engineParameters.tauEx_s = parseDoubleAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_TAU_EX, attrs);
    if (existsAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_TAU_BURN, attrs) != -1) {
        engineParameters.tauBurn_s = parseDoubleAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_TAU_BURN, attrs);
        engineParameters.fixedTauBurn = true;
    } else {
        engineParameters.fixedTauBurn = false;
    }
    engineParameters.maxRpm = parseIntAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_MAXRPM, attrs);
    std::string mapType = parseStringAttribute(ENGINE_TAG_ENGINE, ENGINE_TAG_ENGINE_TYPE, attrs);
    if (mapType != "poly") {
        throw ProcessError("Invalid engine map type. Only \"poly\" is supported for now");
    }
}


void
VehicleEngineHandler::loadGearData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {

    int number = parseIntAttribute(ENGINE_TAG_GEAR, ENGINE_TAG_GEAR_N, attrs);
    if (number != currentGear) {
        //fatal
        std::stringstream ss;
        ss << "Invalid gear number " << number << ". Please check that gears are inserted in order";
        throw ProcessError(ss.str());
    }
    gearRatios.push_back(parseDoubleAttribute(ENGINE_TAG_GEAR, ENGINE_TAG_GEAR_RATIO, attrs));
    currentGear++;

}


void
VehicleEngineHandler::loadDifferentialData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    engineParameters.differentialRatio = parseDoubleAttribute(ENGINE_TAG_GEAR_DIFFERENTIAL, ENGINE_TAG_GEAR_RATIO, attrs);
}


void
VehicleEngineHandler::loadEngineModelData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    //check that the degree is within the maximum supported
    if (attrs.getLength() > MAX_POLY_DEGREE) {
        std::stringstream ss;
        ss << "Maximum degree for the engine polynomial is " << MAX_POLY_DEGREE << ". Please check your model's data";
        throw ProcessError(ss.str());
    }
    //parse all polynomial coefficients
    for (int i = 0; i < (int)attrs.getLength(); i++) {
        engineParameters.engineMapping.x[i] = parsePolynomialCoefficient(i, attrs);
    }
    //save the actual degree
    engineParameters.engineMapping.degree = (int)attrs.getLength();
}


void
VehicleEngineHandler::loadShiftingData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    engineParameters.shiftingRule.rpm = parseDoubleAttribute(ENGINE_TAG_SHIFTING, ENGINE_TAG_SHIFTING_RPM, attrs);
    engineParameters.shiftingRule.deltaRpm = parseDoubleAttribute(ENGINE_TAG_SHIFTING, ENGINE_TAG_SHIFTING_DELTARPM, attrs);
}


void
VehicleEngineHandler::loadBrakesData(const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    engineParameters.brakesTau_s = parseDoubleAttribute(ENGINE_TAG_BRAKES, ENGINE_TAG_BRAKES_TAU, attrs);
}

int VehicleEngineHandler::existsAttribute(std::string /*tag*/, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    return attrs.getIndex(transcode(attribute));
}
std::string VehicleEngineHandler::parseStringAttribute(std::string tag, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    int attributeIndex;
    std::string strValue;
    attributeIndex = existsAttribute(tag, attribute, attrs);
    if (attributeIndex == -1) {
        //raise will stop execution
        raiseMissingAttributeError(tag, attribute);
    }
    return transcode(attrs.getValue(attributeIndex));
}
int VehicleEngineHandler::parseIntAttribute(std::string tag, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    return StringUtils::toInt(parseStringAttribute(tag, attribute, attrs));
}
double VehicleEngineHandler::parseDoubleAttribute(std::string tag, const char* attribute, const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    return StringUtils::toDouble(parseStringAttribute(tag, attribute, attrs));
}
double VehicleEngineHandler::parsePolynomialCoefficient(int index, const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    std::stringstream ss;
    ss << "x" << index;
    return parseDoubleAttribute(ENGINE_TAG_ENGINE_POWER, ss.str().c_str(), attrs);
}


void
VehicleEngineHandler::raiseMissingAttributeError(std::string tag, std::string attribute) {
    std::stringstream ss;
    ss << "Missing attribute \"" << attribute << "\" for tag " << tag;
    throw ProcessError(ss.str());
}


void
VehicleEngineHandler::raiseUnknownTagError(std::string tag) {
    std::stringstream ss;
    ss << "I don't know what to do with this tag: " << tag;
    throw ProcessError(ss.str());
}


/****************************************************************************/
