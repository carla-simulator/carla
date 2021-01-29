/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDispatch_Greedy.h
/// @author  Jakob Erdmann
/// @date    16.12.2019
///
// An algorithm that performs dispatch for the taxi device
/****************************************************************************/
#pragma once
#include <config.h>

#include <set>
#include <vector>
#include <map>
#include <utils/common/Parameterised.h>
#include <utils/common/SUMOTime.h>
#include "MSDispatch.h"
#include "MSDevice_Taxi.h"


// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class MSDispatch_Greedy
 * @brief A dispatch algorithm that services customers in reservation order and always sends the closest available taxi
 */
class MSDispatch_Greedy : public MSDispatch {
public:
    MSDispatch_Greedy(const std::map<std::string, std::string>& params) :
        MSDispatch(params),
        myRoutingMode(StringUtils::toInt(getParameter("routingMode", "1"))),
        myMaximumWaitingTime(TIME2STEPS(StringUtils::toInt(getParameter("maxWaitingTime", "300")))),
        myRecheckTime(TIME2STEPS(StringUtils::toInt(getParameter("recheckTime", "120")))),
        myRecheckSafety(TIME2STEPS(StringUtils::toInt(getParameter("recheckSafety", "3600"))))
    { }

    virtual void computeDispatch(SUMOTime now, const std::vector<MSDevice_Taxi*>& fleet);

protected:
    /// @brief trigger taxi dispatch. @note: method exists so subclasses can inject code at this point (ride sharing)
    virtual int dispatch(MSDevice_Taxi* taxi, std::vector<Reservation*>::iterator& resIt, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, std::vector<Reservation*>& reservations);

    /// @brief which router/edge weights to use
    const int myRoutingMode;

    /// @brief maximum time to arrive earlier at customer
    const SUMOTime myMaximumWaitingTime;

    /// @brief recheck interval for early reservations
    const SUMOTime myRecheckTime;
    const SUMOTime myRecheckSafety;

private:
    /// @brief Invalidated assignment operator.
    MSDispatch_Greedy& operator=(const MSDispatch_Greedy&) = delete;

};


/**
 * @class MSDispatch_GreedyClosest
 * @brief A dispatch algorithm that services the reservations with the shortest traveltime-to-pickup first
 */
class MSDispatch_GreedyClosest : public MSDispatch_Greedy {
public:
    MSDispatch_GreedyClosest(const std::map<std::string, std::string>& params) :
        MSDispatch_Greedy(params)
    {}

    void computeDispatch(SUMOTime now, const std::vector<MSDevice_Taxi*>& fleet);

};
