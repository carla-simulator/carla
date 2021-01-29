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
/// @file    NBHeightMapperTest.cpp
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    2014-09-09
///
// Tests the class NBHeightMapper
/****************************************************************************/

#include <gtest/gtest.h>
#include <netbuild/NBHeightMapper.h>

class NBHeightMapperTest : public testing::Test {
protected :

    virtual void SetUp() {
        NBHeightMapper& hm = NBHeightMapper::myInstance;
        PositionVector t1;
        t1.push_back(Position(0, 0, 0));
        t1.push_back(Position(1, 0, 0));
        t1.push_back(Position(0, 1, 0));
        hm.addTriangle(t1);

        PositionVector t2;
        t2.push_back(Position(1, 0, 1));
        t2.push_back(Position(1, 1, 1));
        t2.push_back(Position(0, 1, 1));
        hm.addTriangle(t2);

        PositionVector t3;
        t3.push_back(Position(1, 0, 0));
        t3.push_back(Position(3, 0, 4));
        t3.push_back(Position(1, 2, 4));
        hm.addTriangle(t3);
    }

    virtual void TearDown() {
        NBHeightMapper& hm = NBHeightMapper::myInstance;
        hm.clearData();
    }
};

/* Test the method 'getZ'*/
TEST_F(NBHeightMapperTest, test_method_getZ) {
    const NBHeightMapper& hm = NBHeightMapper::get();
    EXPECT_TRUE(hm.ready());
    EXPECT_DOUBLE_EQ(0., hm.getZ(Position(0.25, 0.25)));
    EXPECT_DOUBLE_EQ(1., hm.getZ(Position(0.75, 0.75)));
    EXPECT_DOUBLE_EQ(2., hm.getZ(Position(1.5, 0.5)));
    //EXPECT_DOUBLE_EQ(0.5, hm.getZ(Position(0.5, 0.5, 100)));
}


