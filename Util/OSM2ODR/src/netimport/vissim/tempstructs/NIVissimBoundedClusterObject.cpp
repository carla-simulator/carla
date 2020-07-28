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
/// @file    NIVissimBoundedClusterObject.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>


#include <cassert>
#include <utils/geom/Boundary.h>
#include "NIVissimBoundedClusterObject.h"

NIVissimBoundedClusterObject::ContType NIVissimBoundedClusterObject::myDict;

NIVissimBoundedClusterObject::NIVissimBoundedClusterObject()
    : myBoundary(nullptr), myClusterID(-1) {
    myDict.insert(this);
}


NIVissimBoundedClusterObject::~NIVissimBoundedClusterObject() {
    delete myBoundary;
}


bool
NIVissimBoundedClusterObject::crosses(const AbstractPoly& poly,
                                      double offset) const {
    assert(myBoundary != 0 && myBoundary->xmax() >= myBoundary->xmin());
    return myBoundary->overlapsWith(poly, offset);
}


void
NIVissimBoundedClusterObject::inCluster(int id) {
    myClusterID = id;
}


bool
NIVissimBoundedClusterObject::clustered() const {
    return myClusterID > 0;
}


void
NIVissimBoundedClusterObject::closeLoading() {
    for (ContType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        (*i)->computeBounding();
    }
}


const Boundary&
NIVissimBoundedClusterObject::getBoundary() const {
    return *myBoundary;
}


/****************************************************************************/
