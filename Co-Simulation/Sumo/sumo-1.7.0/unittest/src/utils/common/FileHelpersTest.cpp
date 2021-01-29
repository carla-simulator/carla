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
/// @file    FileHelpersTest.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    2009
///
// Tests FileHelpers class from <SUMO>/src/utils/common
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#include <gtest/gtest.h>
#include <utils/common/FileHelpers.h>


// ===========================================================================
// test definitions
// ===========================================================================
/* Tests the method checkForRelativity. Unify special filenames and make standard file paths relative to the config. */
TEST(FileHelpers, test_method_checkForRelativity) {
    EXPECT_EQ("stdout", FileHelpers::checkForRelativity("-", "")) << "Special filename '-' should be treated as stdout";
    EXPECT_EQ("/home/user/test.net.xml", FileHelpers::checkForRelativity("test.net.xml", "/home/user/test.sumocfg")) << "configuration path should be used.";
    EXPECT_EQ("/test.net.xml", FileHelpers::checkForRelativity("test.net.xml", "/test.sumocfg")) << "configuration path should be used.";
}


TEST(FileHelpers, test_method_getConfigurationRelative) {
    EXPECT_EQ("/home/user/test.net.xml", FileHelpers::getConfigurationRelative("/home/user/test.sumocfg", "test.net.xml")) << "configuration path should be used.";
    EXPECT_EQ("/test.net.xml", FileHelpers::getConfigurationRelative("/test.sumocfg", "test.net.xml")) << "configuration path should be used.";
}


TEST(FileHelpers, test_method_getFilePath) {
    EXPECT_EQ("/home/user/", FileHelpers::getFilePath("/home/user/test.sumocfg")) << "configuration path should be used.";
    EXPECT_EQ("/", FileHelpers::getFilePath("/test.sumocfg")) << "configuration path should be used.";
}


