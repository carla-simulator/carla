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
#include "veins/base/utils/Heading.h"

using veins::Coord;
using veins::Heading;

SCENARIO("Heading", "[heading]")
{
    GIVEN("An undefined heading")
    {
        auto a = Heading();

        WHEN("Checking for NaN")
        {
            auto b = a.isNan();

            THEN("The result is true")
            {
                REQUIRE(b == true);
            }
        }
    }

    GIVEN("A heading of 0 degrees")
    {
        auto a = Heading(0.0);

        WHEN("converted to rad")
        {
            auto b = a.getRad();

            THEN("the value is 0")
            {
                REQUIRE(b == Approx(0.0));
            }
        }

        WHEN("converted to Coord")
        {
            auto b = a.toCoord();

            THEN("the value is (1, 0, 0)")
            {
                REQUIRE(b.x == Approx(1.0).margin(1e-9));
                REQUIRE(b.y == Approx(0.0).margin(1e-9));
                REQUIRE(b.z == Approx(0.0).margin(1e-9));
            }
        }
    }

    GIVEN("A heading of 90 degrees")
    {
        auto a = Heading(M_PI / 180 * 90);

        WHEN("converted to Coord")
        {
            auto b = a.toCoord();

            THEN("the value is (0, -1, 0)")
            {
                REQUIRE(b.x == Approx(0.0).margin(1e-9));
                REQUIRE(b.y == Approx(-1.0).margin(1e-9));
                REQUIRE(b.z == Approx(0.0).margin(1e-9));
            }
        }
    }

    GIVEN("A Coord of (0, -1, 0)")
    {
        auto a = Coord(0, -1, 0);

        WHEN("Converted to heading")
        {
            auto b = Heading::fromCoord(a);

            THEN("the value is 90 degrees")
            {
                REQUIRE(b.getRad() == Approx(M_PI / 180 * 90));
            }
        }
    }

    GIVEN("A new Coord of length 2 from a -90 degree heading")
    {
        auto c2 = Heading(-M_PI / 180 * 90).toCoord(2);

        THEN("its value is (0,2,0)")
        {
            REQUIRE(c2.x == Approx(0.0).margin(1e-9));
            REQUIRE(c2.y == Approx(2.0).margin(1e-9));
            REQUIRE(c2.z == 0);
        }

        WHEN("changing its heading by a yaw angle of -90 degrees")
        {
            auto c3 = c2.rotatedYaw(-M_PI / 180 * 90);

            THEN("its x value is 2")
            {
                REQUIRE(c3.x == Approx(2).margin(1e-9));
            }

            THEN("its y value is 0")
            {
                REQUIRE(c3.y == Approx(0).margin(1e-9));
            }

            THEN("its z value is still 0")
            {
                REQUIRE(c3.z == 0);
            }
        }
    }

    GIVEN("A Coord that is (1,1,0)")
    {
        auto c = Coord(1, 1, 0);

        WHEN("converting it to heading")
        {
            auto d = Heading::fromCoord(c);

            THEN("its heading value is -45 degrees")
            {
                REQUIRE(d.getRad() == M_PI / 180 * (-45));
            }
        }
    }
}
