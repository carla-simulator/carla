//
// Copyright (C) 2010-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <algorithm>

#include "veins/modules/obstacle/Obstacle.h"

using namespace veins;

using veins::Obstacle;

Obstacle::Obstacle(std::string id, std::string type, double attenuationPerCut, double attenuationPerMeter)
    : visualRepresentation(nullptr)
    , id(id)
    , type(type)
    , attenuationPerCut(attenuationPerCut)
    , attenuationPerMeter(attenuationPerMeter)
{
}

void Obstacle::setShape(Coords shape)
{
    coords = shape;
    bboxP1 = Coord(1e7, 1e7);
    bboxP2 = Coord(-1e7, -1e7);
    for (Coords::const_iterator i = coords.begin(); i != coords.end(); ++i) {
        bboxP1.x = std::min(i->x, bboxP1.x);
        bboxP1.y = std::min(i->y, bboxP1.y);
        bboxP2.x = std::max(i->x, bboxP2.x);
        bboxP2.y = std::max(i->y, bboxP2.y);
    }
}

const Obstacle::Coords& Obstacle::getShape() const
{
    return coords;
}

const Coord Obstacle::getBboxP1() const
{
    return bboxP1;
}

const Coord Obstacle::getBboxP2() const
{
    return bboxP2;
}

bool Obstacle::containsPoint(Coord point) const
{
    bool isInside = false;
    const Obstacle::Coords& shape = getShape();
    Obstacle::Coords::const_iterator i = shape.begin();
    Obstacle::Coords::const_iterator j = (shape.rbegin() + 1).base();
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

namespace {

double segmentsIntersectAt(const Coord& p1From, const Coord& p1To, const Coord& p2From, const Coord& p2To)
{
    double p1x = p1To.x - p1From.x;
    double p1y = p1To.y - p1From.y;
    double p2x = p2To.x - p2From.x;
    double p2y = p2To.y - p2From.y;
    double p1p2x = p1From.x - p2From.x;
    double p1p2y = p1From.y - p2From.y;
    double D = (p1x * p2y - p1y * p2x);

    double p1Frac = (p2x * p1p2y - p2y * p1p2x) / D;
    if (p1Frac < 0 || p1Frac > 1) return -1;

    double p2Frac = (p1x * p1p2y - p1y * p1p2x) / D;
    if (p2Frac < 0 || p2Frac > 1) return -1;

    return p1Frac;
}
} // namespace

std::vector<double> Obstacle::getIntersections(const Coord& senderPos, const Coord& receiverPos) const
{
    std::vector<double> intersectAt;
    const Obstacle::Coords& shape = getShape();
    Obstacle::Coords::const_iterator i = shape.begin();
    Obstacle::Coords::const_iterator j = (shape.rbegin() + 1).base();
    for (; i != shape.end(); j = i++) {
        const Coord& c1 = *i;
        const Coord& c2 = *j;

        double i = segmentsIntersectAt(senderPos, receiverPos, c1, c2);
        if (i != -1) {
            intersectAt.push_back(i);
        }
    }
    std::sort(intersectAt.begin(), intersectAt.end());
    return intersectAt;
}

std::string Obstacle::getType() const
{
    return type;
}

std::string Obstacle::getId() const
{
    return id;
}

double Obstacle::getAttenuationPerCut() const
{
    return attenuationPerCut;
}

double Obstacle::getAttenuationPerMeter() const
{
    return attenuationPerMeter;
}
