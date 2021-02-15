/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    MELoop.h
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// The main mesocopic simulation loop
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>


// ===========================================================================
// class declarations
// ===========================================================================
class MESegment;
class MEVehicle;
class MSEdge;
class MSLink;
class MSVehicleControl;
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MELoop
 * @brief The main mesocopic simulation loop
 */
class MELoop {
public:
    /// SUMO constructor
    MELoop(const SUMOTime recheckInterval);

    ~MELoop();

    /** @brief Perform simulation up to the given time
     *
     * Checks all vehicles with an event time less or equal than the given time.
     *
     * @param[in] tMax the end time for the sim step
     */
    void simulate(SUMOTime tMax);

    /** @brief Adds the given car to the leading vehicles
     *
     * @param[in] veh the car which became a leading one
     * @param[in] link the link on which the car shall register its approach
     */
    void addLeaderCar(MEVehicle* veh, MSLink* link);

    /** @brief Removes the given car from the leading vehicles
     *
     * @param[in] v the car which was a leading one
     */
    void removeLeaderCar(MEVehicle* v);

    /** @brief remove the given car and clean up the relevant data structures */
    void vaporizeCar(MEVehicle* v, MSMoveReminder::Notification reason);

    /** @brief Compute number of segments per edge (best value stay close to the configured segment length) */
    static int numSegmentsFor(const double length, const double slength);

    /** @brief Build the segments for a given edge
     *
     * @param[in] e the edge to build for
     */
    void buildSegmentsFor(const MSEdge& e, const OptionsCont& oc);

    /** @brief Get the segment for a given edge at a given position
     *
     * @param[in] e the edge to get the segment for
     * @param[in] pos the position to get the segment for
     * @return The relevant segment
     */
    MESegment* getSegmentForEdge(const MSEdge& e, double pos = 0);

    /** @brief change to the next segment
     * this handles combinations of the following cases:
     * (ending / continuing route) and (leaving segment / finishing teleport)
     */
    SUMOTime changeSegment(MEVehicle* veh, SUMOTime leaveTime, MESegment* const toSegment,
                           MSMoveReminder::Notification reason, const bool ignoreLink = false) const;

    /** @brief registers vehicle with the given link
     *
     * @param[in] veh the car to register
     * @param[in] link the link on which the car shall register its approach
     */
    static void setApproaching(MEVehicle* veh, MSLink* link);

    /** @brief Remove all vehicles before quick-loading state */
    void clearState();

private:
    /** @brief Check whether the vehicle may move
     *
     * This method is called when the vehicle reaches its event time and checks
     *  whether it may proceed to the next segment.
     *
     * @param[in] veh The vehicle to check
     */
    void checkCar(MEVehicle* veh);

    /** @brief Retrieve next segment
     *
     * If the segment is not the last on the current edge, its successor is returned.
     *  Otherwise, the first segment of the edge at which the vehicle continues
     *  his journey is returned.
     *
     * @param[in] s The segment the vehicle is currently at
     * @param[in] v The vehicle to get the next segment for
     * @return The vehicle's next segment
     * @todo Recheck the "quick and dirty" stuff (@see MESegment::saveState, @see MESegment::loadState)
     */
    MESegment* nextSegment(MESegment* s, MEVehicle* v);


    /** @brief teleports a vehicle or continues a teleport
     * @param[in] veh The vehicle to teleport
     * @param[in] toSegment The first segment where the vehicle may reenter the network
     */
    void teleportVehicle(MEVehicle* veh, MESegment* const toSegment);

    /// @brief whether the given edge is entering a roundabout
    static bool isEnteringRoundabout(const MSEdge& e);

private:
    /// @brief leader cars in the segments sorted by exit time
    std::map<SUMOTime, std::vector<MEVehicle*> > myLeaderCars;

    /// @brief mapping from internal edge ids to their initial segments
    std::vector<MESegment*> myEdges2FirstSegments;

    /// @brief the interval at which to recheck at full segments (<=0 means asap)
    const SUMOTime myFullRecheckInterval;

    /// @brief the interval at which to recheck at blocked junctions (<=0 means asap)
    const SUMOTime myLinkRecheckInterval;

private:
    /// @brief Invalidated copy constructor.
    MELoop(const MELoop&);

    /// @brief Invalidated assignment operator.
    MELoop& operator=(const MELoop&);
};
