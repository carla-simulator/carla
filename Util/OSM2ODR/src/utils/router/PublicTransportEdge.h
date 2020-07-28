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
/// @file    PublicTransportEdge.h
/// @author  Michael Behrisch
/// @date    Mon, 03 March 2014
///
// The PublicTransportEdge is a special intermodal edge connecting the stop edges with scheduled traffic
/****************************************************************************/
#pragma once
#include <config.h>

#include "IntermodalEdge.h"


// ===========================================================================
// class definitions
// ===========================================================================
/// @brief the public transport edge type connecting the stop edges
template<class E, class L, class N, class V>
class PublicTransportEdge : public IntermodalEdge<E, L, N, V> {
private:
    struct Schedule {
        Schedule(const std::string& _id, const SUMOTime _begin, const int _repetitionNumber, const SUMOTime _period, const SUMOTime _travelTime)
            : ids({
            _id
        }), begin(_begin), repetitionNumber(_repetitionNumber), period(_period), travelTime(_travelTime) {}
        // the id of the vehicle or flow from which this schedule is generated
        std::vector<std::string> ids;
        const SUMOTime begin;
        int repetitionNumber;
        // the repetition period for a flow or -1 for a vehicle
        SUMOTime period;
        const SUMOTime travelTime;
    private:
        /// @brief Invalidated assignment operator
        Schedule& operator=(const Schedule& src);
    };

public:
    PublicTransportEdge(const std::string id, int numericalID, const IntermodalEdge<E, L, N, V>* entryStop, const E* endEdge, const std::string& line, const double length) :
        IntermodalEdge<E, L, N, V>(line + ":" + (id != "" ? id : endEdge->getID()), numericalID, endEdge, line, length), myEntryStop(entryStop) { }

    bool includeInRoute(bool /* allEdges */) const {
        return true;
    }

    bool prohibits(const IntermodalTrip<E, N, V>* const trip) const {
        return (trip->modeSet & SVC_BUS) == 0;
    }

    const IntermodalEdge<E, L, N, V>* getEntryStop() const {
        return myEntryStop;
    }

    bool hasSchedule(const SUMOTime begin) const {
        return mySchedules.find(begin) != mySchedules.end();
    }

    void addSchedule(const std::string id, const SUMOTime begin, const int repetitionNumber, const SUMOTime period, const SUMOTime travelTime) {
        // try to merge with existing vehicle or flow
        bool found = false;
        for (auto& it : mySchedules) {
            Schedule& s = it.second;
            if (travelTime == s.travelTime) {
                if (repetitionNumber == -1 && s.repetitionNumber == 1) {
                    if (begin > s.begin) {
                        s.period = begin - s.begin;
                        found = true;
                    }
                } else if (begin == s.begin + s.repetitionNumber * s.period) {
                    found = true;
                }
                if (found) {
                    s.repetitionNumber += MAX2(repetitionNumber, 1);
                    s.ids.push_back(id);
                    break;
                }
            }
        }
        if (!found) {
            mySchedules.insert(std::make_pair(begin, Schedule(id, begin, MAX2(repetitionNumber, 1), MAX2<SUMOTime>(period, 1), travelTime)));
        }
    }

    double getTravelTime(const IntermodalTrip<E, N, V>* const /* trip */, double time) const {
        SUMOTime minArrival = SUMOTime_MAX;
        const SUMOTime step = TIME2STEPS(time);
        for (typename std::multimap<SUMOTime, Schedule>::const_iterator it = mySchedules.begin(); it != mySchedules.end(); ++it) {
            const Schedule& s = it->second;
            if (it->first > minArrival) {
                break;
            }
            const SUMOTime offset = MAX2<SUMOTime>(0, step - s.begin);
            int running = (int)(offset / s.period);
            if (offset % s.period != 0) {
                running++;
            }
            if (running < s.repetitionNumber) {
                const SUMOTime nextDepart = s.begin + running * s.period;
                minArrival = MIN2(nextDepart + s.travelTime, minArrival);
                //std::cout << " edge=" << myEntryStop->getID() << "->" << this->getID() << " beg=" << s.begin << " end=" << s.end
                //    << " atTime=" << time
                //    << " running=" << running << " nextDepart=" << nextDepart
                //    << " minASec=" << minArrivalSec << " travelTime=" << minArrivalSec - time << "\n";
            }
        }
        return STEPS2TIME(minArrival - step);
    }

    double getIntended(const double time, std::string& intended) const {
        /// @note: duplicates some code of getTravelTime()
        SUMOTime minArrival = SUMOTime_MAX;
        double bestDepartTime = std::numeric_limits<double>::max();
        const SUMOTime step = TIME2STEPS(time);
        for (typename std::multimap<SUMOTime, Schedule>::const_iterator it = mySchedules.begin(); it != mySchedules.end(); ++it) {
            const Schedule& s = it->second;
            if (it->first > minArrival) {
                break;
            }
            const SUMOTime offset = MAX2<SUMOTime>(0, step - s.begin);
            int running = (int)(offset / s.period);
            if (offset % s.period != 0) {
                running++;
            }
            if (running < s.repetitionNumber) {
                const SUMOTime nextDepart = s.begin + running * s.period;
                if (nextDepart + s.travelTime < minArrival) {
                    minArrival = nextDepart + s.travelTime;
                    bestDepartTime = STEPS2TIME(nextDepart);
                    // see naming scheme inMSInsertionControl::determineCandidates()
                    if (s.ids.size() == 1 || running >= (int)s.ids.size()) {
                        intended = s.repetitionNumber == 1 ? s.ids[0] : s.ids[0] + "." + toString(running);
                    } else {
                        intended = s.ids[running];
                    }
                }
            }
        }
        return bestDepartTime;
    }

private:
    std::multimap<SUMOTime, Schedule> mySchedules;
    const IntermodalEdge<E, L, N, V>* const myEntryStop;

};
