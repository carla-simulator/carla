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
/// @file    AGCity.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @author  Jakob Erdmann
/// @date    July 2010
///
// City class that contains all other objects of the city: in particular
// streets, households, bus lines, work positions and school
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include <utils/common/RandHelper.h>
#include <utils/options/OptionsCont.h>
#include <router/RONet.h>
#include <router/ROEdge.h>
#include "AGAdult.h"
#include "AGStreet.h"
#include "AGWorkPosition.h"
#include "AGCity.h"
//#define DRIVING_LICENSE_AGE 18


// ===========================================================================
// method definitions
// ===========================================================================
void
AGCity::completeStreets() {
    if (streetsCompleted) {
        return;
    } else {
        streetsCompleted = true;
    }

    double pop = 0, work = 0;
    std::vector<AGStreet*>::iterator it;

    for (it = streets.begin(); it != streets.end(); ++it) {
        pop += (*it)->getPopulation();
        work += (*it)->getWorkplaceNumber();
    }
    statData.factorInhabitants = (double)statData.inhabitants / pop;
    //can be improved with other input data
    double neededWorkPositionsInCity = (1.0 - statData.unemployement)
                                       * ((double)statData.getPeopleYoungerThan(statData.limitAgeRetirement)
                                          - (double)statData.getPeopleYoungerThan(statData.limitAgeChildren))
                                       + (double)statData.incomingTraffic;
    // by default laborDemand = 1.05. We generate 5% more work positions that really needed to avoid any expensive research of random work positions
    neededWorkPositionsInCity *= statData.laborDemand;
    statData.workPositions = (int)neededWorkPositionsInCity;
    statData.factorWorkPositions = neededWorkPositionsInCity / (double) work;

    for (it = streets.begin(); it != streets.end(); ++it) {
        (*it)->setPopulation((*it)->getPopulation() * statData.factorInhabitants);
        (*it)->setWorkplaceNumber((*it)->getWorkplaceNumber() * statData.factorWorkPositions);
        //it->print();
    }

    //completing streets from edges of the network not handled/present in STAT file (no population no work position)
    for (const auto& itE : net->getEdgeMap()) {
        std::vector<AGStreet*>::iterator itS;
        for (itS = streets.begin(); itS != streets.end(); ++itS) {
            if (*itS == itE.second) {
                break;
            }
        }
        //if this edge isn't represented by a street
        if (itS == streets.end() && !itE.second->isInternal()) {
            streets.push_back(static_cast<AGStreet*>(itE.second));
        }
    }
}

void
AGCity::generateWorkPositions() {
    std::vector<AGStreet*>::iterator it;
    int workPositionCounter = 0;

    try {
        for (it = streets.begin(); it != streets.end(); ++it) {
            //std::cout << "number of work positions in street: " << it->getWorkplaceNumber() << std::endl;
            for (int i = 0; i < (*it)->getWorkplaceNumber(); ++i) {
                workPositions.push_back(AGWorkPosition(&statData, **it));
                ++workPositionCounter;
            }
        }
    } catch (const std::bad_alloc& e) {
        std::cout << "Number of work positions at bad_alloc exception: " << workPositionCounter << std::endl;
        throw e;
    }
    //std::cout << "Inner work positions done. " << workPositionCounter << " generated." << std::endl;

    // Work positions outside the city
    generateOutgoingWP();
    std::cout << "--> work position: " << std::endl;
    std::cout << "  |-> in city: " << workPositionCounter << std::endl;
    std::cout << "  |-> out city: " << statData.workPositions - workPositionCounter << std::endl;
    std::cout << "  |-> in+out city: " << statData.workPositions << std::endl;
}

void
AGCity::generateOutgoingWP() {
    // work positions outside the city
    double nbrWorkers = static_cast<double>(statData.getPeopleYoungerThan(statData.limitAgeRetirement) - statData.getPeopleYoungerThan(statData.limitAgeChildren));
    if (nbrWorkers <= 0) {
        return;
    }
    nbrWorkers *= (1.0 - statData.unemployement);
    /**
     * N_out = N_in * (ProportionOut / (1 - ProportionOut)) = N_out = N_in * (Noutworkers / (Nworkers - Noutworkers))
     */
    int nbrOutWorkPositions = static_cast<int>(workPositions.size() * (static_cast<double>(statData.outgoingTraffic)) / (nbrWorkers - static_cast<double>(statData.outgoingTraffic)));

    if (cityGates.empty()) {
        return;
    }

    for (int i = 0; i < nbrOutWorkPositions; ++i) {
        int posi = statData.getRandomCityGateByOutgoing();
        workPositions.push_back(AGWorkPosition(&statData, cityGates[posi].getStreet(), cityGates[posi].getPosition()));
    }
    //cout << "outgoing traffic: " << statData.outgoingTraffic << std::endl;
    //cout << "total number of workers in the city: " << nbrWorkers << std::endl;
    //cout << "work positions out side the city: " << nbrOutWorkPositions << std::endl;
    //cout << "work positions in and out of the city: " << workPositions.size() << std::endl;
    statData.workPositions = static_cast<int>(workPositions.size());
}

void
AGCity::completeBusLines() {
    std::list<AGBusLine>::iterator it;
    for (it = busLines.begin(); it != busLines.end(); ++it) {
        //it->generateOpositDirection();
        it->setBusNames();
    }
}

void
AGCity::generatePopulation() {
    std::vector<AGStreet*>::iterator it;
    double people = 0;
    nbrCars = 0;
    int idHouseholds = 0;
    std::vector<int> numAdults(statData.households);
    std::vector<int> numChilds(statData.households);
    int totalChildrenLeft = statData.inhabitants - statData.getPeopleOlderThan(statData.limitAgeChildren);
    const double retiredProb = statData.getPeopleOlderThan(statData.limitAgeRetirement) / statData.getPeopleOlderThan(statData.limitAgeChildren);
    for (int i = 0; i < statData.households; i++) {
        numAdults[i] = 1;
        numChilds[i] = 0;
        if (RandHelper::rand() < retiredProb) {
            numAdults[i] = -numAdults[i];
        } else if (totalChildrenLeft > 0) {
            numChilds[i] = statData.getPoissonsNumberOfChildren(statData.meanNbrChildren);
            totalChildrenLeft -= numChilds[i];
        }
    }
    //compensate with adults for too many / missing children
    const int numSecondPers = statData.getPeopleOlderThan(statData.limitAgeChildren) - statData.households + totalChildrenLeft;
    for (int i = 0; i < numSecondPers; i++) {
        int index = i % numAdults.size();
        if (numAdults[index] >= 0) {
            numAdults[index] += 1;
        } else {
            numAdults[index] -= 1;
        }
    }
    for (it = streets.begin(); it != streets.end(); ++it) {
        people += (*it)->getPopulation();
        while (people > 0 && idHouseholds < (int)numAdults.size()) {
            int i = RandHelper::rand((int)numAdults.size() - idHouseholds);
            ++idHouseholds;
            households.push_back(AGHousehold(*it, this, idHouseholds));
            households.back().generatePeople(abs(numAdults[i]), numChilds[i], numAdults[i] < 0); //&statData
            //households.back().generateCars(statData.carRate);
            people -= households.back().getPeopleNbr();
            numAdults[i] = numAdults[numAdults.size() - idHouseholds];
            numChilds[i] = numChilds[numAdults.size() - idHouseholds];
        }
    }

    //people from outside of the city generation:
    generateIncomingPopulation();

    //TEST
    int nbrSingle = 0;
    int nbrCouple = 0;
    int nbrChild = 0;
    int nbrHH = 0;
    int workingP = 0;
    std::list<AGHousehold>::iterator itt;
    for (itt = households.begin(); itt != households.end(); ++itt) {
        if (itt->getAdultNbr() == 1) {
            nbrSingle++;
            if (itt->getAdults().front().isWorking()) {
                workingP++;
            }
        }
        if (itt->getAdultNbr() == 2) {
            nbrCouple += 2;
            if (itt->getAdults().front().isWorking()) {
                workingP++;
            }
            if (itt->getAdults().back().isWorking()) {
                workingP++;
            }
        }
        nbrChild += itt->getPeopleNbr() - itt->getAdultNbr();
        nbrHH++;
    }
    //cout << "number hh: " << nbrHH << std::endl;
    //cout << "number single: " << nbrSingle << std::endl;
    //cout << "number couple: " << nbrCouple << std::endl;
    //cout << "number 3 or more: " << nbr3More << std::endl;
    //cout << "number adults: " << nbrSingle + nbrCouple + nbr3More << std::endl;
    //cout << "number children: " << nbrChild << std::endl;
    //cout << "number people: " << nbrSingle + nbrCouple + nbr3More + nbrChild << std::endl;
    //END TEST

    std::cout << "--> population: " << std::endl;
    std::cout << "  |-> city households: " << nbrHH << std::endl;
    std::cout << "  |-> city people: " << nbrSingle + nbrCouple + nbrChild << std::endl;
    std::cout << "    |-> city single: " << nbrSingle << " / (in) couple: " << nbrCouple << std::endl;
    std::cout << "    |-> city adults: " << nbrSingle + nbrCouple << std::endl;
    std::cout << "      |-> estimation: " << statData.getPeopleOlderThan(statData.limitAgeChildren) << std::endl;
    std::cout << "      |-> retired: " << statData.getPeopleOlderThan(statData.limitAgeRetirement) << std::endl;
    std::cout << "    |-> city children: " << nbrChild << std::endl;
    std::cout << "      |-> estimation: " << statData.getPeopleYoungerThan(statData.limitAgeChildren) << std::endl;

}

void
AGCity::generateIncomingPopulation() {
    for (int i = 0; i < statData.incomingTraffic; ++i) {
        AGAdult ad(statData.getRandomPopDistributed(statData.limitAgeChildren, statData.limitAgeRetirement));
        peopleIncoming.push_back(ad);
    }
}

void
AGCity::schoolAllocation() {
    std::list<AGHousehold>::iterator it;
    bool shortage;
    for (it = households.begin(); it != households.end(); ++it) {
        shortage = !it->allocateChildrenSchool();
        if (shortage) {
            /*ofstream fichier("test.txt", ios::app);  // ouverture en écriture avec effacement du fichier ouvert
            if(fichier)
            {
              fichier << "===> WARNING: Not enough school places in the city for all children..." << std::endl;
              fichier.close();
            }
            else
              cerr << "Impossible d'ouvrir le fichier !" << std::endl;*/

            //std::cout << "===> WARNING: Not enough school places in the city for all children..." << std::endl;
        }
    }
}

void
AGCity::workAllocation() {
    const bool debug = OptionsCont::getOptions().getBool("debug");
    statData.AdultNbr = 0;
    //end tests
    /**
     * people from the city
     */
    std::list<AGHousehold>::iterator it;
    bool shortage;

    if (debug) {
        std::cout << "\n";
    }

    for (it = households.begin(); it != households.end(); ++it) {
        if (it->retiredHouseholders()) {
            continue;
        }
        shortage = !it->allocateAdultsWork();
        if (shortage) {
            std::cout << "===> ERROR: Not enough work positions in the city for all working people..." << std::endl;
        }
        statData.AdultNbr += it->getAdultNbr(); //TESTING
        if (debug) {
            std::cout << " processed " << statData.AdultNbr << " adults\r";
        }
    }

    /**
     * people from outside
     */
    std::list<AGAdult>::iterator itA;
    for (itA = peopleIncoming.begin(); itA != peopleIncoming.end(); ++itA) {
        if (statData.workPositions > 0) {
            itA->tryToWork(1, &workPositions);
        } else {
            //shouldn't happen
            std::cout << "not enough work for incoming people..." << std::endl;
        }
    }

    //BEGIN TESTS
    int workingP = 0;
    std::list<AGHousehold>::iterator itt;
    for (itt = households.begin(); itt != households.end(); ++itt) {
        if (itt->getAdultNbr() == 1) {
            if (itt->getAdults().front().isWorking()) {
                workingP++;
            }
        }
        if (itt->getAdultNbr() == 2) {
            if (itt->getAdults().front().isWorking()) {
                workingP++;
            }
            if (itt->getAdults().back().isWorking()) {
                workingP++;
            }
        }
    }
    std::cout << "  |-> working people: " << peopleIncoming.size() + workingP << std::endl;
    std::cout << "    |-> working people in city: " << workingP << std::endl;
    std::cout << "    |-> working people from outside: " << peopleIncoming.size() << std::endl;
    //END TESTS
}

void
AGCity::carAllocation() {
    statData.hhFarFromPT = 0;
    nbrCars = 0;
    std::list<AGHousehold>::iterator it;
    for (it = households.begin(); it != households.end(); ++it) {
        if (!it->isCloseFromPubTransport(&(statData.busStations))) {
            statData.hhFarFromPT++;
            nbrCars++;
            it->addACar();
        }
        statData.householdsNbr++;
    }
    // new rate: the rate on the people that have'nt any car yet:
    // nR = (R * Drivers - AlreadyCars) / (Drivers - AlreadyCars)
    double newRate = statData.carRate * statData.getPeopleOlderThan(statData.limitAgeChildren) - statData.hhFarFromPT;
    if (statData.getPeopleOlderThan(statData.limitAgeChildren) == statData.hhFarFromPT) {
        newRate = 0.;
    } else {
        newRate /= statData.getPeopleOlderThan(statData.limitAgeChildren) - statData.hhFarFromPT;
    }
    //std::cout << " - " << newRate << std::endl;
    if (newRate < 0 || newRate >= 1) {
        newRate = 0;
    }

    nbrCars = 0;
    int nbrAdults = 0;
    for (it = households.begin(); it != households.end(); ++it) {
        it->generateCars(newRate);
        nbrCars += it->getCarNbr();
        nbrAdults += it->getAdultNbr();
    }
    //TEST RESULTS
    //std::cout << "number of cars: " << nbrCars << std::endl;
    //std::cout << "number of adults: " << statData.getPeopleOlderThan(statData.limitAgeChildren) << std::endl;
    //std::cout << "real number of adults: " << nbrAdults << std::endl;
    //std::cout << "number of people far from public transport: " << statData.hhFarFromPT << std::endl;
    //std::cout << "original rate: " << setprecision(4) << statData.carRate << std::endl;
    //std::cout << "new rate: " << setprecision(4) << newRate << std::endl;
    //std::cout << "real rate: " << setprecision(4) << (double)nbrCars / (double)statData.getPeopleOlderThan(statData.limitAgeChildren) << std::endl;
    //END TEST RESULTS
}

const AGStreet&
AGCity::getStreet(const std::string& edge) {
    /**
     * verify if it is the first time this function is called
     * in this case, we have to complete the streets with the
     * network edges this means that streets are completely
     * loaded (no any more to be read from stat-file)
     */
    if (!streetsCompleted) {
        statData.consolidateStat();
        completeStreets();
        std::cout << "first completed in getStreet() of City: Consolidation of data not needed in ActivityGen any more" << std::endl;
    }
    //rest of the function
    std::vector<AGStreet*>::iterator it = streets.begin();
    while (it != streets.end()) {
        if ((*it)->getID() == edge) {
            return **it;
        }
        ++it;
    }
    std::cout << "===> ERROR: WRONG STREET EDGE (" << edge << ") given and not found in street set." << std::endl;
    throw (std::runtime_error("Street not found with edge id " + edge));
}

const AGStreet&
AGCity::getRandomStreet() {
    if (streets.empty()) {
        throw (std::runtime_error("No street found in this city"));
    }
    return *RandHelper::getRandomFrom(streets);
}


/****************************************************************************/
