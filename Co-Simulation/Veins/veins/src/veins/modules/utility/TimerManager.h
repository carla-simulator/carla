//
// Copyright (C) 2018-2018 Max Schettler <max.schettler@ccs-labs.org>
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

#include <functional>
#include <map>
#include <string>

#include "veins/veins.h"

namespace veins {

/**
 * Abstraction for (recurring) Timers for cSimpleModule.
 *
 * This abstraction takes care of managing the required self-messages to (repeatedly) execute a piece of code after a certain time.
 * To use, instantiate one TimerManager per cSimpleModule, then call its handleMessage method from that of the cSimpleModule.
 *
 * In order to schedule a timer, create a TimerSpecification object using the corresponding methods.
 * After configuration, use the create function from the TimerManager to actually schedule the configured timer.
 */
class TimerManager;

/**
 * A message which is used for triggering Timers.
 *
 * Its implementation is empty as it is only used to differentiate from other
 * messages.
 */
struct TimerMessage;

/**
 * A class which specifies a Timer.
 *
 * This includes timing information as well as its callback.
 */
struct VEINS_API TimerSpecification {
public:
    /**
     * Create a new TimerSpecification.
     *
     * The created timer is invalid, an interval is missing for it to be usable.
     * By default, the timer starts running immediately and triggers first after the first time after the interval.
     * After that, it will continue to run until the simulation ends, calling the callback after the interval has elapsed.
     * In order to create a timer, this needs to be passed to TimerManager::create.
     *
     * @param callback The callback which is executed when the timer is triggered.
     *
     * @see TimerManager
     */
    TimerSpecification(std::function<void()> callback);

    /**
     * Set the period between two timer occurences.
     */
    TimerSpecification& interval(omnetpp::simtime_t interval);

    /**
     * Set the number of repetitions.
     *
     * @note You cannot use both this and absoluteEnd or relativeEnd.
     */
    TimerSpecification& repetitions(size_t n);

    /**
     * Set the timer's start time.
     *
     * Any previously set start time will be overwritten.
     *
     * @param start Time of first execution relative to the current simulation time. E.g., passing simtime_t(1, SIMTIME_S) will execute the timer in one second.
     *
     * @note You cannot use this in conjunction with repetition().
     */
    TimerSpecification& relativeStart(omnetpp::simtime_t start);

    /**
     * Set the timer's start time.
     *
     * Any previously set start time will be overwritten.
     *
     * @param start The absolute start time. The first occurence will be exactly at this time. Passing a value earlier than the current simtime will result in an error.
     *
     * @note You cannot use this in conjunction with repetition().
     */
    TimerSpecification& absoluteStart(omnetpp::simtime_t start);

    /**
     * Set the timer's end time.
     *
     * Any previously set end time will be overwritten.
     *
     * @param end Time after which this timer will no longer be executed, relative to the current simulation time. E.g., passing simtime_t(1, SIMTIME_S) will stop the execution of the time after one second has passed.
     */
    TimerSpecification& relativeEnd(omnetpp::simtime_t end);

    /**
     * Set the timer's end time.
     *
     * Any previously set end time will be overwritten.
     *
     * @param end The absolute end time. The latest possible occurence is at this time. Values before the current start time will prevent any executions.
     */
    TimerSpecification& absoluteEnd(omnetpp::simtime_t end);

    /**
     * Set the timer to be open ended.
     *
     * Any previously set end time will be overwritten.
     */
    TimerSpecification& openEnd();

    /**
     * Set the timer to execute once in a given time.
     *
     * Any previously set start time, end time, and interval will be overwritten.
     */
    TimerSpecification& oneshotIn(omnetpp::simtime_t in);

    /**
     * Set the timer to execute once at a given time.
     *
     * Any previously set start time, end time, and interval will be overwritten.
     */
    TimerSpecification& oneshotAt(omnetpp::simtime_t at);

private:
    friend TimerManager;

    enum class StartMode {
        relative,
        absolute,
        immediate
    };
    enum class EndMode {
        relative,
        absolute,
        repetition,
        open
    };

    /**
     * Finalizes this instance such that its values are independent of current simulation time.
     *
     * After calling this function, start_mode_ is guaranteed to be StartMode::absolute and end_mode_ to be EndMode::absolute or EndMode::open.
     */
    void finalize();

    /**
     * Checks validity of this specification, i.e., whether all necessary information is set.
     */
    bool valid() const
    {
        return period_ != -1;
    }

    /**
     * Check that the given time is a valid occurence for this timer.
     */
    bool validOccurence(omnetpp::simtime_t time) const;

    StartMode start_mode_; ///< Interpretation of start time._
    omnetpp::simtime_t start_; ///< Time of the Timer's first occurence. Interpretation depends on start_mode_.
    EndMode end_mode_; ///< Interpretation of end time._
    unsigned end_count_; ///< Number of repetitions of the timer. Only valid when end_mode_ == repetition.
    omnetpp::simtime_t end_time_; ///< Last possible occurence of the timer. Only valid when end_mode_ != repetition.
    omnetpp::simtime_t period_; ///< Time between events.
    std::function<void()> callback_; ///< The function to be called when the Timer is triggered.
};

class VEINS_API TimerManager {
private:
public:
    using TimerHandle = long;
    using TimerList = std::map<TimerMessage*, const TimerSpecification>;

    TimerManager(omnetpp::cSimpleModule* parent);

    /**
     * Destroy this module.
     *
     * All associated events will be cancelled and the corresponding messages deleted.
     */
    ~TimerManager();

    /**
     * Handle the given message and, if applicable, trigger the associated timer.
     *
     * @param message The received message.
     * @return true, if the message was meant for this TimerManager. In this case, the passed message might be invalidated.
     */
    bool handleMessage(omnetpp::cMessage* message);

    /**
     * Create a new timer.
     *
     * @param timerSpecification Parameters for the new timer
     * @param name The timer's name
     * @return A handle for the timer.
     *
     * @see cancel
     * @see TimerSpecification
     */
    TimerHandle create(TimerSpecification timerSpecification, std::string name = "");

    /**
     * Cancel a timer.
     *
     * Prevents any future executions of the given timer. Expired timers are silently ignored.
     *
     * @param handle A handle which identifies the timer.
     */
    void cancel(TimerHandle handle);

private:
    TimerList timers_; ///< List of all active Timers.
    omnetpp::cSimpleModule* const parent_; ///< A pointer to the module which owns this TimerManager.
};

} // namespace veins
