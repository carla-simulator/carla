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
/// @file    StringTokenizer.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    ?
///
// A java-style StringTokenizer for c++ (stl)
/****************************************************************************/
#include <config.h>

#include <string>
#include <vector>
#include <iostream> // !!! debug only

#include "UtilExceptions.h"
#include "StringTokenizer.h"


// ===========================================================================
// variable definitions
// ===========================================================================
const int StringTokenizer::NEWLINE = -256;
const int StringTokenizer::WHITECHARS = -257;
const int StringTokenizer::SPACE = 32;
const int StringTokenizer::TAB = 9;


// ===========================================================================
// method definitions
// ===========================================================================

StringTokenizer::StringTokenizer() :
    myPos(0) {
}


StringTokenizer::StringTokenizer(std::string tosplit) :
    myTosplit(tosplit), myPos(0) {
    prepareWhitechar(tosplit);
}


StringTokenizer::StringTokenizer(std::string tosplit, std::string token, bool splitAtAllChars) :
    myTosplit(tosplit), myPos(0) {
    prepare(tosplit, token, splitAtAllChars);
}


StringTokenizer::StringTokenizer(std::string tosplit, int special) :
    myTosplit(tosplit), myPos(0) {
    switch (special) {
        case NEWLINE:
            prepare(tosplit, "\r\n", true);
            break;
        case TAB:
            prepare(tosplit, "\t", true);
            break;
        case WHITECHARS:
            prepareWhitechar(tosplit);
            break;
        default:
            char* buf = new char[2];
            buf[0] = (char) special;
            buf[1] = 0;
            prepare(tosplit, buf, false);
            delete[] buf;
            break;
    }
}


StringTokenizer::~StringTokenizer() {}


void StringTokenizer::reinit() {
    myPos = 0;
}


bool StringTokenizer::hasNext() {
    return myPos != (int)myStarts.size();
}


std::string StringTokenizer::next() {
    if (myPos >= (int)myStarts.size()) {
        throw OutOfBoundsException();
    }
    if (myLengths[myPos] == 0) {
        myPos++;
        return "";
    }
    int start = myStarts[myPos];
    int length = myLengths[myPos++];
    return myTosplit.substr(start, length);
}


std::string StringTokenizer::front() {
    if (myStarts.size() == 0) {
        throw OutOfBoundsException();
    }
    if (myLengths[0] == 0) {
        return "";
    }
    return myTosplit.substr(myStarts[0], myLengths[0]);
}


std::string StringTokenizer::get(int pos) const {
    if (pos >= (int)myStarts.size()) {
        throw OutOfBoundsException();
    }
    if (myLengths[pos] == 0) {
        return "";
    }
    int start = myStarts[pos];
    int length = myLengths[pos];
    return myTosplit.substr(start, length);
}


int StringTokenizer::size() const {
    return (int)myStarts.size();
}


void StringTokenizer::prepare(const std::string& tosplit, const std::string& token, bool splitAtAllChars) {
    int beg = 0;
    int len = (int)token.length();
    if (splitAtAllChars) {
        len = 1;
    }
    while (beg < (int)tosplit.length()) {
        std::string::size_type end;
        if (splitAtAllChars) {
            end = tosplit.find_first_of(token, beg);
        } else {
            end = tosplit.find(token, beg);
        }
        if (end == std::string::npos) {
            end = tosplit.length();
        }
        myStarts.push_back(beg);
        myLengths.push_back((int)end - beg);
        beg = (int)end + len;
        if (beg == (int)tosplit.length()) {
            myStarts.push_back(beg - 1);
            myLengths.push_back(0);
        }
    }
}


void StringTokenizer::prepareWhitechar(const std::string& tosplit) {
    std::string::size_type len = tosplit.length();
    std::string::size_type beg = 0;
    while (beg < len && tosplit[beg] <= SPACE) {
        beg++;
    }
    while (beg != std::string::npos && beg < len) {
        std::string::size_type end = beg;
        while (end < len && tosplit[end] > SPACE) {
            end++;
        }
        myStarts.push_back((int)beg);
        myLengths.push_back((int)end - (int)beg);
        beg = end;
        while (beg < len && tosplit[beg] <= SPACE) {
            beg++;
        }
    }
}


std::vector<std::string>
StringTokenizer::getVector() {
    std::vector<std::string> ret;
    ret.reserve(size());
    while (hasNext()) {
        ret.push_back(next());
    }
    reinit();
    return ret;
}


/****************************************************************************/
