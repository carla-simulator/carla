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

#include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
#include "veins/base/messages/AirFrame_m.h"
#include "testutils/Simulation.h"
#include "testutils/AirFrame.h"
#include "testutils/Component.h"

using namespace veins;

SCENARIO("TwoRayInterferenceModel", "[analogueModel]")
{

    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr));
    DummyComponent dc(&ds);

    GIVEN("An AirFrame at 2.4e9 sent from (0,0)")
    {

        AirFrame frame = createAirframe(2.4e9, 10e6, 0, .001, 1);
        TwoRayInterferenceModel tri(&dc, 1.02);
        int dummyId = -1;
        Signal& s = frame.getSignal();
        s.setSenderPoa({{dummyId, Coord(0, 0, 2), Coord(0, 0, 0), simTime()}, {}, nullptr});

        WHEN("the receiver is at (10,0)")
        {
            s.setReceiverPoa({{dummyId, Coord(10, 0, 2), Coord(0, 0, 0), simTime()}, {}, nullptr});

            THEN("TwoRayInterferenceModel drops power from 1 to 959.5e-9")
            {
                tri.filterSignal(&s);
                REQUIRE(s.atFrequency(2.4e9) == Approx(9.5587819943e-07).epsilon(1e-9));
            }
        }

        WHEN("the receiver is at (100,0)")
        {
            s.setReceiverPoa({{dummyId, Coord(100, 0, 2), Coord(0, 0, 0), simTime()}, {}, nullptr});

            THEN("TwoRayInterferenceModel drops power from 1 to 20.3e-9")
            {
                tri.filterSignal(&s);
                REQUIRE(s.atFrequency(2.4e9) == Approx(2.0317806459e-08).epsilon(1e-9));
            }
        }
    }
}
