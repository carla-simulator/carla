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
/// @file    AGTime.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    July 2010
///
// Time manager: able to manipulate the time using Sumo's format (seconds)
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>


// ===========================================================================
// class definitions
// ===========================================================================
class AGTime {
public:
    AGTime(int seconds) :
        mySeconds(seconds) {};
    AGTime(int hour, int minutes) :
        mySeconds(convert(0, hour, minutes, 0)) {};
    AGTime(int day, int hour, int min) :
        mySeconds(convert(day, hour, min, 0)) {};
    AGTime(int day, int hour, int min, int sec) :
        mySeconds(convert(day, hour, min, sec)) {};
    AGTime(const AGTime& time);
    bool operator==(const AGTime& time);
    bool operator<(const AGTime& time);
    bool operator<=(const AGTime& time);
    void operator+=(const AGTime& time);
    void operator+=(int seconds);
    void operator-=(const AGTime& time);
    AGTime operator+(const AGTime& time);

    /********************
     * In/Out functions *
     ********************/
    int getDay();
    int getHour();
    int getMinute();
    int getSecond();
    int getSecondsInCurrentDay();
    /**
     * @brief: returns the number of seconds from the beginning of the first day of simulation
     * this includes
     */
    int getTime();

    void setDay(int d);
    void setHour(int h);
    void setMinute(int m);
    void setSecond(int s);
    /**
     * @brief: sets the time from the beginning of the first day of simulation in seconds
     */
    void setTime(int sec);


    /**************************
     * Manipulation functions *
     **************************/
    /**
     * @brief addition of seconds to the current moment
     *
     * @param[in] sec the number of seconds
     */
    void addSeconds(int sec);

    /**
     * @brief addition of minutes to the current moment
     *
     * @param[in] min the number of minutes
     */
    void addMinutes(int min);

    /**
     * @brief addition of hours to the current moment
     *
     * @param[in] hours the number of hours to add
     */
    void addHours(int hours);

    /**
     * @brief addition of days to the current moment
     *
     * @param[in] days the number of days to add
     */
    void addDays(int days);

    /**
     * @brief computes the number of seconds in the given minutes
     *
     * @param[in] minutes, can be fraction of minutes
     *
     * @return number of seconds
     */
    int getSecondsOf(double minutes);

private:
    /**
     * @brief converts days, hours and minutes to seconds
     */
    int convert(int days, int hours, int minutes, int seconds);


    // @brief: the seconds representing this date (day, hour, minute)
    // @brief: used for in/out
    int mySeconds;
};
