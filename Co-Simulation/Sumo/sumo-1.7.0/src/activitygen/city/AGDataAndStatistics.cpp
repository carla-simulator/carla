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
/// @file    AGDataAndStatistics.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    July 2010
///
// Contains various data, statistical values and functions from input used
// by various objects
/****************************************************************************/
#include <config.h>

#include "AGDataAndStatistics.h"
#include <utils/common/RandHelper.h>
#include <cmath>
#include <iomanip>
#define LIMIT_CHILDREN_NUMBER 3


// ===========================================================================
// method definitions
// ===========================================================================
AGDataAndStatistics&
AGDataAndStatistics::getDataAndStatistics() {
    static AGDataAndStatistics ds;
    return ds;
}

int
AGDataAndStatistics::getRandom(int n, int m) {
    if (m < n) {
        return 0;
    }
    int num = RandHelper::rand(m - n);
    num += n;
    return num;
}

int
AGDataAndStatistics::getRandomPopDistributed(int n, int m) {
    if (m < n || n >= limitEndAge) {
        return -1;
    }
    if (m > limitEndAge) {
        m = limitEndAge;
    }
    const double alea = RandHelper::rand(getPropYoungerThan(n), getPropYoungerThan(m));
    for (int a = n; a < m; ++a) {
        if (alea < getPropYoungerThan(a + 1)) {
            return a;
        }
    }
    return -1;
}

int
AGDataAndStatistics::getPoissonsNumberOfChildren(double mean) {
    double alea = RandHelper::rand();
    double cumul = 0;
    for (int nbr = 0; nbr < LIMIT_CHILDREN_NUMBER; ++nbr) {
        cumul += poisson(mean, nbr);
        if (cumul > alea) {
            return nbr;
        }
    }
    return LIMIT_CHILDREN_NUMBER;
}

double
AGDataAndStatistics::poisson(double mean, int occ) {
    return exp(-mean) * pow(mean, occ) / (double)factorial(occ);
}

int
AGDataAndStatistics::factorial(int fact) {
    if (fact > 0) {
        return fact * factorial(fact - 1);
    }
    return 1;
}

void
AGDataAndStatistics::consolidateStat() {
    normalizeMapProb(&beginWorkHours);
    normalizeMapProb(&endWorkHours);
    normalizeMapProb(&population);
    normalizeMapProb(&incoming);
    normalizeMapProb(&outgoing);
    limitEndAge = population.rbegin()->first;

    oldAgeHhProb = (double)getPeopleOlderThan(limitAgeRetirement) / (double)getPeopleOlderThan(limitAgeChildren);
    secondPersProb = (double)(getPeopleOlderThan(limitAgeChildren) - households) / (double)households;
    meanNbrChildren = (double)getPeopleYoungerThan(limitAgeChildren) / ((1 - oldAgeHhProb) * (double)households);
    //cout << " --> oldAgeHhProb = " << setprecision(3) << oldAgeHhProb << "  - retAge? " << getPeopleOlderThan(limitAgeRetirement) << " adAge? " << getPeopleOlderThan(limitAgeChildren) << endl;
    //cout << " --> secondPersProb = " << setprecision(3) << secondPersProb << "  - adAge? " << getPeopleOlderThan(limitAgeChildren) << " hh?" << households << endl;
    //cout << " --> meanNbrChildren = " << setprecision(3) << meanNbrChildren << "  - chAge? " << getPeopleYoungerThan(limitAgeChildren) << endl;
}

double
AGDataAndStatistics::getPropYoungerThan(int age) {
    std::map<int, double>::iterator it;
    double sum = 0;
    int previousAge = 0;
    double prop = 0;

    for (it = population.begin(); it != population.end(); ++it) {
        if (it->first < age) {
            sum += it->second;
        } else if (it->first >= age && previousAge < age) {
            prop = ((double)(age - previousAge) / (double)(it->first - previousAge));
            sum += prop * it->second;
            break;
        }
        previousAge = it->first;
    }
    return sum;
}

int
AGDataAndStatistics::getPeopleYoungerThan(int age) {
    return (int)((double)inhabitants * getPropYoungerThan(age) + .5);
}

int
AGDataAndStatistics::getPeopleOlderThan(int age) {
    return (inhabitants - getPeopleYoungerThan(age));
}

void
AGDataAndStatistics::normalizeMapProb(std::map<int, double>* myMap) {
    double sum = 0;
    std::map<int, double>::iterator it;
    for (it = myMap->begin(); it != myMap->end(); ++it) {
        sum += it->second;
    }
    if (sum == 0) {
        return;
    }
    for (it = myMap->begin(); it != myMap->end(); ++it) {
        it->second = it->second / sum;
    }
}

double
AGDataAndStatistics::getInverseExpRandomValue(double mean, double maxVar) {
    if (maxVar <= 0) {
        return mean;
    }
    double p = RandHelper::rand(static_cast<double>(0.0001), static_cast<double>(1));
    //we have to scale the distribution because maxVar is different from INF
    double scale = exp((-1) * maxVar);
    //new p: scaled
    p = p * (1 - scale) + scale; // p = [scale; 1) ==> (1-p) = (0; 1-scale]

    double variation = (-1) * log(p);
    //decide the side of the mean value
    if (RandHelper::rand(1000) < 500) {
        return mean + variation;
    } else {
        return mean - variation;
    }

}

int
AGDataAndStatistics::getRandomCityGateByIncoming() {
    double alea = RandHelper::rand();
    double total = 0;
    std::map<int, double>::iterator it;
    for (it = incoming.begin(); it != incoming.end(); ++it) {
        total += it->second;
        if (alea < total) {
            return it->first;
        }
    }
    std::cout << "ERROR: incoming at city gates not normalized" << std::endl;
    return 0;
}

int
AGDataAndStatistics::getRandomCityGateByOutgoing() {
    double alea = RandHelper::rand();
    double total = 0;
    std::map<int, double>::iterator it;
    for (it = outgoing.begin(); it != outgoing.end(); ++it) {
        total += it->second;
        if (alea < total) {
            return it->first;
        }
    }
    std::cout << "ERROR: outgoing at city gates not normalized" << std::endl;
    return 0;
}


/****************************************************************************/
