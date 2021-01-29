/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMOVehicleParserHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Axel Wegener
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Mon, 07.04.2008
///
// Helper methods for parsing vehicle attributes
/****************************************************************************/
#include <config.h>

#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/RandHelper.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/xml/SUMOSAXAttributes.h>

#include "SUMOVehicleParserHelper.h"


// ===========================================================================
// static members
// ===========================================================================

SUMOVehicleParserHelper::CFAttrMap SUMOVehicleParserHelper::allowedCFModelAttrs;
SUMOVehicleParserHelper::LCAttrMap SUMOVehicleParserHelper::allowedLCModelAttrs;
std::set<SumoXMLAttr> SUMOVehicleParserHelper::allowedJMAttrs;


// ===========================================================================
// method definitions
// ===========================================================================

SUMOVehicleParameter*
SUMOVehicleParserHelper::parseFlowAttributes(SumoXMLTag tag, const SUMOSAXAttributes& attrs, const bool hardFail, const SUMOTime beginDefault, const SUMOTime endDefault, bool isPerson) {
    bool ok = true;
    bool abortCreation = true;
    // first parse ID
    std::string id = parseID(attrs, SUMO_TAG_FLOW);
    // check if ID is valid
    if (!id.empty()) {
        if (!SUMOXMLDefinitions::isValidVehicleID(id)) {
            return handleError(hardFail, abortCreation, "Invalid flow id '" + id + "'.");
        }
        // declare flags
        const bool hasPeriod = attrs.hasAttribute(SUMO_ATTR_PERIOD);
        const bool hasVPH = attrs.hasAttribute(SUMO_ATTR_VEHSPERHOUR);
        const bool hasPPH = attrs.hasAttribute(SUMO_ATTR_PERSONSPERHOUR);
        const bool hasXPH = hasVPH || hasPPH;
        const bool hasProb = attrs.hasAttribute(SUMO_ATTR_PROB);
        const bool hasNumber = attrs.hasAttribute(SUMO_ATTR_NUMBER);
        const bool hasBegin = attrs.hasAttribute(SUMO_ATTR_BEGIN);
        const bool hasEnd = attrs.hasAttribute(SUMO_ATTR_END);
        if (hasPeriod && hasXPH) {
            return handleError(hardFail, abortCreation,
                               "At most one of '" + attrs.getName(SUMO_ATTR_PERIOD) +
                               "' and '" + attrs.getName(SUMO_ATTR_VEHSPERHOUR) +
                               "' has to be given in the definition of flow '" + id + "'.");
        }
        if (hasPeriod && hasProb) {
            return handleError(hardFail, abortCreation,
                               "At most one of '" + attrs.getName(SUMO_ATTR_PERIOD) +
                               "' and '" + attrs.getName(SUMO_ATTR_PROB) +
                               "' has to be given in the definition of flow '" + id + "'.");
        }
        if (hasProb && hasXPH) {
            return handleError(hardFail, abortCreation,
                               "At most one of '" + attrs.getName(SUMO_ATTR_PROB) +
                               "' and '" + attrs.getName(SUMO_ATTR_VEHSPERHOUR) +
                               "' has to be given in the definition of flow '" + id + "'.");
        }
        if (hasPeriod || hasXPH || hasProb) {
            if (hasEnd && hasNumber) {
                return handleError(hardFail, abortCreation,
                                   "If '" + attrs.getName(SUMO_ATTR_PERIOD) +
                                   "', '" + attrs.getName(SUMO_ATTR_VEHSPERHOUR) +
                                   "' or '" + attrs.getName(SUMO_ATTR_PROB) +
                                   "' are given at most one of '" + attrs.getName(SUMO_ATTR_END) +
                                   "' and '" + attrs.getName(SUMO_ATTR_NUMBER) +
                                   "' are allowed in flow '" + id + "'.");
            }
        } else {
            if (!hasNumber) {
                return handleError(hardFail, abortCreation,
                                   "At least one of '" + attrs.getName(SUMO_ATTR_PERIOD) +
                                   "', '" + attrs.getName(SUMO_ATTR_VEHSPERHOUR) +
                                   "', '" + attrs.getName(SUMO_ATTR_PROB) +
                                   "', and '" + attrs.getName(SUMO_ATTR_NUMBER) +
                                   "' is needed in flow '" + id + "'.");
            }
        }
        SUMOVehicleParameter* ret = new SUMOVehicleParameter();
        // set tag
        ret->tag = tag;
        // set id
        ret->id = id;
        if (isPerson) {
            ret->vtypeid = DEFAULT_PEDTYPE_ID;
        }
        try {
            parseCommonAttributes(attrs, hardFail, ret, "flow");
        } catch (ProcessError&) {
            delete ret;
            throw;
        }
        // parse repetition information
        if (hasPeriod) {
            ret->parametersSet |= VEHPARS_PERIOD_SET;
            ret->repetitionOffset = attrs.getSUMOTimeReporting(SUMO_ATTR_PERIOD, id.c_str(), ok);
        }
        if (hasXPH) {
            ret->parametersSet |= VEHPARS_VPH_SET;
            const double vph = attrs.get<double>(hasVPH ? SUMO_ATTR_VEHSPERHOUR : SUMO_ATTR_PERSONSPERHOUR, id.c_str(), ok);
            if (ok && vph <= 0) {
                delete ret;
                return handleError(hardFail, abortCreation,
                                   "Invalid repetition rate in the definition of "
                                   + std::string(hasVPH ? "flow" : "personFlow") + " '" + id + "'.");
            }
            if (ok && vph != 0) {
                ret->repetitionOffset = TIME2STEPS(3600. / vph);
            }
        }
        if (hasProb) {
            ret->parametersSet |= VEHPARS_PROB_SET;
            ret->repetitionProbability = attrs.get<double>(SUMO_ATTR_PROB, id.c_str(), ok);
            if (ok && (ret->repetitionProbability <= 0 || ret->repetitionProbability > 1)) {
                delete ret;
                return handleError(hardFail, abortCreation, "Invalid repetition probability in the definition of flow '" + id + "'.");
            }
        }
        ret->depart = beginDefault;
        if (hasBegin) {
            ret->depart = attrs.getSUMOTimeReporting(SUMO_ATTR_BEGIN, id.c_str(), ok);
        }
        if (ok && ret->depart < 0) {
            delete ret;
            return handleError(hardFail, abortCreation, "Negative begin time in the definition of flow '" + id + "'.");
        }
        ret->repetitionEnd = endDefault;
        if (ret->repetitionEnd < 0) {
            ret->repetitionEnd = SUMOTime_MAX;
        }
        if (hasEnd) {
            ret->repetitionEnd = attrs.getSUMOTimeReporting(SUMO_ATTR_END, id.c_str(), ok);
            ret->parametersSet |= VEHPARS_END_SET;
        } else if ((endDefault >= TIME2STEPS(9223372036854773) || endDefault < 0)
                   // see SUMOTIME_MAXSTRING (which differs slightly from SUMOTime_MAX)
                   && (!hasNumber || (!hasProb && !hasPeriod && !hasXPH))) {
            WRITE_WARNING("Undefined end for flow '" + id + "', defaulting to 24hour duration.");
            ret->repetitionEnd = ret->depart + TIME2STEPS(24 * 3600);
        }
        if (ok && ret->repetitionEnd < ret->depart) {
            delete ret;
            return handleError(hardFail, abortCreation, "Flow '" + id + "' ends before its begin time.");
        }
        if (hasNumber) {
            ret->repetitionNumber = attrs.get<int>(SUMO_ATTR_NUMBER, id.c_str(), ok);
            ret->parametersSet |= VEHPARS_NUMBER_SET;
            if (ret->repetitionNumber == 0) {
                WRITE_WARNING("Flow '" + id + "' has 0 vehicles; will skip it.");
            } else {
                if (ok && ret->repetitionNumber < 0) {
                    delete ret;
                    return handleError(hardFail, abortCreation, "Negative repetition number in the definition of flow '" + id + "'.");
                }
                if (ok && ret->repetitionOffset < 0) {
                    ret->repetitionOffset = (ret->repetitionEnd - ret->depart) / ret->repetitionNumber;
                }
            }
            ret->repetitionEnd = ret->depart + ret->repetitionNumber * ret->repetitionOffset;
        } else {
            // interpret repetitionNumber
            if (ok && ret->repetitionProbability > 0) {
                ret->repetitionNumber = std::numeric_limits<int>::max();
            } else {
                if (ok && ret->repetitionOffset <= 0) {
                    delete ret;
                    return handleError(hardFail, abortCreation, "Invalid repetition rate in the definition of flow '" + id + "'.");
                }
                if (ret->repetitionEnd == SUMOTime_MAX) {
                    ret->repetitionNumber = std::numeric_limits<int>::max();
                } else {
                    const double repLength = (double)(ret->repetitionEnd - ret->depart);
                    ret->repetitionNumber = (int)ceil(repLength / ret->repetitionOffset);
                }
            }
        }
        if (!ok) {
            delete ret;
            return handleError(hardFail, abortCreation, "Flow cannot be created");
        }
        return ret;
    } else {
        if (hardFail) {
            throw ProcessError("Flow cannot be created");
        } else {
            return nullptr;
        }
    }
}


SUMOVehicleParameter*
SUMOVehicleParserHelper::parseVehicleAttributes(int element, const SUMOSAXAttributes& attrs, const bool hardFail, const bool optionalID, const bool skipDepart) {
    bool ok = true;
    std::string id, errorMsg;
    // for certain vehicles, ID can be optional
    if (optionalID) {
        id = attrs.getOpt<std::string>(SUMO_ATTR_ID, nullptr, ok, "");
    } else {
        // parse ID
        id = parseID(attrs, (SumoXMLTag)element);
    }
    // only continue if id is valid, or if is optional
    if (optionalID || !id.empty()) {
        SUMOVehicleParameter* ret = new SUMOVehicleParameter();
        ret->id = id;
        if (element == SUMO_TAG_PERSON) {
            ret->vtypeid = DEFAULT_PEDTYPE_ID;
        } else if (element == SUMO_TAG_CONTAINER) {
            ret->vtypeid = DEFAULT_CONTAINERTYPE_ID;
        }
        try {
            parseCommonAttributes(attrs, hardFail, ret, "vehicle");
            if (!skipDepart) {
                const std::string helper = attrs.get<std::string>(SUMO_ATTR_DEPART, ret->id.c_str(), ok);
                if (!ok || !SUMOVehicleParameter::parseDepart(helper, "vehicle", ret->id, ret->depart, ret->departProcedure, errorMsg)) {
                    throw ProcessError(errorMsg);
                }
            }
        } catch (ProcessError&) {
            delete ret;
            if (hardFail) {
                throw;
            } else {
                WRITE_ERROR(errorMsg);
                return nullptr;
            }
        }
        // set tag
        ret->tag = (SumoXMLTag)element;
        return ret;
    } else {
        std::string error = toString((SumoXMLTag)element) + " cannot be created";
        if (hardFail) {
            throw ProcessError(error);
        } else {
            WRITE_ERROR(error);
            return nullptr;
        }
    }
}

std::string
SUMOVehicleParserHelper::parseID(const SUMOSAXAttributes& attrs, const SumoXMLTag element) {
    bool ok = true;
    std::string id;
    // first check if attrs contain an ID
    if (attrs.hasAttribute(SUMO_ATTR_ID)) {
        id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
        if (SUMOXMLDefinitions::isValidVehicleID(id)) {
            return id;
        } else if (id.empty()) {
            // add extra information for empty IDs
            WRITE_ERROR("Invalid " + toString(element) + " id '" + id + "'.");
        } else {
            WRITE_ERROR("Invalid " + toString(element) + " id '" + id + "'. Contains invalid characters.");
        }
    } else {
        WRITE_ERROR("Attribute '" + toString(SUMO_ATTR_ID) + "' is missing in definition of " + toString(element));
    }
    // return empty (invalid) ID
    return "";
}


void
SUMOVehicleParserHelper::parseCommonAttributes(const SUMOSAXAttributes& attrs, const bool hardFail, SUMOVehicleParameter* ret, std::string element) {
    //ret->refid = attrs.getStringSecure(SUMO_ATTR_REFID, "");
    bool ok = true;
    bool abortCreation = true;
    // parse route information
    if (attrs.hasAttribute(SUMO_ATTR_ROUTE)) {
        ret->parametersSet |= VEHPARS_ROUTE_SET; // !!! needed?
        ret->routeid = attrs.get<std::string>(SUMO_ATTR_ROUTE, ret->id.c_str(), ok);
    }
    // parse type information
    if (attrs.hasAttribute(SUMO_ATTR_TYPE)) {
        ret->parametersSet |= VEHPARS_VTYPE_SET; // !!! needed?
        ret->vtypeid = attrs.get<std::string>(SUMO_ATTR_TYPE, ret->id.c_str(), ok);
    }
    // parse line information
    if (attrs.hasAttribute(SUMO_ATTR_LINE)) {
        ret->parametersSet |= VEHPARS_LINE_SET; // !!! needed?
        ret->line = attrs.get<std::string>(SUMO_ATTR_LINE, ret->id.c_str(), ok);
    }
    // parse zone information
    if (attrs.hasAttribute(SUMO_ATTR_FROM_TAZ)) {
        ret->parametersSet |= VEHPARS_FROM_TAZ_SET;
        ret->fromTaz = attrs.get<std::string>(SUMO_ATTR_FROM_TAZ, ret->id.c_str(), ok);
    }
    if (attrs.hasAttribute(SUMO_ATTR_TO_TAZ)) {
        ret->parametersSet |= VEHPARS_TO_TAZ_SET;
        ret->toTaz = attrs.get<std::string>(SUMO_ATTR_TO_TAZ, ret->id.c_str(), ok);
    }
    // parse reroute information
    if (attrs.getOpt<bool>(SUMO_ATTR_REROUTE, nullptr, ok, false)) {
        ret->parametersSet |= VEHPARS_FORCE_REROUTE;
    }

    std::string error;
    // parse depart lane information
    if (attrs.hasAttribute(SUMO_ATTR_DEPARTLANE)) {
        const std::string helper = attrs.get<std::string>(SUMO_ATTR_DEPARTLANE, ret->id.c_str(), ok);
        int lane;
        DepartLaneDefinition dld;
        if (SUMOVehicleParameter::parseDepartLane(helper, element, ret->id, lane, dld, error)) {
            ret->parametersSet |= VEHPARS_DEPARTLANE_SET;
            ret->departLane = lane;
            ret->departLaneProcedure = dld;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse depart position information
    if (attrs.hasAttribute(SUMO_ATTR_DEPARTPOS)) {
        const std::string helper = attrs.get<std::string>(SUMO_ATTR_DEPARTPOS, ret->id.c_str(), ok);
        double pos;
        DepartPosDefinition dpd;
        if (SUMOVehicleParameter::parseDepartPos(helper, element, ret->id, pos, dpd, error)) {
            ret->parametersSet |= VEHPARS_DEPARTPOS_SET;
            ret->departPos = pos;
            ret->departPosProcedure = dpd;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse lateral depart position information
    if (attrs.hasAttribute(SUMO_ATTR_DEPARTPOS_LAT)) {
        const std::string helper = attrs.get<std::string>(SUMO_ATTR_DEPARTPOS_LAT, ret->id.c_str(), ok);
        double pos;
        DepartPosLatDefinition dpd;
        if (SUMOVehicleParameter::parseDepartPosLat(helper, element, ret->id, pos, dpd, error)) {
            ret->parametersSet |= VEHPARS_DEPARTPOSLAT_SET;
            ret->departPosLat = pos;
            ret->departPosLatProcedure = dpd;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse depart speed information
    if (attrs.hasAttribute(SUMO_ATTR_DEPARTSPEED)) {
        std::string helper = attrs.get<std::string>(SUMO_ATTR_DEPARTSPEED, ret->id.c_str(), ok);
        double speed;
        DepartSpeedDefinition dsd;
        if (SUMOVehicleParameter::parseDepartSpeed(helper, element, ret->id, speed, dsd, error)) {
            ret->parametersSet |= VEHPARS_DEPARTSPEED_SET;
            ret->departSpeed = speed;
            ret->departSpeedProcedure = dsd;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse arrival lane information
    if (attrs.hasAttribute(SUMO_ATTR_ARRIVALLANE)) {
        std::string helper = attrs.get<std::string>(SUMO_ATTR_ARRIVALLANE, ret->id.c_str(), ok);
        int lane;
        ArrivalLaneDefinition ald;
        if (SUMOVehicleParameter::parseArrivalLane(helper, element, ret->id, lane, ald, error)) {
            ret->parametersSet |= VEHPARS_ARRIVALLANE_SET;
            ret->arrivalLane = lane;
            ret->arrivalLaneProcedure = ald;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse arrival position information
    if (attrs.hasAttribute(SUMO_ATTR_ARRIVALPOS)) {
        std::string helper = attrs.get<std::string>(SUMO_ATTR_ARRIVALPOS, ret->id.c_str(), ok);
        double pos;
        ArrivalPosDefinition apd;
        if (SUMOVehicleParameter::parseArrivalPos(helper, element, ret->id, pos, apd, error)) {
            ret->parametersSet |= VEHPARS_ARRIVALPOS_SET;
            ret->arrivalPos = pos;
            ret->arrivalPosProcedure = apd;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse lateral arrival position information
    if (attrs.hasAttribute(SUMO_ATTR_ARRIVALPOS_LAT)) {
        std::string helper = attrs.get<std::string>(SUMO_ATTR_ARRIVALPOS_LAT, ret->id.c_str(), ok);
        double pos;
        ArrivalPosLatDefinition apd;
        if (SUMOVehicleParameter::parseArrivalPosLat(helper, element, ret->id, pos, apd, error)) {
            ret->parametersSet |= VEHPARS_ARRIVALPOSLAT_SET;
            ret->arrivalPosLat = pos;
            ret->arrivalPosLatProcedure = apd;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse arrival speed information
    if (attrs.hasAttribute(SUMO_ATTR_ARRIVALSPEED)) {
        std::string helper = attrs.get<std::string>(SUMO_ATTR_ARRIVALSPEED, ret->id.c_str(), ok);
        double speed;
        ArrivalSpeedDefinition asd;
        if (SUMOVehicleParameter::parseArrivalSpeed(helper, element, ret->id, speed, asd, error)) {
            ret->parametersSet |= VEHPARS_ARRIVALSPEED_SET;
            ret->arrivalSpeed = speed;
            ret->arrivalSpeedProcedure = asd;
        } else {
            handleError(hardFail, abortCreation, error);
        }
    }
    // parse color
    if (attrs.hasAttribute(SUMO_ATTR_COLOR)) {
        ret->parametersSet |= VEHPARS_COLOR_SET;
        ret->color = attrs.get<RGBColor>(SUMO_ATTR_COLOR, ret->id.c_str(), ok);
    } else {
        ret->color = RGBColor::DEFAULT_COLOR;
    }
    // parse person number
    if (attrs.hasAttribute(SUMO_ATTR_PERSON_NUMBER)) {
        int personNumber = attrs.get<int>(SUMO_ATTR_PERSON_NUMBER, ret->id.c_str(), ok);
        if (personNumber >= 0) {
            ret->parametersSet |= VEHPARS_PERSON_NUMBER_SET;
            ret->personNumber = personNumber;
        } else {
            handleError(hardFail, abortCreation, toString(SUMO_ATTR_PERSON_NUMBER) + " cannot be negative");
        }
    }
    // parse container number
    if (attrs.hasAttribute(SUMO_ATTR_CONTAINER_NUMBER)) {
        int containerNumber = attrs.get<int>(SUMO_ATTR_CONTAINER_NUMBER, ret->id.c_str(), ok);
        if (containerNumber >= 0) {
            ret->parametersSet |= VEHPARS_CONTAINER_NUMBER_SET;
            ret->containerNumber = containerNumber;
        } else {
            handleError(hardFail, abortCreation, toString(SUMO_ATTR_CONTAINER_NUMBER) + " cannot be negative");
        }
    }
    // parse individual speedFactor
    if (attrs.hasAttribute(SUMO_ATTR_SPEEDFACTOR)) {
        double speedFactor = attrs.get<double>(SUMO_ATTR_SPEEDFACTOR, ret->id.c_str(), ok);
        if (speedFactor > 0) {
            ret->parametersSet |= VEHPARS_SPEEDFACTOR_SET;
            ret->speedFactor = speedFactor;
        } else {
            handleError(hardFail, abortCreation, toString(SUMO_ATTR_SPEEDFACTOR) + " must be positive");
        }
    }
    /*/ parse via
    if (attrs.hasAttribute(SUMO_ATTR_VIA)) {
        ret->setParameter |= VEHPARS_VIA_SET;
        SUMOSAXAttributes::parseStringVector(attrs.get<std::string>(SUMO_ATTR_VIA, ret->id.c_str(), ok), ret->via);
    }
    */
}


SUMOVTypeParameter*
SUMOVehicleParserHelper::beginVTypeParsing(const SUMOSAXAttributes& attrs, const bool hardFail, const std::string& file) {
    bool abortCreation = true;
    // first obtain ID
    std::string id = parseID(attrs, SUMO_TAG_VTYPE);
    // check if ID is valid
    if (!id.empty()) {
        SUMOVehicleClass vClass = SVC_PASSENGER;
        if (attrs.hasAttribute(SUMO_ATTR_VCLASS)) {
            vClass = parseVehicleClass(attrs, id);
        }
        SUMOVTypeParameter* vtype = new SUMOVTypeParameter(id, vClass);
        try {
            if (attrs.hasAttribute(SUMO_ATTR_VCLASS)) {
                vtype->parametersSet |= VTYPEPARS_VEHICLECLASS_SET;
            }
            if (attrs.hasAttribute(SUMO_ATTR_LENGTH)) {
                bool ok = true;
                double length = attrs.get<double>(SUMO_ATTR_LENGTH, vtype->id.c_str(), ok);
                if (ok) {
                    if (length <= 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_LENGTH) + " must be greater than 0");
                    } else {
                        vtype->length = length;
                        vtype->parametersSet |= VTYPEPARS_LENGTH_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_MINGAP)) {
                bool ok = true;
                double minGap = attrs.get<double>(SUMO_ATTR_MINGAP, vtype->id.c_str(), ok);
                if (ok) {
                    if (minGap < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_MINGAP) + " must be equal or greater than 0");
                    } else {
                        vtype->minGap = minGap;
                        vtype->parametersSet |= VTYPEPARS_MINGAP_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_MAXSPEED)) {
                bool ok = true;
                double maxSpeed = attrs.get<double>(SUMO_ATTR_MAXSPEED, vtype->id.c_str(), ok);
                if (ok) {
                    if (maxSpeed <= 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_MAXSPEED) + " must be greater than 0");
                    } else {
                        vtype->maxSpeed = maxSpeed;
                        vtype->parametersSet |= VTYPEPARS_MAXSPEED_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_SPEEDFACTOR)) {
                bool ok = true;
                vtype->speedFactor.parse(attrs.get<std::string>(SUMO_ATTR_SPEEDFACTOR, vtype->id.c_str(), ok), hardFail);
                if (ok) {
                    vtype->parametersSet |= VTYPEPARS_SPEEDFACTOR_SET;
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_SPEEDDEV)) {
                bool ok = true;
                double speedDev = attrs.get<double>(SUMO_ATTR_SPEEDDEV, vtype->id.c_str(), ok);
                if (ok) {
                    if (speedDev < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_SPEEDDEV) + " must be equal or greater than 0");
                    } else {
                        vtype->speedFactor.getParameter()[1] = speedDev;
                        vtype->parametersSet |= VTYPEPARS_SPEEDFACTOR_SET;
                    }
                }
            }
            // validate speed distribution
            std::string error;
            if (!vtype->speedFactor.isValid(error)) {
                handleError(hardFail, abortCreation, "Invalid speed distribution when parsing vType '" + vtype->id + "' (" + error + ")");
            }
            if (attrs.hasAttribute(SUMO_ATTR_ACTIONSTEPLENGTH)) {
                bool ok = true;
                double actionStepLengthSecs = attrs.get<double>(SUMO_ATTR_ACTIONSTEPLENGTH, vtype->id.c_str(), ok);
                // processActionStepLength(...) function includes warnings
                vtype->actionStepLength = processActionStepLength(actionStepLengthSecs);
                vtype->parametersSet |= VTYPEPARS_ACTIONSTEPLENGTH_SET;
            }
            if (attrs.hasAttribute(SUMO_ATTR_EMISSIONCLASS)) {
                bool ok = true;
                std::string parsedEmissionClass = attrs.getOpt<std::string>(SUMO_ATTR_EMISSIONCLASS, id.c_str(), ok, "");
                // check if emission class is correct
                try {
                    vtype->emissionClass = PollutantsInterface::getClassByName(parsedEmissionClass);
                    vtype->parametersSet |= VTYPEPARS_EMISSIONCLASS_SET;
                } catch (...) {
                    if (hardFail) {
                        throw InvalidArgument(toString(SUMO_ATTR_EMISSIONCLASS) + " with name '" + parsedEmissionClass + "' doesn't exist.");
                    } else {
                        WRITE_ERROR(toString(SUMO_ATTR_EMISSIONCLASS) + " with name '" + parsedEmissionClass + "' doesn't exist.");
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_IMPATIENCE)) {
                bool okString = true;
                if (attrs.get<std::string>(SUMO_ATTR_IMPATIENCE, vtype->id.c_str(), okString) == "off") {
                    vtype->impatience = -std::numeric_limits<double>::max();
                } else {
                    bool okDouble = true;
                    const double impatience = attrs.get<double>(SUMO_ATTR_IMPATIENCE, vtype->id.c_str(), okDouble);
                    if (okDouble) {
                        vtype->impatience = impatience;
                        vtype->parametersSet |= VTYPEPARS_IMPATIENCE_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_WIDTH)) {
                bool ok = true;
                double width = attrs.get<double>(SUMO_ATTR_WIDTH, vtype->id.c_str(), ok);
                if (ok) {
                    if (width <= 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_WIDTH) + " must be greater than 0");
                    } else {
                        vtype->width = width;
                        vtype->parametersSet |= VTYPEPARS_WIDTH_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_HEIGHT)) {
                bool ok = true;
                double height = attrs.get<double>(SUMO_ATTR_HEIGHT, vtype->id.c_str(), ok);
                if (ok) {
                    if (height < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_HEIGHT) + " must be equal or greater than 0");
                    } else {
                        vtype->height = height;
                        vtype->parametersSet |= VTYPEPARS_HEIGHT_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_GUISHAPE)) {
                vtype->shape = parseGuiShape(attrs, vtype->id);
                if (vtype->shape != SVS_UNKNOWN) {
                    vtype->parametersSet |= VTYPEPARS_SHAPE_SET;
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_OSGFILE)) {
                bool ok = true;
                std::string osgFile = attrs.get<std::string>(SUMO_ATTR_OSGFILE, vtype->id.c_str(), ok);
                if (ok) {
                    vtype->osgFile = osgFile;
                    vtype->parametersSet |= VTYPEPARS_OSGFILE_SET;
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_IMGFILE)) {
                bool ok = true;
                std::string imgFile = attrs.get<std::string>(SUMO_ATTR_IMGFILE, vtype->id.c_str(), ok);
                if (ok) {
                    // check relative path
                    if ((imgFile != "") && !FileHelpers::isAbsolute(imgFile)) {
                        imgFile = FileHelpers::getConfigurationRelative(file, imgFile);
                    }
                    vtype->imgFile = imgFile;
                    vtype->parametersSet |= VTYPEPARS_IMGFILE_SET;
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_COLOR)) {
                bool ok = true;
                RGBColor color = attrs.get<RGBColor>(SUMO_ATTR_COLOR, vtype->id.c_str(), ok);
                if (ok) {
                    vtype->color = color;
                    vtype->parametersSet |= VTYPEPARS_COLOR_SET;
                }
            } else {
                vtype->color = RGBColor::YELLOW;
            }
            if (attrs.hasAttribute(SUMO_ATTR_PROB)) {
                bool ok = true;
                double defaultProbability = attrs.get<double>(SUMO_ATTR_PROB, vtype->id.c_str(), ok);
                if (ok) {
                    if (defaultProbability < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_PROB) + " must be equal or greater than 0");
                    } else {
                        vtype->defaultProbability = defaultProbability;
                        vtype->parametersSet |= VTYPEPARS_PROBABILITY_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_LANE_CHANGE_MODEL)) {
                bool ok = true;
                std::string lcmS = attrs.get<std::string>(SUMO_ATTR_LANE_CHANGE_MODEL, vtype->id.c_str(), ok);
                if (lcmS == "JE2013") {
                    WRITE_WARNING("Lane change model 'JE2013' is deprecated. Using default model instead.");
                    lcmS = "default";
                }
                if (SUMOXMLDefinitions::LaneChangeModels.hasString(lcmS)) {
                    vtype->lcModel = SUMOXMLDefinitions::LaneChangeModels.get(lcmS);
                    vtype->parametersSet |= VTYPEPARS_LANE_CHANGE_MODEL_SET;
                } else {
                    handleError(hardFail, abortCreation, "Unknown lane change model '" + lcmS + "' when parsing vType '" + vtype->id + "'");
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_CAR_FOLLOW_MODEL)) {
                bool ok = true;
                const std::string cfmValue = attrs.get<std::string>(SUMO_ATTR_CAR_FOLLOW_MODEL, vtype->id.c_str(), ok);
                if (ok && SUMOXMLDefinitions::CarFollowModels.hasString(cfmValue)) {
                    vtype->cfModel = SUMOXMLDefinitions::CarFollowModels.get(cfmValue);
                    vtype->parametersSet |= VTYPEPARS_CAR_FOLLOW_MODEL;
                } else {
                    handleError(hardFail, abortCreation, "Unknown car following model '" + cfmValue + "' when parsing vType '" + vtype->id + "'");
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_PERSON_CAPACITY)) {
                bool ok = true;
                int personCapacity = attrs.get<int>(SUMO_ATTR_PERSON_CAPACITY, vtype->id.c_str(), ok);
                if (ok) {
                    if (personCapacity < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_PERSON_CAPACITY) + " must be equal or greater than 0");
                    } else {
                        vtype->personCapacity = personCapacity;
                        vtype->parametersSet |= VTYPEPARS_PERSON_CAPACITY;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_CONTAINER_CAPACITY)) {
                bool ok = true;
                int containerCapacity = attrs.get<int>(SUMO_ATTR_CONTAINER_CAPACITY, vtype->id.c_str(), ok);
                if (ok) {
                    if (containerCapacity < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_CONTAINER_CAPACITY) + " must be equal or greater than 0");
                    } else {
                        vtype->containerCapacity = containerCapacity;
                        vtype->parametersSet |= VTYPEPARS_CONTAINER_CAPACITY;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_BOARDING_DURATION)) {
                bool ok = true;
                SUMOTime boardingDuration = attrs.getSUMOTimeReporting(SUMO_ATTR_BOARDING_DURATION, vtype->id.c_str(), ok);
                if (ok) {
                    if (boardingDuration < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_BOARDING_DURATION) + " must be equal or greater than 0");
                    } else {
                        vtype->boardingDuration = boardingDuration;
                        vtype->parametersSet |= VTYPEPARS_BOARDING_DURATION;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_LOADING_DURATION)) {
                bool ok = true;
                SUMOTime loadingDuration = attrs.getSUMOTimeReporting(SUMO_ATTR_LOADING_DURATION, vtype->id.c_str(), ok);
                if (ok) {
                    if (loadingDuration < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_LOADING_DURATION) + " must be equal or greater than 0");
                    } else {
                        vtype->loadingDuration = loadingDuration;
                        vtype->parametersSet |= VTYPEPARS_LOADING_DURATION;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_MAXSPEED_LAT)) {
                bool ok = true;
                double maxSpeedLat = attrs.get<double>(SUMO_ATTR_MAXSPEED_LAT, vtype->id.c_str(), ok);
                if (ok) {
                    if (maxSpeedLat <= 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_MAXSPEED_LAT) + " must be greater than 0");
                    } else {
                        vtype->maxSpeedLat = maxSpeedLat;
                        vtype->parametersSet |= VTYPEPARS_MAXSPEED_LAT_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_MINGAP_LAT)) {
                bool ok = true;
                double minGapLat = attrs.get<double>(SUMO_ATTR_MINGAP_LAT, vtype->id.c_str(), ok);
                if (ok) {
                    if (minGapLat < 0) {
                        handleError(hardFail, abortCreation, toString(SUMO_ATTR_MINGAP_LAT) + " must be equal or greater than 0");
                    } else {
                        vtype->minGapLat = minGapLat;
                        vtype->parametersSet |= VTYPEPARS_MINGAP_LAT_SET;
                    }
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_LATALIGNMENT)) {
                bool ok = true;
                const std::string alignS = attrs.get<std::string>(SUMO_ATTR_LATALIGNMENT, vtype->id.c_str(), ok);
                if (ok && SUMOXMLDefinitions::LateralAlignments.hasString(alignS)) {
                    vtype->latAlignment = SUMOXMLDefinitions::LateralAlignments.get(alignS);
                    vtype->parametersSet |= VTYPEPARS_LATALIGNMENT_SET;
                } else {
                    handleError(hardFail, abortCreation, "Unknown lateral alignment '" + alignS + "' when parsing vType '" + vtype->id + "'");
                }
            }
            if (attrs.hasAttribute(SUMO_ATTR_MANEUVER_ANGLE_TIMES)) {
                bool ok = true;
                const std::string angleTimesS = attrs.get<std::string>(SUMO_ATTR_MANEUVER_ANGLE_TIMES, vtype->id.c_str(), ok);
                if (ok && parseAngleTimesMap(*vtype, angleTimesS, hardFail)) {
                    vtype->parametersSet |= VTYPEPARS_MANEUVER_ANGLE_TIMES_SET;
                } else {
                    handleError(hardFail, abortCreation, "Invalid manoeuver angle times map for vType '" + vtype->id + "'");
                }
            }

            // try to parse embedded vType
            if (!parseVTypeEmbedded(*vtype, vtype->cfModel, attrs, hardFail, true)) {
                handleError(hardFail, abortCreation, "Invalid parsing embedded VType");
            }
            // try to parse Lane Change Model params
            if (!parseLCParams(*vtype, vtype->lcModel, attrs, hardFail)) {
                handleError(hardFail, abortCreation, "Invalid Lane Change Model Parameters");
            }
            // try to Junction Model params
            if (!parseJMParams(*vtype, attrs, hardFail)) {
                handleError(hardFail, abortCreation, "Invalid Junction Model Parameters");
            }
        } catch (ProcessError&) {
            delete vtype;
            throw;
        }
        if (!abortCreation) {
            delete vtype;
            if (hardFail) {
                throw ProcessError();
            } else {
                return nullptr;
            }
        }
        return vtype;
    } else {
        if (hardFail) {
            throw ProcessError("VType cannot be created");
        } else {
            return nullptr;
        }
    }
}

bool
SUMOVehicleParserHelper::parseAngleTimesMap(SUMOVTypeParameter& vtype, const std::string atm, const bool hardFail) {
    StringTokenizer st(atm, ",");
    std::map<int, std::pair<SUMOTime, SUMOTime>> angleTimesMap;
    int tripletCount = 0;

    while (st.hasNext()) {
        StringTokenizer pos(st.next());
        if (pos.size() != 3) {
            if (hardFail) {
                throw ProcessError("manoeuverAngleTimes format for vType '" + vtype.id + "' " + atm + " contains an invalid triplet.");
            } else {
                WRITE_ERROR("manoeuverAngleTimes format for vType '" + vtype.id + "' " + atm + " contains an invalid triplet.");
            }
        } else {
            try {
                int angle = StringUtils::toInt(pos.next());
                SUMOTime t1 = static_cast<SUMOTime>(StringUtils::toDouble(pos.next()));
                t1 = TIME2STEPS(t1);
                SUMOTime t2 = static_cast<SUMOTime>(StringUtils::toDouble(pos.next()));
                t2 = TIME2STEPS(t2);

                angleTimesMap.insert((std::pair<int, std::pair<SUMOTime, SUMOTime>>(angle, std::pair< SUMOTime, SUMOTime>(t1, t2))));
            } catch (...) {
                WRITE_ERROR("Triplet '" + st.get(tripletCount) + "' for vType '" + vtype.id + "' manoeuverAngleTimes cannot be parsed as 'int double double'");
            }
            tripletCount++;
        }
    }

    if (angleTimesMap.size() > 0) {
        vtype.myManoeuverAngleTimes.clear();
        for (std::pair<int, std::pair<SUMOTime, SUMOTime>> angleTime : angleTimesMap) {
            vtype.myManoeuverAngleTimes.insert(std::pair<int, std::pair<SUMOTime, SUMOTime>>(angleTime));
        }
        angleTimesMap.clear();
        return true;
    } else {
        return false;
    }
}


bool
SUMOVehicleParserHelper::parseVTypeEmbedded(SUMOVTypeParameter& into, const SumoXMLTag element, const SUMOSAXAttributes& attrs, const bool hardFail, const bool fromVType) {
    const CFAttrMap& allowedCFM = getAllowedCFModelAttrs();
    bool abortCreation = true;
    CFAttrMap::const_iterator cf_it = allowedCFM.find(element);
    // check if given CFM is allowed
    if (cf_it == allowedCFM.end()) {
        if (SUMOXMLDefinitions::Tags.has((int)element)) {
            handleError(hardFail, abortCreation, "Unknown car following model " + toString(element) + " when parsing vType '" + into.id + "'");
        } else {
            handleError(hardFail, abortCreation, "Unknown car following model when parsing vType '" + into.id + "'");
        }
        return false;
    }
    // set car following model
    if (!fromVType) {
        into.cfModel = cf_it->first;
        into.parametersSet |= VTYPEPARS_CAR_FOLLOW_MODEL;
    }
    // set CFM values
    bool ok = true;
    for (const auto& it : cf_it->second) {
        if (attrs.hasAttribute(it)) {
            // first obtain  CFM attribute in string format
            std::string parsedCFMAttribute = attrs.get<std::string>(it, into.id.c_str(), ok);
            // check if attribute is of type "train"
            if (it == SUMO_ATTR_TRAIN_TYPE) {
                // check if train value is valid
                if (SUMOXMLDefinitions::TrainTypes.hasString(parsedCFMAttribute)) {
                    // add parsedCFMAttribute to cfParameter
                    into.cfParameter[it] = parsedCFMAttribute;
                } else if (hardFail) {
                    throw ProcessError("Invalid train type '" + parsedCFMAttribute + "' used in Car-Following-Attribute " + toString(it));
                } else {
                    WRITE_ERROR("Invalid train type '" + parsedCFMAttribute + "' used in Car-Following-Attribute " + toString(it));
                }
            } else if (it == SUMO_ATTR_CF_IDM_STEPPING) {
                // declare a int in wich save CFM int attribute
                int CFMIntAttribute = -1;
                try {
                    // obtain CFM attribute in int format
                    CFMIntAttribute = StringUtils::toInt(parsedCFMAttribute);
                } catch (...) {
                    ok = false;
                    if (hardFail) {
                        throw ProcessError("Invalid Car-Following-Model Attribute " + toString(it) + ". Cannot be parsed to int");
                    } else {
                        WRITE_ERROR("Invalid Car-Following-Model Attribute " + toString(it) + ". Cannot be parsed to int");
                    }
                }
                // now continue checking other properties
                if (ok) {
                    if (CFMIntAttribute <= 0) {
                        ok = false;
                        if (hardFail) {
                            throw ProcessError("Invalid Car-Following-Model Attribute " + toString(it) + ". Must be greater than 0");
                        } else {
                            WRITE_ERROR("Invalid Car-Following-Model Attribute " + toString(it) + ". Must be greater than 0");
                        }
                    }
                    if (ok) {
                        // add parsedCFMAttribute to cfParameter
                        into.cfParameter[it] = parsedCFMAttribute;
                    }
                }
            } else {
                // declare a double in wich save CFM float attribute
                double CFMDoubleAttribute = -1;
                try {
                    // obtain CFM attribute in double format
                    CFMDoubleAttribute = StringUtils::toDouble(parsedCFMAttribute);
                } catch (...) {
                    ok = false;
                    if (hardFail) {
                        throw ProcessError("Invalid Car-Following-Model Attribute " + toString(it) + ". Cannot be parsed to float");
                    } else {
                        WRITE_ERROR("Invalid Car-Following-Model Attribute " + toString(it) + ". Cannot be parsed to float");
                    }
                    if (ok) {
                        // add parsedCFMAttribute to cfParameter
                        into.cfParameter[it] = parsedCFMAttribute;
                    }
                }
                // now continue checking other properties
                if (ok) {
                    // check attributes of type "positiveFloatType" (> 0)
                    switch (it) {
                        case SUMO_ATTR_ACCEL:
                        case SUMO_ATTR_DECEL:
                        case SUMO_ATTR_APPARENTDECEL:
                        case SUMO_ATTR_EMERGENCYDECEL:
                        case SUMO_ATTR_TAU:
                            if (CFMDoubleAttribute <= 0) {
                                ok = false;
                                if (hardFail) {
                                    throw ProcessError("Invalid Car-Following-Model Attribute " + toString(it) + ". Must be greater than 0");
                                } else {
                                    WRITE_ERROR("Invalid Car-Following-Model Attribute " + toString(it) + ". Must be greater than 0");
                                }
                            }
                        default:
                            break;
                    }
                    // check attributes restricted to [0-1]
                    switch (it) {
                        case SUMO_ATTR_SIGMA:
                            if ((CFMDoubleAttribute < 0) || (CFMDoubleAttribute > 1)) {
                                ok = false;
                                if (hardFail) {
                                    throw ProcessError("Invalid Car-Following-Model Attribute " + toString(it) + ". Only values between [0-1] are allowed");
                                } else {
                                    WRITE_ERROR("Invalid Car-Following-Model Attribute " + toString(it) + ". Only values between [0-1] are allowed");
                                }
                            }
                        default:
                            break;
                    }
                    // special check for TAU attribute
                    if (it == SUMO_ATTR_TAU) {
                        // check tau in time format
                        if ((string2time(parsedCFMAttribute) < DELTA_T) && gSimulation) {
                            WRITE_WARNING("Value of tau=" + parsedCFMAttribute + " in car following model '" +
                                          toString(into.cfModel) + "' lower than simulation step size may cause collisions");
                        }
                    }
                    if (ok) {
                        // add parsedCFMAttribute to cfParameter
                        into.cfParameter[it] = parsedCFMAttribute;
                    }
                }
            }
        }
    }
    return ok;
}


const SUMOVehicleParserHelper::CFAttrMap&
SUMOVehicleParserHelper::getAllowedCFModelAttrs() {
    // init on first use
    if (allowedCFModelAttrs.size() == 0) {
        std::set<SumoXMLAttr> kraussParams;
        kraussParams.insert(SUMO_ATTR_ACCEL);
        kraussParams.insert(SUMO_ATTR_DECEL);
        kraussParams.insert(SUMO_ATTR_APPARENTDECEL);
        kraussParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        kraussParams.insert(SUMO_ATTR_SIGMA);
        kraussParams.insert(SUMO_ATTR_TAU);
        allowedCFModelAttrs[SUMO_TAG_CF_KRAUSS] = kraussParams;
        allowedCFModelAttrs[SUMO_TAG_CF_KRAUSS_ORIG1] = kraussParams;
        allowedCFModelAttrs[SUMO_TAG_CF_KRAUSS_PLUS_SLOPE] = kraussParams;
        std::set<SumoXMLAttr> allParams(kraussParams);

        std::set<SumoXMLAttr> kraussXParams(kraussParams);
        kraussXParams.insert(SUMO_ATTR_TMP1);
        kraussXParams.insert(SUMO_ATTR_TMP2);
        kraussXParams.insert(SUMO_ATTR_TMP3);
        kraussXParams.insert(SUMO_ATTR_TMP4);
        kraussXParams.insert(SUMO_ATTR_TMP5);
        allowedCFModelAttrs[SUMO_TAG_CF_KRAUSSX] = kraussXParams;
        allParams.insert(kraussXParams.begin(), kraussXParams.end());

        std::set<SumoXMLAttr> smartSKParams;
        smartSKParams.insert(SUMO_ATTR_ACCEL);
        smartSKParams.insert(SUMO_ATTR_DECEL);
        smartSKParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        smartSKParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        smartSKParams.insert(SUMO_ATTR_SIGMA);
        smartSKParams.insert(SUMO_ATTR_TAU);
        smartSKParams.insert(SUMO_ATTR_TMP1);
        smartSKParams.insert(SUMO_ATTR_TMP2);
        smartSKParams.insert(SUMO_ATTR_TMP3);
        smartSKParams.insert(SUMO_ATTR_TMP4);
        smartSKParams.insert(SUMO_ATTR_TMP5);
        allowedCFModelAttrs[SUMO_TAG_CF_SMART_SK] = smartSKParams;
        allParams.insert(smartSKParams.begin(), smartSKParams.end());

        std::set<SumoXMLAttr> daniel1Params;
        daniel1Params.insert(SUMO_ATTR_ACCEL);
        daniel1Params.insert(SUMO_ATTR_DECEL);
        daniel1Params.insert(SUMO_ATTR_EMERGENCYDECEL);
        daniel1Params.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        daniel1Params.insert(SUMO_ATTR_SIGMA);
        daniel1Params.insert(SUMO_ATTR_TAU);
        daniel1Params.insert(SUMO_ATTR_TMP1);
        daniel1Params.insert(SUMO_ATTR_TMP2);
        daniel1Params.insert(SUMO_ATTR_TMP3);
        daniel1Params.insert(SUMO_ATTR_TMP4);
        daniel1Params.insert(SUMO_ATTR_TMP5);
        allowedCFModelAttrs[SUMO_TAG_CF_DANIEL1] = daniel1Params;
        allParams.insert(daniel1Params.begin(), daniel1Params.end());

        std::set<SumoXMLAttr> pwagParams;
        pwagParams.insert(SUMO_ATTR_ACCEL);
        pwagParams.insert(SUMO_ATTR_DECEL);
        pwagParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        pwagParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        pwagParams.insert(SUMO_ATTR_SIGMA);
        pwagParams.insert(SUMO_ATTR_TAU);
        pwagParams.insert(SUMO_ATTR_CF_PWAGNER2009_TAULAST);
        pwagParams.insert(SUMO_ATTR_CF_PWAGNER2009_APPROB);
        allowedCFModelAttrs[SUMO_TAG_CF_PWAGNER2009] = pwagParams;
        allParams.insert(pwagParams.begin(), pwagParams.end());

        std::set<SumoXMLAttr> idmParams;
        idmParams.insert(SUMO_ATTR_ACCEL);
        idmParams.insert(SUMO_ATTR_DECEL);
        idmParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        idmParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        idmParams.insert(SUMO_ATTR_TAU);
        idmParams.insert(SUMO_ATTR_CF_IDM_DELTA);
        idmParams.insert(SUMO_ATTR_CF_IDM_STEPPING);
        allowedCFModelAttrs[SUMO_TAG_CF_IDM] = idmParams;
        allParams.insert(idmParams.begin(), idmParams.end());

        std::set<SumoXMLAttr> idmmParams;
        idmmParams.insert(SUMO_ATTR_ACCEL);
        idmmParams.insert(SUMO_ATTR_DECEL);
        idmmParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        idmmParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        idmmParams.insert(SUMO_ATTR_TAU);
        idmmParams.insert(SUMO_ATTR_CF_IDMM_ADAPT_FACTOR);
        idmmParams.insert(SUMO_ATTR_CF_IDMM_ADAPT_TIME);
        idmmParams.insert(SUMO_ATTR_CF_IDM_STEPPING);
        allowedCFModelAttrs[SUMO_TAG_CF_IDMM] = idmmParams;
        allParams.insert(idmmParams.begin(), idmmParams.end());

        std::set<SumoXMLAttr> bkernerParams;
        bkernerParams.insert(SUMO_ATTR_ACCEL);
        bkernerParams.insert(SUMO_ATTR_DECEL);
        bkernerParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        bkernerParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        bkernerParams.insert(SUMO_ATTR_TAU);
        bkernerParams.insert(SUMO_ATTR_K);
        bkernerParams.insert(SUMO_ATTR_CF_KERNER_PHI);
        allowedCFModelAttrs[SUMO_TAG_CF_BKERNER] = bkernerParams;
        allParams.insert(bkernerParams.begin(), bkernerParams.end());

        std::set<SumoXMLAttr> wiedemannParams;
        wiedemannParams.insert(SUMO_ATTR_ACCEL);
        wiedemannParams.insert(SUMO_ATTR_DECEL);
        wiedemannParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        wiedemannParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        wiedemannParams.insert(SUMO_ATTR_CF_WIEDEMANN_SECURITY);
        wiedemannParams.insert(SUMO_ATTR_CF_WIEDEMANN_ESTIMATION);
        allowedCFModelAttrs[SUMO_TAG_CF_WIEDEMANN] = wiedemannParams;
        allParams.insert(wiedemannParams.begin(), wiedemannParams.end());

        std::set<SumoXMLAttr> w99Params;
        w99Params.insert(SUMO_ATTR_DECEL); // used when patching speed during lane-changing
        w99Params.insert(SUMO_ATTR_EMERGENCYDECEL);
        w99Params.insert(SUMO_ATTR_CF_W99_CC1);
        w99Params.insert(SUMO_ATTR_CF_W99_CC2);
        w99Params.insert(SUMO_ATTR_CF_W99_CC3);
        w99Params.insert(SUMO_ATTR_CF_W99_CC4);
        w99Params.insert(SUMO_ATTR_CF_W99_CC5);
        w99Params.insert(SUMO_ATTR_CF_W99_CC6);
        w99Params.insert(SUMO_ATTR_CF_W99_CC7);
        w99Params.insert(SUMO_ATTR_CF_W99_CC8);
        w99Params.insert(SUMO_ATTR_CF_W99_CC9);
        allowedCFModelAttrs[SUMO_TAG_CF_W99] = w99Params;
        allParams.insert(w99Params.begin(), w99Params.end());

        std::set<SumoXMLAttr> railParams;
        railParams.insert(SUMO_ATTR_TRAIN_TYPE);
        railParams.insert(SUMO_ATTR_ACCEL);
        railParams.insert(SUMO_ATTR_DECEL);
        railParams.insert(SUMO_ATTR_APPARENTDECEL);
        railParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        railParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        allowedCFModelAttrs[SUMO_TAG_CF_RAIL] = railParams;
        allParams.insert(railParams.begin(), railParams.end());

        std::set<SumoXMLAttr> ACCParams;
        ACCParams.insert(SUMO_ATTR_ACCEL);
        ACCParams.insert(SUMO_ATTR_DECEL);
        ACCParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        ACCParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        ACCParams.insert(SUMO_ATTR_TAU);
        ACCParams.insert(SUMO_ATTR_SC_GAIN);
        ACCParams.insert(SUMO_ATTR_GCC_GAIN_SPEED);
        ACCParams.insert(SUMO_ATTR_GCC_GAIN_SPACE);
        ACCParams.insert(SUMO_ATTR_GC_GAIN_SPEED);
        ACCParams.insert(SUMO_ATTR_GC_GAIN_SPACE);
        ACCParams.insert(SUMO_ATTR_CA_GAIN_SPEED);
        ACCParams.insert(SUMO_ATTR_CA_GAIN_SPACE);
        allowedCFModelAttrs[SUMO_TAG_CF_ACC] = ACCParams;
        allParams.insert(ACCParams.begin(), ACCParams.end());

        std::set<SumoXMLAttr> CACCParams;
        CACCParams.insert(SUMO_ATTR_ACCEL);
        CACCParams.insert(SUMO_ATTR_DECEL);
        CACCParams.insert(SUMO_ATTR_EMERGENCYDECEL);
        CACCParams.insert(SUMO_ATTR_COLLISION_MINGAP_FACTOR);
        CACCParams.insert(SUMO_ATTR_TAU);
        CACCParams.insert(SUMO_ATTR_SC_GAIN_CACC);
        CACCParams.insert(SUMO_ATTR_GCC_GAIN_GAP_CACC);
        CACCParams.insert(SUMO_ATTR_GCC_GAIN_GAP_DOT_CACC);
        CACCParams.insert(SUMO_ATTR_GC_GAIN_GAP_CACC);
        CACCParams.insert(SUMO_ATTR_GC_GAIN_GAP_DOT_CACC);
        CACCParams.insert(SUMO_ATTR_CA_GAIN_GAP_CACC);
        CACCParams.insert(SUMO_ATTR_CA_GAIN_GAP_DOT_CACC);
        CACCParams.insert(SUMO_ATTR_GCC_GAIN_SPEED);
        CACCParams.insert(SUMO_ATTR_GCC_GAIN_SPACE);
        CACCParams.insert(SUMO_ATTR_GC_GAIN_SPEED);
        CACCParams.insert(SUMO_ATTR_GC_GAIN_SPACE);
        CACCParams.insert(SUMO_ATTR_CA_GAIN_SPEED);
        CACCParams.insert(SUMO_ATTR_CA_GAIN_SPACE);
        CACCParams.insert(SUMO_ATTR_HEADWAY_TIME_CACC_TO_ACC);
        allowedCFModelAttrs[SUMO_TAG_CF_CACC] = CACCParams;
        allParams.insert(CACCParams.begin(), CACCParams.end());

        std::set<SumoXMLAttr> ccParams;
        ccParams.insert(SUMO_ATTR_ACCEL);
        ccParams.insert(SUMO_ATTR_DECEL);
        ccParams.insert(SUMO_ATTR_TAU);
        ccParams.insert(SUMO_ATTR_CF_CC_C1);
        ccParams.insert(SUMO_ATTR_CF_CC_CCDECEL);
        ccParams.insert(SUMO_ATTR_CF_CC_CONSTSPACING);
        ccParams.insert(SUMO_ATTR_CF_CC_KP);
        ccParams.insert(SUMO_ATTR_CF_CC_LAMBDA);
        ccParams.insert(SUMO_ATTR_CF_CC_OMEGAN);
        ccParams.insert(SUMO_ATTR_CF_CC_TAU);
        ccParams.insert(SUMO_ATTR_CF_CC_XI);
        ccParams.insert(SUMO_ATTR_CF_CC_LANES_COUNT);
        ccParams.insert(SUMO_ATTR_CF_CC_CCACCEL);
        ccParams.insert(SUMO_ATTR_CF_CC_PLOEG_KP);
        ccParams.insert(SUMO_ATTR_CF_CC_PLOEG_KD);
        ccParams.insert(SUMO_ATTR_CF_CC_PLOEG_H);
        ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_KA);
        ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_KV);
        ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_KP);
        ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_D);
        ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_H);
        allowedCFModelAttrs[SUMO_TAG_CF_CC] = ccParams;
        allParams.insert(ccParams.begin(), ccParams.end());

        allowedCFModelAttrs[SUMO_TAG_NOTHING] = allParams;
    }
    return allowedCFModelAttrs;
}


bool
SUMOVehicleParserHelper::parseLCParams(SUMOVTypeParameter& into, LaneChangeModel model, const SUMOSAXAttributes& attrs, const bool hardFail) {
    if (allowedLCModelAttrs.size() == 0) {
        // init static map
        std::set<SumoXMLAttr> lc2013Params;
        lc2013Params.insert(SUMO_ATTR_LCA_STRATEGIC_PARAM);
        lc2013Params.insert(SUMO_ATTR_LCA_COOPERATIVE_PARAM);
        lc2013Params.insert(SUMO_ATTR_LCA_SPEEDGAIN_PARAM);
        lc2013Params.insert(SUMO_ATTR_LCA_KEEPRIGHT_PARAM);
        lc2013Params.insert(SUMO_ATTR_LCA_OPPOSITE_PARAM);
        lc2013Params.insert(SUMO_ATTR_LCA_LOOKAHEADLEFT);
        lc2013Params.insert(SUMO_ATTR_LCA_SPEEDGAINRIGHT);
        lc2013Params.insert(SUMO_ATTR_LCA_MAXSPEEDLATSTANDING);
        lc2013Params.insert(SUMO_ATTR_LCA_MAXSPEEDLATFACTOR);
        lc2013Params.insert(SUMO_ATTR_LCA_ASSERTIVE);
        lc2013Params.insert(SUMO_ATTR_LCA_SPEEDGAIN_LOOKAHEAD);
        lc2013Params.insert(SUMO_ATTR_LCA_COOPERATIVE_ROUNDABOUT);
        lc2013Params.insert(SUMO_ATTR_LCA_COOPERATIVE_SPEED);
        lc2013Params.insert(SUMO_ATTR_LCA_OVERTAKE_RIGHT);
        lc2013Params.insert(SUMO_ATTR_LCA_SIGMA);
        lc2013Params.insert(SUMO_ATTR_LCA_EXPERIMENTAL1);
        allowedLCModelAttrs[LCM_LC2013] = lc2013Params;

        std::set<SumoXMLAttr> sl2015Params = lc2013Params;
        sl2015Params.insert(SUMO_ATTR_LCA_PUSHY);
        sl2015Params.insert(SUMO_ATTR_LCA_PUSHYGAP);
        sl2015Params.insert(SUMO_ATTR_LCA_SUBLANE_PARAM);
        sl2015Params.insert(SUMO_ATTR_LCA_IMPATIENCE);
        sl2015Params.insert(SUMO_ATTR_LCA_TIME_TO_IMPATIENCE);
        sl2015Params.insert(SUMO_ATTR_LCA_ACCEL_LAT);
        sl2015Params.insert(SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE);
        sl2015Params.insert(SUMO_ATTR_LCA_LANE_DISCIPLINE);
        allowedLCModelAttrs[LCM_SL2015] = sl2015Params;

        std::set<SumoXMLAttr> noParams;
        allowedLCModelAttrs[LCM_DK2008] = noParams;

        // default model may be either LC2013 or SL2015
        // we allow both sets (sl2015 is a superset of lc2013Params)
        allowedLCModelAttrs[LCM_DEFAULT] = sl2015Params;
    }
    bool ok = true;
    std::set<SumoXMLAttr> allowed = allowedLCModelAttrs[model];
    for (const auto& it : allowed) {
        if (attrs.hasAttribute(it)) {
            // first obtain  CFM attribute in string format
            std::string parsedLCMAttribute = attrs.get<std::string>(it, into.id.c_str(), ok);
            // declare a double in wich save CFM attribute
            double LCMAttribute = -1;
            try {
                // obtain CFM attribute in double format
                LCMAttribute = StringUtils::toDouble(parsedLCMAttribute);
            } catch (...) {
                ok = false;
                if (hardFail) {
                    throw ProcessError("Invalid Lane-Change-Model Attribute " + toString(it) + ". Cannot be parsed to float");
                } else {
                    WRITE_ERROR("Invalid Lane-Change-Model Attribute " + toString(it) + ". Cannot be parsed to float");
                }
            }
            // now continue checking other properties
            if (ok) {
                // check attributes of type "nonNegativeFloatType" (>= 0)
                switch (it) {
                    case SUMO_ATTR_LCA_PUSHYGAP:
                    case SUMO_ATTR_LCA_MAXSPEEDLATSTANDING:
                    case SUMO_ATTR_LCA_IMPATIENCE:
                    case SUMO_ATTR_LCA_OVERTAKE_RIGHT:
                    case SUMO_ATTR_LCA_ASSERTIVE:
                    case SUMO_ATTR_LCA_LOOKAHEADLEFT:
                    case SUMO_ATTR_LCA_SPEEDGAINRIGHT:
                    case SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE:
                    case SUMO_ATTR_LCA_TIME_TO_IMPATIENCE:
                    case SUMO_ATTR_LCA_LANE_DISCIPLINE:
                    case SUMO_ATTR_LCA_SIGMA:
                        if (LCMAttribute < 0) {
                            ok = false;
                            if (hardFail) {
                                throw ProcessError("Invalid Lane-Change-Model Attribute " + toString(it) + ". Must be equal or greater than 0");
                            } else {
                                WRITE_ERROR("Invalid Lane-Change-Model Attribute " + toString(it) + ". Must be equal or greater than 0");
                            }
                        }
                    default:
                        break;
                }
                // check attributes of type "positiveFloatType" (> 0)
                switch (it) {
                    case SUMO_ATTR_LCA_ACCEL_LAT:
                    case SUMO_ATTR_LCA_MAXSPEEDLATFACTOR:
                        if (LCMAttribute <= 0) {
                            ok = false;
                            if (hardFail) {
                                throw ProcessError("Invalid Lane-Change-Model Attribute " + toString(it) + ". Must be greater than 0");
                            } else {
                                WRITE_ERROR("Invalid Lane-Change-Model Attribute " + toString(it) + ". Must be greater than 0");
                            }
                        }
                    default:
                        break;
                }
                if (ok) {
                    // add parsedLCMAttribute to cfParameter
                    into.lcParameter[it] = parsedLCMAttribute;
                }
            }
        }
    }
    return ok;
}


bool
SUMOVehicleParserHelper::parseJMParams(SUMOVTypeParameter& into, const SUMOSAXAttributes& attrs, const bool hardFail) {
    if (allowedJMAttrs.size() == 0) {
        // init static set (there is only one model)
        allowedJMAttrs.insert(SUMO_ATTR_JM_CROSSING_GAP);
        allowedJMAttrs.insert(SUMO_ATTR_JM_DRIVE_AFTER_YELLOW_TIME);
        allowedJMAttrs.insert(SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME);
        allowedJMAttrs.insert(SUMO_ATTR_JM_DRIVE_RED_SPEED);
        allowedJMAttrs.insert(SUMO_ATTR_JM_IGNORE_KEEPCLEAR_TIME);
        allowedJMAttrs.insert(SUMO_ATTR_JM_IGNORE_FOE_SPEED);
        allowedJMAttrs.insert(SUMO_ATTR_JM_IGNORE_FOE_PROB);
        allowedJMAttrs.insert(SUMO_ATTR_JM_SIGMA_MINOR);
        allowedJMAttrs.insert(SUMO_ATTR_JM_TIMEGAP_MINOR);
    }
    bool ok = true;
    for (const auto& it : allowedJMAttrs) {
        if (attrs.hasAttribute(it)) {
            // first obtain  CFM attribute in string format
            std::string parsedJMAttribute = attrs.get<std::string>(it, into.id.c_str(), ok);
            // declare a double in wich save CFM attribute
            double JMAttribute = -1;
            try {
                // obtain CFM attribute in double format
                JMAttribute = StringUtils::toDouble(parsedJMAttribute);
            } catch (...) {
                ok = false;
                if (hardFail) {
                    throw ProcessError("Invalid Junction-Model Attribute " + toString(it) + ". Cannot be parsed to float");
                } else {
                    WRITE_ERROR("Invalid Junction-Model Attribute " + toString(it) + ". Cannot be parsed to float");
                }
            }
            // now continue checking other properties (-1 is the default value)
            if (ok && (JMAttribute != -1)) {
                // special case for sigma minor
                if (it == SUMO_ATTR_JM_SIGMA_MINOR) {
                    // check attributes sigma minor
                    if ((JMAttribute < 0) || (JMAttribute > 1)) {
                        ok = false;
                        if (hardFail) {
                            throw ProcessError("Invalid Junction-Model Attribute " + toString(it) + ". Only values between [0-1] are allowed");
                        } else {
                            WRITE_ERROR("Invalid Junction-Model Attribute " + toString(it) + ". Only values between [0-1] are allowed");
                        }
                    }
                } else {
                    // check attributes of type "nonNegativeFloatType" (>= 0)
                    if (JMAttribute < 0) {
                        ok = false;
                        if (hardFail) {
                            throw ProcessError("Invalid Junction-Model Attribute " + toString(it) + ". Must be equal or greater than 0");
                        } else {
                            WRITE_ERROR("Invalid Junction-Model Attribute " + toString(it) + ". Must be equal or greater than 0");
                        }
                    }
                }
                if (ok) {
                    // add parsedJMAttribute to cfParameter
                    into.jmParameter[it] = parsedJMAttribute;
                }
            }
        }
    }
    return ok;
}


SUMOVehicleClass
SUMOVehicleParserHelper::parseVehicleClass(const SUMOSAXAttributes& attrs, const std::string& id) {
    SUMOVehicleClass vclass = SVC_IGNORING;
    bool ok = true;
    std::string vclassS = attrs.getOpt<std::string>(SUMO_ATTR_VCLASS, id.c_str(), ok, "");
    if (vclassS == "") {
        return vclass;
    }
    try {
        const SUMOVehicleClass result = getVehicleClassID(vclassS);
        const std::string& realName = SumoVehicleClassStrings.getString(result);
        if (realName != vclassS) {
            WRITE_WARNING("The vehicle class '" + vclassS + "' for " + attrs.getObjectType() + " '" + id + "' is deprecated, use '" + realName + "' instead.");
        }
        return result;
    } catch (...) {
        WRITE_ERROR("The vehicle class '" + vclassS + "' for " + attrs.getObjectType() + " '" + id + "' is not known.");
    }
    return vclass;
}


SUMOVehicleShape
SUMOVehicleParserHelper::parseGuiShape(const SUMOSAXAttributes& attrs, const std::string& id) {
    bool ok = true;
    std::string vclassS = attrs.getOpt<std::string>(SUMO_ATTR_GUISHAPE, id.c_str(), ok, "");
    if (SumoVehicleShapeStrings.hasString(vclassS)) {
        const SUMOVehicleShape result = SumoVehicleShapeStrings.get(vclassS);
        const std::string& realName = SumoVehicleShapeStrings.getString(result);
        if (realName != vclassS) {
            WRITE_WARNING("The shape '" + vclassS + "' for " + attrs.getObjectType() + " '" + id + "' is deprecated, use '" + realName + "' instead.");
        }
        return result;
    } else {
        WRITE_ERROR("The shape '" + vclassS + "' for " + attrs.getObjectType() + " '" + id + "' is not known.");
        return SVS_UNKNOWN;
    }
}


double
SUMOVehicleParserHelper::parseWalkPos(SumoXMLAttr attr, const bool hardFail, const std::string& id, double maxPos, const std::string& val, std::mt19937* rng) {
    double result;
    std::string error;
    ArrivalPosDefinition proc;
    // only supports 'random' and 'max'
    if (!SUMOVehicleParameter::parseArrivalPos(val, toString(SUMO_TAG_WALK), id, result, proc, error)) {
        if (hardFail) {
            throw ProcessError(error);
        } else {
            WRITE_ERROR(error);
        }
    }
    if (proc == ArrivalPosDefinition::RANDOM) {
        result = RandHelper::rand(maxPos, rng);
    } else if (proc == ArrivalPosDefinition::CENTER) {
        result = maxPos / 2.;
    } else if (proc == ArrivalPosDefinition::MAX) {
        result = maxPos;
    }
    return SUMOVehicleParameter::interpretEdgePos(result, maxPos, attr, id);
}


SUMOTime
SUMOVehicleParserHelper::processActionStepLength(double given) {
    SUMOTime result = TIME2STEPS(given);
    if (result <= 0) {
        if (result < 0) {
            std::stringstream ss;
            ss << "The parameter action-step-length must be a non-negative multiple of the simulation step-length. Ignoring given value (="
               <<  STEPS2TIME(result) << " s.)";
            WRITE_WARNING(ss.str());
        }
        result = DELTA_T;
    } else if (result % DELTA_T != 0) {
        std::stringstream ss;
        result = (SUMOTime)(DELTA_T * floor(double(result) / double(DELTA_T)));
        result = MAX2(DELTA_T, result);
        if (fabs(given * 1000. - double(result)) > NUMERICAL_EPS) {
            ss << "The parameter action-step-length must be a non-negative multiple of the simulation step-length. Parsing given value ("
               << given << " s.) to the adjusted value "
               <<  STEPS2TIME(result) << " s.";
            WRITE_WARNING(ss.str());
        }
    }
    return result;
}


SUMOVehicleParameter*
SUMOVehicleParserHelper::handleError(const bool hardFail, bool& abortCreation, const std::string& message) {
    if (hardFail) {
        abortCreation = true;
        throw ProcessError(message);
    } else {
        WRITE_ERROR(message);
        return nullptr;
    }
}


/****************************************************************************/
