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

#include "veins/base/utils/AntennaPosition.h"
#include "testutils/Simulation.h"

using veins::AntennaPosition;
using veins::Coord;

#ifndef NDEBUG
SCENARIO("Using non-initialized AntennaPosition", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works

    GIVEN("A default-constructed AntennaPosition")
    {
        AntennaPosition p;

        WHEN("Its position is requested")
        {
            THEN("An error should be raised (when in DEBUG mode)")
            {
                REQUIRE_THROWS(p.getPositionAt(0));
            }
        }
    }
}
#endif

SCENARIO("Using AntennaPosition", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works

    GIVEN("An AntennaPosition at (1, 0, 0) moving by (1, 0, 0) each second after 0")
    {
        int hostA = 1;
        Coord posA = Coord(1, 0, 0);
        Coord speedA = Coord(1, 0, 0);
        simtime_t timeA = SimTime(0, SIMTIME_S);
        auto p = AntennaPosition(hostA, posA, speedA, timeA);

        THEN("its x value at t=1 is 2")
        {
            REQUIRE(p.getPositionAt(1).x == 2);
        }

        WHEN("compared with a different antenna")
        {
            int hostB = 2;
            auto p2 = AntennaPosition(hostB, posA, speedA, timeA);
            THEN("it is found to be different")
            {
                REQUIRE(p.isSameAntenna(p2) == false);
            }
        }
    }
}
