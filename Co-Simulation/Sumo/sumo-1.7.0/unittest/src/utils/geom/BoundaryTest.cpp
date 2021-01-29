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
/// @file    BoundaryTest.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    2009-05-27
///
// Tests the class Boundary
/****************************************************************************/

#include <gtest/gtest.h>
#include <utils/geom/Boundary.h>


/* Test the method 'add'*/
TEST(Boundary, test_method_add) {
    Boundary bound;
    bound.add(1, 2);
    EXPECT_DOUBLE_EQ(bound.xmax(), 1);
    EXPECT_DOUBLE_EQ(bound.xmin(), 1);
    EXPECT_DOUBLE_EQ(bound.ymax(), 2);
    EXPECT_DOUBLE_EQ(bound.ymin(), 2);
}

/* Test the method 'add' with multiple calls*/
TEST(Boundary, test_method_add_multiple) {
    Boundary bound;
    bound.add(-1, -2);
    bound.add(3, 5);
    bound.add(5, 8);
    EXPECT_DOUBLE_EQ(bound.xmax(), 5);
    EXPECT_DOUBLE_EQ(bound.xmin(), -1);
    EXPECT_DOUBLE_EQ(bound.ymax(), 8);
    EXPECT_DOUBLE_EQ(bound.ymin(), -2);
}

/* Test the method 'getCenter'*/
TEST(Boundary, test_method_getCenter) {
    Boundary bound(-2, -4, 4, 8);
    Position pos = bound.getCenter();
    EXPECT_DOUBLE_EQ(pos.x(), 1);
    EXPECT_DOUBLE_EQ(pos.y(), 2);
}

/* Test the method 'getWidth' and getHeight*/
TEST(Boundary, test_method_getWidthHeight) {
    Boundary bound(-2, -4, 4, 8);
    EXPECT_DOUBLE_EQ(bound.getHeight(), 12);
    EXPECT_DOUBLE_EQ(bound.getWidth(), 6);
}

/* Test the method 'around'*/
TEST(Boundary, test_method_around) {
    Boundary bound(1, 2, 3, 6);
    EXPECT_TRUE(bound.around(Position(2, 4)));
    EXPECT_FALSE(bound.around(Position(0, 4)));
    EXPECT_FALSE(bound.around(Position(2, 7)));
    EXPECT_TRUE(bound.around(Position(0, 7), 2));
}

/* Test the method 'overlapsWith'*/
TEST(Boundary, test_method_overlapsWith) {
    Boundary bound(1, 2, 3, 6);
    EXPECT_FALSE(bound.overlapsWith(Boundary(10, 17, 13, 16)));
    EXPECT_TRUE(bound.overlapsWith(Boundary(-1, -7, 2, 4)));
    EXPECT_TRUE(bound.overlapsWith(Boundary(1, 2, 3, 6)));
    EXPECT_TRUE(bound.overlapsWith(Boundary(0, 3, 4, 5)));
    EXPECT_FALSE(bound.overlapsWith(Boundary(4, 2, 5, 7), 0));
    EXPECT_TRUE(bound.overlapsWith(Boundary(4, 2, 5, 7), 1));
}

/* Test the method 'crosses'*/
TEST(Boundary, test_method_crosses) {
    Boundary bound(1, 2, 3, 6);
    EXPECT_TRUE(bound.crosses(Position(3, 2), Position(4, 2)));
    EXPECT_TRUE(bound.crosses(Position(2, 1), Position(0, 3)));
    EXPECT_TRUE(bound.crosses(Position(1, 2), Position(3, 6)));
    EXPECT_FALSE(bound.crosses(Position(0, 0), Position(0, 8)));
}

/* Test the method 'partialWithin'*/
TEST(Boundary, test_method_partialWithin) {
    Boundary bound(1, 2, 3, 6);
    EXPECT_TRUE(bound.partialWithin(Boundary(1, 2, 1, 2)));
    EXPECT_FALSE(bound.partialWithin(Boundary(10, 17, 13, 16)));
    EXPECT_TRUE(bound.partialWithin(Boundary(1, 2, 3, 6)));
    EXPECT_TRUE(bound.partialWithin(Boundary(4, 2, 5, 7), 1));
}

/* Test the method 'flipY'*/
TEST(Boundary, test_method_flipY) {
    Boundary bound(1, 2, 3, 6);
    bound.flipY();
    EXPECT_DOUBLE_EQ(bound.xmax(), 3);
    EXPECT_DOUBLE_EQ(bound.xmin(), 1);
    EXPECT_DOUBLE_EQ(bound.ymax(), -2);
    EXPECT_DOUBLE_EQ(bound.ymin(), -6);
}

/* Test the method 'moveby'*/
TEST(Boundary, test_method_moveby) {
    Boundary bound(1, 2, 3, 6);
    bound.moveby(2.5, -3.5);
    EXPECT_DOUBLE_EQ(bound.xmax(), 5.5);
    EXPECT_DOUBLE_EQ(bound.xmin(), 3.5);
    EXPECT_DOUBLE_EQ(bound.ymax(), 2.5);
    EXPECT_DOUBLE_EQ(bound.ymin(), -1.5);
}
