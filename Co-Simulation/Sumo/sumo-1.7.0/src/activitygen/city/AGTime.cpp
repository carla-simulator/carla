/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
/// @file    AGTime.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    July 2010
///
// Time manager: able to manipulate the time using Sumo's format (seconds)
/****************************************************************************/
#include <config.h>

#include "AGTime.h"


// ===========================================================================
// method definitions
// ===========================================================================
AGTime::AGTime(const AGTime& time) {
    mySeconds = time.mySeconds;
}

int
AGTime::convert(int days, int hours, int minutes, int seconds) {
    mySeconds = seconds + 60 * (minutes + 60 * (hours + 24 * (days)));
    return mySeconds;
}

int
AGTime::getSecondsOf(double minutes) {
    return static_cast<int>(60.0 * minutes);
}

bool
AGTime::operator==(const AGTime& time) {
    if (this->mySeconds == time.mySeconds) {
        return true;
    } else {
        return false;
    }
}

bool
AGTime::operator<(const AGTime& time) {
    if (this->mySeconds < time.mySeconds) {
        return true;
    } else {
        return false;
    }
}

bool
AGTime::operator<=(const AGTime& time) {
    if (this->mySeconds <= time.mySeconds) {
        return true;
    } else {
        return false;
    }
}

void
AGTime::operator+=(const AGTime& time) {
    this->mySeconds += time.mySeconds;
}

void
AGTime::operator+=(int seconds) {
    this->mySeconds += seconds;
}

void
AGTime::operator-=(const AGTime& time) {
    this->mySeconds -= time.mySeconds;
}

AGTime
AGTime::operator+(const AGTime& time) {
    AGTime newtime(time.mySeconds + this->mySeconds);
    return newtime;
}

int
AGTime::getDay() {
    return (mySeconds / 86400);
}

int
AGTime::getHour() {
    return ((mySeconds / 3600) % 24);
}

int
AGTime::getMinute() {
    return ((mySeconds / 60) % 60);
}

int
AGTime::getSecond() {
    return (mySeconds % 60);
}

int
AGTime::getSecondsInCurrentDay() {
    return (mySeconds % 86400);
}

int
AGTime::getTime() {
    return this->mySeconds;
}

void
AGTime::setDay(int d) {
    if (0 <= d) {
        mySeconds -= 86400 * getDay();
        mySeconds += 86400 * d;
    }
}

void
AGTime::setHour(int h) {
    if (0 <= h && h < 24) {
        mySeconds -= 3600 * getHour();
        mySeconds += 3600 * h;
    }
}

void
AGTime::setMinute(int m) {
    if (0 <= m && m < 60) {
        mySeconds -= 60 * getMinute();
        mySeconds += 60 * m;
    }
}

void
AGTime::setSecond(int s) {
    if (0 <= s && s < 60) {
        mySeconds -= getSecond();
        mySeconds += s;
    }
}

void
AGTime::setTime(int seconds) {
    mySeconds = seconds;
}

void
AGTime::addDays(int d) {
    mySeconds += 86400 * d;
}

void
AGTime::addHours(int h) {
    mySeconds += 3600 * h;
}

void
AGTime::addMinutes(int m) {
    mySeconds += 60 * m;
}

void
AGTime::addSeconds(int s) {
    mySeconds += s;
}


/****************************************************************************/
