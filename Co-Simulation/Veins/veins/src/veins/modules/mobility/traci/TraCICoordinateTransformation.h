//
// Copyright (C) 2018 Dominik S. Buse <buse@ccs-labs.org>
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

#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/Heading.h"

#include <list>

namespace veins {

/**
 * Helper class for converting SUMO coordinates to OMNeT++ Coordinates for a given network.
 */
class VEINS_API TraCICoordinateTransformation {
public:
    using OmnetCoord = Coord;
    using OmnetCoordList = std::list<OmnetCoord>;
    using TraCICoordList = std::list<TraCICoord>;
    using TraCIHeading = double;
    using OmnetHeading = Heading;

    TraCICoordinateTransformation(TraCICoord topleft, TraCICoord bottomright, float margin);
    TraCICoord omnet2traci(const OmnetCoord& coord) const;
    TraCICoordList omnet2traci(const OmnetCoordList& coords) const;
    TraCIHeading omnet2traciHeading(OmnetHeading heading) const; /**< TraCI's heading interpretation: 0 is north, 90 is east */

    OmnetCoord traci2omnet(const TraCICoord& coord) const;
    OmnetCoordList traci2omnet(const TraCICoordList& coords) const;
    OmnetHeading traci2omnetHeading(TraCIHeading heading) const; /**<  OMNeT++'s heading interpretation: 0 is east, pi/2 is north */
private:
    TraCICoord dimensions;
    TraCICoord topleft;
    TraCICoord bottomright;
    float margin;
}; // end class NetworkCoordinateTranslator

} // end namespace veins
