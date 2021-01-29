/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSLaneChangerSublane.h
/// @author  Jakob Erdmann
/// @author  Leonhard Luecken
/// @date    Oct 2015
///
// Performs sub-lane changing of vehicles
/****************************************************************************/
#pragma once
#include <config.h>

#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include "MSLaneChanger.h"


// ===========================================================================
// class declarations
// ===========================================================================


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLaneChangerSublane
 * @brief Performs lane changing of vehicles
 */
class MSLaneChangerSublane : public MSLaneChanger {
public:
    /// Constructor
    MSLaneChangerSublane(const std::vector<MSLane*>* lanes, bool allowChanging);

    /// Destructor.
    ~MSLaneChangerSublane();

protected:

    /** Find a new candidate and try to change it. */
    virtual bool change();

    /// @brief  Initialize the changer before looping over all vehicles.
    virtual void initChanger();

    /** After the possible change, update the changer. */
    virtual void updateChanger(bool vehHasChanged);

    /** @brief check whether sub-lane changing in the given direction is desirable
     * and possible
     * @param[in] laneOffset The direction in which changing should be checked
     * @param[in] leaders The candidate vehicle's leaders
     * @param[in] preb The bestLanse of the candidaet vehicle
     * @param[out] latDist The distance by which the vehicle changes laterally
     * @param[out] maneuverDist The lateral distance for the complete envisioned maneuver
     *                           (used for maneuver continuation in non-actionsteps).
     */
    int checkChangeSublane(
        int laneOffset,
        LaneChangeAction alternatives,
        const std::vector<MSVehicle::LaneQ>& preb,
        double& latDist,
        double& maneuverDist) const;

    ///  @brief Continue a sublane-lane change maneuver and return whether the midpoint was passed in this step
    //          (used to continue sublane changing in non-action steps).
    bool continueChangeSublane(MSVehicle* vehicle, ChangerIt& from);

    ///  @brief change by the specified amount and return whether a new lane was entered
    bool startChangeSublane(MSVehicle* vehicle, ChangerIt& from, double latDist, double maneuverDist);

    /// @brief check whether the given vehicle has entered the new lane 'to->lane' during a sublane LC-step
    bool checkChangeToNewLane(MSVehicle* vehicle, const int direction, ChangerIt from, ChangerIt to);

    /// @brief get leaders for ego on the given lane
    MSLeaderDistanceInfo getLeaders(const ChangerIt& target, const MSVehicle* ego) const;

    /// @brief immediately stop lane-changing and register vehicle as unchanged
    void abortLCManeuver(MSVehicle* vehicle);

    typedef MSAbstractLaneChangeModel::StateAndDist StateAndDist;
    /// @brief helper function that calls checkChangeSublane and sets blocker information
    StateAndDist checkChangeHelper(MSVehicle* vehicle, int laneOffset, LaneChangeAction alternatives);

    /// @brief optional output for start of lane-change maneuvre
    void outputLCStarted(MSVehicle* vehicle, ChangerIt& from, ChangerIt& to, int direction, double maneuverDist);
    /// @brief optional output for end of lane-change maneuvre
    void outputLCEnded(MSVehicle* vehicle, ChangerIt& from, ChangerIt& to, int direction);

private:
    /// Default constructor.
    MSLaneChangerSublane();

    /// Copy constructor.
    MSLaneChangerSublane(const MSLaneChangerSublane&);

    /// Assignment operator.
    MSLaneChangerSublane& operator=(const MSLaneChangerSublane&);
};
