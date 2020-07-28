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
/// @file    RouterProvider.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 03 March 2014
///
// The RouterProvider provides car, pedestrian and intermodal routing in one object
/****************************************************************************/
#pragma once
#include <config.h>

#include "IntermodalRouter.h"
#include "RailwayRouter.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RouterProvider
 * The encapsulation of the routers for vehicles and pedestrians
 */
template<class E, class L, class N, class V>
class RouterProvider {
public:
    RouterProvider(SUMOAbstractRouter<E, V>* vehRouter,
                   PedestrianRouter<E, L, N, V>* pedRouter,
                   IntermodalRouter<E, L, N, V>* interRouter,
                   RailwayRouter<E, V>* railRouter) :
        myVehRouter(vehRouter),
        myPedRouter(pedRouter),
        myInterRouter(interRouter),
        myRailRouter(railRouter)
    {}

    RouterProvider(const RouterProvider& original) :
        myVehRouter(original.myVehRouter->clone()),
        myPedRouter(static_cast<PedestrianRouter<E, L, N, V>*>(original.myPedRouter == 0 ? 0 : original.myPedRouter->clone())),
        myInterRouter(static_cast<IntermodalRouter<E, L, N, V>*>(original.myInterRouter == 0 ? 0 : original.myInterRouter->clone())),
        myRailRouter(static_cast<RailwayRouter<E, V>*>(original.myRailRouter == 0 ? 0 : original.myRailRouter->clone()))
    {}

    RouterProvider* clone() {
        return new RouterProvider(*this);
    }

    SUMOAbstractRouter<E, V>& getVehicleRouter(SUMOVehicleClass svc) const {
        if (myRailRouter == nullptr || !isRailway(svc)) {
            return *myVehRouter;
        } else {
            return *myRailRouter;
        }
    }

    PedestrianRouter<E, L, N, V>& getPedestrianRouter() const {
        return *myPedRouter;
    }

    IntermodalRouter<E, L, N, V>& getIntermodalRouter() const {
        return *myInterRouter;
    }

    RailwayRouter<E, V>& getRailwayRouter() const {
        return *myRailRouter;
    }

    virtual ~RouterProvider() {
        delete myVehRouter;
        delete myPedRouter;
        delete myInterRouter;
        delete myRailRouter;
    }


private:
    SUMOAbstractRouter<E, V>* const myVehRouter;
    PedestrianRouter<E, L, N, V>* const myPedRouter;
    IntermodalRouter<E, L, N, V>* const myInterRouter;
    RailwayRouter<E, V>* const myRailRouter;


private:
    /// @brief Invalidated assignment operator
    RouterProvider& operator=(const RouterProvider& src);

};
