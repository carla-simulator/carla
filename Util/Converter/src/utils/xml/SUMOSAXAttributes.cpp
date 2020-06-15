/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMOSAXAttributes.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Fri, 30 Mar 2007
///
// Encapsulated SAX-Attributes
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <sstream>
#include <utils/common/MsgHandler.h>
#include <utils/common/RGBColor.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/PositionVector.h>
#include "SUMOSAXAttributes.h"


// ===========================================================================
// static members
// ===========================================================================
const std::string SUMOSAXAttributes::ENCODING = " encoding=\"UTF-8\"";


// ===========================================================================
// method definitions
// ===========================================================================
SUMOSAXAttributes::SUMOSAXAttributes(const std::string& objectType):
    myObjectType(objectType) {}


const std::string invalid_return<std::string>::value = "";
const std::string invalid_return<std::string>::type = "string";
template<>
std::string SUMOSAXAttributes::getInternal(const int attr) const {
    const std::string ret = getString(attr);
    if (ret == "") {
        throw EmptyData();
    }
    return ret;
}


SUMOTime
SUMOSAXAttributes::getSUMOTimeReporting(int attr, const char* objectid,
                                        bool& ok, bool report) const {
    if (!hasAttribute(attr)) {
        if (report) {
            emitUngivenError(getName(attr), objectid);
        }
        ok = false;
        return -1;
    }
    try {
        const std::string val = getInternal<std::string>(attr);
        return string2time(val);
    } catch (EmptyData&) {
        if (report) {
            emitEmptyError(getName(attr), objectid);
        }
    } catch (ProcessError&) {
        if (report) {
            emitFormatError(getName(attr), "a time value", objectid);
        }
    }
    ok = false;
    return (SUMOTime) - 1;
}


SUMOTime
SUMOSAXAttributes::getOptSUMOTimeReporting(int attr, const char* objectid,
        bool& ok, SUMOTime defaultValue, bool report) const {
    if (!hasAttribute(attr)) {
        return defaultValue;
    }
    try {
        const std::string val = getInternal<std::string>(attr);
        return string2time(val);
    } catch (EmptyData&) {
        if (report) {
            emitEmptyError(getName(attr), objectid);
        }
    } catch (ProcessError&) {
        if (report) {
            emitFormatError(getName(attr), "a real number", objectid);
        }
    }
    ok = false;
    return (SUMOTime) - 1;
}


const std::vector<std::string>
SUMOSAXAttributes::getStringVector(int attr) const {
    const std::vector<std::string>& ret = StringTokenizer(getString(attr)).getVector();
    if (ret.empty()) {
        throw EmptyData();
    }
    return ret;
}


const std::vector<std::string>
SUMOSAXAttributes::getOptStringVector(int attr, const char* objectid, bool& ok, bool report) const {
    return getOpt<std::vector<std::string> >(attr, objectid, ok, std::vector<std::string>(), report);
}

const std::vector<int>
SUMOSAXAttributes::getIntVector(int attr) const {
    const std::vector<std::string>& tmp = StringTokenizer(getString(attr)).getVector();
    if (tmp.empty()) {
        throw EmptyData();
    }
    std::vector<int> ret;
    for (const std::string& s : tmp) {
        ret.push_back(StringUtils::toInt(s));
    }
    return ret;
}


const std::vector<int>
SUMOSAXAttributes::getOptIntVector(int attr, const char* objectid, bool& ok, bool report) const {
    return getOpt<std::vector<int> >(attr, objectid, ok, std::vector<int>(), report);
}

void
SUMOSAXAttributes::emitUngivenError(const std::string& attrname, const char* objectid) const {
    std::ostringstream oss;
    oss << "Attribute '" << attrname << "' is missing in definition of ";
    if (objectid == nullptr || objectid[0] == 0) {
        oss << "a " << myObjectType;
    } else {
        oss << myObjectType << " '" << objectid << "'";
    }
    oss << ".";
    WRITE_ERROR(oss.str());
}


void
SUMOSAXAttributes::emitEmptyError(const std::string& attrname, const char* objectid) const {
    std::ostringstream oss;
    oss << "Attribute '" << attrname << "' in definition of ";
    if (objectid == nullptr || objectid[0] == 0) {
        oss << "a " << myObjectType;
    } else {
        oss << myObjectType << " '" << objectid << "'";
    }
    oss << " is empty.";
    WRITE_ERROR(oss.str());
}


void
SUMOSAXAttributes::emitFormatError(const std::string& attrname, const std::string& type, const char* objectid) const {
    std::ostringstream oss;
    oss << "Attribute '" << attrname << "' in definition of ";
    if (objectid == nullptr || objectid[0] == 0) {
        oss << "a " << myObjectType;
    } else {
        oss << myObjectType << " '" << objectid << "'";
    }
    oss << " is not " << type << ".";
    WRITE_ERROR(oss.str());
}


const int invalid_return<int>::value = -1;
const std::string invalid_return<int>::type = "int";
template<>
int SUMOSAXAttributes::getInternal(const int attr) const {
    return getInt(attr);
}


const long long int invalid_return<long long int>::value = -1;
const std::string invalid_return<long long int>::type = "long";
template<>
long long int SUMOSAXAttributes::getInternal(const int attr) const {
    return getLong(attr);
}


const double invalid_return<double>::value = -1;
const std::string invalid_return<double>::type = "float";
template<>
double SUMOSAXAttributes::getInternal(const int attr) const {
    return getFloat(attr);
}


const bool invalid_return<bool>::value = false;
const std::string invalid_return<bool>::type = "bool";
template<>
bool SUMOSAXAttributes::getInternal(const int attr) const {
    return getBool(attr);
}


const RGBColor invalid_return<RGBColor>::value = RGBColor();
const std::string invalid_return<RGBColor>::type = "color";
template<>
RGBColor SUMOSAXAttributes::getInternal(const int /* attr */) const {
    return getColor();
}


const PositionVector invalid_return<PositionVector>::value = PositionVector();
const std::string invalid_return<PositionVector>::type = "PositionVector";
template<>
PositionVector SUMOSAXAttributes::getInternal(const int attr) const {
    return getShape(attr);
}


const Boundary invalid_return<Boundary>::value = Boundary();
const std::string invalid_return<Boundary>::type = "Boundary";
template<>
Boundary SUMOSAXAttributes::getInternal(const int attr) const {
    return getBoundary(attr);
}


const std::vector<std::string> invalid_return<std::vector<std::string> >::value = std::vector<std::string>();
const std::string invalid_return<std::vector<std::string> >::type = "StringVector";
template<>
std::vector<std::string> SUMOSAXAttributes::getInternal(const int attr) const {
    return getStringVector(attr);
}


const std::vector<int> invalid_return<std::vector<int> >::value = std::vector<int>();
const std::string invalid_return<std::vector<int> >::type = "StringVector";
template<>
std::vector<int> SUMOSAXAttributes::getInternal(const int attr) const {
    return getIntVector(attr);
}


/****************************************************************************/
