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

SCENARIO("Integers can be added and subtracted", "[helloworld]")
{

    GIVEN("An integer that is 42")
    {
        int i = 42;
        REQUIRE(i == 42);

        WHEN("take away one")
        {
            i -= 1;

            THEN("the integer is 41")
            {
                REQUIRE(i == 41);
            }
        }

        WHEN("add one")
        {
            i += 1;

            THEN("the integer is 43")
            {
                REQUIRE(i == 43);
            }
        }
    }
}
