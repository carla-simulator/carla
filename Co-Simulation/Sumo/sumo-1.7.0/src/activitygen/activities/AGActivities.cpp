/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    AGActivities.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Main class that manages activities taken in account and generates the
// inhabitants' trip list.
/****************************************************************************/
#include <config.h>

#include "AGActivities.h"
#include "AGWorkAndSchool.h"
#include "AGFreeTime.h"
#include "../city/AGTime.h"
#include <sstream>
#include <utils/common/RandHelper.h>

#define REBUILD_ITERATION_LIMIT 2


// ===========================================================================
// method definitions
// ===========================================================================
void
AGActivities::addTrip(AGTrip t, std::list<AGTrip>* tripSet) {
    tripSet->push_back(t);
}

void
AGActivities::addTrips(std::list<AGTrip> t, std::list<AGTrip>* tripSet) {
    std::list<AGTrip>::iterator it;
    for (it = t.begin(); it != t.end(); ++it) {
        tripSet->push_back(*it);
    }
}

void
AGActivities::generateActivityTrips() {
    int numbErr;
    /**
     * trips due to public transportation
     */
    numbErr = 0;
    std::list<AGBusLine>::iterator itBL;
    for (itBL = myCity->busLines.begin(); itBL != myCity->busLines.end(); ++itBL) {
        if (! generateBusTraffic(*itBL)) {
            ++numbErr;
        }
    }
    if (numbErr != 0) {
        std::cerr << "ERROR: " << numbErr << " bus lines couldn't been completely generated ( " << (float)numbErr * 100.0 / (float)myCity->busLines.size() << "% )..." << std::endl;
    } else {
        std::cout << "no problem during bus line trip generation..." << std::endl;
    }

    std::cout << "after public transportation: " << trips.size() << std::endl;
    /**
     * trips due to activities in the city
     * @NOTICE: includes people working in work positions out of the city
     */
    numbErr = 0;
    std::list<AGHousehold>::iterator itHH;
    for (itHH = myCity->households.begin(); itHH != myCity->households.end(); ++itHH) {
        if (! generateTrips(*itHH)) {
            ++numbErr;
        }
    }
    if (numbErr != 0) {
        std::cout << "WARNING: " << numbErr << " ( " << (float)numbErr * 100.0 / (float)myCity->households.size() << "% ) households' trips haven't been generated: would probably need more iterations for rebuilding..." << std::endl;
    } else {
        std::cout << "no problem during households' trips generation..." << std::endl;
    }

    std::cout << "after household activities: " << trips.size() << std::endl;
    /**
     * trips due to incoming and outgoing traffic
     * @WARNING: the outgoing traffic is already done: households in which someone works on a work position that is out of the city.
     */
    if (! generateInOutTraffic()) {
        std::cerr << "ERROR while generating in/Out traffic..." << std::endl;
    } else {
        std::cout << "no problem during in/out traffic generation..." << std::endl;
    }

    std::cout << "after incoming/outgoing traffic: " << trips.size() << std::endl;
    /**
     * random traffic trips
     * @NOTICE: this includes uniform and proportional random traffic
     */
    if (! generateRandomTraffic()) {
        std::cerr << "ERROR while generating random traffic..." << std::endl;
    } else {
        std::cout << "no problem during random traffic generation..." << std::endl;
    }

    std::cout << "after random traffic: " << trips.size() << std::endl;
}

bool
AGActivities::generateTrips(AGHousehold& hh) {
    int iteration = 0;
    bool generated = false;
    std::list<AGTrip> temporaTrips;
    while (!generated && iteration < REBUILD_ITERATION_LIMIT) {
        if (!temporaTrips.empty()) {
            temporaTrips.clear();
        }
        // Work and school activities
        AGWorkAndSchool ws(&hh, &(myCity->statData), &temporaTrips);
        generated = ws.generateTrips();
        if (!generated) {
            hh.regenerate();
            ++iteration;
            continue;
        }
        addTrips(ws.getPartialActivityTrips(), &temporaTrips);

        // free time activities
        AGFreeTime ft(&hh, &(myCity->statData), &temporaTrips, nbrDays);
        generated = ft.generateTrips();
        if (!generated) {
            hh.regenerate();
            ++iteration;
            continue;
        }
        addTrips(ft.getPartialActivityTrips(), &temporaTrips);
        //cout << "after this hh: " << temporaTrips.size() << " we have: " << trips.size() << endl;
        //trips of all activities generated:
        addTrips(temporaTrips, &trips);
    }
    return generated;
}

bool
AGActivities::generateBusTraffic(AGBusLine bl) {
    std::list<AGBus>::iterator itB;
    std::list<AGPosition>::iterator itS;
    /**
     * Buses in the first direction
     */
    for (itB = bl.buses.begin(); itB != bl.buses.end(); ++itB) {
        if (bl.stations.size() < 1) {
            return false;
        }
        AGTrip t(bl.stations.front(), bl.stations.back(), *itB, itB->getDeparture());
        for (itS = bl.stations.begin(); itS != bl.stations.end(); ++itS) {
            if (*itS == t.getDep() || *itS == t.getArr()) {
                continue;
            }
            t.addLayOver(*itS);
        }
        trips.push_back(t);
    }
    /**
     * Buses in the return direction
     */
    //verify that buses return back to the beginning
    if (bl.revStations.empty()) {
        return true;    //in this case, no return way: everything is ok.
    }
    for (itB = bl.revBuses.begin(); itB != bl.revBuses.end(); ++itB) {
        if (bl.revStations.size() < 1) {
            return false;
        }
        AGTrip t(bl.revStations.front(), bl.revStations.back(), *itB, itB->getDeparture());
        for (itS = bl.revStations.begin(); itS != bl.revStations.end(); ++itS) {
            if (*itS == t.getDep() || *itS == t.getArr()) {
                continue;
            }
            t.addLayOver(*itS);
        }
        trips.push_back(t);
    }
    return true;
}

bool
AGActivities::generateInOutTraffic() {
    /**
     * outgoing traffic already done by generateTrips():
     * people who work out of the city.
     * Here are people from outside the city coming to work.
     */
    if (myCity->peopleIncoming.empty()) {
        return true;
    }
    if (myCity->cityGates.empty()) {
        return false;
    }
    int num = 1;
    std::list<AGAdult>::iterator itA;

    for (itA = myCity->peopleIncoming.begin(); itA != myCity->peopleIncoming.end(); ++itA) {
        int posi = myCity->statData.getRandomCityGateByIncoming();
        std::string nom(generateName(num, "carIn"));
        AGTrip wayTrip(myCity->cityGates[posi], itA->getWorkPosition().getPosition(), nom, itA->getWorkPosition().getOpening());
        //now we put the estimated time of entrance in the city.
        wayTrip.setDepTime(wayTrip.estimateDepTime(wayTrip.getTime(), myCity->statData.speedTimePerKm));
        AGTrip retTrip(itA->getWorkPosition().getPosition(), myCity->cityGates[posi], nom, itA->getWorkPosition().getClosing());
        trips.push_back(wayTrip);
        trips.push_back(retTrip);
        ++num;
    }
    return true;
}

std::string
AGActivities::generateName(int i, std::string prefix) {
    std::ostringstream os;
    os << i;
    return prefix + os.str();
}

bool
AGActivities::generateRandomTraffic() {
    //total number of trips during the whole simulation
    int totalTrips = 0, ttOneDayTrips = 0, ttDailyTrips = 0;
    std::list<AGTrip>::iterator it;
    for (it = trips.begin(); it != trips.end(); ++it) {
        if (it->isDaily()) {
            ++ttDailyTrips;
        } else {
            ++ttOneDayTrips;
        }
    }
    totalTrips = ttOneDayTrips + ttDailyTrips * nbrDays;
    //TESTS
    std::cout << "Before Random traffic generation (days are still entire):" << std::endl;
    std::cout << "- Total number of trips: " << totalTrips << std::endl;
    std::cout << "- Total daily trips: " << ttDailyTrips << std::endl;
    std::cout << "- Total one-day trips: " << ttOneDayTrips << std::endl;
    //END OF TESTS

    //random uniform distribution:
    int nbrRandUni = (int)((float)totalTrips * myCity->statData.uniformRandomTrafficRate / (1.0f - myCity->statData.uniformRandomTrafficRate));
    //TESTS
    std::cout << "added uniform random trips: " << nbrRandUni << std::endl;
    //END OF TESTS
    for (int i = 0; i < nbrRandUni; ++i) {
        AGPosition dep(myCity->getRandomStreet());
        AGPosition arr(myCity->getRandomStreet());
        AGTime depTime(RandHelper::rand(nbrDays * 86400));
        AGTrip rdtr(dep, arr, generateName(i, "randUni"), depTime.getTime() % 86400, depTime.getDay() + 1);
        rdtr.setType("random");
        trips.push_back(rdtr);
    }

    //random proportional distribution:
    //float proportionalPercentage = 0.05f;
    //TODO generate a proportionally distributed random traffic

    return true;
}


/****************************************************************************/
