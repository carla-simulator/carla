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

#include "catch2/catch.hpp"

#include "veins/base/utils/Coord.h"

using veins::Coord;

SCENARIO("Coord", "[coord]")
{

    GIVEN("A Coord that is (1,2,3)")
    {
        auto c = Coord(1, 2, 3);

        THEN("its z value is 3")
        {
            REQUIRE(c.z == 3);
        }

        WHEN("changing its heading by a yaw angle by 30 degrees")
        {
            auto c2 = c.rotatedYaw(M_PI / 180 * 30);

            THEN("its x value is -1+0.5*sqrt(3)")
            {
                REQUIRE(c2.x == Approx(-1 + 0.5 * sqrt(3)));
            }

            THEN("its y value is 0.5+sqrt(3)")
            {
                REQUIRE(c2.y == Approx(0.5 + sqrt(3)));
            }

            THEN("its z value is still 3")
            {
                REQUIRE(c2.z == 3);
            }
        }

        WHEN("inverting its y axis")
        {
            auto c2 = c.flippedY();

            THEN("its z value is still 3")
            {
                REQUIRE(c2.z == 3);
            }

            THEN("its y value is -2")
            {
                REQUIRE(c2.y == -2);
            }
        }
    }
}
