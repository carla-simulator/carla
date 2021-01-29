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
/// @file    GeomHelperTest.cpp
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    2011-12-12
///
// Tests the class GeomHelper
/****************************************************************************/

#include <gtest/gtest.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>

using namespace std;

/*
TEST(GeomHelper, test_method_intersects) {
    // on the same line but non-overlapping segments
	EXPECT_FALSE(GeomHelper::intersects(
                Position(0,0), Position(1,0),
                Position(2,0), Position(3,0)));

    // overlapping line segments
	EXPECT_TRUE(GeomHelper::intersects(
                Position(0,0), Position(2,0),
                Position(1,0), Position(3,0)));

    // identical line segments
	EXPECT_TRUE(GeomHelper::intersects(
                Position(1,0), Position(3,0),
                Position(1,0), Position(3,0)));

    // parallel line segments
	EXPECT_FALSE(GeomHelper::intersects(
                Position(0,0), Position(1,0),
                Position(0,1), Position(1,1)));

    // intersection outside of segments
	EXPECT_FALSE(GeomHelper::intersects(
                Position(0,0), Position(2,0),
                Position(1,3), Position(1,1)));

    // intersection at (1,0)
	EXPECT_TRUE(GeomHelper::intersects(
                Position(0,0), Position(2,0),
                Position(1,3), Position(1,-1)));
}


TEST(GeomHelper, test_method_intersection_position2D) {
    Position expected(1,0);
    Position pos = GeomHelper::intersection_position2D(
            Position(0,0), Position(2,0),
            Position(1,3), Position(1,-1));
	EXPECT_FLOAT_EQ(expected.x(), pos.x());
	EXPECT_FLOAT_EQ(expected.y(), pos.y());
    // overlapping line segments
    Position expected2(1.5,0);
    Position pos2 = GeomHelper::intersection_position2D(
            Position(0,0), Position(2,0),
            Position(1,0), Position(3,0));
	EXPECT_FLOAT_EQ(expected2.x(), pos2.x());
	EXPECT_FLOAT_EQ(expected2.y(), pos2.y());
    // identical line segments
    Position expected3(1,0);
    Position pos3 = GeomHelper::intersection_position2D(
            Position(0,0), Position(2,0),
            Position(0,0), Position(2,0));
	EXPECT_FLOAT_EQ(expected3.x(), pos3.x());
	EXPECT_FLOAT_EQ(expected3.y(), pos3.y());
}


TEST(GeomHelper, test_method_closestDistancePointLine_basic) {
    Position expected(1,0);
    double expectedDistance = 1;
    Position point(1,1);
    Position start(0,0);
    Position end(2,0);
    Position closestPoint;
    double result = GeomHelper::closestDistancePointLine2D(point, start, end, closestPoint);
	EXPECT_FLOAT_EQ(expected.x(), closestPoint.x());
	EXPECT_FLOAT_EQ(expected.y(), closestPoint.y());
	EXPECT_FLOAT_EQ(expected.z(), closestPoint.z());
	EXPECT_FLOAT_EQ(expectedDistance, result);
}
TEST(GeomHelper, test_method_closestDistancePointLine_onLine) {
    Position expected(1,0);
    double expectedDistance = 0;
    Position point(1,0);
    Position start(0,0);
    Position end(2,0);
    Position closestPoint;
    double result = GeomHelper::closestDistancePointLine2D(point, start, end, closestPoint);
	EXPECT_FLOAT_EQ(expected.x(), closestPoint.x());
	EXPECT_FLOAT_EQ(expected.y(), closestPoint.y());
	EXPECT_FLOAT_EQ(expected.z(), closestPoint.z());
	EXPECT_FLOAT_EQ(expectedDistance, result);
}
TEST(GeomHelper, test_method_closestDistancePointLine_outside_after) {
    Position expected(2,0);
    double expectedDistance = 5;
    Position point(5,4);
    Position start(0,0);
    Position end(2,0);
    Position closestPoint;
    double result = GeomHelper::closestDistancePointLine2D(point, start, end, closestPoint);
	EXPECT_FLOAT_EQ(expected.x(), closestPoint.x());
	EXPECT_FLOAT_EQ(expected.y(), closestPoint.y());
	EXPECT_FLOAT_EQ(expected.z(), closestPoint.z());
	EXPECT_FLOAT_EQ(expectedDistance, result);
}
TEST(GeomHelper, test_method_closestDistancePointLine_outside_before) {
    Position expected(0,0);
    double expectedDistance = 5;
    Position point(-3,4);
    Position start(0,0);
    Position end(2,0);
    Position closestPoint;
    double result = GeomHelper::closestDistancePointLine2D(point, start, end, closestPoint);
	EXPECT_FLOAT_EQ(expected.x(), closestPoint.x());
	EXPECT_FLOAT_EQ(expected.y(), closestPoint.y());
	EXPECT_FLOAT_EQ(expected.z(), closestPoint.z());
	EXPECT_FLOAT_EQ(expectedDistance, result);
}*/
