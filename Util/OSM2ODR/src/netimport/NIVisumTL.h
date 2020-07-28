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
/// @file    NIVisumTL.h
/// @author  Daniel Krajzewicz
/// @date    Wed, 07 May 2003
///
// Intermediate class for storing visum traffic lights during their import
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>
#include <string>
#include <netbuild/NBConnectionDefs.h>
#include <netbuild/NBNodeCont.h>
#include <utils/common/SUMOTime.h>

class NBTrafficLightLogicCont;
class NBEdgeCont;


// ===========================================================================
// class declaration
// ===========================================================================
/**
 * @class NIVisumTL
 * @brief Intermediate class for storing visum traffic lights during their import
 */
class NIVisumTL {
public:
    /** @class TimePeriod
     * @brief A time period with a start and an end time
     */
    class TimePeriod {
    public:
        /// @brief Constructor
        TimePeriod(SUMOTime startTime, SUMOTime endTime, SUMOTime yellowTime)
            : myStartTime(startTime), myEndTime(endTime), myYellowTime(yellowTime) {}

        /// @brief Destructor
        ~TimePeriod() {}

        /// @brief Returns the stored start time
        SUMOTime getStartTime() {
            return myStartTime;
        }

        /// @brief Returns the stored end time
        SUMOTime getEndTime() {
            return myEndTime;
        }

        /// @brief Returns the stored yellow time
        SUMOTime getYellowTime() {
            return myYellowTime;
        }

    private:
        /// @brief Start time
        SUMOTime myStartTime;
        /// @brief End time
        SUMOTime myEndTime;
        /// @brief Yellow time
        SUMOTime myYellowTime;
    };



    /** @class Phase
     * @brief A phase
     */
    class Phase : public TimePeriod {
    public:
        /// @brief Constructor
        Phase(SUMOTime startTime, SUMOTime endTime, SUMOTime yellowTime) : NIVisumTL::TimePeriod(startTime, endTime, yellowTime) {}

        /// @brief Destructor
        ~Phase() {}

    };



    /** @class SignalGroup
     * @brief A signal group can be defined either by a time period or by phases
     */
    class SignalGroup : public TimePeriod {
    public:
        /// @brief constructor
        SignalGroup(const std::string& name, SUMOTime startTime, SUMOTime endTime, SUMOTime yellowTime)
            : NIVisumTL::TimePeriod(startTime, endTime, yellowTime), myName(name) {}

        /// @brief destructor
        ~SignalGroup() {}

        /// @brief Returns the connections vector
        NBConnectionVector& connections() {
            return myConnections;
        }

        /// @brief Returns the phases map
        std::map<std::string, Phase*>& phases() {
            return myPhases;
        }

    private:
        /// @brief Connections
        NBConnectionVector myConnections;
        /// @brief phases
        std::map<std::string, Phase*> myPhases;
        /// @brief name
        std::string myName;
    };



public:
    /** @brief Constructor
     * @param[in] name The name of the TLS
     * @param[in] cycleTime The cycle time of the TLS
     * @param[in] offset Seconds to skip
     * @param[in] intermediateTime The name of the TLS
     * @param[in] phaseDefined Whether phases are defined
     */
    NIVisumTL(const std::string& name, SUMOTime cycleTime, SUMOTime offset, SUMOTime intermediateTime,
              bool phaseDefined);

    /// @brief Destructor
    ~NIVisumTL();

    /// @brief Adds a node to control
    void addNode(NBNode* n) {
        myNodes.push_back(n);
    }

    /// @brief Adds a signal group
    void addSignalGroup(const std::string& name, SUMOTime startTime, SUMOTime endTime, SUMOTime yellowTime);

    /// @brief Adds a phase
    void addPhase(const std::string& name, SUMOTime startTime, SUMOTime endTime, SUMOTime yellowTime);

    /// @brief Returns the map of named phases
    std::map<std::string, Phase*>& getPhases() {
        return myPhases;
    }

    /// @brief Returns the named signal group
    SignalGroup& getSignalGroup(const std::string& name);

    /// @brief build the traffic light and add it to the given container
    void build(NBEdgeCont& ec, NBTrafficLightLogicCont& tlc);

private:
    /// @brief The name of traffic light
    std::string myName;

    /// @brief The cycle time of traffic light in seconds
    SUMOTime myCycleTime;

    /// @brief The offset in the plan
    SUMOTime myOffset;

    /// @brief The all-red time (unused here)
    SUMOTime myIntermediateTime;

    /// @brief Toogles the usage either of phases or of time periods in signal groups
    bool myPhaseDefined;

    /// @brief Vector of nodes belonging to this traffic light
    std::vector<NBNode*> myNodes;

    /// @brief Map of used phases if phases defined
    std::map<std::string, Phase*> myPhases;

    /// @brief Map of used signal groups
    std::map<std::string, SignalGroup*> mySignalGroups;


};
