/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMORouteLoader.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 6 Nov 2002
///
// A class that performs the loading of routes
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMORouteHandler;
class SUMOSAXReader;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class SUMORouteLoader
 */
class SUMORouteLoader {
public:
    /// @brief constructor
    SUMORouteLoader(SUMORouteHandler* handler);

    /// @brief destructor
    ~SUMORouteLoader();

    /// @brief loads vehicles until a vehicle is read that starts after the specified time
    SUMOTime loadUntil(SUMOTime time);

    /// @brief returns the information whether new data is available
    bool moreAvailable() const;

    /// @brief returns the first departure time that was ever read
    SUMOTime getFirstDepart() const;

private:
    /// @brief the used SAXReader
    SUMOSAXReader* myParser;

    /// @brief flag with information whether more vehicles should be available
    bool myMoreAvailable;

    /// @brief the used Handler
    SUMORouteHandler* myHandler;
};
