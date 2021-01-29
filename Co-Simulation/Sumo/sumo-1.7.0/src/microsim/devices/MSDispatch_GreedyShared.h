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
/// @file    MSDispatch_GreedyShared.h
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
#include <utils/common/SUMOTime.h>
#include "MSDispatch_Greedy.h"
#include "MSDevice_Taxi.h"


/**
 * @class MSDispatch_Greedy
 * @brief A dispatch algorithm that services customers in reservation order but also tries to do ride sharing
 */
class MSDispatch_GreedyShared : public MSDispatch_Greedy {
public:
    MSDispatch_GreedyShared(const std::map<std::string, std::string>& params) :
        MSDispatch_Greedy(params),
        myAbsoluteLossThreshold(StringUtils::toDouble(getParameter("absLossThreshold", "300"))),
        myRelativeLossThreshold(StringUtils::toDouble(getParameter("relLossThreshold", "0.2")))
    {}

protected:
    /// @brief trigger taxi dispatch. @note: method exists so subclasses can inject code at this point (ride sharing)
    virtual int dispatch(MSDevice_Taxi* taxi, std::vector<Reservation*>::iterator& resIt, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, std::vector<Reservation*>& reservations);

    /// @brief absolute time threshold for declining shared ride in s
    const double myAbsoluteLossThreshold;

    /// @brief relative time threshold for declining shared ride
    const double myRelativeLossThreshold;
};
