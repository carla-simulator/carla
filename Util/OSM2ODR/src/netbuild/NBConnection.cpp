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
/// @file    NBConnection.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The class holds a description of a connection between two edges
/****************************************************************************/
#include <config.h>

#include <sstream>
#include <iostream>
#include <cassert>
#include "NBEdgeCont.h"
#include "NBEdge.h"
#include "NBConnection.h"


// ===========================================================================
// static members
// ===========================================================================
const int NBConnection::InvalidTlIndex = -1;
const NBConnection NBConnection::InvalidConnection("invalidFrom", nullptr, "invalidTo", nullptr);

// ===========================================================================
// method definitions
// ===========================================================================
NBConnection::NBConnection(NBEdge* from, NBEdge* to) :
    myFrom(from), myTo(to),
    myFromID(from->getID()), myToID(to->getID()),
    myFromLane(-1), myToLane(-1),
    myTlIndex(InvalidTlIndex),
    myTlIndex2(InvalidTlIndex) {
}


NBConnection::NBConnection(const std::string& fromID, NBEdge* from,
                           const std::string& toID, NBEdge* to) :
    myFrom(from), myTo(to),
    myFromID(fromID), myToID(toID),
    myFromLane(-1), myToLane(-1),
    myTlIndex(InvalidTlIndex),
    myTlIndex2(InvalidTlIndex) {
}


NBConnection::NBConnection(NBEdge* from, int fromLane,
                           NBEdge* to, int toLane, int tlIndex, int tlIndex2) :
    myFrom(from), myTo(to),
    myFromLane(fromLane), myToLane(toLane),
    myTlIndex(tlIndex),
    myTlIndex2(tlIndex2) {
    /* @todo what should we assert here?
    assert(myFromLane<0||from->getNumLanes()>(int) myFromLane);
    assert(myToLane<0||to->getNumLanes()>(int) myToLane);
    */
    myFromID = from != nullptr ? from->getID() : "";
    myToID = to != nullptr ? to->getID() : "";
}


NBConnection::~NBConnection() {}


NBConnection::NBConnection(const NBConnection& c) :
    myFrom(c.myFrom), myTo(c.myTo),
    myFromID(c.myFromID), myToID(c.myToID),
    myFromLane(c.myFromLane), myToLane(c.myToLane),
    myTlIndex(c.myTlIndex),
    myTlIndex2(c.myTlIndex2) {
}


NBEdge*
NBConnection::getFrom() const {
    return myFrom;
}


NBEdge*
NBConnection::getTo() const {
    return myTo;
}


bool
NBConnection::replaceFrom(NBEdge* which, NBEdge* by) {
    if (myFrom == which) {
        myFrom = by;
        if (myFrom != nullptr) {
            myFromID = myFrom->getID();
        } else {
            myFromID = "invalidFrom";
        }
        return true;
    }
    return false;
}


bool
NBConnection::replaceFrom(NBEdge* which, int whichLane,
                          NBEdge* by, int byLane) {
    if (myFrom == which && (myFromLane == whichLane || myFromLane < 0 || whichLane < 0)) {
        myFrom = by;
        if (myFrom != nullptr) {
            myFromID = myFrom->getID();
        } else {
            myFromID = "invalidFrom";
        }
        if (byLane >= 0) {
            myFromLane = byLane;
        }
        return true;
    }
    return false;
}


bool
NBConnection::replaceTo(NBEdge* which, NBEdge* by) {
    if (myTo == which) {
        myTo = by;
        if (myTo != nullptr) {
            myToID = myTo->getID();
        } else {
            myToID = "invalidTo";
        }
        return true;
    }
    return false;
}


bool
NBConnection::replaceTo(NBEdge* which, int whichLane,
                        NBEdge* by, int byLane) {
    if (myTo == which && (myToLane == whichLane || myFromLane < 0 || whichLane < 0)) {
        myTo = by;
        if (myTo != nullptr) {
            myToID = myTo->getID();
        } else {
            myToID = "invalidTo";
        }
        if (byLane >= 0) {
            myToLane = byLane;
        }
        return true;
    }
    return false;
}


bool
operator<(const NBConnection& c1, const NBConnection& c2) {
    if (c1.myFromID   != c2.myFromID) {
        return c1.myFromID   < c2.myFromID;
    }
    if (c1.myToID     != c2.myToID) {
        return c1.myToID     < c2.myToID;
    }
    if (c1.myFromLane != c2.myFromLane) {
        return c1.myFromLane < c2.myFromLane;
    }
    return c1.myToLane < c2.myToLane;
}


bool
NBConnection::operator==(const NBConnection& c) const {
    return (myFrom    == c.myFrom     && myTo    == c.myTo &&
            myFromID  == c.myFromID   && myToID  == c.myToID &&
            myFromLane == c.myFromLane && myToLane == c.myToLane &&
            myTlIndex == c.myTlIndex &&
            myTlIndex2 == c.myTlIndex2);
}


bool
NBConnection::check(const NBEdgeCont& ec) {
    myFrom = checkFrom(ec);
    myTo = checkTo(ec);
    return myFrom != nullptr && myTo != nullptr;
}


NBEdge*
NBConnection::checkFrom(const NBEdgeCont& ec) {
    NBEdge* e = ec.retrieve(myFromID);
    // ok, the edge was not changed
    if (e == myFrom) {
        return myFrom;
    }
    // try to get the edge
    return ec.retrievePossiblySplit(myFromID, myToID, true);
}


NBEdge*
NBConnection::checkTo(const NBEdgeCont& ec) {
    NBEdge* e = ec.retrieve(myToID);
    // ok, the edge was not changed
    if (e == myTo) {
        return myTo;
    }
    // try to get the edge
    return ec.retrievePossiblySplit(myToID, myFromID, false);
}


std::string
NBConnection::getID() const {
    std::stringstream str;
    str << myFromID << "_" << myFromLane << "->" << myToID << "_" << myToLane;
    return str.str();
}


int
NBConnection::getFromLane() const {
    return myFromLane;
}


int
NBConnection::getToLane() const {
    return myToLane;
}


void
NBConnection::shiftLaneIndex(NBEdge* edge, int offset, int threshold) {
    if (myFrom == edge && myFromLane > threshold) {
        myFromLane += offset;
    } else if (myTo == edge && myToLane > threshold) {
        myToLane += offset;
    }
}


std::ostream&
operator<<(std::ostream& os, const NBConnection& c) {
    os
            << "Con(from=" << Named::getIDSecure(c.getFrom())
            << " fromLane=" << c.getFromLane()
            << " to=" << Named::getIDSecure(c.getTo())
            << " toLane=" << c.getToLane()
            << " tlIndex=" << c.getTLIndex()
            << ")";
    return os;
}


/****************************************************************************/
