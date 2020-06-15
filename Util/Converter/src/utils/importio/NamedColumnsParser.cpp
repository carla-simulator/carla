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
/// @file    NamedColumnsParser.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Fri, 19 Jul 2002
///
// A parser to retrieve information from a table with known column
/****************************************************************************/
#include <config.h>

#include <map>
#include <string>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include "NamedColumnsParser.h"


// ===========================================================================
// method definitions
// ===========================================================================
NamedColumnsParser::NamedColumnsParser() {}


NamedColumnsParser::NamedColumnsParser(const std::string& def,
                                       const std::string& defDelim,
                                       const std::string& lineDelim,
                                       bool prune, bool ignoreCase)
    : myLineDelimiter(lineDelim), myAmCaseInsensitive(ignoreCase) {
    reinitMap(def, defDelim, prune);
}


NamedColumnsParser::~NamedColumnsParser() {}


void
NamedColumnsParser::reinit(const std::string& def,
                           const std::string& defDelim,
                           const std::string& lineDelim,
                           bool prune, bool ignoreCase) {
    myAmCaseInsensitive = ignoreCase;
    reinitMap(def, defDelim, prune);
    myLineDelimiter = lineDelim;
}


void
NamedColumnsParser::parseLine(const std::string& line) {
    myLineParser = StringTokenizer(line, myLineDelimiter);
}


std::string
NamedColumnsParser::get(const std::string& name, bool prune) const {
    PosMap::const_iterator i = myDefinitionsMap.find(name);
    if (i == myDefinitionsMap.end()) {
        if (myAmCaseInsensitive) {
            i = myDefinitionsMap.find(StringUtils::to_lower_case(name));
        }
        if (i == myDefinitionsMap.end()) {
            throw UnknownElement("Element '" + name + "' is missing");
        }
    }
    int pos = (*i).second;
    if (myLineParser.size() <= pos) {
        throw OutOfBoundsException();
    }
    std::string ret = myLineParser.get(pos);
    checkPrune(ret, prune);
    return ret;
}


bool
NamedColumnsParser::know(const std::string& name) const {
    PosMap::const_iterator i = myDefinitionsMap.find(name);
    if (i == myDefinitionsMap.end()) {
        if (myAmCaseInsensitive) {
            i = myDefinitionsMap.find(StringUtils::to_lower_case(name));
        }
    }
    if (i == myDefinitionsMap.end()) {
        return false;
    }
    int pos = (*i).second;
    return myLineParser.size() > pos;
}


bool
NamedColumnsParser::hasFullDefinition() const {
    return (int)myDefinitionsMap.size() == myLineParser.size();
}


void
NamedColumnsParser::reinitMap(std::string s,
                              const std::string& delim,
                              bool prune) {
    if (myAmCaseInsensitive) {
        s = StringUtils::to_lower_case(s);
    }
    myDefinitionsMap.clear();
    int pos = 0;
    StringTokenizer st(s, delim);
    while (st.hasNext()) {
        std::string next = st.next();
        checkPrune(next, prune);
        myDefinitionsMap.insert(std::map<std::string, int>::value_type(next, pos++));
    }
}


void
NamedColumnsParser::checkPrune(std::string& str, bool prune) const {
    if (!prune) {
        return;
    }
    std::string::size_type idx = str.find_first_not_of(" ");
    if (idx != std::string::npos) {
        str = str.substr(idx);
    }
    idx = str.find_last_not_of(" ");
    if (idx != std::string::npos && idx != str.length() - 1) {
        str = str.substr(0, idx + 1);
    }
}


/****************************************************************************/
