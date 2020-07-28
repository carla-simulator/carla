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
/// @file    CHRouterWrapper.h
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    March 2012
///
// Wraps multiple CHRouters for different vehicle types
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <functional>
#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <iterator>
#include <utils/common/SysUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/common/SUMOVehicleClass.h>
#include "CHRouter.h"

#ifdef HAVE_FOX
#include <utils/foxtools/FXWorkerThread.h>
#endif


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class CHRouterWrapper
 * @brief Computes the shortest path through a contracted network
 *
 * The template parameters are:
 * @param E The edge class to use (MSEdge/ROEdge)
 * @param V The vehicle class to use (MSVehicle/ROVehicle)
 *
 * The router is edge-based. It must know the number of edges for internal reasons
 *  and whether a missing connection between two given edges (unbuild route) shall
 *  be reported as an error or as a warning.
 *
 */
template<class E, class V>
class CHRouterWrapper: public SUMOAbstractRouter<E, V> {

public:
    /** @brief Constructor
     */
    CHRouterWrapper(const std::vector<E*>& edges, const bool ignoreErrors, typename SUMOAbstractRouter<E, V>::Operation operation,
                    const SUMOTime begin, const SUMOTime end, const SUMOTime weightPeriod, const int numThreads) :
        SUMOAbstractRouter<E, V>("CHRouterWrapper", ignoreErrors, operation, nullptr, false, false),
        myEdges(edges),
        myIgnoreErrors(ignoreErrors),
        myBegin(begin),
        myEnd(end),
        myWeightPeriod(weightPeriod),
        myMaxNumInstances(numThreads) {
    }

    ~CHRouterWrapper() {
        for (typename RouterMap::iterator i = myRouters.begin(); i != myRouters.end(); ++i) {
            delete i->second;
        }
    }


    virtual SUMOAbstractRouter<E, V>* clone() {
        CHRouterWrapper<E, V>* clone = new CHRouterWrapper<E, V>(myEdges, myIgnoreErrors, this->myOperation, myBegin, myEnd, myWeightPeriod, myMaxNumInstances);
        for (const auto& item : myRouters) {
            clone->myRouters[item.first] = static_cast<CHRouterType*>(item.second->clone());
        }
        return clone;
    }


    bool compute(const E* from, const E* to, const V* const vehicle,
                 SUMOTime msTime, std::vector<const E*>& into, bool silent = false) {
        const std::pair<const SUMOVehicleClass, const double> svc = std::make_pair(vehicle->getVClass(), vehicle->getMaxSpeed());
        if (myRouters.count(svc) == 0) {
            // create new router for the given permissions and maximum speed
            // XXX a new router may also be needed if vehicles differ in speed factor
            myRouters[svc] = new CHRouterType(
                myEdges, myIgnoreErrors, &E::getTravelTimeStatic, svc.first, myWeightPeriod, false, false);
        }
        return myRouters[svc]->compute(from, to, vehicle, msTime, into, silent);
    }


private:
    typedef CHRouter<E, V> CHRouterType;

private:
    typedef std::map<std::pair<const SUMOVehicleClass, const double>, CHRouterType*> RouterMap;

    RouterMap myRouters;

    /// @brief all edges with numerical ids
    const std::vector<E*>& myEdges;

    const bool myIgnoreErrors;

    const SUMOTime myBegin;
    const SUMOTime myEnd;
    const SUMOTime myWeightPeriod;
    const int myMaxNumInstances;
};
