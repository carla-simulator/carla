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

#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"

namespace veins {

using OmnetCoord = TraCICoordinateTransformation::OmnetCoord;
using OmnetCoordList = TraCICoordinateTransformation::OmnetCoordList;
using TraCICoordList = TraCICoordinateTransformation::TraCICoordList;
using OmnetHeading = TraCICoordinateTransformation::OmnetHeading;
using TraCIHeading = TraCICoordinateTransformation::TraCIHeading;

TraCICoordinateTransformation::TraCICoordinateTransformation(TraCICoord topleft, TraCICoord bottomright, float margin)
    : dimensions({bottomright.x - topleft.x, bottomright.y - topleft.y})
    , topleft(topleft)
    , bottomright(bottomright)
    , margin(margin)
{
}

TraCICoord TraCICoordinateTransformation::omnet2traci(const OmnetCoord& coord) const
{
    return {coord.x + topleft.x - margin, dimensions.y - (coord.y - topleft.y) + margin};
}

TraCICoordList TraCICoordinateTransformation::omnet2traci(const OmnetCoordList& coords) const
{
    TraCICoordList result;
    for (auto&& coord : coords) {
        result.push_back(omnet2traci(coord));
    }
    return result;
}

TraCIHeading TraCICoordinateTransformation::omnet2traciHeading(OmnetHeading o) const
{
    // convert to degrees
    auto angle = o.getRad() * 180 / M_PI;

    // rotate angle
    angle = 90 - angle;

    // normalize angle to -180 <= angle < 180
    while (angle < -180) {
        angle += 360;
    }
    while (angle >= 180) {
        angle -= 360;
    }

    return angle;
}

OmnetCoord TraCICoordinateTransformation::traci2omnet(const TraCICoord& coord) const
{
    return {coord.x - topleft.x + margin, dimensions.y - (coord.y - topleft.y) + margin};
}

OmnetCoordList TraCICoordinateTransformation::traci2omnet(const TraCICoordList& coords) const
{
    OmnetCoordList result;
    for (auto&& coord : coords) {
        result.push_back(traci2omnet(coord));
    }
    return result;
}

OmnetHeading TraCICoordinateTransformation::traci2omnetHeading(TraCIHeading o) const
{
    // rotate angle
    auto angle = 90 - o;

    // convert to rad
    angle = angle * M_PI / 180.0;

    // normalize angle to -M_PI <= angle < M_PI
    while (angle < -M_PI) {
        angle += 2 * M_PI;
    }
    while (angle >= M_PI) {
        angle -= 2 * M_PI;
    }

    return OmnetHeading(angle);
}

} // end namespace veins
