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
/// @file    MSDispatch_RouteExtension.h
/// @author  Michael Behrisch
/// @date    06.08.2020
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
 *  by extending or reusing the existing route
 */
class MSDispatch_RouteExtension : public MSDispatch_Greedy {
public:
    MSDispatch_RouteExtension(const std::map<std::string, std::string>& params) :
        MSDispatch_Greedy(params)
    {}

protected:
    /// @brief trigger taxi dispatch. @note: method exists so subclasses can inject code at this point (ride sharing)
    virtual int dispatch(MSDevice_Taxi* taxi, std::vector<Reservation*>::iterator& resIt, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, std::vector<Reservation*>& reservations);

private:
    typedef std::vector<std::pair<const MSEdge*, double> > EdgePosVector;
    void findInsertionPoint(std::vector<const Reservation*>::iterator& resIt, EdgePosVector::iterator& edgeIt,
                            const EdgePosVector::iterator& edgeEnd, ConstMSEdgeVector& route,
                            const MSEdge* newEdge, const double newPos) const;
};
