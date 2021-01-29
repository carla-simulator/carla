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
/// @file    AGBusLine.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
///
// Bus line of the city: contains all the buses of this line
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <string>
#include <list>
#include "AGBus.h"
#include "AGPosition.h"
#include "AGDataAndStatistics.h"


// ===========================================================================
// class definitions
// ===========================================================================
class AGBusLine {
public:
    AGBusLine(std::string lineNr) :
        lineNumber(lineNr) {};
    void setMaxTripTime(int time);
    void setBusNames();
    int nbrBuses();
    void locateStation(AGPosition pos);
    void locateRevStation(AGPosition pos);
    void generateBuses(int start, int stop, int rate);
    void printBuses();

    std::list<AGPosition> stations;
    std::list<AGPosition> revStations;
    std::list<AGBus> buses;
    std::list<AGBus> revBuses;

private:
    /**
     * @return: a name for a new Bus. unique for the city.
     */
    std::string createName();

    /**
     * @param time: time of departure of the bus in one direction (current time)
     * @return: time at which it will be ready for going in the opposite direction
     */
    int getReady(int time);

    std::string lineNumber;
    int maxTripTime;
    int busNbr;
};
