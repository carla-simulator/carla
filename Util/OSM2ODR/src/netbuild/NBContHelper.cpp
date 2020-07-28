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
/// @file    NBContHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Some methods for traversing lists of edges
/****************************************************************************/
#include <config.h>

#include <vector>
#include <map>
#include <cassert>
#include "NBContHelper.h"
#include <utils/geom/GeomHelper.h>


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * utility methods
 * ----------------------------------------------------------------------- */
void
NBContHelper::nextCW(const EdgeVector& edges, EdgeVector::const_iterator& from) {
    from++;
    if (from == edges.end()) {
        from = edges.begin();
    }
}


void
NBContHelper::nextCCW(const EdgeVector& edges, EdgeVector::const_iterator& from) {
    if (from == edges.begin()) {
        from = edges.end() - 1;
    } else {
        --from;
    }
}


std::ostream&
NBContHelper::out(std::ostream& os, const std::vector<bool>& v) {
    for (std::vector<bool>::const_iterator i = v.begin(); i != v.end(); i++) {
        os << *i;
    }
    return os;
}


NBEdge*
NBContHelper::findConnectingEdge(const EdgeVector& edges,
                                 NBNode* from, NBNode* to) {
    for (EdgeVector::const_iterator i = edges.begin(); i != edges.end(); i++) {
        if ((*i)->getToNode() == to && (*i)->getFromNode() == from) {
            return *i;
        }
    }
    return nullptr;
}



double
NBContHelper::maxSpeed(const EdgeVector& ev) {
    assert(ev.size() > 0);
    double max = (*(ev.begin()))->getSpeed();
    for (EdgeVector::const_iterator i = ev.begin() + 1; i != ev.end(); i++) {
        max =
            max > (*i)->getSpeed()
            ? max : (*i)->getSpeed();
    }
    return max;
}



/* -------------------------------------------------------------------------
 * methods from node_with_incoming_finder
 * ----------------------------------------------------------------------- */
NBContHelper::node_with_incoming_finder::node_with_incoming_finder(const NBEdge* const e)
    : myEdge(e) {}


bool
NBContHelper::node_with_incoming_finder::operator()(const NBNode* const n) const {
    const EdgeVector& incoming = n->getIncomingEdges();
    return std::find(incoming.begin(), incoming.end(), myEdge) != incoming.end();
}



/* -------------------------------------------------------------------------
 * methods from node_with_outgoing_finder
 * ----------------------------------------------------------------------- */
NBContHelper::node_with_outgoing_finder::node_with_outgoing_finder(const NBEdge* const e)
    : myEdge(e) {}


bool
NBContHelper::node_with_outgoing_finder::operator()(const NBNode* const n) const {
    const EdgeVector& outgoing = n->getOutgoingEdges();
    return std::find(outgoing.begin(), outgoing.end(), myEdge) != outgoing.end();
}


/* -------------------------------------------------------------------------
 * methods from edge_with_destination_finder
 * ----------------------------------------------------------------------- */
NBContHelper::edge_with_destination_finder::edge_with_destination_finder(NBNode* dest)
    : myDestinationNode(dest) {}


bool
NBContHelper::edge_with_destination_finder::operator()(NBEdge* e) const {
    return e->getToNode() == myDestinationNode;
}


/* -------------------------------------------------------------------------
 * methods from relative_outgoing_edge_sorter
 * ----------------------------------------------------------------------- */
bool
NBContHelper::relative_outgoing_edge_sorter::operator()(const NBEdge* e1, const NBEdge* e2) const {
    assert(e1 != nullptr && e2 != nullptr);
    double relAngle1 = NBHelpers::normRelAngle(myAngle, e1->getStartAngle());
    double relAngle2 = NBHelpers::normRelAngle(myAngle, e2->getStartAngle());
    const double length1 = e1->getGeometry().length2D();
    const double length2 = e2->getGeometry().length2D();

    double lookAhead = 2 * NBEdge::ANGLE_LOOKAHEAD;
    while (fabs(relAngle1 - relAngle2) < 3.0) {
        // look at further geometry segments to resolve ambiguity
        const double offset1 = MAX2(0.0, MIN2(length1, lookAhead));
        const double offset2 = MAX2(0.0, MIN2(length2, lookAhead));
        const Position referencePos1 = e1->getGeometry().positionAtOffset2D(offset1);
        const Position referencePos2 = e2->getGeometry().positionAtOffset2D(offset2);
        const double angle1 = GeomHelper::legacyDegree(e1->getFromNode()->getPosition().angleTo2D(referencePos1), true);
        const double angle2 = GeomHelper::legacyDegree(e2->getFromNode()->getPosition().angleTo2D(referencePos2), true);
        relAngle1 = NBHelpers::normRelAngle(myAngle, angle1);
        relAngle2 = NBHelpers::normRelAngle(myAngle, angle2);
        if (lookAhead > MAX2(length1, length2)) {
            break;
        }
        lookAhead *= 2;
    }
    if (fabs(relAngle1 - relAngle2) < NUMERICAL_EPS) {
        // need to break ties for windows debug version, numerical id may be -1 for both
        return e1->getID() > e2->getID();
    }
    return relAngle1 > relAngle2;
}


/* -------------------------------------------------------------------------
 * methods from relative_incoming_edge_sorter
 * ----------------------------------------------------------------------- */
bool
NBContHelper::relative_incoming_edge_sorter::operator()(const NBEdge* e1, const NBEdge* e2) const {
    assert(e1 != nullptr && e2 != nullptr);
    double relAngle1 = NBHelpers::normRelAngle(myAngle, e1->getEndAngle());
    double relAngle2 = NBHelpers::normRelAngle(myAngle, e2->getEndAngle());
    const double length1 = e1->getGeometry().length2D();
    const double length2 = e2->getGeometry().length2D();

    double lookAhead = 2 * NBEdge::ANGLE_LOOKAHEAD;
    while (fabs(relAngle1 - relAngle2) < 3.0) {
        // look at further geometry segments to resolve ambiguity
        const double offset1 = MAX2(0.0, MIN2(length1, length1 - lookAhead));
        const double offset2 = MAX2(0.0, MIN2(length2, length2 - lookAhead));
        const Position referencePos1 = e1->getGeometry().positionAtOffset2D(offset1);
        const Position referencePos2 = e2->getGeometry().positionAtOffset2D(offset2);
        const double angle1 = GeomHelper::legacyDegree(referencePos1.angleTo2D(e1->getToNode()->getPosition()), true);
        const double angle2 = GeomHelper::legacyDegree(referencePos2.angleTo2D(e2->getToNode()->getPosition()), true);
        relAngle1 = NBHelpers::normRelAngle(myAngle, angle1);
        relAngle2 = NBHelpers::normRelAngle(myAngle, angle2);
        if (lookAhead > MAX2(length1, length2)) {
            break;
        }
        lookAhead *= 2;
    }
    if (fabs(relAngle1 - relAngle2) < NUMERICAL_EPS) {
        // need to break ties for windows debug version, numerical id may be -1 for both
        return e1->getID() > e2->getID();
    }
    return relAngle1 > relAngle2;
}


std::ostream&
operator<<(std::ostream& os, const EdgeVector& ev) {
    for (EdgeVector::const_iterator i = ev.begin(); i != ev.end(); i++) {
        if (i != ev.begin()) {
            os << ", ";
        }
        os << (*i)->getID();
    }
    return os;
}


double
NBContHelper::getMaxSpeed(const EdgeVector& edges) {
    if (edges.size() == 0) {
        return -1;
    }
    double ret = (*(edges.begin()))->getSpeed();
    for (EdgeVector::const_iterator i = edges.begin() + 1; i != edges.end(); i++) {
        if ((*i)->getSpeed() > ret) {
            ret = (*i)->getSpeed();
        }
    }
    return ret;
}


double
NBContHelper::getMinSpeed(const EdgeVector& edges) {
    if (edges.size() == 0) {
        return -1;
    }
    double ret = (*(edges.begin()))->getSpeed();
    for (EdgeVector::const_iterator i = edges.begin() + 1; i != edges.end(); i++) {
        if ((*i)->getSpeed() < ret) {
            ret = (*i)->getSpeed();
        }
    }
    return ret;
}


bool
NBContHelper::edge_by_angle_to_nodeShapeCentroid_sorter::operator()(const NBEdge* e1, const NBEdge* e2) const {
    assert(e1->getFromNode() == myNode || e1->getToNode() == myNode);
    assert(e2->getFromNode() == myNode || e2->getToNode() == myNode);
    const double angle1 = e1->getAngleAtNodeToCenter(myNode);
    const double angle2 = e2->getAngleAtNodeToCenter(myNode);
    const double absDiff = fabs(angle1 - angle2);

    // cannot trust the angle difference hence a heuristic:
    if (absDiff < 2 || absDiff > (360 - 2)) {
        const bool sameDir = ((e1->getFromNode() == myNode && e2->getFromNode() == myNode)
                              || (e1->getToNode() == myNode && e2->getToNode() == myNode));
        if (sameDir) {
            // put edges that allow pedestrians on the 'outside', but be aware if both allow / disallow
            const bool e1Peds = (e1->getPermissions() & SVC_PEDESTRIAN) != 0;
            const bool e2Peds = (e2->getPermissions() & SVC_PEDESTRIAN) != 0;
            if (e1->getToNode() == myNode) {
                if (e1Peds && !e2Peds) {
                    return true;
                } else if (!e1Peds && e2Peds) {
                    return false;
                }
            } else {
                if (!e1Peds && e2Peds) {
                    return true;
                } else if (e1Peds && !e2Peds) {
                    return false;
                }
            }
            // break ties to ensure strictly weak ordering
            return e1->getID() < e2->getID();
        } else {
            // sort incoming before outgoing, no need to break ties here
            return e1->getToNode() == myNode;
        }
    }
    return angle1 < angle2;
}


/****************************************************************************/
