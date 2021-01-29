//
// Copyright (C) 2005, 2010 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Andreas Koepke, Michael Swigulski
// part of:     framework implementation developed by tkn

#pragma once

#include <string>

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"

namespace veins {

/**
 * @brief Class to store object position and movement
 *
 * @ingroup baseUtils
 * @ingroup utils
 *
 * @author Andreas Koepke, Michael Swigulski
 **/
class VEINS_API Move : public cObject {
protected:
    /** @brief Start position of the host (in meters)**/
    Coord startPos;
    /** @brief Last position which was set. */
    Coord lastPos;
    /** @brief start time at which host started at startPos **/
    simtime_t startTime;
    /** @brief orientation the host is pointing to **/
    Coord orientation;
    /** @brief direction the host is moving to, must be normalized **/
    Coord direction;
    /** @brief speed of the host in meters per second **/
    double speed;

public:
    Move()
        : startPos()
        , lastPos(0.0, 0.0, DBL_MAX)
        , startTime()
        , orientation()
        , direction()
        , speed(0.0)
    {
    }
    Move(const Move& mSrc)
        : startPos(mSrc.startPos)
        , lastPos(mSrc.lastPos)
        , startTime(mSrc.startTime)
        , orientation(mSrc.orientation)
        , direction(mSrc.direction)
        , speed(mSrc.speed)
    {
    }

    /**
     * @brief Returns the current speed.
     */
    double getSpeed() const
    {
        return speed;
    }

    /**
     * @brief Sets the current speed in meters per second.
     */
    void setSpeed(double speed)
    {
        this->speed = speed;
    }

    /**
     * @brief Returns the start position.
     */
    const Coord& getStartPos() const
    {
        return startPos;
    }

    /**
     * @brief Returns start time, i.e. time point of the start at start position.
     */
    simtime_t_cref getStartTime() const
    {
        return startTime;
    }

    /**
     * @brief Sets start position (components in meters) and start time.
     */
    void setStart(const Coord& startPos, simtime_t_cref startTime)
    {
        this->lastPos = startPos;
        this->startPos = startPos;
        this->startTime = startTime;
    }

    /**
     * @brief Sets start position to passed value (components in meters),
     * start time will be set to current simulation-time.
     */
    void setStart(const Coord& startPos)
    {
        setStart(startPos, simTime());
    }

    /**
     * @brief Returns the direction vector (which is unit less, because of normalization).
     */
    const Coord& getDirection() const
    {
        return direction;
    }

    /**
     * @brief Returns the orientation vector, i.e. the direction the host is pointing in.
     * The difference to direction is that the x and y components are never both 0, which
     * is important for the calculation of the antenna gain. At simulation start, it is
     * initialized with a (user defined) value. If the host stops during simulation,
     * the last direction is stored in the orientation field.
     */
    const Coord& getOrientation() const
    {
        return orientation;
    }

    /**
     * @brief Sets the orientation to the passed vector. At least one of the x or y
     * component has to be nonzero.
     */
    void setOrientationByVector(const Coord& orientation)
    {
        ASSERT(orientation.x != 0 || orientation.y != 0);
        this->orientation = orientation;
    }

    /**
     * @brief Sets the direction to the passed vector,
     * which must be already normalized or the 0-vector.
     */
    void setDirectionByVector(const Coord& direction)
    {
        ASSERT(math::almost_equal(direction.squareLength(), 1.0) || math::almost_equal(direction.squareLength(), 0.0));
        this->direction = direction;
    }

    /**
     * @brief Sets the direction to the normalized vector pointing
     * from the current start position to the passed target point.
     *
     * NOTE: The target point must not be the current start position
     * or too close to it.
     */
    void setDirectionByTarget(const Coord& target)
    {
        direction = target - startPos;

        ASSERT(!math::almost_equal(direction.length(), 0.0));
        direction /= direction.length();
    }

    /**
     * @brief Returns the position of the Move (Host) at the specified point in time.
     * It is intended to be passed simTime() as actualTime and returns the actual position.
     *
     * Assumes that direction represents a normalized vector (length equals 1.0).
     * Further this function does not check whether the given time point is before
     * the startTime of the actual move pattern. So in this case one might obtain
     * an unintended result.
     *
     */
    virtual Coord getPositionAt(simtime_t_cref actualTime = simTime()) const
    {
        // if speed is very close to 0.0, the host is practically standing still
        if (math::almost_equal(speed, 0.0)) return startPos;

        // otherwise: actualPos = startPos + ( direction * v * t )
        return startPos + (direction * speed * SIMTIME_DBL(actualTime - startTime));
    }
    virtual const Coord& getStartPosition() const
    {
        if (lastPos.z != DBL_MAX) return lastPos;
        return startPos;
    }

public:
    /**
     * @brief Returns information about the current state.
     */
    std::string info() const override
    {
        std::ostringstream ost;
        ost << " HostMove "
            << " startPos: " << startPos.info() << " direction: " << direction.info() << " orientation: " << orientation.info() << " startTime: " << startTime << " speed: " << speed;
        return ost.str();
    }
};

} // namespace veins
