//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Christian Frank
// part of:     framework implementation developed by tkn

#include "veins/base/utils/Coord.h"

using namespace veins;

const Coord Coord::ZERO = Coord(0.0, 0.0, 0.0);

namespace {
/**
 * On a torus the end and the begin of the axes are connected so you
 * get a circle. On a circle the distance between two points can't be greater
 * than half of the circumference.
 * If the normal distance between two points on one axis is bigger than
 * half of the size there must be a "shorter way" over the border on this axis
 */
static double dist(double coord1, double coord2, double size)
{
    double difference = fabs(coord1 - coord2);
    if (difference == 0)
        // NOTE: event if size is zero
        return 0;
    else {
        ASSERT(size != 0);
        double dist = FWMath::modulo(difference, size);
        return std::min(dist, size - dist);
    }
}
} // namespace

double Coord::sqrTorusDist(const Coord& b, const Coord& size) const
{
    double xDist = dist(x, b.x, size.x);
    double yDist = dist(y, b.y, size.y);
    double zDist = dist(z, b.z, size.z);
    return xDist * xDist + yDist * yDist + zDist * zDist;
}
