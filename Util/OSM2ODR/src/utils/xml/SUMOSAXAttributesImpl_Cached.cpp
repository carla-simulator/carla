/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMOSAXAttributesImpl_Cached.cpp
/// @author  Jakob Erdmann
/// @date    Dec 2016
///
// Encapsulated xml-attributes that use a map from string-attr-names to string-attr-values as backend
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XercesVersion.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <utils/common/RGBColor.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StringBijection.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/PositionVector.h>
#include "SUMOSAXAttributesImpl_Cached.h"
#include "SUMOSAXAttributesImpl_Cached.h"


// ===========================================================================
// class definitions
// ===========================================================================
SUMOSAXAttributesImpl_Cached::SUMOSAXAttributesImpl_Cached(
    const std::map<std::string, std::string>& attrs,
    const std::vector<std::string>& predefinedTagsMML,
    const std::string& objectType) :
    SUMOSAXAttributes(objectType),
    myAttrs(attrs),
    myPredefinedTagsMML(predefinedTagsMML) { }


SUMOSAXAttributesImpl_Cached::SUMOSAXAttributesImpl_Cached(
    const std::map<SumoXMLAttr, std::string>& attrs,
    const std::vector<std::string>& predefinedTagsMML,
    const std::string& objectType) :
    SUMOSAXAttributes(objectType),
    myPredefinedTagsMML(predefinedTagsMML) {
    // parse <SumoXMLAttr, string> to <string, string>
    for (const auto& i : attrs) {
        myAttrs[toString(i.first)] = i.second;
    }
}


SUMOSAXAttributesImpl_Cached::~SUMOSAXAttributesImpl_Cached() { }


bool
SUMOSAXAttributesImpl_Cached::hasAttribute(int id) const {
    assert(id >= 0);
    assert(id < (int)myPredefinedTagsMML.size());
    return myAttrs.find(myPredefinedTagsMML[id]) != myAttrs.end();
}


bool
SUMOSAXAttributesImpl_Cached::getBool(int id) const {
    return StringUtils::toBool(getAttributeValueSecure(id));
}


int
SUMOSAXAttributesImpl_Cached::getInt(int id) const {
    return StringUtils::toInt(getAttributeValueSecure(id));
}


long long int
SUMOSAXAttributesImpl_Cached::getLong(int id) const {
    return StringUtils::toLong(getAttributeValueSecure(id));
}


std::string
SUMOSAXAttributesImpl_Cached::getString(int id) const {
    return getAttributeValueSecure(id);
}


std::string
SUMOSAXAttributesImpl_Cached::getStringSecure(int id, const std::string& str) const {
    const std::string& result = getAttributeValueSecure(id);
    return result.size() == 0 ? str : result;
}


double
SUMOSAXAttributesImpl_Cached::getFloat(int id) const {
    return StringUtils::toDouble(getAttributeValueSecure(id));
}


const std::string&
SUMOSAXAttributesImpl_Cached::getAttributeValueSecure(int id) const {
    assert(id >= 0);
    assert(id < (int)myPredefinedTagsMML.size());
    return myAttrs.find(myPredefinedTagsMML[id])->second;
}


double
SUMOSAXAttributesImpl_Cached::getFloat(const std::string& id) const {
    return StringUtils::toDouble(myAttrs.find(id)->second);
}


bool
SUMOSAXAttributesImpl_Cached::hasAttribute(const std::string& id) const {
    return myAttrs.find(id) != myAttrs.end();
}


std::string
SUMOSAXAttributesImpl_Cached::getStringSecure(const std::string& id,
        const std::string& str) const {
    std::map<std::string, std::string>::const_iterator it = myAttrs.find(id);
    if (it != myAttrs.end() && it->second != "") {
        return it->second;
    } else {
        return str;
    }
}


SumoXMLEdgeFunc
SUMOSAXAttributesImpl_Cached::getEdgeFunc(bool& ok) const {
    if (hasAttribute(SUMO_ATTR_FUNCTION)) {
        std::string funcString = getString(SUMO_ATTR_FUNCTION);
        if (SUMOXMLDefinitions::EdgeFunctions.hasString(funcString)) {
            return SUMOXMLDefinitions::EdgeFunctions.get(funcString);
        }
        ok = false;
    }
    return SumoXMLEdgeFunc::NORMAL;
}


SumoXMLNodeType
SUMOSAXAttributesImpl_Cached::getNodeType(bool& ok) const {
    if (hasAttribute(SUMO_ATTR_TYPE)) {
        std::string typeString = getString(SUMO_ATTR_TYPE);
        if (SUMOXMLDefinitions::NodeTypes.hasString(typeString)) {
            return SUMOXMLDefinitions::NodeTypes.get(typeString);
        }
        ok = false;
    }
    return SumoXMLNodeType::UNKNOWN;
}


RightOfWay
SUMOSAXAttributesImpl_Cached::getRightOfWay(bool& ok) const {
    if (hasAttribute(SUMO_ATTR_RIGHT_OF_WAY)) {
        std::string rowString = getString(SUMO_ATTR_RIGHT_OF_WAY);
        if (SUMOXMLDefinitions::RightOfWayValues.hasString(rowString)) {
            return SUMOXMLDefinitions::RightOfWayValues.get(rowString);
        }
        ok = false;
    }
    return RightOfWay::DEFAULT;
}

FringeType
SUMOSAXAttributesImpl_Cached::getFringeType(bool& ok) const {
    if (hasAttribute(SUMO_ATTR_FRINGE)) {
        std::string fringeString = getString(SUMO_ATTR_FRINGE);
        if (SUMOXMLDefinitions::FringeTypeValues.hasString(fringeString)) {
            return SUMOXMLDefinitions::FringeTypeValues.get(fringeString);
        }
        ok = false;
    }
    return FringeType::DEFAULT;
}

RGBColor
SUMOSAXAttributesImpl_Cached::getColor() const {
    return RGBColor::parseColor(getString(SUMO_ATTR_COLOR));
}


PositionVector
SUMOSAXAttributesImpl_Cached::getShape(int attr) const {
    StringTokenizer st(getString(attr));
    PositionVector shape;
    while (st.hasNext()) {
        StringTokenizer pos(st.next(), ",");
        if (pos.size() != 2 && pos.size() != 3) {
            throw FormatException("shape format");
        }
        double x = StringUtils::toDouble(pos.next());
        double y = StringUtils::toDouble(pos.next());
        if (pos.size() == 2) {
            shape.push_back(Position(x, y));
        } else {
            double z = StringUtils::toDouble(pos.next());
            shape.push_back(Position(x, y, z));
        }
    }
    return shape;
}


Boundary
SUMOSAXAttributesImpl_Cached::getBoundary(int attr) const {
    std::string def = getString(attr);
    StringTokenizer st(def, ",");
    if (st.size() != 4) {
        throw FormatException("boundary format");
    }
    const double xmin = StringUtils::toDouble(st.next());
    const double ymin = StringUtils::toDouble(st.next());
    const double xmax = StringUtils::toDouble(st.next());
    const double ymax = StringUtils::toDouble(st.next());
    return Boundary(xmin, ymin, xmax, ymax);
}


std::string
SUMOSAXAttributesImpl_Cached::getName(int attr) const {
    assert(attr >= 0);
    assert(attr < (int)myPredefinedTagsMML.size());
    return myPredefinedTagsMML[attr];
}


void
SUMOSAXAttributesImpl_Cached::serialize(std::ostream& os) const {
    for (std::map<std::string, std::string>::const_iterator it = myAttrs.begin(); it != myAttrs.end(); ++it) {
        os << " " << it->first;
        os << "=\"" << it->second << "\"";
    }
}

std::vector<std::string>
SUMOSAXAttributesImpl_Cached::getAttributeNames() const {
    std::vector<std::string> result;
    for (std::map<std::string, std::string>::const_iterator it = myAttrs.begin(); it != myAttrs.end(); ++it) {
        result.push_back(it->first);
    }
    return result;
}

SUMOSAXAttributes*
SUMOSAXAttributesImpl_Cached::clone() const {
    return new SUMOSAXAttributesImpl_Cached(myAttrs, myPredefinedTagsMML, getObjectType());
}


/****************************************************************************/
