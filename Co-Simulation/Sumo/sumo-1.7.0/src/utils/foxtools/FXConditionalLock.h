/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2018-2020 German Aerospace Center (DLR) and others.
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
/// @file    FXConditionalLock.h
/// @author  Michael Behrisch
/// @date    2018-11-14
///
// A scoped lock which only triggers on condition
/****************************************************************************/

#ifndef FXConditionalLock_h
#define FXConditionalLock_h
#include <config.h>

#include <fx.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class FXConditionalLock
 * @brief A scoped lock which only triggers on condition
 */
class FXConditionalLock {
public:
    /// Construct & lock associated mutex if the condition is true
    FXConditionalLock(FXMutex& m, const bool condition)
        : myMutex(m), myCondition(condition) {
        if (condition) {
            m.lock();
        }
    }

    /// Destroy and unlock associated mutex
    ~FXConditionalLock() {
        if (myCondition) {
            myMutex.unlock();
        }
    }

private:
    FXMutex& myMutex;
    const bool myCondition;

private:
    FXConditionalLock& operator=(const FXConditionalLock&) = delete;
};


#endif
