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
/// @file    GUIEvent_SimulationLoaded.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// Event send when the simulation has been loaded by GUILadThread
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <iostream>
#include <utils/gui/events/GUIEvent.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUINet;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class  GUIEvent_SimulationLoaded
 *
 * Throw to GUIApplicationWindow from GUILoadThread after a simulation has
 * been loaded or the loading process failed
 */
class GUIEvent_SimulationLoaded : public GUIEvent {
public:
    /// constructor
    GUIEvent_SimulationLoaded(GUINet* net,
                              SUMOTime startTime, SUMOTime endTime,
                              const std::string& file,
                              const std::vector<std::string>& settingsFiles,
                              const bool osgView,
                              const bool viewportFromRegistry)
        : GUIEvent(EVENT_SIMULATION_LOADED),
          myNet(net), myBegin(startTime), myEnd(endTime),
          myFile(file), mySettingsFiles(settingsFiles),
          myOsgView(osgView),
          myViewportFromRegistry(viewportFromRegistry)
    { }

    /// destructor
    ~GUIEvent_SimulationLoaded() { }

public:
    /// the loaded net
    GUINet*  myNet;

    /// the time the simulation shall start with
    const SUMOTime myBegin;

    /// the time the simulation shall end with
    const SUMOTime myEnd;

    /// the name of the loaded file
    const std::string myFile;

    /// the name of the settings file to load
    const std::vector<std::string> mySettingsFiles;

    /// whether to load the OpenSceneGraph view
    const bool myOsgView;

    /// @brief whether loading viewport from registry
    const bool myViewportFromRegistry;

private:
    /// @brief Invalidated assignment operator
    GUIEvent_SimulationLoaded& operator=(const GUIEvent_SimulationLoaded& s);
};
