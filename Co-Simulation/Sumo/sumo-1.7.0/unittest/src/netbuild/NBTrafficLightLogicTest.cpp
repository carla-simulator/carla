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
/// @file    NBTrafficLightLogicTest.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2009-10-24
///
// Tests the class NBTrafficLightLogic
/****************************************************************************/

#include <gtest/gtest.h>
#include <netbuild/NBTrafficLightLogic.h>
#include "../utils/iodevices/OutputDeviceMock.h"


/* Test the method 'getDuration'*/
TEST(NBTrafficLightLogic, test_method_getDuration) {
    NBTrafficLightLogic tll("1", "11", 2);
    EXPECT_EQ(0, tll.getDuration());

    tll.addStep(3, "gr");
    tll.addStep(5, "rG");
    EXPECT_EQ(8, tll.getDuration());
}
