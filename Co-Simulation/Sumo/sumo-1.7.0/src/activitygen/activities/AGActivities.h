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
/// @file    AGActivities.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
///
// Main class that manages activities taken in account and generates the
// inhabitants' trip list.
/****************************************************************************/
#pragma once
#include <config.h>

#include <list>
#include "AGTrip.h"
#include "../city/AGCity.h"
#include "../city/AGBusLine.h"
#include "../city/AGHousehold.h"


// ===========================================================================
// class definitions
// ===========================================================================
class AGActivities {
public:
    AGActivities(AGCity* city, int days) :
        myCity(city),
        nbrDays(days) {};
    void addTrip(AGTrip t, std::list<AGTrip>* tripSet);
    void addTrips(std::list<AGTrip> t, std::list<AGTrip>* tripSet);
    void generateActivityTrips();

    /**
     * trips contains trips as well for one day as for every day,
     * these trips will be regenerated with small variations
     * by ActivityGen at the end of the simulation
     * before generating the trip file
     */
    std::list<AGTrip> trips;

private:
    bool generateTrips(AGHousehold& hh);
    bool generateBusTraffic(AGBusLine bl);
    bool generateInOutTraffic();
    bool generateRandomTraffic();

    /**
     * generates car names, given the unique (number, prefix)
     */
    std::string generateName(int i, std::string prefix);

    AGCity* myCity;

    int nbrDays;

};
