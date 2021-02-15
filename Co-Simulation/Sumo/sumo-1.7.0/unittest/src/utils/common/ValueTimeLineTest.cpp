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
/// @file    ValueTimeLineTest.cpp
/// @author  Daniel Krajzewicz
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    Sept 2009
///
// Tests ValueTimeLine class from <SUMO>/src/utils/common
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <gtest/gtest.h>
#include <utils/common/ValueTimeLine.h>


// ===========================================================================
// test definitions
// ===========================================================================
/* Tests what happens if one tries to get a value from an empty ValueTimeLine. */
/*
TEST(ValueTimeLine, test_get_from_empty) {
    ValueTimeLine<int> vtl;
    EXPECT_EQ(1, vtl.getValue(0)) << "Something should happen if nothing was stored.";
}
*/


// --------------------------------
// plain retrieval / overwriting tests
// --------------------------------

/* Tests what happens if one tries to get a stored value (one value stored, fillGaps not called). */
TEST(ValueTimeLine, test_get_single_nocollect) {
    ValueTimeLine<int> vtl;
    vtl.add(0, 100, 2);
    EXPECT_EQ(2, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
}

/* Tests what happens if one tries to get a stored value (three values stored, fillGaps not called). */
TEST(ValueTimeLine, test_get_multi_nocollect) {
    ValueTimeLine<int> vtl;
    vtl.add(0, 100, 1);
    vtl.add(100, 200, 2);
    vtl.add(200, 300, 3);
    EXPECT_EQ(1, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}

/* Tests what happens if one tries to get a stored value (one value stored, fillGaps called). */
TEST(ValueTimeLine, test_get_single_collect) {
    ValueTimeLine<int> vtl;
    vtl.add(0, 100, 2);
    vtl.fillGaps(0);
    EXPECT_EQ(2, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
}

/* Tests what happens if one tries to get a stored value (three values stored, fillGaps called). */
TEST(ValueTimeLine, test_get_multi_collect) {
    ValueTimeLine<int> vtl;
    vtl.add(0, 100, 1);
    vtl.add(100, 200, 2);
    vtl.add(200, 300, 3);
    vtl.fillGaps(0);
    EXPECT_EQ(1, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}

// --------------------------------
// overwriting filling tests
// --------------------------------

/* Tests what happens if one overwrites a value (three values stored, fillGaps not called). */
TEST(ValueTimeLine, test_overwrite_nocollect) {
    ValueTimeLine<int> vtl;
    vtl.add(0, 100, 1);
    vtl.add(200, 300, 3);
    vtl.add(50, 250, 2);
    EXPECT_EQ(1, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}

/* Tests what happens if one overwrites a value (three values stored, fillGaps called). */
TEST(ValueTimeLine, test_overwrite_collect) {
    ValueTimeLine<int> vtl;
    vtl.add(0, 100, 1);
    vtl.add(200, 300, 3);
    vtl.add(50, 250, 2);
    vtl.fillGaps(0);
    EXPECT_EQ(1, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}

/* Tests what happens if one overwrites a value (three values stored, fillGaps not called, order changed). */
TEST(ValueTimeLine, test_overwrite_nocollect2) {
    ValueTimeLine<int> vtl;
    vtl.add(50, 250, 2);
    vtl.add(0, 100, 1);
    vtl.add(200, 300, 3);
    EXPECT_EQ(1, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}

/* Tests what happens if one overwrites a value (three values stored, fillGaps called, order changed). */
TEST(ValueTimeLine, test_overwrite_collect2) {
    ValueTimeLine<int> vtl;
    vtl.add(50, 250, 2);
    vtl.add(0, 100, 1);
    vtl.add(200, 300, 3);
    vtl.fillGaps(0);
    EXPECT_EQ(1, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(1, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(3, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}


// --------------------------------
// gap filling tests
// --------------------------------

/* Tests what happens if one overwrites a value (three values stored, fillGaps called). */
TEST(ValueTimeLine, test_fill_gaps_withbounds) {
    ValueTimeLine<int> vtl;
    vtl.add(50, 250, 2);
    vtl.fillGaps(4, true);
    EXPECT_EQ(2, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}


/* Tests what happens if one overwrites a value (three values stored, fillGaps called). */
TEST(ValueTimeLine, test_fill_gaps_nobounds) {
    ValueTimeLine<int> vtl;
    vtl.add(50, 250, 2);
    vtl.fillGaps(4, false);
    EXPECT_EQ(4, vtl.getValue(0)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(99)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(50)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(100)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(199)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(150)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(2, vtl.getValue(200)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(4, vtl.getValue(299)) << "The stored number should be returned when asking within the interval.";
    EXPECT_EQ(4, vtl.getValue(250)) << "The stored number should be returned when asking within the interval.";
}


