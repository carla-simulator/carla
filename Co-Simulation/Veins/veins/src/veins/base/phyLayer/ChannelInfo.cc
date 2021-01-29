//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

#include "veins/base/phyLayer/ChannelInfo.h"

#include <iostream>

using namespace veins;

using veins::AirFrame;

void ChannelInfo::addAirFrame(AirFrame* frame, simtime_t_cref startTime)
{
    ASSERT(airFrameStarts.count(frame) == 0);

    // check if we were previously empty
    if (isChannelEmpty()) {
        // earliest time point is current sim time
        earliestInfoPoint = startTime;
    }

    // calculate endTime of AirFrame
    simtime_t_cref endTime = startTime + frame->getDuration();

    // add AirFrame to active AirFrames
    activeAirFrames[endTime].push_back(AirFrameTimePair(startTime, frame));

    // add to start time map
    airFrameStarts[frame] = startTime;

    ASSERT(!isChannelEmpty());
}

simtime_t ChannelInfo::findEarliestInfoPoint()
{
    // TODO: check for a more efficient way to find out that earliest time-point

    // make a variable for the earliest-start-time of all remaining AirFrames
    simtime_t earliestStart = SIMTIME_ZERO;
    AirFrameStartMap::const_iterator it = airFrameStarts.begin();

    // if there is an entry for an AirFrame
    if (it != airFrameStarts.end()) {
        // store the start-time of the first entry as earliestStart so far
        earliestStart = (*it).second;

        // go through all other start-time-points
        for (; it != airFrameStarts.end(); ++it) {
            // and check if an earlier start-time was found,
            // if so, replace earliestStart with it
            if ((*it).second < earliestStart) earliestStart = (*it).second;
        }
    }

    return earliestStart;
}

simtime_t ChannelInfo::removeAirFrame(AirFrame* frame)
{
    ASSERT(airFrameStarts.count(frame) > 0);

    // get start of AirFrame
    simtime_t_cref startTime = airFrameStarts[frame];

    // calculate end time
    simtime_t_cref endTime = startTime + frame->getDuration();

    // remove this AirFrame from active AirFrames
    deleteAirFrame(activeAirFrames, frame, startTime, endTime);

    // add to inactive AirFrames
    addToInactives(frame, startTime, endTime);

    // Now check, whether the earliest time-point we need to store information
    // for might have moved on in time, since an AirFrame has been deleted.
    if (isChannelEmpty()) {
        earliestInfoPoint = -1;
    }
    else {
        earliestInfoPoint = findEarliestInfoPoint();
    }

    return earliestInfoPoint;
}

void ChannelInfo::assertNoIntersections()
{
    for (AirFrameMatrix::iterator it1 = inactiveAirFrames.begin(); it1 != inactiveAirFrames.end(); ++it1) {
        simtime_t_cref e0 = it1->first;
        for (AirFrameTimeList::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
            simtime_t_cref s0 = it2->first;

            bool intersects = (recordStartTime > -1 && recordStartTime <= e0);

            for (AirFrameMatrix::iterator it3 = activeAirFrames.begin(); it3 != activeAirFrames.end() && !intersects; ++it3) {
                simtime_t_cref e1 = it3->first;
                for (AirFrameTimeList::iterator it4 = it3->second.begin(); it4 != it3->second.end() && !intersects; ++it4) {
                    simtime_t_cref s1 = it4->first;

                    if (e0 >= s1 && s0 <= e1) intersects = true;
                }
            }
            ASSERT(intersects);
        }
    }
}

void ChannelInfo::deleteAirFrame(AirFrameMatrix& airFrames, AirFrame* frame, simtime_t_cref startTime, simtime_t_cref endTime)
{
    AirFrameMatrix::iterator listIt = airFrames.find(endTime);
    AirFrameTimeList* list = &listIt->second;

    for (AirFrameTimeList::iterator it = list->begin(); it != list->end(); it++) {
        if (it->second == frame) {
            it = list->erase(it);
            if (list->empty()) {
                airFrames.erase(listIt);
            }
            return;
        }
    }

    ASSERT(false);
}

bool ChannelInfo::canDiscardInterval(simtime_t_cref startTime, simtime_t_cref endTime)
{
    ASSERT(recordStartTime >= 0 || recordStartTime == -1);

    // only if it ends before the point in time we started recording or if
    // we aren't recording at all and it does not intersect with any active one
    // anymore this AirFrame can be deleted
    return (recordStartTime > endTime || recordStartTime == -1) && !isIntersecting(activeAirFrames, startTime, endTime);
}

void ChannelInfo::checkAndCleanInterval(simtime_t_cref startTime, simtime_t_cref endTime)
{

    // get through inactive AirFrame which intersected with the passed interval
    IntersectionIterator inactiveIntersectIt(&inactiveAirFrames, startTime, endTime);

    AirFrame* inactiveIntersect = inactiveIntersectIt.next();
    while (inactiveIntersect != nullptr) {
        simtime_t_cref currentStart = airFrameStarts[inactiveIntersect];
        simtime_t_cref currentEnd = currentStart + inactiveIntersect->getDuration();

        if (canDiscardInterval(currentStart, currentEnd)) {
            inactiveIntersectIt.eraseAirFrame();

            airFrameStarts.erase(inactiveIntersect);

            delete inactiveIntersect;
            inactiveIntersect = nullptr;
        }
        inactiveIntersect = inactiveIntersectIt.next();
    }
}

void ChannelInfo::addToInactives(AirFrame* frame, simtime_t_cref startTime, simtime_t_cref endTime)
{
    // At first, check if some inactive AirFrames can be removed because the
    // AirFrame to in-activate was the last one they intersected with.
    checkAndCleanInterval(startTime, endTime);

    if (!canDiscardInterval(startTime, endTime)) {
        inactiveAirFrames[endTime].push_back(AirFrameTimePair(startTime, frame));
    }
    else {
        airFrameStarts.erase(frame);

        delete frame;
    }
}

bool ChannelInfo::isIntersecting(const AirFrameMatrix& airFrames, simtime_t_cref from, simtime_t_cref to) const
{
    ConstIntersectionIterator it(&airFrames, from, to);
    return (it.next() != nullptr);
}

void ChannelInfo::getIntersections(const AirFrameMatrix& airFrames, simtime_t_cref from, simtime_t_cref to, AirFrameVector& outVector) const
{
    ConstIntersectionIterator it(&airFrames, from, to);

    AirFrame* intersect = it.next();
    while (intersect != nullptr) {
        outVector.push_back(intersect);
        intersect = it.next();
    }
}

void ChannelInfo::getAirFrames(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) const
{
    // check for intersecting inactive AirFrames
    getIntersections(inactiveAirFrames, from, to, out);

    // check for intersecting active AirFrames
    getIntersections(activeAirFrames, from, to, out);
}
