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
/// @file    ToString.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 23 Sep 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <list>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/Named.h>
#include <utils/distribution/Distribution_Parameterized.h>
#include "StdDefs.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * Template for conversions from origin format to string representation
 * (when supplied by c++/the stl)
 */
template <class T>
inline std::string toString(const T& t, std::streamsize accuracy = gPrecision) {
    std::ostringstream oss;
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss << std::setprecision(accuracy);
    oss << t;
    return oss.str();
}


template<typename T>
inline std::string toHex(const T i, std::streamsize numDigits = 0) {
    // taken from http://stackoverflow.com/questions/5100718/int-to-hex-string-in-c
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(numDigits == 0 ? sizeof(T) * 2 : numDigits) << std::hex << i;
    return stream.str();
}


inline std::string toString(const Named* obj, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return Named::getIDSecure(obj);
}


template <>
inline std::string toString<SumoXMLTag>(const SumoXMLTag& tag, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::Tags.getString(tag);
}


template <>
inline std::string toString<SumoXMLAttr>(const SumoXMLAttr& attr, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::Attrs.getString(attr);
}


template <>
inline std::string toString<SumoXMLNodeType>(const SumoXMLNodeType& nodeType, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::NodeTypes.getString(nodeType);
}


template <>
inline std::string toString<SumoXMLEdgeFunc>(const SumoXMLEdgeFunc& edgeFunc, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::EdgeFunctions.getString(edgeFunc);
}


template <>
inline std::string toString<SUMOVehicleClass>(const SUMOVehicleClass& vClass, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SumoVehicleClassStrings.getString(vClass);
}


template <>
inline std::string toString<LaneSpreadFunction>(const LaneSpreadFunction& lsf, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::LaneSpreadFunctions.getString(lsf);
}

template <>
inline std::string toString<RightOfWay>(const RightOfWay& row, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::RightOfWayValues.getString(row);
}

template <>
inline std::string toString<FringeType>(const FringeType& fringeType, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::FringeTypeValues.getString(fringeType);
}

template <>
inline std::string toString<PersonMode>(const PersonMode& personMode, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::PersonModeValues.getString(personMode);
}

template <>
inline std::string toString<LinkState>(const LinkState& linkState, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::LinkStates.getString(linkState);
}


template <>
inline std::string toString<LinkDirection>(const LinkDirection& linkDir, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::LinkDirections.getString(linkDir);
}


template <>
inline std::string toString<TrafficLightType>(const TrafficLightType& type, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::TrafficLightTypes.getString(type);
}


template <>
inline std::string toString<LaneChangeModel>(const LaneChangeModel& model, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::LaneChangeModels.getString(model);
}

template <>
inline std::string toString<LateralAlignment>(const LateralAlignment& latA, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    return SUMOXMLDefinitions::LateralAlignments.getString(latA);
}

template <>
inline std::string toString<LaneChangeAction>(const LaneChangeAction& action, std::streamsize accuracy) {
    UNUSED_PARAMETER(accuracy);
    std::vector<std::string> strings = SUMOXMLDefinitions::LaneChangeActions.getStrings();
    bool hadOne = false;
    std::ostringstream oss;
    for (std::vector<std::string>::const_iterator it = strings.begin(); it != strings.end(); ++it) {
        if ((action & SUMOXMLDefinitions::LaneChangeActions.get(*it)) != 0) {
            if (hadOne) {
                oss << "|";
            } else {
                hadOne = true;
            }
            oss << (*it);
        }
    }
    return oss.str();
}

template <>
inline std::string toString<Distribution_Parameterized>(const Distribution_Parameterized& dist, std::streamsize accuracy) {
    return dist.toStr(accuracy);
}

template <typename V>
inline std::string toString(const std::vector<V*>& v, std::streamsize accuracy = gPrecision) {
    return toString<V>(v.begin(), v.end(), accuracy);
}

template <typename V>
inline std::string toString(const typename std::vector<V*>::const_iterator& b, const typename std::vector<V*>::const_iterator& e, std::streamsize accuracy = gPrecision) {
    UNUSED_PARAMETER(accuracy);
    std::ostringstream oss;
    for (typename std::vector<V*>::const_iterator it = b; it != e; ++it) {
        if (it != b) {
            oss << " ";
        }
        oss << Named::getIDSecure(*it);
    }
    return oss.str();
}

template <typename V>
inline std::string toString(const std::list<V*>& v, std::streamsize accuracy = gPrecision) {
    return toString<V>(v.begin(), v.end(), accuracy);
}

template <typename V>
inline std::string toString(const typename std::list<V*>::const_iterator& b, const typename std::list<V*>::const_iterator& e, std::streamsize accuracy = gPrecision) {
    UNUSED_PARAMETER(accuracy);
    std::ostringstream oss;
    for (typename std::list<V*>::const_iterator it = b; it != e; ++it) {
        if (it != b) {
            oss << " ";
        }
        oss << Named::getIDSecure(*it);
    }
    return oss.str();
}



//template <typename V>
//inline std::string toString(const std::vector<V>& v, std::streamsize accuracy = gPrecision) {
//    return toString<V>(v.begin(), v.end(), accuracy);
//}
//
//
//template <typename V>
//inline std::string toString(const typename std::vector<V>::const_iterator& b, const typename std::vector<V>::const_iterator& e, std::streamsize accuracy = gPrecision) {
//    UNUSED_PARAMETER(accuracy);
//    std::ostringstream oss;
//    for (typename std::vector<V>::const_iterator it = b; it != e; ++it) {
//        if (it != b) {
//            oss << " ";
//        }
//        oss << Named::getIDSecure(*it);
//    }
//    return oss.str();
//}


template <typename T, typename T_BETWEEN>
inline std::string joinToString(const std::vector<T>& v, const T_BETWEEN& between, std::streamsize accuracy = gPrecision) {
    std::ostringstream oss;
    bool connect = false;
    for (typename std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (connect) {
            oss << toString(between, accuracy);
        } else {
            connect = true;
        }
        oss << toString(*it, accuracy);
    }
    return oss.str();
}


template <typename T, typename T_BETWEEN>
inline std::string joinToStringSorting(const std::vector<T>& v, const T_BETWEEN& between, std::streamsize accuracy = gPrecision) {
    std::vector<T> sorted(v);
    std::sort(sorted.begin(), sorted.end());
    return joinToString(sorted, between, accuracy);
}


template <typename T, typename T_BETWEEN>
inline std::string joinNamedToStringSorting(const std::set<T*>& ns, const T_BETWEEN& between) {
    std::vector<std::string> ids;
    for (T* n : ns) {
        ids.push_back(Named::getIDSecure(n));
    }
    return joinToStringSorting(ids, between);
}


template <typename T, typename C, typename T_BETWEEN>
inline std::string joinNamedToString(const std::set<T*, C>& ns, const T_BETWEEN& between) {
    std::vector<std::string> ids;
    for (T* n : ns) {
        ids.push_back(Named::getIDSecure(n));
    }
    return joinToString(ids, between);
}


template <typename V>
inline std::string toString(const std::set<V*>& v, std::streamsize accuracy = gPrecision) {
    UNUSED_PARAMETER(accuracy);
    std::vector<std::string> ids;
    for (typename std::set<V*>::const_iterator it = v.begin(); it != v.end(); ++it) {
        ids.push_back((*it)->getID());
    }
    return joinToStringSorting(ids, " ");
}


template <>
inline std::string toString(const std::vector<int>& v, std::streamsize accuracy) {
    return joinToString(v, " ", accuracy);
}


template <>
inline std::string toString(const std::vector<long long int>& v, std::streamsize accuracy) {
    return joinToString(v, " ", accuracy);
}


template <>
inline std::string toString(const std::vector<double>& v, std::streamsize accuracy) {
    return joinToString(v, " ", accuracy);
}


template <typename T, typename T_BETWEEN>
inline std::string joinToString(const std::set<T>& s, const T_BETWEEN& between, std::streamsize accuracy = gPrecision) {
    std::ostringstream oss;
    bool connect = false;
    for (typename std::set<T>::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (connect) {
            oss << toString(between, accuracy);
        } else {
            connect = true;
        }
        oss << toString(*it, accuracy);
    }
    return oss.str();
}


template <>
inline std::string toString(const std::vector<std::string>& v, std::streamsize) {
    return joinToString(v, " ");
}


template <>
inline std::string toString(const std::set<std::string>& v, std::streamsize) {
    return joinToString(v, " ");
}


template <typename KEY, typename VAL, typename T_BETWEEN, typename T_BETWEEN_KEYVAL>
inline std::string joinToString(const std::map<KEY, VAL>& s, const T_BETWEEN& between, const T_BETWEEN_KEYVAL& between_keyval, std::streamsize accuracy = gPrecision) {
    std::ostringstream oss;
    bool connect = false;
    for (typename std::map<KEY, VAL>::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (connect) {
            oss << toString(between, accuracy);
        } else {
            connect = true;
        }
        oss << toString(it->first, accuracy) << between_keyval << toString(it->second, accuracy);
    }
    return oss.str();
}


template <>
inline std::string toString(const std::map<std::string, std::string>& v, std::streamsize) {
    return joinToString(v, ", ", ":");
}
