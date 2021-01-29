/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2018-2020 German Aerospace Center (DLR) and others.
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
/// @file    NiceTimeTool.java
/// @author  Maximiliano Bottazzi
/// @date    2018
///
//
/****************************************************************************/
package de.dlr.ts.commons.tools;

/**
 *
 * @author Praktikant-Q2-2015
 */
public class NiceTimeTool {
    private final long secondsInMilli = 1000;
    private final long minutesInMilli = secondsInMilli * 60;
    private final long hoursInMilli = minutesInMilli * 60;
    private final long daysInMilli = hoursInMilli * 24;
    private final long weeksInMilli = daysInMilli * 7;
    private final long monthsInMilli = weeksInMilli * 4;
    private final long yearsInMilli = monthsInMilli * 12;

    private String year = "y";
    private String month = "M";
    private String week = "w";
    private String day = "d";
    private String hour = "h";
    private String minute = "m";
    private String second = "s";



    /**
     *
     * @param time
     * @return
     */
    public String getTime(long time) {
        StringBuilder sb = new StringBuilder();

        long tmp = time;

        if (time >= yearsInMilli) {
            long yearsAgo = time / yearsInMilli;
            time = time % yearsInMilli;
            sb.append(yearsAgo).append(" ").append(year).append(" ");
        }

        if (time >= monthsInMilli && tmp < yearsInMilli) {
            long monthsAgo = time / monthsInMilli;
            time = time % monthsInMilli;
            sb.append(monthsAgo).append(" ").append(month).append(" ");
        }

        if (time >= weeksInMilli && tmp < yearsInMilli) {
            long weeksAgo = time / weeksInMilli;
            time = time % weeksInMilli;
            sb.append(weeksAgo).append(" ").append(week).append(" ");
        }

        if (time >= daysInMilli && tmp < monthsInMilli) {
            long daysAgo = time / daysInMilli;
            time = time % daysInMilli;
            sb.append(daysAgo).append(" ").append(day).append(" ");
        }

        if (time >= hoursInMilli && tmp < weeksInMilli) {
            long hoursAgo = time / hoursInMilli;
            time = time % hoursInMilli;
            sb.append(hoursAgo).append(" ").append(hour).append(" ");
        }

        if (time >= minutesInMilli && time > 2 * minutesInMilli
                && time < 60 * minutesInMilli && tmp < daysInMilli) {
            long minutesAgo = time / minutesInMilli;

            if (time > 10 * minutesInMilli) {
                minutesAgo = Math.round((minutesAgo / 5)) * 5;
            }

            sb.append(minutesAgo).append(" ").append(minute).append(" ");
        }

        if (time >= secondsInMilli && time <= 120 * secondsInMilli) {
            long secondsAgo = time / secondsInMilli;
            sb.append(secondsAgo).append(" ").append(second);
        }

        return sb.toString().trim();
    }

    public void setDay(String day) {
        this.day = day;
    }

    public void setSecond(String second) {
        this.second = second;
    }

    public void setMinute(String minute) {
        this.minute = minute;
    }

    public void setHour(String hour) {
        this.hour = hour;
    }

    public void setWeek(String week) {
        this.week = week;
    }

    public void setMonth(String month) {
        this.month = month;
    }

    public void setYear(String year) {
        this.year = year;
    }
}
