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
/// @file    RandHelperTest.cpp
/// @author  Michael Behrisch
/// @date    Oct 2010
///
// Tests the class RandHelper
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <gtest/gtest.h>
#include <utils/common/StdDefs.h>
#include <utils/common/RandHelper.h>


// ===========================================================================
// test definitions
// ===========================================================================
/* Test the method 'rand' without parameters.*/
TEST(RandHelper, test_rand_range) {
    for (int i = 0; i < 1000; i++) {
        const double rand = RandHelper::rand();
        EXPECT_LT(rand, double(1));
        EXPECT_LE(0., rand);
    }
}

/* Test the method 'rand' with an upper float limit.*/
TEST(RandHelper, test_rand_range_float) {
    for (int i = 0; i < 1000; i++) {
        const double rand = RandHelper::rand(double(10));
        EXPECT_LT(rand, double(10));
        EXPECT_LE(0., rand);
    }
}

/* Test the method 'rand' with an upper int limit.*/
TEST(RandHelper, test_rand_range_int) {
    for (int i = 0; i < 1000; i++) {
        const double rand = RandHelper::rand(100);
        EXPECT_LT(rand, 100);
        EXPECT_LE(0, rand);
    }
}

/* Test the method 'rand' with two float limits.*/
TEST(RandHelper, test_rand_range_two_float) {
    for (int i = 0; i < 1000; i++) {
        const double rand = RandHelper::rand(double(0.1), double(0.5));
        EXPECT_LT(rand, double(0.5));
        EXPECT_LE(double(0.1), rand);
    }
}

/* Test the method 'rand' with two int limits.*/
TEST(RandHelper, test_rand_range_two_int) {
    for (int i = 0; i < 1000; i++) {
        const double rand = RandHelper::rand(50, 100);
        EXPECT_LT(rand, 100);
        EXPECT_LE(50, rand);
    }
}

/* Test whether the 'rand' distribution is more or less uniform.*/
TEST(RandHelper, test_uniform) {
    int count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 1000; i++) {
        const double rand = RandHelper::rand(0., double(10));
        count[(int)rand]++;
    }
    for (int i = 0; i < 10; i++) {
        EXPECT_LE(50, count[i]) << "Testing interval " << i;
        EXPECT_LT(count[i], 150) << "Testing interval " << i;
    }
}

/* Test whether the 'randNorm' distribution is more or less gaussian.*/
TEST(RandHelper, test_norm) {
    int count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 1000; i++) {
        const double rand = RandHelper::randNorm(double(5), double(2));
        count[MIN2(MAX2((int)rand, 0), 9)]++;
    }
    EXPECT_LE(0, count[0]);
    EXPECT_LT(count[0], 100);
    EXPECT_LE(0, count[1]);
    EXPECT_LT(count[1], 100);
    EXPECT_LE(50, count[2]);
    EXPECT_LT(count[2], 150);
    EXPECT_LE(100, count[3]);
    EXPECT_LT(count[3], 200);
    EXPECT_LE(100, count[4]);
    EXPECT_LT(count[4], 250);
    EXPECT_LE(100, count[5]);
    EXPECT_LT(count[5], 250);
    EXPECT_LE(100, count[6]);
    EXPECT_LT(count[6], 200);
    EXPECT_LE(0, count[7]);
    EXPECT_LT(count[7], 100);
    EXPECT_LE(0, count[8]);
    EXPECT_LT(count[8], 100);
    EXPECT_LE(0, count[9]);
    EXPECT_LT(count[9], 100);
}

/* Test whether the 'rand' sequence is always the same.*/
TEST(RandHelper, test_sequence) {
    RandHelper::initRand();
    int expect[] = { 46, 17, 19, 75, 11, 42, 28, 22, 77, 11 };
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(expect[i], RandHelper::rand(100));
    }
}
