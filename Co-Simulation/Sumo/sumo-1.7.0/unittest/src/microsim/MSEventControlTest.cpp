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
/// @file    MSEventControlTest.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2009-11-23
///
// Tests the class MSEventControl
/****************************************************************************/

#include <gtest/gtest.h>
#include <microsim/MSEventControl.h>
#include "../utils/common/CommandMock.h"


/* Test the method 'execute'. Tests if the execute method from the Command Class is called.*/

TEST(MSEventControl, test_method_execute) {

    MSEventControl eventControl;
    CommandMock* mock = new CommandMock();
    eventControl.addEvent(mock);

    EXPECT_FALSE(mock->isExecuteCalled());
    eventControl.execute(5);
    EXPECT_TRUE(mock->isExecuteCalled());
}
