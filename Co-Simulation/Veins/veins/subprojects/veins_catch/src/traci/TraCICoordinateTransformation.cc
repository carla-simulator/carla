//
// Copyright (C) 2018-2019 Dominik S. Buse <buse@ccs-labs.org>
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
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"

using veins::Heading;
using veins::TraCICoord;
using veins::TraCICoordinateTransformation;
using OmnetCoord = TraCICoordinateTransformation::OmnetCoord;
using TraCICoordList = TraCICoordinateTransformation::TraCICoordList;
using OmnetCoordList = TraCICoordinateTransformation::OmnetCoordList;

SCENARIO("TraCICoordinateTransformation gets built with network boundaries", "[netbound]")
{
    GIVEN("A network size from (5, 10) to (105, 210) with margin 10")
    {
        veins::TraCICoord topleft(5, 10);
        veins::TraCICoord bottomright(105, 210);
        float margin = 10;
        THEN("A NetworkCoordinateTranslator can be built from it")
        {
            TraCICoordinateTransformation nb{topleft, bottomright, margin};
            REQUIRE(&nb);
        }
    }
}

SCENARIO("TraCICoordinateTransformation correctly translates for Erlangen boundaries", "[netbound]")
{
    auto orig_omnet = OmnetCoord(2414.90142, 1578.44161, 0.0);
    auto orig_traci = TraCICoord(646854.991, 5493242.54);
    OmnetCoordList orig_omnet_list = {orig_omnet};
    TraCICoordList orig_traci_list = {orig_traci};
    double orig_traci_heading = -114.542;
    Heading orig_omnet_heading = Heading(-2.71324);

    GIVEN("The boundaries from the Erlangen scenario")
    {
        TraCICoordinateTransformation nb{{644465.09, 5491786.25}, {647071.55, 5494795.98}, 25};

        // single coordinates
        THEN("omnet coords correctly translate to traci coords")
        {
            auto new_traci = nb.omnet2traci(orig_omnet);
            REQUIRE(new_traci.x == Approx(orig_traci.x));
            REQUIRE(new_traci.y == Approx(orig_traci.y));
        }
        THEN("traci coords correctly translate to omnet coords")
        {
            auto new_omnet = nb.traci2omnet(orig_traci);
            REQUIRE(new_omnet.x == Approx(orig_omnet.x));
            REQUIRE(new_omnet.y == Approx(orig_omnet.y));
        }

        // lists of coordinates
        THEN("omnet coord lists correctly translate to traci coord lists")
        {
            auto new_traci = nb.omnet2traci(orig_omnet_list);
            REQUIRE(new_traci.front().x == Approx(orig_traci.x));
            REQUIRE(new_traci.front().y == Approx(orig_traci.y));
        }
        THEN("traci coord lists correctly translate to omnet coord lists")
        {
            auto new_omnet = nb.traci2omnet(orig_traci_list);
            REQUIRE(new_omnet.front().x == Approx(orig_omnet.x));
            REQUIRE(new_omnet.front().y == Approx(orig_omnet.y));
        }

        // headings
        THEN("omnet headings correctly translate to traci headings")
        {
            auto new_traci = nb.omnet2traciHeading(orig_omnet_heading);
            REQUIRE(new_traci == Approx(orig_traci_heading));
        }
        THEN("traci headings correctly translate to omnet headings")
        {
            auto new_omnet = nb.traci2omnetHeading(orig_traci_heading);
            REQUIRE(new_omnet.getRad() == Approx(orig_omnet_heading.getRad()));
        }
    }
}
