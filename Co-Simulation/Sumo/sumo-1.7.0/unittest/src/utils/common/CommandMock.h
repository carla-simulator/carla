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
/// @file    CommandMock.h
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    2009-11-30
///
//
/****************************************************************************/

#ifndef Command_Mock_h
#define Command_Mock_h

#include <utils/common/Command.h>

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class CommandMock
 * Mock Implementation for Unit Tests
 */
class CommandMock : public Command {
public:
    /** @brief Constructor
     */
    CommandMock() {
        hasRun = false;
    }

    /// @brief Destructor
    ~CommandMock() {}

    /** @brief Executes the command.*/
    SUMOTime execute(SUMOTime currentTime) {
        hasRun = true;
        return currentTime;
    }

    /** return true, when execute was called, otherwise false */
    bool isExecuteCalled() {
        return hasRun;
    }

private:
    bool hasRun;

};


#endif

/****************************************************************************/

