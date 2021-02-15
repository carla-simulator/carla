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
/// @file    PositionVectorTest.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2009-10-24
///
// Tests the class PositionVector
/****************************************************************************/

#include <gtest/gtest.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/iodevices/OutputDevice.h>


#define EXPECT_DOUBLEVEC_EQUAL(v1, v2) \
{ \
    EXPECT_EQ(v1.size(), v2.size()); \
    if (v1.size() == v2.size()) { \
        for (int i = 0; i < (int)v1.size(); ++i) { \
            EXPECT_DOUBLE_EQ(v1[i], v2[i]); \
        } \
    } \
} \

class PositionVectorTest : public testing::Test {
protected :
    PositionVector* vectorPolygon;
    PositionVector* vectorLine;

    virtual void SetUp() {
        vectorPolygon = new PositionVector();
        vectorPolygon->push_back(Position(0, 0));
        vectorPolygon->push_back(Position(0, 2));
        vectorPolygon->push_back(Position(2, 4));
        vectorPolygon->push_back(Position(4, 2));
        vectorPolygon->push_back(Position(4, 0));

        vectorLine = new PositionVector();
        vectorLine->push_back(Position(0, 0));
        vectorLine->push_back(Position(2, 2));
    }

    virtual void TearDown() {
        delete vectorPolygon;
        delete vectorLine;
    }

};

/* Test the method 'around'*/
TEST_F(PositionVectorTest, test_method_around) {

    EXPECT_TRUE(vectorPolygon->around(Position(1, 1)));
    EXPECT_TRUE(vectorPolygon->around(Position(1, 2)));
    EXPECT_FALSE(vectorPolygon->around(Position(4, 4)));
    EXPECT_FALSE(vectorPolygon->around(Position(0, 0)));

    EXPECT_FALSE(vectorLine->around(Position(1, 1)));
    EXPECT_FALSE(vectorLine->around(Position(0, 2)));

    // with positive offset
    EXPECT_TRUE(vectorPolygon->around(Position(4, 2), 1));
    EXPECT_FALSE(vectorPolygon->around(Position(5, 2), 1));
    // what was true remains true
    EXPECT_TRUE(vectorPolygon->around(Position(1, 1), POSITION_EPS));
    EXPECT_TRUE(vectorPolygon->around(Position(1, 2), POSITION_EPS));

    // with negative offset
    EXPECT_FALSE(vectorPolygon->around(Position(4, 2), -POSITION_EPS));
    EXPECT_TRUE(vectorPolygon->around(Position(1, 1), -1));
    EXPECT_FALSE(vectorPolygon->around(Position(0.5, 0.5), -1));
}

/* Test the method 'area'*/
TEST_F(PositionVectorTest, test_method_area) {
    PositionVector square;
    square.push_back(Position(0, 0));
    square.push_back(Position(1, 0));
    square.push_back(Position(1, 1));
    square.push_back(Position(0, 1)); // open
    EXPECT_DOUBLE_EQ(square.area(), 1);
    square.push_back(Position(0, 0)); // closed
    EXPECT_DOUBLE_EQ(square.area(), 1);
}

/* Test the method 'scaleRelative'.*/
TEST_F(PositionVectorTest, test_method_scaleRelative) {
    PositionVector square;
    square.push_back(Position(0, 0));
    square.push_back(Position(1, 0));
    square.push_back(Position(1, 1));
    square.push_back(Position(0, 1));
    square.push_back(Position(0, 0));
    EXPECT_DOUBLE_EQ(square.area(), 1);
    square.scaleRelative(3);
    EXPECT_DOUBLE_EQ(square.area(), 9);

    PositionVector expected;
    expected.push_back(Position(-1, -1));
    expected.push_back(Position(2, -1));
    expected.push_back(Position(2, 2));
    expected.push_back(Position(-1, 2));
    expected.push_back(Position(-1, -1));

    EXPECT_EQ(expected.getCentroid(), square.getCentroid());
    for (int i = 0; i < (int)square.size(); i++) {
        EXPECT_DOUBLE_EQ(expected[i].x(), square[i].x());
        EXPECT_DOUBLE_EQ(expected[i].y(), square[i].y());
    }
}

/* Test the method 'getCentroid'.*/
TEST_F(PositionVectorTest, test_method_getCentroid) {
    PositionVector square;
    square.push_back(Position(0, 0));
    square.push_back(Position(1, 0));
    square.push_back(Position(1, 1));
    square.push_back(Position(0, 1));
    EXPECT_EQ(Position(0.5, 0.5), square.getCentroid());

    Position pos2 = vectorLine->getCentroid();
    EXPECT_DOUBLE_EQ(1, pos2.x());
    EXPECT_DOUBLE_EQ(1, pos2.y());
}

/* Test the method 'getPolygonCenter'.*/
TEST_F(PositionVectorTest, test_method_getPolygonCenter) {
    Position pos = vectorPolygon->getPolygonCenter();
    EXPECT_DOUBLE_EQ(2, pos.x());
    EXPECT_DOUBLE_EQ(1.6, pos.y());
    Position pos2 = vectorLine->getPolygonCenter();
}


/* Test the method 'getBoxBoundary'*/
TEST_F(PositionVectorTest, test_method_getBoxBoundary) {
    Boundary bound = vectorPolygon->getBoxBoundary();
    EXPECT_DOUBLE_EQ(bound.xmax(), 4);
    EXPECT_DOUBLE_EQ(bound.xmin(), 0);
    EXPECT_DOUBLE_EQ(bound.ymax(), 4);
    EXPECT_DOUBLE_EQ(bound.ymin(), 0);
}

/* Test the method 'splitAt'*/
TEST_F(PositionVectorTest, test_method_splitAt) {
    PositionVector vec;
    vec.push_back(Position(0, 0));
    vec.push_back(Position(2, 0));
    vec.push_back(Position(5, 0));
    double smallDiff = POSITION_EPS / 2;
    std::pair<PositionVector, PositionVector> result;
    // split in first segment
    result = vec.splitAt(1);
    EXPECT_EQ(2, (int)result.first.size());
    EXPECT_DOUBLE_EQ(0., result.first[0].x());
    EXPECT_DOUBLE_EQ(1., result.first[1].x());
    EXPECT_EQ(3, (int)result.second.size());
    EXPECT_DOUBLE_EQ(1., result.second[0].x());
    EXPECT_DOUBLE_EQ(2., result.second[1].x());
    EXPECT_DOUBLE_EQ(5., result.second[2].x());
    // split in second segment
    result = vec.splitAt(4);
    EXPECT_EQ(3, (int)result.first.size());
    EXPECT_DOUBLE_EQ(0., result.first[0].x());
    EXPECT_DOUBLE_EQ(2., result.first[1].x());
    EXPECT_DOUBLE_EQ(4., result.first[2].x());
    EXPECT_EQ(2, (int)result.second.size());
    EXPECT_DOUBLE_EQ(4., result.second[0].x());
    EXPECT_DOUBLE_EQ(5., result.second[1].x());
    // split close before inner point
    result = vec.splitAt(2 - smallDiff);
    EXPECT_EQ(2, (int)result.first.size());
    EXPECT_DOUBLE_EQ(0., result.first[0].x());
    EXPECT_DOUBLE_EQ(2., result.first[1].x());
    EXPECT_EQ(2, (int)result.second.size());
    EXPECT_DOUBLE_EQ(2., result.second[0].x());
    EXPECT_DOUBLE_EQ(5., result.second[1].x());
    // split close after inner point
    result = vec.splitAt(2 + smallDiff);
    EXPECT_EQ(2, (int)result.first.size());
    EXPECT_DOUBLE_EQ(0., result.first[0].x());
    EXPECT_DOUBLE_EQ(2., result.first[1].x());
    EXPECT_EQ(2, (int)result.second.size());
    EXPECT_DOUBLE_EQ(2., result.second[0].x());
    EXPECT_DOUBLE_EQ(5., result.second[1].x());

    // catch a bug
    vec.push_back(Position(6, 0));
    vec.push_back(Position(8, 0));
    // split at inner point
    result = vec.splitAt(5);
    EXPECT_EQ(3, (int)result.first.size());
    EXPECT_DOUBLE_EQ(0., result.first[0].x());
    EXPECT_DOUBLE_EQ(2., result.first[1].x());
    EXPECT_DOUBLE_EQ(5., result.first[2].x());
    EXPECT_EQ(3, (int)result.second.size());
    EXPECT_DOUBLE_EQ(5., result.second[0].x());
    EXPECT_DOUBLE_EQ(6., result.second[1].x());
    EXPECT_DOUBLE_EQ(8., result.second[2].x());

    // split short vector
    PositionVector vec2;
    vec2.push_back(Position(0, 0));
    vec2.push_back(Position(2, 0));
    result = vec2.splitAt(1);
    EXPECT_EQ(2, (int)result.first.size());
    EXPECT_DOUBLE_EQ(0., result.first[0].x());
    EXPECT_DOUBLE_EQ(1., result.first[1].x());
    EXPECT_EQ(2, (int)result.second.size());
    EXPECT_DOUBLE_EQ(1., result.second[0].x());
    EXPECT_DOUBLE_EQ(2., result.second[1].x());

    // split very short vector
    PositionVector vec3;
    vec3.push_back(Position(0, 0));
    vec3.push_back(Position(POSITION_EPS, 0));
    // supress expected warning
    MsgHandler::getWarningInstance()->removeRetriever(&OutputDevice::getDevice("stderr"));
    result = vec3.splitAt(smallDiff);
    MsgHandler::getWarningInstance()->addRetriever(&OutputDevice::getDevice("stderr"));

    EXPECT_EQ(2, (int)result.first.size());
    EXPECT_DOUBLE_EQ(0., result.first[0].x());
    EXPECT_DOUBLE_EQ(smallDiff, result.first[1].x());
    EXPECT_EQ(2, (int)result.second.size());
    EXPECT_DOUBLE_EQ(smallDiff, result.second[0].x());
    EXPECT_DOUBLE_EQ(POSITION_EPS, result.second[1].x());
}


/* Test the method 'intersectsAtLengths2D'*/
TEST_F(PositionVectorTest, test_method_intersectsAtLengths2D) {
    PositionVector vec1;
    vec1.push_back(Position(0, 0, 42));
    vec1.push_back(Position(100, 0, 0));

    PositionVector vec2;
    vec2.push_back(Position(0, 0, 0));
    vec2.push_back(Position(3, 1, 0));
    EXPECT_DOUBLE_EQ(0, vec1.intersectsAtLengths2D(vec2)[0]);
}


/* Test the method 'nearest_offset_to_point2D'*/
TEST_F(PositionVectorTest, test_method_nearest_offset_to_point2D) {
    PositionVector vec1;
    vec1.push_back(Position(0, 1, 0));
    vec1.push_back(Position(0, 0, 0));
    vec1.push_back(Position(1, 0, 0));

    EXPECT_DOUBLE_EQ(1, vec1.nearest_offset_to_point2D(Position(-1, -1), false));
    EXPECT_DOUBLE_EQ(1, vec1.nearest_offset_to_point2D(Position(-1, -1), true));
    EXPECT_DOUBLE_EQ(2, vec1.nearest_offset_to_point2D(Position(2, 1), false));
    EXPECT_DOUBLE_EQ(0, vec1.nearest_offset_to_point2D(Position(2, 1), true));
    EXPECT_DOUBLE_EQ(2, vec1.nearest_offset_to_point2D(Position(3, 2), false));
    EXPECT_DOUBLE_EQ(-1, vec1.nearest_offset_to_point2D(Position(3, 2), true));
}


TEST_F(PositionVectorTest, test_method_extrapolate2D) {
    PositionVector vec1;
    vec1.push_back(Position(0, 1, 0));
    vec1.push_back(Position(0, 0, 0));
    vec1.push_back(Position(1, 0, 0));
    vec1.extrapolate2D(1);
    EXPECT_EQ(Position(0, 2, 0), vec1[0]);
    EXPECT_EQ(Position(0, 0, 0), vec1[1]);
    EXPECT_EQ(Position(2, 0, 0), vec1[2]);

    PositionVector vec2;
    vec2.push_back(Position(0, 1, 0));
    vec2.push_back(Position(0, 0, 1));
    vec2.push_back(Position(1, 0, 0));
    vec2.extrapolate2D(1);
//    EXPECT_EQ(Position(0,2,0), vec2[0]);
//    EXPECT_EQ(Position(0,0,0), vec2[1]);
//    EXPECT_EQ(Position(2,0,0), vec2[2]);

    PositionVector vec3;
    vec3.push_back(Position(-.5, 1));
    vec3.push_back(Position(-.5, -.5));
    vec3.push_back(Position(1, -.5));
    vec3.extrapolate2D(.5);
    EXPECT_EQ(Position(-.5, 1.5), vec3[0]);
    EXPECT_EQ(Position(-.5, -.5), vec3[1]);
    EXPECT_EQ(Position(1.5, -.5), vec3[2]);

}


/* Test the method 'move2side'*/
TEST_F(PositionVectorTest, test_method_move2side) {
    PositionVector vec1;
    vec1.push_back(Position(0, 1, 0));
    vec1.push_back(Position(0, 0, 0));
    vec1.push_back(Position(1, 0, 0));
    vec1.move2side(.5);
    EXPECT_EQ(Position(-.5, 1), vec1[0]);
    EXPECT_EQ(Position(-.5, -.5), vec1[1]);
    EXPECT_EQ(Position(1, -.5), vec1[2]);
    vec1.move2side(-1);
    EXPECT_EQ(Position(.5, 1), vec1[0]);
    EXPECT_EQ(Position(.5, .5), vec1[1]);
    EXPECT_EQ(Position(1, .5), vec1[2]);

    // parallel case
    PositionVector vec2;
    vec2.push_back(Position(0, 0, 0));
    vec2.push_back(Position(1, 0, 0));
    vec2.push_back(Position(3, 0, 0));
    vec2.move2side(.5);
    EXPECT_EQ(Position(0, -.5), vec2[0]);
    EXPECT_EQ(Position(1, -.5), vec2[1]);
    EXPECT_EQ(Position(3, -.5), vec2[2]);
    vec2.move2side(-1);
    EXPECT_EQ(Position(0, .5), vec2[0]);
    EXPECT_EQ(Position(1, .5), vec2[1]);
    EXPECT_EQ(Position(3, .5), vec2[2]);

    // counterparallel case
    {
        PositionVector vec3;
        vec3.push_back(Position(0, 0, 0));
        vec3.push_back(Position(3, 0, 0));
        vec3.push_back(Position(1, 0, 0));
        vec3.move2side(.5);
        EXPECT_EQ(Position(0, -.5), vec3[0]);
        EXPECT_EQ(Position(3.5, 0), vec3[1]);
        EXPECT_EQ(Position(1, .5), vec3[2]);
    }
    // bad input: subsequent identical points
    {
        PositionVector vec4;
        vec4.push_back(Position(0, 0, 0));
        vec4.push_back(Position(0, 0, 0));
        vec4.push_back(Position(1, 0, 0));
        vec4.move2side(-2);
        EXPECT_EQ(2, (int)vec4.size());
        EXPECT_EQ(Position(0, 2), vec4[0]);
        EXPECT_EQ(Position(1, 2), vec4[1]);
    }
}

/* Test the method 'transformToVectorCoordinates'*/
TEST_F(PositionVectorTest, test_method_transformToVectorCoordinates) {
    {
        PositionVector vec1;
        vec1.push_back(Position(1, 0));
        vec1.push_back(Position(10, 0));
        vec1.push_back(Position(10, 5));
        vec1.push_back(Position(20, 5));
        Position on(4, 0);
        Position left(4, 1);
        Position right(4, -1);
        Position left2(4, 2);
        Position right2(4, -2);
        Position cornerRight(13, -4);
        Position cornerLeft(7, 9);
        Position before(0, -1);
        Position beyond(24, 9);

        EXPECT_EQ(Position(3, 0),  vec1.transformToVectorCoordinates(on));
        EXPECT_EQ(Position(3, -1),  vec1.transformToVectorCoordinates(left));
        EXPECT_EQ(Position(3, 1),  vec1.transformToVectorCoordinates(right));
        EXPECT_EQ(Position(3, -2),  vec1.transformToVectorCoordinates(left2));
        EXPECT_EQ(Position(3, 2),  vec1.transformToVectorCoordinates(right2));
        EXPECT_EQ(Position(9, 5),  vec1.transformToVectorCoordinates(cornerRight));
        EXPECT_EQ(Position(14, -5),  vec1.transformToVectorCoordinates(cornerLeft));

        EXPECT_EQ(Position::INVALID,  vec1.transformToVectorCoordinates(before));
        EXPECT_EQ(Position::INVALID,  vec1.transformToVectorCoordinates(beyond));
        EXPECT_EQ(Position(-1, 1),  vec1.transformToVectorCoordinates(before, true));
        EXPECT_EQ(Position(28, -4),  vec1.transformToVectorCoordinates(beyond, true));
    }

    {
        PositionVector vec1; // the same tests as before, mirrored on x-axis
        vec1.push_back(Position(1, 0));
        vec1.push_back(Position(10, 0));
        vec1.push_back(Position(10, -5));
        vec1.push_back(Position(20, -5));
        Position on(4, 0);
        Position left(4, -1);
        Position right(4, 1);
        Position left2(4, -2);
        Position right2(4, 2);
        Position cornerRight(13, 4);
        Position cornerLeft(7, -9);
        Position before(0, 1);
        Position beyond(24, -9);

        EXPECT_EQ(Position(3, 0),  vec1.transformToVectorCoordinates(on));
        EXPECT_EQ(Position(3, 1),  vec1.transformToVectorCoordinates(left));
        EXPECT_EQ(Position(3, -1),  vec1.transformToVectorCoordinates(right));
        EXPECT_EQ(Position(3, 2),  vec1.transformToVectorCoordinates(left2));
        EXPECT_EQ(Position(3, -2),  vec1.transformToVectorCoordinates(right2));
        EXPECT_EQ(Position(9, -5),  vec1.transformToVectorCoordinates(cornerRight));
        EXPECT_EQ(Position(14, 5),  vec1.transformToVectorCoordinates(cornerLeft));

        EXPECT_EQ(Position::INVALID,  vec1.transformToVectorCoordinates(before));
        EXPECT_EQ(Position::INVALID,  vec1.transformToVectorCoordinates(beyond));
        EXPECT_EQ(Position(-1, -1),  vec1.transformToVectorCoordinates(before, true));
        EXPECT_EQ(Position(28, 4),  vec1.transformToVectorCoordinates(beyond, true));
    }
}


/* Test the method 'distance'*/
TEST_F(PositionVectorTest, test_method_distance) {
    {
        PositionVector vec1;
        vec1.push_back(Position(1, 0));
        vec1.push_back(Position(10, 0));
        vec1.push_back(Position(10, 5));
        vec1.push_back(Position(20, 5));
        Position on(4, 0);
        Position left(4, 1);
        Position right(4, -1);
        Position left2(4, 2);
        Position right2(4, -2);
        Position cornerRight(13, -4);
        Position cornerLeft(7, 9);
        Position before(-3, -3);
        Position beyond(24, 8);

        EXPECT_EQ(0, vec1.distance2D(on));
        EXPECT_EQ(1, vec1.distance2D(left));
        EXPECT_EQ(1, vec1.distance2D(right));
        EXPECT_EQ(2, vec1.distance2D(left2));
        EXPECT_EQ(2, vec1.distance2D(right2));
        EXPECT_EQ(5, vec1.distance2D(cornerRight));
        EXPECT_EQ(5, vec1.distance2D(cornerLeft));

        EXPECT_EQ(GeomHelper::INVALID_OFFSET,  vec1.distance2D(before, true));
        EXPECT_EQ(GeomHelper::INVALID_OFFSET,  vec1.distance2D(beyond, true));
        EXPECT_EQ(5, vec1.distance2D(before));
        EXPECT_EQ(5, vec1.distance2D(beyond));
    }

    {
        PositionVector vec1; // the same tests as before, mirrored on x-axis
        vec1.push_back(Position(1, 0));
        vec1.push_back(Position(10, 0));
        vec1.push_back(Position(10, -5));
        vec1.push_back(Position(20, -5));
        Position on(4, 0);
        Position left(4, -1);
        Position right(4, 1);
        Position left2(4, -2);
        Position right2(4, 2);
        Position cornerRight(13, 4);
        Position cornerLeft(7, -9);
        Position before(-3, 3);
        Position beyond(24, -8);

        EXPECT_EQ(0, vec1.distance2D(on));
        EXPECT_EQ(1, vec1.distance2D(left));
        EXPECT_EQ(1, vec1.distance2D(right));
        EXPECT_EQ(2, vec1.distance2D(left2));
        EXPECT_EQ(2, vec1.distance2D(right2));
        EXPECT_EQ(5, vec1.distance2D(cornerRight));
        EXPECT_EQ(5, vec1.distance2D(cornerLeft));

        EXPECT_EQ(GeomHelper::INVALID_OFFSET,  vec1.distance2D(before, true));
        EXPECT_EQ(GeomHelper::INVALID_OFFSET,  vec1.distance2D(beyond, true));
        EXPECT_EQ(5, vec1.distance2D(before));
        EXPECT_EQ(5, vec1.distance2D(beyond));
    }
}


/* Test the method 'distance'*/
TEST_F(PositionVectorTest, test_method_distances) {
    {
        PositionVector vec1;
        vec1.push_back(Position(0, 0));
        vec1.push_back(Position(10, 0));

        PositionVector vec2;
        vec2.push_back(Position(1, 0));
        vec2.push_back(Position(5, 2));
        vec2.push_back(Position(10, 0));
        vec2.push_back(Position(14, 3));

        PositionVector vec3;

        std::vector<double> res1;
        res1.push_back(1);
        res1.push_back(0);
        res1.push_back(0);
        res1.push_back(2);
        res1.push_back(0);
        res1.push_back(5);
        EXPECT_DOUBLEVEC_EQUAL(res1,  vec1.distances(vec2));


        std::vector<double> res2;
        //invalid: res1.push_back(1);
        res2.push_back(0);
        res2.push_back(0);
        res2.push_back(2);
        res2.push_back(0);
        //invalid: res2.push_back(5);
        EXPECT_DOUBLEVEC_EQUAL(res2,  vec1.distances(vec2, true));


        std::vector<double> res3;
        res3.push_back(std::numeric_limits<double>::max());
        res3.push_back(std::numeric_limits<double>::max());
        EXPECT_DOUBLEVEC_EQUAL(res3,  vec1.distances(vec3));
    }

}

/* Test the method 'overlapsWith'*/
TEST_F(PositionVectorTest, test_method_overlapsWith) {
    PositionVector vec1;
    vec1.push_back(Position(1, 2));
    vec1.push_back(Position(3, 2));
    vec1.push_back(Position(3, 6));
    vec1.push_back(Position(1, 6));

    PositionVector vec2;
    vec2.push_back(Position(10, 17));
    vec2.push_back(Position(13, 17));
    vec2.push_back(Position(13, 16));
    vec2.push_back(Position(10, 16));

    PositionVector vec3;
    vec3.push_back(Position(-1, -7));
    vec3.push_back(Position(2, -7));
    vec3.push_back(Position(2,  4));
    vec3.push_back(Position(-1,  4));

    PositionVector vec4;
    vec4.push_back(Position(0, 3));
    vec4.push_back(Position(4, 3));
    vec4.push_back(Position(4, 5));
    vec4.push_back(Position(0, 5));

    PositionVector vec5;
    vec5.push_back(Position(4, 2));
    vec5.push_back(Position(5, 2));
    vec5.push_back(Position(5, 7));
    vec5.push_back(Position(4, 7));

    PositionVector vec6;
    vec6.push_back(Position(4, 0));
    vec6.push_back(Position(4, 8));
    vec6.push_back(Position(-4, 8));

    PositionVector empty;

    EXPECT_TRUE(vec1.overlapsWith(vec1));
    EXPECT_FALSE(vec1.overlapsWith(vec2));
    EXPECT_TRUE(vec1.overlapsWith(vec3));
    EXPECT_TRUE(vec1.overlapsWith(vec4));
    EXPECT_FALSE(vec1.overlapsWith(vec5, 0));
    EXPECT_TRUE(vec1.overlapsWith(vec6)); // overlapsWith implicitly closes the shape of vec6
    EXPECT_TRUE(vec6.overlapsWith(vec1)); // overlapsWith implicitly closes the shape of vec6
    // growth is from centroid and thus different from Boundary behavior
    EXPECT_FALSE(vec1.overlapsWith(vec5, 1));
    EXPECT_TRUE(vec1.overlapsWith(vec5, 3));
    EXPECT_TRUE(vec1.overlapsWith(vec5, 6));
    EXPECT_FALSE(vec1.overlapsWith(empty));
}
