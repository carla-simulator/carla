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
/// @file    AGPosition.cpp
/// @author  Piotr Woznica
/// @author  Walter Bamberger
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    July 2010
///
// References a street of the city and defines a position in this street
/****************************************************************************/
#include <config.h>

#include "AGPosition.h"
#include "AGStreet.h"
#include "router/ROEdge.h"
#include "utils/common/RandHelper.h"
#include <iostream>
#include <limits>


// ===========================================================================
// method definitions
// ===========================================================================
AGPosition::AGPosition(const AGStreet& str, double pos) :
    street(&str), position(pos), pos2d(compute2dPosition()) {
}


AGPosition::AGPosition(const AGStreet& str) :
    street(&str), position(randomPositionInStreet(str)), pos2d(compute2dPosition()) {
}


void
AGPosition::print() const {
    std::cout << "- AGPosition: *Street=" << street << " position=" << position << "/" << street->getLength() << std::endl;
}


bool
AGPosition::operator==(const AGPosition& pos) const {
    return pos2d.almostSame(pos.pos2d);
}


double
AGPosition::distanceTo(const AGPosition& otherPos) const {
    return pos2d.distanceTo(otherPos.pos2d);
}


double
AGPosition::minDistanceTo(const std::list<AGPosition>& positions) const {
    double minDist = std::numeric_limits<double>::infinity();
    double tempDist;
    std::list<AGPosition>::const_iterator itt;

    for (itt = positions.begin(); itt != positions.end(); ++itt) {
        tempDist = this->distanceTo(*itt);
        if (tempDist < minDist) {
            minDist = tempDist;
        }
    }
    return minDist;
}


double
AGPosition::minDistanceTo(const std::map<int, AGPosition>& positions) const {
    double minDist = std::numeric_limits<double>::infinity();
    double tempDist;
    std::map<int, AGPosition>::const_iterator itt;

    for (itt = positions.begin(); itt != positions.end(); ++itt) {
        tempDist = this->distanceTo(itt->second);
        if (tempDist < minDist) {
            minDist = tempDist;
        }
    }
    return minDist;
}


const AGStreet&
AGPosition::getStreet() const {
    return *street;
}


double
AGPosition::getPosition() const {
    return position;
}


double
AGPosition::randomPositionInStreet(const AGStreet& s) {
    return RandHelper::rand(0.0, s.getLength());
}


Position
AGPosition::compute2dPosition() const {
    // P = From + pos*(To - From) = pos*To + (1-pos)*From
    Position From = street->getFromJunction()->getPosition();
    Position To = street->getToJunction()->getPosition();
    Position position2d(To);

    position2d.sub(From);
    position2d.mul(position / street->getLength());
    position2d.add(From);

    return position2d;
}


/****************************************************************************/
