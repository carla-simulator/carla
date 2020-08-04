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
/// @file    Parameterised.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// A super class for objects with additional parameters
/****************************************************************************/
#include <config.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StringTokenizer.h>
#include <utils/iodevices/OutputDevice.h>

#include "Parameterised.h"


// ===========================================================================
// method definitions
// ===========================================================================

Parameterised::Parameterised() :
    myAttrType(ParameterisedAttrType::STRING) {
}


Parameterised::Parameterised(ParameterisedAttrType attrType) :
    myAttrType(attrType) {
}


Parameterised::Parameterised(const std::map<std::string, std::string>& mapArg) :
    myAttrType(ParameterisedAttrType::STRING),
    myMap(mapArg) {
}


Parameterised::Parameterised(ParameterisedAttrType attrType, const std::map<std::string, std::string>& mapArg) :
    myAttrType(attrType) {
    // check if map has to be cleaned
    if (myAttrType == ParameterisedAttrType::DOUBLE) {
        // iterate over map
        for (const auto& keyValue : mapArg) {
            try {
                // try to parse to do double, and if fails, write warning
                StringUtils::toDouble(keyValue.second);
                // insert keyValue in map
                myMap.insert(keyValue);
            } catch (NumberFormatException&) {
                WRITE_WARNING("Invalid conversion from string to double (" + keyValue.second + ")");
            }
        }
    } else {
        // just update myMap
        myMap = mapArg;
    }
}


Parameterised::~Parameterised() {}


void
Parameterised::setParameter(const std::string& key, const std::string& value) {
    if (myAttrType == ParameterisedAttrType::DOUBLE) {
        try {
            // try to parse to do double, and if fails, write warning
            StringUtils::toDouble(value);
            // insert in map
            myMap[key] = value;
        } catch (NumberFormatException&) {
            WRITE_WARNING("Invalid conversion from string to double (" + value + ")");
        }
    } else {
        myMap[key] = value;
    }
}


void
Parameterised::unsetParameter(const std::string& key) {
    myMap.erase(key);
}


void
Parameterised::updateParameters(const std::map<std::string, std::string>& mapArg) {
    for (const auto& keyValue : mapArg) {
        setParameter(keyValue.first, keyValue.second);
    }
}


bool
Parameterised::knowsParameter(const std::string& key) const {
    return myMap.find(key) != myMap.end();
}


const std::string
Parameterised::getParameter(const std::string& key, const std::string defaultValue) const {
    const auto i = myMap.find(key);
    if (i != myMap.end()) {
        return i->second;
    }
    return defaultValue;
}


double
Parameterised::getDouble(const std::string& key, const double defaultValue) const {
    const auto i = myMap.find(key);
    if (i != myMap.end()) {
        try {
            return StringUtils::toDouble(i->second);
        } catch (NumberFormatException&) {
            WRITE_WARNING("Invalid conversion from string to double (" + i->second + ")");
            return defaultValue;
        } catch (EmptyData&) {
            WRITE_WARNING("Invalid conversion from string to double (empty value)");
            return defaultValue;
        }
    }
    return defaultValue;
}


void
Parameterised::clearParameter() {
    myMap.clear();
}


const std::map<std::string, std::string>&
Parameterised::getParametersMap() const {
    return myMap;
}


std::string
Parameterised::getParametersStr(const std::string kvsep, const std::string sep) const {
    std::string result;
    // Generate an string using configurable seperatrs, default: "key1=value1|key2=value2|...|keyN=valueN"
    bool addSep = false;
    for (const auto& keyValue : myMap) {
        if (addSep) {
            result += sep;
        }
        result += keyValue.first + kvsep + keyValue.second;
        addSep = true;
    }
    return result;
}


void
Parameterised::setParameters(const Parameterised& params) {
    // first clear map
    myMap.clear();
    // set parameter
    for (const auto& keyValue : params.getParametersMap()) {
        setParameter(keyValue.first, keyValue.second);
    }
}


void
Parameterised::setParametersMap(const std::map<std::string, std::string>& paramsMap) {
    // first clear map
    myMap.clear();
    // set parameter
    for (const auto& keyValue : paramsMap) {
        setParameter(keyValue.first, keyValue.second);
    }
}


void
Parameterised::setParametersStr(const std::string& paramsString, const std::string kvsep, const std::string sep) {
    // clear parameters
    myMap.clear();
    // separate value in a vector of string using | as separator
    std::vector<std::string> parameters = StringTokenizer(paramsString, sep).getVector();
    // iterate over all values
    for (const auto& keyValue : parameters) {
        // obtain key and value and save it in myParameters
        std::vector<std::string> keyValueStr = StringTokenizer(keyValue, kvsep).getVector();
        setParameter(keyValueStr.front(), keyValueStr.back());
    }
}


void
Parameterised::writeParams(OutputDevice& device) const {
    // iterate over all parameters and write it
    for (const auto& keyValue : myMap) {
        device.openTag(SUMO_TAG_PARAM);
        device.writeAttr(SUMO_ATTR_KEY, StringUtils::escapeXML(keyValue.first));
        device.writeAttr(SUMO_ATTR_VALUE, StringUtils::escapeXML(keyValue.second));
        device.closeTag();
    }
}


bool
Parameterised::areParametersValid(const std::string& value, bool report,
                                  const ParameterisedAttrType attrType, const std::string kvsep, const std::string sep) {
    std::vector<std::string> parameters = StringTokenizer(value, sep).getVector();
    // first check if parsed parameters are valid
    for (const auto& keyValueStr : parameters) {
        // check if parameter is valid
        if (!isParameterValid(keyValueStr, attrType, kvsep, sep)) {
            // report depending of flag
            if (report) {
                WRITE_WARNING("Invalid format of parameter (" + keyValueStr + ")");
            }
            return false;
        }
    }
    // all ok, then return true
    return true;
}

// ===========================================================================
// private
// ===========================================================================

bool
Parameterised::isParameterValid(const std::string& value, ParameterisedAttrType attrType,
                                const std::string& kvsep, const std::string& sep) {
    if (value.find(sep) != std::string::npos || value.find(kvsep) == std::string::npos) {
        return false;
    }
    // separate key and value
    std::vector<std::string> keyValueStr = StringTokenizer(value, kvsep).getVector();
    // Check that keyValue size is exactly 2 (key, value)
    if (keyValueStr.size() == 2) {
        // check if key and value contains valid characters
        if (SUMOXMLDefinitions::isValidParameterKey(keyValueStr.front()) == false) {
            return false;
        } else if (SUMOXMLDefinitions::isValidParameterValue(keyValueStr.back()) == false) {
            return false;
        } else if (attrType == ParameterisedAttrType::DOUBLE) {
            // check if can be parsed to double
            try {
                StringUtils::toDouble(keyValueStr.back());
                return true;
            } catch (NumberFormatException&) {
                return false;
            }
        } else {
            // key=value valid, then return true
            return true;
        }
    } else {
        // invalid format
        return false;
    }
}

/****************************************************************************/
