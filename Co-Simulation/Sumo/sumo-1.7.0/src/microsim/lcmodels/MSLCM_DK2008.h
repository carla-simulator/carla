/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    MSLCM_DK2008.h
/// @author  Daniel Krajzewicz
/// @author  Friedemann Wesner
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Fri, 29.04.2005
///
// A lane change model developed by D. Krajzewicz between 2004 and 2010
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSAbstractLaneChangeModel.h"
#include <vector>

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLCM_DK2008
 * @brief A lane change model developed by D. Krajzewicz between 2004 and 2010
 */
class MSLCM_DK2008 : public MSAbstractLaneChangeModel {
public:

    MSLCM_DK2008(MSVehicle& v);

    virtual ~MSLCM_DK2008();

    /// @brief Returns the model's id
    LaneChangeModel getModelID() const {
        return LCM_DK2008;
    }

    /** @brief Called to examine whether the vehicle wants to change
     * using the given laneOffset.
     * This method gets the information about the surrounding vehicles
     * and whether another lane may be more preferable */
    int wantsChange(
        int laneOffset,
        MSAbstractLaneChangeModel::MSLCMessager& msgPass, int blocked,
        const std::pair<MSVehicle*, double>& leader,
        const std::pair<MSVehicle*, double>& neighLead,
        const std::pair<MSVehicle*, double>& neighFollow,
        const MSLane& neighLane,
        const std::vector<MSVehicle::LaneQ>& preb,
        MSVehicle** lastBlocked,
        MSVehicle** firstBlocked);

    virtual void* inform(void* info, MSVehicle* sender);

    /** @brief Called to adapt the speed in order to allow a lane change.
     *
     * @param min The minimum resulting speed
     * @param wanted The aspired speed of the car following model
     * @param max The maximum resulting speed
     * @param cfModel The model used
     * @return the new speed of the vehicle as proposed by the lane changer
     */
    virtual double patchSpeed(const double min, const double wanted, const double max,
                              const MSCFModel& cfModel);

    virtual void changed();

    virtual void prepareStep();


protected:
    /** @brief Called to examine whether the vehicle wants to change to right
        This method gets the information about the surrounding vehicles
        and whether another lane may be more preferable */
    virtual int wantsChangeToRight(
        MSAbstractLaneChangeModel::MSLCMessager& msgPass, int blocked,
        const std::pair<MSVehicle*, double>& leader,
        const std::pair<MSVehicle*, double>& neighLead,
        const std::pair<MSVehicle*, double>& neighFollow,
        const MSLane& neighLane,
        const std::vector<MSVehicle::LaneQ>& preb,
        MSVehicle** lastBlocked,
        MSVehicle** firstBlocked);

    /** @brief Called to examine whether the vehicle wants to change to left
        This method gets the information about the surrounding vehicles
        and whether another lane may be more preferable */
    virtual int wantsChangeToLeft(
        MSAbstractLaneChangeModel::MSLCMessager& msgPass, int blocked,
        const std::pair<MSVehicle*, double>& leader,
        const std::pair<MSVehicle*, double>& neighLead,
        const std::pair<MSVehicle*, double>& neighFollow,
        const MSLane& neighLane,
        const std::vector<MSVehicle::LaneQ>& preb,
        MSVehicle** lastBlocked,
        MSVehicle** firstBlocked);

    void informBlocker(MSAbstractLaneChangeModel::MSLCMessager& msgPass,
                       int& blocked, int dir,
                       const std::pair<MSVehicle*, double>& neighLead,
                       const std::pair<MSVehicle*, double>& neighFollow);

    inline bool amBlockingLeader() {
        return (myOwnState & LCA_AMBLOCKINGLEADER) != 0;
    }
    inline bool amBlockingFollower() {
        return (myOwnState & LCA_AMBLOCKINGFOLLOWER) != 0;
    }
    inline bool amBlockingFollowerNB() {
        return (myOwnState & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0;
    }
    inline bool amBlockingFollowerPlusNB() {
        return (myOwnState & (LCA_AMBLOCKINGFOLLOWER | LCA_AMBLOCKINGFOLLOWER_DONTBRAKE)) != 0;
    }
    inline bool currentDistDisallows(double dist, int laneOffset, double lookForwardDist) {
        return dist / (abs(laneOffset)) < lookForwardDist;
    }
    inline bool currentDistAllows(double dist, int laneOffset, double lookForwardDist) {
        return dist / abs(laneOffset) > lookForwardDist;
    }

    typedef std::pair<double, int> Info;



protected:
    double myChangeProbability;

    double myLeadingBlockerLength;
    double myLeftSpace;

    std::vector<double> myVSafes;
    bool myDontBrake;

};
