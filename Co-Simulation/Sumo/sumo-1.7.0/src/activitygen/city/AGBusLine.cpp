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
/// @file    AGBusLine.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    July 2010
///
// Bus line of the city: contains all the buses of this line
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utility>
#include <sstream>
#include <string>
#include <list>
#include "AGBusLine.h"
#include "AGBus.h"
#include "AGPosition.h"
#include "AGTime.h"
#include <utils/common/StdDefs.h>

#define PAUSE_TIME 15 //time (in minutes) a bus waits before going in the opposite direction.


// ===========================================================================
// method definitions
// ===========================================================================
void
AGBusLine::setMaxTripTime(int time) {
    this->maxTripTime = time;
}

void
AGBusLine::setBusNames() {
    busNbr = 0;
    std::list<AGBus>::iterator it1 = buses.begin(); //iterator on buses in the first direction
    std::list<AGBus>::iterator it2 = revBuses.begin(); //iterator on buses in the second direction

    std::list<std::pair<int, std::string> > drivingBuses1, drivingBuses2; //buses on the road or in the parking of the corresponding end: int: the time of availability

    while (it1 != buses.end() && it2 != revBuses.end()) {
        if (it1->getDeparture() > it2->getDeparture()) {
            if (drivingBuses2.size() == 0) {
                drivingBuses2.push_front(make_pair(it2->getDeparture(), createName()));
            } else if (drivingBuses2.front().first > it2->getDeparture()) {
                drivingBuses2.push_front(make_pair(it2->getDeparture(), createName()));
            }
            //here the first in drivingBuses2 is available for the trip
            it2->setName(drivingBuses2.front().second);
            drivingBuses2.pop_front();
            //the same bus will be available for the main direction after some time (see function getReady):
            drivingBuses1.push_back(make_pair(getReady(it2->getDeparture()), it2->getName()));
            it2++;
        } else {
            if (drivingBuses1.size() == 0) {
                drivingBuses1.push_front(make_pair(it1->getDeparture(), createName()));
            } else if (drivingBuses1.front().first > it1->getDeparture()) {
                drivingBuses1.push_front(make_pair(it1->getDeparture(), createName()));
            }
            //here the first in drivingBuses1 is available for the trip
            it1->setName(drivingBuses1.front().second);
            drivingBuses1.pop_front();
            //the same bus will be available for the return way after some time (see function getReady):
            drivingBuses2.push_back(make_pair(getReady(it1->getDeparture()), it1->getName()));
            it1++;
        }
    }
    if (it1 != buses.end()) {
        if (drivingBuses1.size() == 0) {
            it1->setName(createName());
        } else if (drivingBuses1.front().first > it1->getDeparture()) {
            it1->setName(createName());
        } else {
            it1->setName(drivingBuses1.front().second);
            drivingBuses1.pop_front();
        }
        it1++;
    }
    if (it2 != revBuses.end()) {
        if (drivingBuses2.size() == 0) {
            it2->setName(createName());
        } else if (drivingBuses2.front().first > it2->getDeparture()) {
            it2->setName(createName());
        } else {
            it2->setName(drivingBuses2.front().second);
            drivingBuses2.pop_front();
        }
        it2++;
    }
}

std::string
AGBusLine::createName() {
    ++busNbr; //initialized in setBusNames()
    std::ostringstream os;
    os << busNbr;
    return "bl" + lineNumber + "b" + os.str();
}

int
AGBusLine::getReady(int time) {
    AGTime current(time);
    current.addSeconds(maxTripTime);
    current.addMinutes(PAUSE_TIME);
    return current.getTime();
}

int
AGBusLine::nbrBuses() {
    return static_cast<int>(buses.size());
}

void
AGBusLine::locateStation(AGPosition pos) {
    stations.push_back(pos);
}

void
AGBusLine::locateRevStation(AGPosition pos) {
    revStations.push_back(pos);
}

void
AGBusLine::generateBuses(int start, int stop, int rate) {
    int t = start;
    while (t < stop) {
        buses.push_back(AGBus(t)); //one direction
        revBuses.push_back(AGBus(t)); //return direction
        t += rate;
    }
}


void
AGBusLine::printBuses() {
    std::list<AGBus>::iterator it;
    std::cout << "\n ----------- BUS LINE " << lineNumber << " PRINTING -------------\n" << std::endl;
    std::cout << "\n -------------------------- First way ---------------------------\n" << std::endl;
    for (it = buses.begin(); it != buses.end(); ++it) {
        it->print();
    }
    std::cout << "\n -------------------------- Second way --------------------------\n" << std::endl;
    for (it = revBuses.begin(); it != revBuses.end(); ++it) {
        it->print();
    }
    std::cout << "\n ----------------------------------------------------------------\n" << std::endl;
}


/****************************************************************************/
