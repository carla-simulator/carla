//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <set>
#include <limits>
#include "veins/modules/obstacle/MobileHostObstacle.h"
#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/Heading.h"

using veins::Coord;
using veins::MobileHostObstacle;

namespace {

bool isPointInObstacle(Coord point, const MobileHostObstacle::Coords& shape)
{
    bool isInside = false;
    auto i = shape.begin();
    auto j = (shape.rbegin() + 1).base();
    for (; i != shape.end(); j = i++) {
        bool inYRangeUp = (point.y >= i->y) && (point.y < j->y);
        bool inYRangeDown = (point.y >= j->y) && (point.y < i->y);
        bool inYRange = inYRangeUp || inYRangeDown;
        if (!inYRange) continue;
        bool intersects = point.x < (i->x + ((point.y - i->y) * (j->x - i->x) / (j->y - i->y)));
        if (!intersects) continue;
        isInside = !isInside;
    }
    return isInside;
}

double segmentsIntersectAt(Coord p1From, Coord p1To, Coord p2From, Coord p2To)
{
    Coord p1Vec = p1To - p1From;
    Coord p2Vec = p2To - p2From;
    Coord p1p2 = p1From - p2From;

    double D = (p1Vec.x * p2Vec.y - p1Vec.y * p2Vec.x);

    double p1Frac = (p2Vec.x * p1p2.y - p2Vec.y * p1p2.x) / D;
    if (p1Frac < 0 || p1Frac > 1) return -1;

    double p2Frac = (p1Vec.x * p1p2.y - p1Vec.y * p1p2.x) / D;
    if (p2Frac < 0 || p2Frac > 1) return -1;

    return p1Frac;
}

} // namespace

MobileHostObstacle::Coords MobileHostObstacle::getShape(simtime_t t) const
{
    double l = getLength();
    double o = getHostPositionOffset(); // this is the shift we have to undo in order to (given the OMNeT++ host position) get the car's front bumper position
    double w = getWidth() / 2;
    const BaseMobility* m = getMobility();
    Coord p = m->getPositionAt(t);
    double a = Heading::fromCoord(m->getCurrentOrientation()).getRad();

    Coords shape;
    shape.push_back(p + Coord(-(l - o), -w).rotatedYaw(-a));
    shape.push_back(p + Coord(+o, -w).rotatedYaw(-a));
    shape.push_back(p + Coord(+o, +w).rotatedYaw(-a));
    shape.push_back(p + Coord(-(l - o), +w).rotatedYaw(-a));

    return shape;
}

bool MobileHostObstacle::maybeInBounds(double x1, double y1, double x2, double y2, simtime_t t) const
{
    double l = getLength();
    double o = getHostPositionOffset(); // this is the shift we have to undo in order to (given the OMNeT++ host position) get the car's front bumper position
    double w = getWidth() / 2;
    const BaseMobility* m = getMobility();
    Coord p = m->getPositionAt(t);

    double lw = std::max(l, w);

    double xx1 = p.x - std::abs(o) - lw;
    double xx2 = p.x + std::abs(o) + lw;
    double yy1 = p.y - std::abs(o) - lw;
    double yy2 = p.y + std::abs(o) + lw;

    if (xx2 < x1) return false;
    if (xx1 > x2) return false;
    if (yy2 < y1) return false;
    if (yy1 > y2) return false;

    return true;
}

double MobileHostObstacle::getIntersectionPoint(const Coord& senderPos, const Coord& receiverPos, simtime_t t) const
{
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    MobileHostObstacle::Coords shape = getShape(t);

    // shortcut if sender is inside
    bool senderInside = isPointInObstacle(senderPos, shape);
    if (senderInside) return 0;

    // get a list of points (in [0, 1]) along the line between sender and receiver where the beam intersects with this obstacle
    std::multiset<double> intersectAt;
    bool doesIntersect = false;
    MobileHostObstacle::Coords::const_iterator i = shape.begin();
    MobileHostObstacle::Coords::const_iterator j = (shape.rbegin() + 1).base();
    for (; i != shape.end(); j = i++) {
        Coord c1 = *i;
        Coord c2 = *j;

        double inter = segmentsIntersectAt(senderPos, receiverPos, c1, c2);
        if (inter != -1) {
            doesIntersect = true;
            EV << "intersect: " << inter << endl;
            intersectAt.insert(inter);
        }
    }

    // shortcut if no intersections
    if (!doesIntersect) {
        bool receiverInside = isPointInObstacle(receiverPos, shape);
        if (receiverInside) return senderPos.distance(receiverPos);
        return not_a_number;
    }

    return (*intersectAt.begin() * senderPos.distance(receiverPos));
}
