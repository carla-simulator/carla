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
/// @file    MSVehicleContainer.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 12 Mar 2001
///
// vehicles sorted by their departures
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include "MSVehicle.h"
#include "MSVehicleContainer.h"


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * methods from MSVehicleContainer::VehicleDepartureVectorSortCrit
 * ----------------------------------------------------------------------- */
bool
MSVehicleContainer::VehicleDepartureVectorSortCrit::operator()
(const VehicleDepartureVector& e1, const VehicleDepartureVector& e2) const {
    return e1.first < e2.first;
}



/* -------------------------------------------------------------------------
 * methods from MSVehicleContainer::DepartFinder
 * ----------------------------------------------------------------------- */
MSVehicleContainer::DepartFinder::DepartFinder(SUMOTime time)
    : myTime(time) {}


bool
MSVehicleContainer::DepartFinder::operator()
(const VehicleDepartureVector& e) const {
    return myTime + DELTA_T > e.first && myTime <= e.first;
}



/* -------------------------------------------------------------------------
 * methods from MSVehicleContainer
 * ----------------------------------------------------------------------- */
MSVehicleContainer::MSVehicleContainer(int capacity)
    : currentSize(0), array(capacity + 1, VehicleDepartureVector()) {}


MSVehicleContainer::~MSVehicleContainer() {
    // !!! vehicles are deleted in MSVehicle
}


void
MSVehicleContainer::add(SUMOVehicle* veh) {
    // check whether a new item shall be added or the vehicle may be
    //  added to an existing list
    VehicleHeap::iterator i =
        find_if(array.begin() + 1, array.begin() + currentSize + 1, DepartFinder(veh->getParameter().depart));
    if (currentSize == 0 || i == array.begin() + currentSize + 1) {
        // a new heap-item is necessary
        const SUMOTime delay = veh->getParameter().depart % DELTA_T;
        const SUMOTime depart = veh->getParameter().depart + (delay == 0 ? 0 : DELTA_T - delay);
        VehicleDepartureVector newElem(depart, VehicleVector());
        newElem.second.push_back(veh);
        addReplacing(newElem);
    } else {
        // add vehicle to an existing heap-item
        (*i).second.push_back(veh);
    }
}


void
MSVehicleContainer::remove(SUMOVehicle* veh) {
    // check whether a new item shall be added or the vehicle may be
    //  added to an existing list
    VehicleHeap::iterator i =
        find_if(array.begin() + 1, array.begin() + currentSize + 1, DepartFinder(veh->getParameter().depart));
    if (!(currentSize == 0 || i == array.begin() + currentSize + 1)) {
        // remove vehicle from an existing heap-item
        (*i).second.erase(std::remove((*i).second.begin(), (*i).second.end(), veh), (*i).second.end());
    }
}


void
MSVehicleContainer::add(SUMOTime time, const VehicleVector& cont) {
    VehicleHeap::iterator j =
        find_if(array.begin() + 1, array.begin() + currentSize + 1,
                DepartFinder(time));
    if (currentSize == 0 || j == array.begin() + currentSize + 1) {
        VehicleDepartureVector newElem(time,
                                       VehicleVector(cont));
        addReplacing(newElem);
    } else {
        VehicleVector& stored = (*j).second;
        stored.reserve(stored.size() + cont.size());
        copy(cont.begin(), cont.end(), back_inserter(stored));
    }
}



void
MSVehicleContainer::addReplacing(const VehicleDepartureVector& x) {
    if (isFull()) {
        std::vector<VehicleDepartureVector> array2((array.size() - 1) * 2 + 1, VehicleDepartureVector());
        for (int i = (int)array.size(); i-- > 0;) {
            assert(i < (int)array2.size());
            array2[i] = array[i];
        }
        array = array2;
    }

    // Percolate up
    int hole = ++currentSize;
    for (; hole > 1 && (x.first < array[ hole / 2 ].first); hole /= 2) {
        assert((int)array.size() > hole);
        array[hole] = array[ hole / 2 ];
    }
    assert((int)array.size() > hole);
    array[hole] = x;
}


bool
MSVehicleContainer::anyWaitingBefore(SUMOTime time) const {
    return !isEmpty() && topTime() <= time;
}


const MSVehicleContainer::VehicleVector&
MSVehicleContainer::top() {
    if (isEmpty()) {
        throw 1;    //!!!Underflow( );
    }
    assert(array.size() > 1);
    return array[ 1 ].second;
}


SUMOTime
MSVehicleContainer::topTime() const {
    if (isEmpty()) {
        throw 1;    //!!!Underflow( );
    }
    assert(array.size() > 1);
    return array[ 1 ].first;
}


void
MSVehicleContainer::pop()

{
    if (isEmpty()) {
        throw 1;    //!!!Underflow( );
    }

    assert(array.size() > 1);
    array[ 1 ] = array[ currentSize-- ];
    percolateDown(1);
}


bool
MSVehicleContainer::isEmpty() const {
    return currentSize == 0;
}


bool
MSVehicleContainer::isFull() const {
    return currentSize >= ((int) array.size()) - 1;
}


void
MSVehicleContainer::percolateDown(int hole) {
    int child;
    assert((int)array.size() > hole);
    VehicleDepartureVector tmp = array[ hole ];

    for (; hole * 2 <= currentSize; hole = child) {
        child = hole * 2;
        if (child != currentSize && (array[child + 1].first < array[child].first)) {
            child++;
        }
        if ((array[ child ].first < tmp.first)) {
            assert((int)array.size() > hole);
            array[hole] = array[child];
        } else {
            break;
        }
    }
    assert((int)array.size() > hole);
    array[hole] = tmp;
}


int
MSVehicleContainer::size() const {
    return currentSize;
}


void
MSVehicleContainer::showArray() const {
    for (VehicleHeap::const_iterator i = array.begin() + 1; i != array.begin() + currentSize + 1; ++i) {
        if (i != array.begin() + 1) {
            std::cout << ", ";
        }
        std::cout << (*i).first;
    }
    std::cout << std::endl << "-------------------------" << std::endl;
}


std::ostream& operator << (std::ostream& strm, MSVehicleContainer& cont) {
    strm << "------------------------------------" << std::endl;
    while (!cont.isEmpty()) {
        const MSVehicleContainer::VehicleVector& v = cont.top();
        for (MSVehicleContainer::VehicleVector::const_iterator i = v.begin(); i != v.end(); ++i) {
            strm << (*i)->getParameter().depart << std::endl;
        }
        cont.pop();
    }
    return strm;
}

void
MSVehicleContainer::clearState() {
    for (VehicleDepartureVector& vdv : array) {
        vdv.first = 0;
        vdv.second.clear();
    }
    currentSize = 0;
}

/****************************************************************************/
