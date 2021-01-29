//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <vector>

#include "veins/base/utils/Coord.h"
#include "veins/base/utils/AntennaPosition.h"

namespace veins {

class BaseMobility;

/**
 * Obstacle for radio waves derived from a mobile host's body.
 *
 * Primarily used for VehicleObstacleShadowing.
 */
class VEINS_API MobileHostObstacle {
public:
    using Coords = std::vector<Coord>;

    MobileHostObstacle(std::vector<AntennaPosition> initialAntennaPositions, BaseMobility* mobility, double length, double hostPositionOffset, double width, double height)
        : initialAntennaPositions(std::move(initialAntennaPositions))
        , mobility(mobility)
        , length(length)
        , hostPositionOffset(hostPositionOffset)
        , width(width)
        , height(height)
    {
    }

    void setMobility(BaseMobility* mobility)
    {
        this->mobility = mobility;
    }
    void setLength(double d)
    {
        this->length = d;
    }
    void setHostPositionOffset(double d)
    {
        this->hostPositionOffset = d;
    }
    void setWidth(double d)
    {
        this->width = d;
    }
    void setHeight(double d)
    {
        this->height = d;
    }

    const std::vector<AntennaPosition>& getInitialAntennaPositions() const
    {
        return initialAntennaPositions;
    }
    const BaseMobility* getMobility() const
    {
        return mobility;
    }
    double getLength() const
    {
        return length;
    }
    double getHostPositionOffset() const
    {
        return hostPositionOffset;
    }
    double getWidth() const
    {
        return width;
    }
    double getHeight() const
    {
        return height;
    }

    Coords getShape(simtime_t t) const;

    bool maybeInBounds(double x1, double y1, double x2, double y2, simtime_t t) const;

    /**
     * return closest point (in meters) along (senderPos--receiverPos) where this obstacle overlaps, or NAN if it doesn't
     */
    double getIntersectionPoint(const Coord& senderPos, const Coord& receiverPos, simtime_t t) const;

protected:
    /**
     * Positions with identiers for all antennas connected to the host of this obstacle.
     *
     * Used to identify which host this obstacle belongs to.
     * Even works after the host has been destroyed (as AntennaPosition only stores the comonnent id).
     */
    std::vector<AntennaPosition> initialAntennaPositions;
    BaseMobility* mobility;
    double length;
    double hostPositionOffset;
    double width;
    double height;
};
} // namespace veins
