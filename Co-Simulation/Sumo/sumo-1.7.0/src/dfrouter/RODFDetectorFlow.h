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
/// @file    RODFDetectorFlow.h
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// missing_desc
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOTime.h>
#include <map>
#include <string>
#include <vector>


// ===========================================================================
// struct definitions
// ===========================================================================
/**
 * @class FlowDef
 * @brief Definition of the traffic during a certain time containing the flows and speeds
 */
struct FlowDef {
    // Number of passenger vehicles that passed within the described time
    double qPKW;
    // Number of heavy duty vehicles that passed within the described time
    double qLKW;
    // Mean velocity of passenger vehicles within the described time
    double vPKW;
    // Mean velocity of heavy duty vehicles within the described time
    double vLKW;
    // begin time (in s)
//    int time;
    // probability for having a heavy duty vehicle(qKFZ!=0 ? (qLKW / qKFZ) : 0;)
    double fLKW;
    // initialise with 0
    mutable double isLKW;
    //
    bool firstSet;
};


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFDetectorFlows
 * @brief A container for flows
 */
class RODFDetectorFlows {
public:
    RODFDetectorFlows(SUMOTime startTime, SUMOTime endTime,
                      SUMOTime stepOffset);
    ~RODFDetectorFlows();
    void addFlow(const std::string& detector_id, SUMOTime timestamp,
                 const FlowDef& fd);
    void removeFlow(const std::string& detector_id);
    void setFlows(const std::string& detector_id, std::vector<FlowDef>&);

    const std::vector<FlowDef>& getFlowDefs(const std::string& id) const;
    bool knows(const std::string& det_id) const;
    double getFlowSumSecure(const std::string& id) const;
    double getMaxDetectorFlow() const;
    void printAbsolute() const;

    void mesoJoin(const std::string& nid, const std::vector<std::string>& oldids);

protected:
    std::map<std::string, std::vector<FlowDef> > myFastAccessFlows;
    SUMOTime myBeginTime, myEndTime, myStepOffset;
    mutable double myMaxDetectorFlow;

private:
    /// @brief Invalidated copy constructor
    RODFDetectorFlows(const RODFDetectorFlows& src);

    /// @brief Invalidated assignment operator
    RODFDetectorFlows& operator=(const RODFDetectorFlows& src);

};
