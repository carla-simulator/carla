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
#include "veins/modules/utility/TimerManager.h"

#include <algorithm>

using omnetpp::simTime;
using omnetpp::simtime_t;
using veins::TimerManager;
using veins::TimerMessage;
using veins::TimerSpecification;

struct veins::TimerMessage : public omnetpp::cMessage {
    TimerMessage(const std::string& name)
        : omnetpp::cMessage(name.c_str())
    {
    }
};

TimerSpecification::TimerSpecification(std::function<void()> callback)
    : start_mode_(StartMode::immediate)
    , end_mode_(EndMode::open)
    , period_(-1)
    , callback_(callback)
{
}

TimerSpecification& TimerSpecification::interval(simtime_t interval)
{
    ASSERT(interval > 0);
    period_ = interval;
    return *this;
}

TimerSpecification& TimerSpecification::relativeStart(simtime_t start)
{
    start_mode_ = StartMode::relative;
    start_ = start;
    return *this;
}

TimerSpecification& TimerSpecification::absoluteStart(simtime_t start)
{
    start_mode_ = StartMode::absolute;
    start_ = start;
    return *this;
}

TimerSpecification& TimerSpecification::relativeEnd(simtime_t end)
{
    end_mode_ = EndMode::relative;
    end_time_ = end;
    return *this;
}

TimerSpecification& TimerSpecification::absoluteEnd(simtime_t end)
{
    end_mode_ = EndMode::absolute;
    end_time_ = end;
    return *this;
}

TimerSpecification& TimerSpecification::repetitions(size_t n)
{
    end_mode_ = EndMode::repetition;
    end_count_ = n;
    return *this;
}

TimerSpecification& TimerSpecification::openEnd()
{
    end_mode_ = EndMode::open;
    return *this;
}

TimerSpecification& TimerSpecification::oneshotIn(omnetpp::simtime_t in)
{
    return this->relativeStart(in).interval(1).repetitions(1);
}

TimerSpecification& TimerSpecification::oneshotAt(omnetpp::simtime_t at)
{
    return this->absoluteStart(at).interval(1).repetitions(1);
}

void TimerSpecification::finalize()
{
    switch (start_mode_) {
    case StartMode::relative:
        start_ += simTime();
        start_mode_ = StartMode::absolute;
        break;
    case StartMode::absolute:
        break;
    case StartMode::immediate:
        start_ = simTime() + period_;
        break;
    }

    switch (end_mode_) {
    case EndMode::relative:
        end_time_ += simTime();
        end_mode_ = EndMode::absolute;
        break;
    case EndMode::absolute:
        break;
    case EndMode::repetition:
        end_time_ = start_ + ((end_count_ - 1) * period_);
        end_mode_ = EndMode::absolute;
        break;
    case EndMode::open:
        break;
    }
}

bool TimerSpecification::validOccurence(simtime_t time) const
{
    const bool afterStart = time >= start_;
    const bool beforeEnd = time <= end_time_;
    const bool atPeriod = omnetpp::fmod(time - start_, period_) == 0;
    return afterStart && (beforeEnd || end_mode_ == EndMode::open) && atPeriod;
}

TimerManager::TimerManager(omnetpp::cSimpleModule* parent)
    : parent_(parent)
{
    ASSERT(parent_);
}

TimerManager::~TimerManager()
{
    for (const auto& timer : timers_) {
        parent_->cancelAndDelete(timer.first);
    }
}

bool TimerManager::handleMessage(omnetpp::cMessage* message)
{
    auto* timerMessage = dynamic_cast<TimerMessage*>(message);
    if (!timerMessage) {
        return false;
    }
    ASSERT(timerMessage->isSelfMessage());
    std::string s = timerMessage->getName();

    auto timer = timers_.find(timerMessage);
    if (timer == timers_.end()) {
        return false;
    }
    ASSERT(timer->second.valid() && timer->second.validOccurence(simTime()));

    timer->second.callback_();

    if (timers_.find(timerMessage) != timers_.end()) { // confirm that the timer has not been cancelled during the callback
        const auto next_event = simTime() + timer->second.period_;
        if (timer->second.validOccurence(next_event)) {
            parent_->scheduleAt(next_event, timer->first);
        }
        else {
            parent_->cancelAndDelete(timer->first);
            timers_.erase(timer);
        }
    }

    return true;
}

TimerManager::TimerHandle TimerManager::create(TimerSpecification timerSpecification, const std::string name)
{
    ASSERT(timerSpecification.valid());
    timerSpecification.finalize();

    const auto ret = timers_.insert(std::make_pair(new TimerMessage(name), std::move(timerSpecification)));
    ASSERT(ret.second);
    parent_->scheduleAt(ret.first->second.start_, ret.first->first);

    return ret.first->first->getId();
}

void TimerManager::cancel(TimerManager::TimerHandle handle)
{
    const auto entryMatchesHandle = [handle](const std::pair<TimerMessage*, TimerSpecification>& entry) { return entry.first->getId() == handle; };
    auto timer = std::find_if(timers_.begin(), timers_.end(), entryMatchesHandle);
    if (timer != timers_.end()) {
        parent_->cancelAndDelete(timer->first);
        timers_.erase(timer);
    }
}
