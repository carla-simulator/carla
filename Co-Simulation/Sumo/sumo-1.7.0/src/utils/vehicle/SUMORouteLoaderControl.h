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
/// @file    SUMORouteLoaderControl.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Wed, 06 Nov 2002
///
// Class responsible for loading of routes from some files
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMORouteLoader;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class SUMORouteLoaderControl
 *
 * SUMORouteLoaderControl
 * This controls is initialised with the list of route loaders and uses them
 * to load routes step wise.
 * The parameter myInAdvanceStepNo holds the number of time steps to read the
 * routes in forward. If it is 0 (default), all routes will be read at once.
 */
class SUMORouteLoaderControl {
public:
    /// @brief constructor
    SUMORouteLoaderControl(SUMOTime inAdvanceStepNo);

    /// @brief destructor
    ~SUMORouteLoaderControl();

    /// @brief add another loader
    void add(SUMORouteLoader* loader);

    /// @brief loads the next routes up to and including the given time step
    void loadNext(SUMOTime step);

    /// @brief returns the timestamp of the first loaded vehicle or flow
    SUMOTime getFirstLoadTime() const {
        return myFirstLoadTime;
    }

    /// @brief returns whether loading is completed
    bool haveAllLoaded() const {
        return myAllLoaded;
    }

private:
    /// @brief the first time step for which vehicles were loaded
    SUMOTime myFirstLoadTime;

    /// @brief the time step up to which vehicles were loaded
    SUMOTime myCurrentLoadTime;

    /// @brief the number of routes to read in forward
    const SUMOTime myInAdvanceStepNo;

    /// @brief the list of route loaders
    std::vector<SUMORouteLoader*> myRouteLoaders;

    /// @brief information whether all routes shall be loaded and whether they were loaded
    bool myLoadAll, myAllLoaded;

private:
    /// @brief Invalidated copy constructor
    SUMORouteLoaderControl(const SUMORouteLoaderControl& src);

    /// @brief Invalidated assignment operator
    SUMORouteLoaderControl& operator=(const SUMORouteLoaderControl& src);
};
