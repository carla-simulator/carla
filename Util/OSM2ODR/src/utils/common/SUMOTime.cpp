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
/// @file    SUMOTime.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Fri, 29.04.2005
///
// Variables, methods, and tools for internal time representation
/****************************************************************************/
#include <config.h>

#include <sstream>
#include <iostream>
#include <iomanip>
#include "SUMOTime.h"
#include "StringTokenizer.h"
#include "StringUtils.h"
#include "StdDefs.h"
#include "MsgHandler.h"


// ===========================================================================
// type definitions
// ===========================================================================
SUMOTime DELTA_T = 1000;


// ===========================================================================
// method definitions
// ===========================================================================

SUMOTime
string2time(const std::string& r) {
    if (r.find(":") == std::string::npos) {
        const double time = StringUtils::toDouble(r);
        if (time > STEPS2TIME(SUMOTime_MAX)) {
            throw TimeFormatException("Input string '" + r + "' exceeds the time value range.");
        }
        return TIME2STEPS(time);
    } else {
        // try to parse jj:hh:mm:ss.s
        std::vector<std::string> hrt = StringTokenizer(r, ":").getVector();
        if (hrt.size() == 3) {
            //std::cout << "parsed '" << r << "' as " << (3600 * string2time(hrt[0]) + 60 * string2time(hrt[1]) + string2time(hrt[2])) << "\n";
            return 3600 * string2time(hrt[0]) + 60 * string2time(hrt[1]) + string2time(hrt[2]);
        } else if (hrt.size() == 4) {
            //std::cout << "parsed '" << r << "' as " << (24 * 3600 * string2time(hrt[0]) + 3600 * string2time(hrt[1]) + 60 * string2time(hrt[2]) + string2time(hrt[3])) << "\n";
            return 24 * 3600 * string2time(hrt[0]) + 3600 * string2time(hrt[1]) + 60 * string2time(hrt[2]) + string2time(hrt[3]);
        }
        throw TimeFormatException("Input string '" + r + "' is not a valid time format (jj:HH:MM:SS.S).");
    }
}


std::string
time2string(SUMOTime t) {
    std::ostringstream oss;
    if (t < 0) {
        oss << "-";
    }
    // needed for signed zero errors, see #5926
    t = abs(t);
    const SUMOTime scale = (SUMOTime)pow(10, MAX2(0, 3 - gPrecision));
    if (scale > 1 && t != SUMOTime_MAX) {
        t = (t + scale / 2) / scale;
    }
    const SUMOTime second = TIME2STEPS(1) / scale;
    if (gHumanReadableTime) {
        const SUMOTime minute = 60 * second;
        const SUMOTime hour = 60 * minute;
        const SUMOTime day = 24 * hour;
        // 123456 -> "00:00:12.34"
        if (t > day) {
            oss << t / day << ":";
            t %= day;
        }
        oss << std::setfill('0') << std::setw(2);
        oss << t / hour << ":";
        t %= hour;
        oss << std::setw(2) << t / minute << ":";
        t %= minute;
        oss << std::setw(2) << t / second;
        t %= second;
        if (t != 0 || TS < 1.) {
            oss << std::setw(MIN2(3, gPrecision));
            oss << "." << t;
        }
    } else {
        oss << t / second << ".";
        oss << std::setfill('0') << std::setw(MIN2(3, gPrecision));
        oss << t % second;
    }
    return oss.str();
}


bool checkStepLengthMultiple(const SUMOTime t, const std::string& error, SUMOTime deltaT) {
    if (t % deltaT != 0) {
        WRITE_WARNING("The given time value " + time2string(t) + " is not a multiple of the step length " + time2string(deltaT) + error + ".")
    }
    // next line used to fix build
    return false;
}


/****************************************************************************/
