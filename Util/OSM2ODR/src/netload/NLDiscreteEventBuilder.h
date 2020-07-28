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
/// @file    NLDiscreteEventBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sep, 2003
///
// missing_desc
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/xml/GenericSAXHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NLDiscreteEventBuilder
 * This class is responsible for building event-handling actions which
 *  the simulation shall execute.
 */
class NLDiscreteEventBuilder {
public:
    /// Known action types
    enum ActionType {
        /// "SaveTLSStates"
        EV_SAVETLSTATE,
        /// "SaveTLSSwitchTimes"
        EV_SAVETLSWITCHES,
        /// "SaveTLSSwitchStates"
        EV_SAVETLSWITCHSTATES,
        /// "SaveTLSProgram"
        EV_SAVETLSPROGRAM
    };

    /// Constructor
    NLDiscreteEventBuilder(MSNet& net);

    /// Destructor
    ~NLDiscreteEventBuilder();

    /// Builds an action and saves it for further use
    void addAction(const SUMOSAXAttributes& attrs, const std::string& basePath);

private:
    /// Builds an action which saves the state of a certain tls into a file
    void buildSaveTLStateCommand(const SUMOSAXAttributes& attrs, const std::string& basePath);

    /// Builds an action which saves the switch times of links into a file
    void buildSaveTLSwitchesCommand(const SUMOSAXAttributes& attrs, const std::string& basePath);

    /// Builds an action which saves the switch times and states of tls into a file
    void buildSaveTLSwitchStatesCommand(const SUMOSAXAttributes& attrs, const std::string& basePath);

    /// Builds an action which saves the tls states as a loadable program into a file
    void buildSaveTLSProgramCommand(const SUMOSAXAttributes& attrs, const std::string& basePath);

private:
    NLDiscreteEventBuilder& operator=(const NLDiscreteEventBuilder&); // just to avoid a compiler warning

protected:
    /// Definitions of a storage for build actions
    typedef std::map<std::string, ActionType> KnownActions;

    /// Build actions that shall be executed during the simulation
    KnownActions myActions;

    MSNet& myNet;

};
