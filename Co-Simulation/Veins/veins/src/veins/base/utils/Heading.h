//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
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

#pragma once

#include "veins/veins.h"

namespace veins {
class Heading;
} // namespace veins

#include "veins/base/utils/Coord.h"

namespace veins {

/**
 * Stores a Heading in rad, with 0 degrees being east and 90 degrees being north.
 */
class VEINS_API Heading {
public:
    static const Heading nan;

    /**
     * Creates an undefined Heading.
     */
    Heading()
        : rad(std::numeric_limits<double>::quiet_NaN())
    {
    }

    /**
     * Creates a new Heading from an angle (in rad, with 0 degrees being east and 90 degrees being north).
     */
    explicit Heading(double rad)
        : rad(rad)
    {
    }

    /**
     * Returns the angle (in rad, with 0 degrees being east and 90 degrees being north).
     */
    double getRad() const
    {
        return rad;
    }

    /**
     * Test for nan.
     */
    bool isNan() const
    {
        return std::isnan(rad);
    }

    /**
     * @brief Returns Coord of a unit vector (with x pointing east and y pointing south).
     * @param length: length of vector (dimensionless)
     */
    Coord toCoord(double length = 1) const
    {
        return Coord(cos(rad) * length, -sin(rad) * length);
    }

    /**
     * @brief Converts Coord to heading (with x pointing east and y pointing south).
     */
    static Heading fromCoord(Coord o)
    {
        return Heading(atan2(-o.y, o.x));
    }

protected:
    double rad;
};

inline std::ostream& operator<<(std::ostream& os, const Heading& o)
{
    return os << "(" << o.getRad() << " rad)";
}

} // namespace veins
