//
// Copyright (C) 2007 Parallel and Distributed Systems Group (PDS) at Technische Universiteit Delft, The Netherlands.
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

// author:      Tom Parker
// description: basic world utility class
//              provides world-required values

#pragma once

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"

namespace veins {

/**
 * @brief Provides information and utility methods for the whole simulation.
 *
 * @ingroup baseModules
 */
class VEINS_API BaseWorldUtility : public cSimpleModule {
protected:
    /**
     * @brief Size of the area the nodes are in (in meters)
     *
     * Note: The playground is in the range [0, playground size].
     * Meaning the upper borders (at pg-size) as well as the lower
     * borders (at 0) are part of the playground.
     **/
    Coord playgroundSize;

    /** @brief Should the playground be treated as a torus?*/
    bool useTorusFlag;

    /** @brief Should the world be 2-dimensional? */
    bool use2DFlag;

    /** @brief Provides a unique number for AirFrames per simulation */
    long airFrameId;

    /** @brief Stores if members are already initialized. */
    bool isInitialized;

public:
    /** @brief Speed of light in meters per second. */
    static const double speedOfLight()
    {
        return 299792458.0; ///< meters per second
    }

protected:
    /**
     * @brief Initializes all members accessible by other modules.
     *
     * Called once the first time another module accesses a member or during
     * initialize stage 0 at the latest.
     */
    virtual void initializeIfNecessary();

public:
    BaseWorldUtility();

    void initialize(int stage) override;

    /**
     * @brief Returns the playgroundSize
     *
     * Note: The playground is in the range [0, playground size].
     * Meaning the upper borders (at pg-size) as well as the lower
     * borders (at 0) are part of the playground.
     **/
    const Coord* getPgs()
    {
        initializeIfNecessary();
        return &playgroundSize;
    };

    /** @brief Returns true if our playground represents a torus (borders are connected)*/
    bool useTorus()
    {
        initializeIfNecessary();
        return useTorusFlag;
    };

    /** @brief Random position somewhere in the playground */
    virtual Coord getRandomPosition();

    /** @brief Returns true if the world is 2-dimensional */
    bool use2D()
    {
        initializeIfNecessary();
        return use2DFlag;
    }

    /** @brief Returns an Id for an AirFrame, at the moment simply an incremented long-value */
    long getUniqueAirFrameId()
    {
        initializeIfNecessary();

        // if counter has done one complete cycle and will be set to a value it already had
        if (airFrameId == -1) {
            // print a warning
            EV << "WARNING: AirFrameId-Counter has done one complete cycle. AirFrameIds are repeating now and may not be unique anymore." << endl;
        }

        return airFrameId++;
    }
};

} // namespace veins
