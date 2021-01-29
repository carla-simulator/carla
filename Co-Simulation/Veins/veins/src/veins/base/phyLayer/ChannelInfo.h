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

#pragma once

#include <list>

#include "veins/veins.h"

#include "veins/base/messages/AirFrame_m.h"

using veins::AirFrame;

namespace veins {

/**
 * @brief This class is used by the BasePhyLayer to keep track of the AirFrames
 * on the channel.
 *
 * ChannelInfo is able to return every AirFrame which intersects with a
 * specified interval. This is mainly used to get the noise for a received
 * signal.
 *
 * ChannelInfo is a passive class meaning the user has to tell it when a new
 * AirFrame starts and an existing ends.
 *
 * Once an AirFrame has been added to the ChannelInfo the ChannelInfo holds the
 * ownership of this AirFrame even if the AirFrame is removed again from the
 * ChannelInfo. This is necessary because the ChannelInfo has to be able to
 * store also the AirFrames which are over but still intersect with an currently
 * running AirFrame.
 *
 * Note: ChannelInfo assumes that the AirFrames are added and removed
 *          chronologically. This means every time you add an AirFrame with a
 *          specific start time ChannelInfo assumes that start time as the current
 *          time and assumes that every following action happens after that moment.
 *          The same goes for "removeAirFrame". When removing an AirFrames,
 *          ChannelInfo assumes the start time plus the duration of the AirFrame as
 *          the current time.
 *          This also affects "getAirFrames" in the way that you may only ask for
 *          intervals which lie before the "current time" of ChannelInfo.
 *
 * @ingroup phyLayer
 */
class VEINS_API ChannelInfo {

protected:
    /** @brief Type for a pair of an AirFrame and a simulation time.*/
    typedef std::pair<simtime_t, AirFrame*> AirFrameTimePair;
    /** @brief Type for a list of AirFrames and a simulation time.*/
    using AirFrameTimeList = std::list<AirFrameTimePair>;
    /**
     * The AirFrames are stored in a Matrix with start- and end time as
     * dimensions.
     */
    typedef std::map<simtime_t, AirFrameTimeList> AirFrameMatrix;

    /**
     * @brief Iterator for every intersection of a specific interval in a
     * AirFrameMatrix.
     *
     * A time interval A_start to A_end intersects with another interval B_start
     * to B_end iff the following two conditions are fulfilled:
     *
     *         1. A_end >= B_start.
     *         2. A_start <= B_end and
     *
     * Or the defined by the
     * opposite: The two intervals do not intersect iff A_end < B_start or
     * A_start > B_end.
     *
     * To iterate over a two dimensional (end-time x start-time) container
     * of AirFrames this class iterates over all AirFrame-lists whose end-time
     * is bigger or equal than the interval start (intersect condition 1) and in
     * each of these lists iterates over all entries but only stops at and
     * returns entries which start-time is smaller or equal than the interval
     * end (intersection condition 2).
     *
     * In template form to work as const- and non-const iterator.
     */
    template <class C, class ItMatrix, class ItList>
    class VEINS_API BaseIntersectionIterator {
    public:
        /** @brief Pointer to the matrix holding the intervals.*/
        C* intervals;

        /** @brief Point in time to start iterating over intersections.*/
        simtime_t from;

        /** @brief Point in time to end iterating over intersections.*/
        simtime_t to;

        /** @brief Iterator over AirFrame end times.*/
        ItMatrix endIt;

        /** @brief Iterator over AirFrame start times.*/
        ItList startIt;

        /** @brief True if the we are already pointing to the next entry.*/
        bool alreadyNext;

    public:
        /**
         * @brief Creates an iterator for the specified interval at the
         * specified AirFrameMatrix.
         */
        BaseIntersectionIterator(C* airFrames, simtime_t_cref from, simtime_t_cref to)
            : intervals(airFrames)
            , from(from)
            , to(to)
        {
            // begin at the smallest end-time-entry fulfilling the intersection
            // condition 1
            endIt = intervals->lower_bound(from);

            if (endIt != intervals->end()) {
                startIt = endIt->second.begin();
            }
            // we are already pointing at the first unchecked interval
            alreadyNext = true;
        }

        /**
         * @brief Increases the iterator to the next intersecting AirFrame and
         * returns a pointer to this AirFrame.
         */
        AirFrame* next()
        {
            if (endIt == intervals->end()) return nullptr;

            // "alreadyNext" indicates that some previous iterator function has
            // already increased the intern iterators to a yet unchecked values.
            // This happens after initialization of the iterator and when an
            // element is erased.
            if (alreadyNext)
                alreadyNext = false;
            else
                startIt++;

            while (endIt != intervals->end()) {
                // while there are entries left at the current end-time
                while (startIt != endIt->second.end()) {
                    // check if this entry fulfilles the intersection condition
                    // 2 (condition 1 is already fulfilled in the constructor by
                    // the start-value of the end-time-iterator)
                    if (startIt->first <= to) {
                        return startIt->second;
                    }
                    startIt++;
                }

                endIt++;
                if (endIt == intervals->end()) return nullptr;

                startIt = endIt->second.begin();
            }

            return nullptr;
        }
    };

    /** @brief Type for a const-iterator over an AirFrame interval matrix.*/
    typedef BaseIntersectionIterator<const AirFrameMatrix, AirFrameMatrix::const_iterator, AirFrameTimeList::const_iterator> ConstIntersectionIterator;

    /**
     * @brief Type for a iterator over an AirFrame interval matrix.
     *
     * Extends the const-version by an erase method.
     */
    class VEINS_API IntersectionIterator : public BaseIntersectionIterator<AirFrameMatrix, AirFrameMatrix::iterator, AirFrameTimeList::iterator> {
    private:
        /** @brief Type for shortcut to base class type.*/
        typedef BaseIntersectionIterator<AirFrameMatrix, AirFrameMatrix::iterator, AirFrameTimeList::iterator> Base;

    public:
        /**
         * @brief Creates an iterator for the specified interval at the
         * specified AirFrameMatrix.
         */
        IntersectionIterator(AirFrameMatrix* airFrames, simtime_t_cref from, simtime_t_cref to)
            : Base(airFrames, from, to)
        {
        }

        /**
         * @brief Erases the AirFrame the iterator currently points to from the
         * AirFrameMatrix.
         *
         * After the erase the iterator points to an invalid value and "next()"
         * should be called.
         */
        void eraseAirFrame()
        {
            ASSERT(endIt != intervals->end());
            ASSERT(startIt != endIt->second.end());

            // erase AirFrame from list
            startIt = endIt->second.erase(startIt);

            // check if we've deleted the last entry in the list
            if (startIt == endIt->second.end()) {
                // check if we deleted the only entry in the list
                if (endIt->second.empty()) {
                    intervals->erase(endIt++); // delete list from map
                }
                else {
                    endIt++;
                }

                // increase to a valid value if we are not done
                if (endIt != intervals->end()) {
                    startIt = endIt->second.begin();
                }
            }
            alreadyNext = true;
        }
    };

    /**
     * @brief Stores the currently active AirFrames.
     *
     * This means every AirFrame which was added but not yet removed.
     */
    AirFrameMatrix activeAirFrames;

    /**
     * @brief Stores inactive AirFrames.
     *
     * This means every AirFrame which has been already removed but still is
     * needed because it intersect with one or more active AirFrames.
     */
    AirFrameMatrix inactiveAirFrames;

    /** @brief Type for a map of AirFrame pointers to their start time.*/
    typedef std::map<AirFrame*, simtime_t> AirFrameStartMap;

    /** @brief Stores the start time of every AirFrame.*/
    AirFrameStartMap airFrameStarts;

    /** @brief Stores the point in history up to which we have some (but not
     * necessarily all) channel information stored.*/
    simtime_t earliestInfoPoint;

    /** @brief Stores a point in history up to which we need to keep all channel
     * information stored.*/
    simtime_t recordStartTime;

public:
    /**
     * @brief Type for a container of AirFrames.
     *
     * Used as out type for "getAirFrames" method.
     */
    using AirFrameVector = std::list<AirFrame*>;

protected:
    /**
     * @brief Asserts that every inactive AirFrame is still intersecting with at
     * least one active airframe or with the current record start time.
     */
    void assertNoIntersections();

    /**
     * @brief Returns every AirFrame of an AirFrameMatrix which intersect with a
     * given interval.
     *
     * The intersecting AirFrames are stored in the AirFrameVector reference
     * passed as parameter.
     */
    void getIntersections(const AirFrameMatrix& airFrames, simtime_t_cref from, simtime_t_cref to, AirFrameVector& outVector) const;

    /**
     * @brief Returns true if there is at least one AirFrame in the passed
     * AirFrameMatrix which intersect with the given interval.
     */
    bool isIntersecting(const AirFrameMatrix& airFrames, simtime_t_cref from, simtime_t_cref to) const;

    /**
     * @brief Moves a previously active AirFrame to the inactive AirFrames.
     *
     * This methods checks if there are some inactive AirFrames which can be
     * deleted because the AirFrame to in-activate was the last one they
     * intersected with.
     * It also checks if the AirFrame to in-activate still intersect with at
     * least one active AirFrame before it is moved to inactive AirFrames.
     */
    void addToInactives(AirFrame* a, simtime_t_cref startTime, simtime_t_cref endTime);

    /**
     * @brief Deletes an AirFrame from an AirFrameMatrix.
     */
    void deleteAirFrame(AirFrameMatrix& airFrames, AirFrame* a, simtime_t_cref startTime, simtime_t_cref endTime);

    /**
     * @brief Returns the start time of the odlest AirFrame on the channel.
     */
    simtime_t findEarliestInfoPoint();

    /**
     * @brief Checks if any information inside the passed interval can be
     * discarded.
     *
     * This method should be called every time the information for a certain
     * interval changes (AirFrame is removed or record time changed).
     *
     * @param startTime The start of the interval to check
     * @param endTime The end of the interval to check
     */
    void checkAndCleanInterval(simtime_t_cref startTime, simtime_t_cref endTime);

    /**
     * @brief Returns true if all information inside the passed interval can be
     * deleted.
     *
     * For example this method is used to check if information for the duration
     * of an AirFrame is needed anymore and if not the AirFrame is deleted.
     *
     * @param startTime The start time of the interval (e.g. AirFrame start)
     * @param endTime The end time of the interval (e.g. AirFrame end)
     * @return returns true if any information for the passed interval can be
     * discarded.
     */
    bool canDiscardInterval(simtime_t_cref startTime, simtime_t_cref endTime);

    /**
     * @brief Checks if any information up from the passed start time can be
     * discarded.
     *
     * @param start The start of the interval to check
     */
    void checkAndCleanFrom(simtime_t_cref start)
    {
        // nothing to do
        if (inactiveAirFrames.empty()) return;

        // take last ended inactive airframe as end of interval
        checkAndCleanInterval(start, inactiveAirFrames.rbegin()->first);
    }

public:
    ChannelInfo()
        : earliestInfoPoint(-1)
        , recordStartTime(-1)
    {
    }

    virtual ~ChannelInfo()
    {
    }

    /**
     * @brief Tells the ChannelInfo that an AirFrame has started.
     *
     * From this point ChannelInfo gets the ownership of the AirFrame.
     *
     * parameter startTime holds the time the receiving of the AirFrame has
     * started in seconds.
     */
    void addAirFrame(AirFrame* a, simtime_t_cref startTime);

    /**
     * @brief Tells the ChannelInfo that an AirFrame is over.
     *
     * This does not mean that it loses ownership of the AirFrame.
     *
     * @return The current time-point from on which information concerning
     * AirFrames is needed to be stored.
     */
    simtime_t removeAirFrame(AirFrame* a);

    /**
     * @brief Fills the passed AirFrameVector reference with the AirFrames which
     * intersect with the given time interval.
     *
     * Note: Completeness of the list of AirFrames for specific interval can
     * only be assured if start and end point of the interval lies inside the
     * duration of at least one currently active AirFrame.
     * An AirFrame is called active if it has been added but not yet removed
     * from ChannelInfo.
     */
    void getAirFrames(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) const;

    /**
     * @brief Returns the current time-point from that information concerning
     * AirFrames is needed to be stored.
     */
    simtime_t getEarliestInfoPoint()
    {
        ASSERT(!isChannelEmpty() || earliestInfoPoint == -1);

        return earliestInfoPoint;
    }

    /**
     * @brief Tells ChannelInfo to keep from now on all channel information
     * until the passed time in history.
     *
     * The passed start-time should be the current simulation time, otherwise
     * ChannelInfo can't assure that it hasn't already thrown away some
     * information for that passed time period.
     *
     * Subsequent calls to this method will update the recording start time and
     * information from old start times is thrown away.
     *
     * @param start The point in time from which to keep all channel information
     * stored.
     */
    void startRecording(simtime_t_cref start)
    {
        // clean up until old record start
        if (recordStartTime > -1) {
            recordStartTime = start;
            checkAndCleanInterval(0, recordStartTime);
        }
        else {
            recordStartTime = start;
        }
    }

    /**
     * @brief Tells ChannelInfo to stop recording Information from now on.
     *
     * Its up to ChannelInfo to decide when to actually throw away the
     * information it doesn't need to store anymore now.
     */
    void stopRecording()
    {
        if (recordStartTime > -1) {
            simtime_t old = recordStartTime;
            recordStartTime = -1;
            checkAndCleanFrom(old);
        }
    }

    /**
     * @brief Returns true if ChannelInfo is currently recording.
     * @return true if ChannelInfo is recording
     */
    bool isRecording() const
    {
        return recordStartTime > -1;
    }

    /**
     * @brief Returns true if there are currently no active or inactive
     * AirFrames on the channel.
     */
    bool isChannelEmpty() const
    {
        ASSERT(recordStartTime != -1 || activeAirFrames.empty() == airFrameStarts.empty());

        return airFrameStarts.empty();
    }
};

} // namespace veins
