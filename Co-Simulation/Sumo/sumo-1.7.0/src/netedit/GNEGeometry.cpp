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
/// @file    GNEGeometry.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2019
///
// File for geometry classes and functions
/****************************************************************************/
#include <netedit/elements/GNEHierarchicalElement.h>
#include <netedit/elements/network/GNEJunction.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEGeometry.h"
#include "GNENet.h"
#include "GNEViewNet.h"

#define CIRCLE_RESOLUTION (double)10 // inverse in degrees

// ===========================================================================
// static member definitions
// ===========================================================================
PositionVector GNEGeometry::myCircleCoords;

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEGeometry::ExtremeGeometry - methods
// ---------------------------------------------------------------------------

GNEGeometry::ExtremeGeometry::ExtremeGeometry() :
    laneStartPosition(-1),
    laneEndPosition(-1),
    viewStartPos(Position::INVALID),
    viewEndPos(Position::INVALID) {
}

// ---------------------------------------------------------------------------
// GNEGeometry::Geometry - methods
// ---------------------------------------------------------------------------

GNEGeometry::Geometry::Geometry() :
    myPosition(Position::INVALID),
    myRotation(0),
    myLane(nullptr),
    myAdditional(nullptr) {
}


GNEGeometry::Geometry::Geometry(const PositionVector& shape, const std::vector<double>& shapeRotations, const std::vector<double>& shapeLengths) :
    myPosition(Position::INVALID),
    myRotation(0),
    myShape(shape),
    myShapeRotations(shapeRotations),
    myShapeLengths(shapeLengths),
    myLane(nullptr),
    myAdditional(nullptr) {
}


void
GNEGeometry::Geometry::updateGeometry(const PositionVector& shape, double startPos, double endPos,
                                      const Position& extraFirstPosition, const Position& extraLastPosition) {
    // first clear geometry
    clearGeometry();
    // set new shape
    myShape = shape;
    // check if we have to split the lane
    if ((startPos != -1) || (endPos != -1)) {
        // check if both start and end position must be swapped
        if ((startPos != -1) && (endPos != -1) && (endPos < startPos)) {
            std::swap(startPos, endPos);
        }
        // check that split positions are correct
        if (startPos <= POSITION_EPS) {
            if (endPos == -1) {
                // leave shape unmodified
            } else if (endPos <= POSITION_EPS) {
                // use only first shape position
                myShape = PositionVector({myShape.front()});
            } else if (endPos < (shape.length() - POSITION_EPS)) {
                // split shape using end position and use left part
                myShape = myShape.splitAt(endPos).first;
            }
        } else if (startPos >= (shape.length() - POSITION_EPS)) {
            // use only last position
            myShape = PositionVector({myShape.back()});
        } else if (endPos == -1) {
            // split shape using start position and use the right part
            myShape = myShape.splitAt(startPos).second;
        } else if (endPos <= POSITION_EPS) {
            // use only first shape position
            myShape = PositionVector({myShape.front()});
        } else if (endPos >= (shape.length() - POSITION_EPS)) {
            // split shape using start position and use the right part
            myShape = myShape.splitAt(startPos).second;
        } else {
            // split shape using start and end position
            myShape = myShape.getSubpart(startPos, endPos);
        }
    }
    // check if we have to add an extra first position
    if (extraFirstPosition != Position::INVALID) {
        myShape.push_front(extraFirstPosition);
    }
    // check if we have to add an extra last position
    if (extraLastPosition != Position::INVALID) {
        myShape.push_back(extraLastPosition);
    }
    // calculate shape rotation and lengths
    calculateShapeRotationsAndLengths();
}


void
GNEGeometry::Geometry::updateGeometry(const Position& position, const double rotation) {
    // first clear geometry
    clearGeometry();
    // set position and rotation
    myPosition = position;
    myRotation = rotation;
}


void
GNEGeometry::Geometry::updateGeometry(const GNELane* lane, const double posOverLane) {
    // first clear geometry
    clearGeometry();
    // get lane length
    const double laneLength = lane->getLaneShape().length();
    // calculate position and rotation
    if (posOverLane < 0) {
        myPosition = lane->getLaneShape().positionAtOffset(0);
        myRotation = lane->getLaneShape().rotationDegreeAtOffset(0);
    } else if (posOverLane > laneLength) {
        myPosition = lane->getLaneShape().positionAtOffset(laneLength);
        myRotation = lane->getLaneShape().rotationDegreeAtOffset(laneLength);
    } else {
        myPosition = lane->getLaneShape().positionAtOffset(posOverLane);
        myRotation = lane->getLaneShape().rotationDegreeAtOffset(posOverLane);
    }
}


void
GNEGeometry::Geometry::updateGeometry(const GNELane* lane) {
    // first clear geometry
    clearGeometry();
    // set lane
    myLane = lane;
}


void
GNEGeometry::Geometry::updateGeometry(const GNEAdditional* additional) {
    // first clear geometry
    clearGeometry();
    // set additional
    myAdditional = additional;
}


void
GNEGeometry::Geometry::updateGeometry(const Geometry& geometry) {
    // first clear geometry
    clearGeometry();
    // set geometry
    myShape = geometry.getShape();
    myShapeRotations = geometry.getShapeRotations();
    myShapeLengths = geometry.getShapeLengths();
}


void
GNEGeometry::Geometry::scaleGeometry(const double scale) {
    // scale shape and lenghts
    myShape.scaleRelative(scale);
    // scale lenghts
    for (auto& shapeLength : myShapeLengths) {
        shapeLength *= scale;
    }
}


const Position&
GNEGeometry::Geometry::getPosition() const {
    return myPosition;
}


double
GNEGeometry::Geometry::getRotation() const {
    return myRotation;
}


const PositionVector&
GNEGeometry::Geometry::getShape() const {
    if (myLane) {
        return myLane->getLaneShape();
    } else if (myAdditional) {
        return myAdditional->getAdditionalGeometry().getShape();
    } else {
        return myShape;
    }
}


const std::vector<double>&
GNEGeometry::Geometry::getShapeRotations() const {
    if (myLane) {
        return myLane->getShapeRotations();
    } else if (myAdditional) {
        return myAdditional->getAdditionalGeometry().getShapeRotations();
    } else {
        return myShapeRotations;
    }
}


const std::vector<double>&
GNEGeometry::Geometry::getShapeLengths() const {
    if (myLane) {
        return myLane->getShapeLengths();
    } else if (myAdditional) {
        return myAdditional->getAdditionalGeometry().getShapeLengths();
    } else {
        return myShapeLengths;
    }
}


void GNEGeometry::Geometry::clearGeometry() {
    // clear single position
    myPosition.set(0, 0, 0);
    myRotation = 0;
    // clear shapes
    myShape.clear();
    myShapeRotations.clear();
    myShapeLengths.clear();
    // clear pointers
    myLane = nullptr;
    myAdditional = nullptr;
}


void
GNEGeometry::Geometry::calculateShapeRotationsAndLengths() {
    // clear rotations and lengths
    myShapeRotations.clear();
    myShapeLengths.clear();
    // Get number of parts of the shape
    int numberOfSegments = (int)myShape.size() - 1;
    // If number of segments is more than 0
    if (numberOfSegments >= 0) {
        // Reserve memory (To improve efficiency)
        myShapeRotations.reserve(numberOfSegments);
        myShapeLengths.reserve(numberOfSegments);
        // Calculate lengths and rotations for every shape
        for (int i = 0; i < numberOfSegments; i++) {
            myShapeRotations.push_back(calculateRotation(myShape[i], myShape[i + 1]));
            myShapeLengths.push_back(calculateLength(myShape[i], myShape[i + 1]));
        }
    }
}

// ---------------------------------------------------------------------------
// GNEGeometry::DottedGeometryColor - methods
// ---------------------------------------------------------------------------

GNEGeometry::DottedGeometryColor::DottedGeometryColor(const GUIVisualizationSettings& settings) :
    mySettings(settings),
    myColorFlag(true) {}


const RGBColor&
GNEGeometry::DottedGeometryColor::getInspectedColor() {
    if (myColorFlag) {
        myColorFlag = false;
        return mySettings.dottedContourSettings.firstInspectedColor;
    } else {
        myColorFlag = true;
        return mySettings.dottedContourSettings.secondInspectedColor;
    }
}


const RGBColor&
GNEGeometry::DottedGeometryColor::getFrontColor() {
    if (myColorFlag) {
        myColorFlag = false;
        return mySettings.dottedContourSettings.firstFrontColor;
    } else {
        myColorFlag = true;
        return mySettings.dottedContourSettings.secondFrontColor;
    }
}


void
GNEGeometry::DottedGeometryColor::changeColor() {
    if (myColorFlag) {
        myColorFlag = false;
    } else {
        myColorFlag = true;
    }
}


void
GNEGeometry::DottedGeometryColor::reset() {
    myColorFlag = true;
}

// ---------------------------------------------------------------------------
// GNEGeometry::DottedGeometry - methods
// ---------------------------------------------------------------------------


GNEGeometry::DottedGeometry::Segment::Segment() :
    offset(-1) {
}


GNEGeometry::DottedGeometry::Segment::Segment(PositionVector newShape) :
    shape(newShape),
    offset(-1) {
}


GNEGeometry::DottedGeometry::DottedGeometry() :
    myWidth(0) {
}


GNEGeometry::DottedGeometry::DottedGeometry(const GUIVisualizationSettings& s, PositionVector shape, const bool closeShape) :
    myWidth(s.dottedContourSettings.segmentWidth) {
    // check if shape has to be closed
    if (closeShape && (shape.size() > 2)) {
        shape.closePolygon();
    }
    if (shape.size() > 1) {
        // get shape
        for (int i = 1; i < (int)shape.size(); i++) {
            myDottedGeometrySegments.push_back(Segment({shape[i - 1], shape[i]}));
        }
        // resample
        for (auto& segment : myDottedGeometrySegments) {
            segment.shape = segment.shape.resample(s.dottedContourSettings.segmentLength, true);
        }
        // calculate shape rotations and lenghts
        calculateShapeRotationsAndLengths();
    }
}


GNEGeometry::DottedGeometry::DottedGeometry(const GUIVisualizationSettings& s,
        const DottedGeometry& topDottedGeometry, const bool drawFirstExtrem,
        const DottedGeometry& botDottedGeometry, const bool drawLastExtrem) :
    myWidth(s.dottedContourSettings.segmentWidth) {
    // check size of both geometries
    if ((topDottedGeometry.myDottedGeometrySegments.size() > 0) &&
            (botDottedGeometry.myDottedGeometrySegments.size() > 0)) {
        // add extremes
        if (drawFirstExtrem &&
                (topDottedGeometry.myDottedGeometrySegments.front().shape.size() > 0) &&
                (botDottedGeometry.myDottedGeometrySegments.front().shape.size() > 0)) {
            // add first extreme
            myDottedGeometrySegments.push_back(Segment({
                topDottedGeometry.myDottedGeometrySegments.front().shape.front(),
                botDottedGeometry.myDottedGeometrySegments.front().shape.front()}));
        }
        if (drawLastExtrem &&
                (topDottedGeometry.myDottedGeometrySegments.back().shape.size() > 0) &&
                (botDottedGeometry.myDottedGeometrySegments.back().shape.size() > 0)) {
            // add last extreme
            myDottedGeometrySegments.push_back(Segment({
                topDottedGeometry.myDottedGeometrySegments.back().shape.back(),
                botDottedGeometry.myDottedGeometrySegments.back().shape.back()}));
            // invert offset of second dotted geometry
            myDottedGeometrySegments.back().offset *= -1;
        }
    }
    // resample
    for (auto& segment : myDottedGeometrySegments) {
        segment.shape = segment.shape.resample(s.dottedContourSettings.segmentLength, true);
    }
    // calculate shape rotations and lenghts
    calculateShapeRotationsAndLengths();
}


void
GNEGeometry::DottedGeometry::updateDottedGeometry(const GUIVisualizationSettings& s, const GNELane* lane) {
    // update settings and width
    myWidth = s.dottedContourSettings.segmentWidth;
    // reset segments
    myDottedGeometrySegments.clear();
    // get shape
    for (int i = 1; i < (int)lane->getLaneShape().size(); i++) {
        myDottedGeometrySegments.push_back(Segment({lane->getLaneShape()[i - 1], lane->getLaneShape()[i]}));
    }
    // resample
    for (auto& segment : myDottedGeometrySegments) {
        segment.shape = segment.shape.resample(s.dottedContourSettings.segmentLength, true);
    }
    // calculate shape rotations and lenghts
    calculateShapeRotationsAndLengths();
}


void
GNEGeometry::DottedGeometry::updateDottedGeometry(const GUIVisualizationSettings& s, PositionVector shape, const bool closeShape) {
    // update settings and width
    myWidth = s.dottedContourSettings.segmentWidth;
    // reset segments
    myDottedGeometrySegments.clear();
    // check if shape has to be closed
    if (closeShape && (shape.size() > 2)) {
        shape.closePolygon();
    }
    if (shape.size() > 1) {
        // get shape
        for (int i = 1; i < (int)shape.size(); i++) {
            myDottedGeometrySegments.push_back(Segment({shape[i - 1], shape[i]}));
        }
        // resample
        for (auto& segment : myDottedGeometrySegments) {
            segment.shape = segment.shape.resample(s.dottedContourSettings.segmentLength, true);
        }
        // calculate shape rotations and lenghts
        calculateShapeRotationsAndLengths();
    }
}


void
GNEGeometry::DottedGeometry::drawInspectedDottedGeometry(DottedGeometryColor& dottedGeometryColor) const {
    // iterate over all segments
    for (auto& segment : myDottedGeometrySegments) {
        // iterate over shape
        for (int i = 0; i < ((int)segment.shape.size() - 1); i++) {
            // set color
            GLHelper::setColor(dottedGeometryColor.getInspectedColor());
            // draw box line
            GLHelper::drawBoxLine(segment.shape[i],
                                  segment.rotations.at(i),
                                  segment.lengths.at(i),
                                  myWidth, myWidth * segment.offset);
        }
    }
}


void
GNEGeometry::DottedGeometry::drawFrontDottedGeometry(DottedGeometryColor& dottedGeometryColor) const {
    // iterate over all segments
    for (auto& segment : myDottedGeometrySegments) {
        // iterate over shape
        for (int i = 0; i < ((int)segment.shape.size() - 1); i++) {
            // set color
            GLHelper::setColor(dottedGeometryColor.getFrontColor());
            // draw box line
            GLHelper::drawBoxLine(segment.shape[i],
                                  segment.rotations.at(i),
                                  segment.lengths.at(i),
                                  myWidth, myWidth * segment.offset);
        }
    }
}


void
GNEGeometry::DottedGeometry::moveShapeToSide(const double value) {
    // move 2 side
    for (auto& segment : myDottedGeometrySegments) {
        segment.shape.move2side(value);
    }
}


double
GNEGeometry::DottedGeometry::getWidth() const {
    return myWidth;
}


void
GNEGeometry::DottedGeometry::setWidth(const double width) {
    myWidth = width;
}


void
GNEGeometry::DottedGeometry::invertOffset() {
    // iterate over all segments
    for (auto& segment : myDottedGeometrySegments) {
        segment.offset *= -1;
    }
}


void
GNEGeometry::DottedGeometry::calculateShapeRotationsAndLengths() {
    // iterate over all segments
    for (auto& segment : myDottedGeometrySegments) {
        // Get number of parts of the shape
        int numberOfSegments = (int)segment.shape.size() - 1;
        // If number of segments is more than 0
        if (numberOfSegments >= 0) {
            // Reserve memory (To improve efficiency)
            segment.rotations.reserve(numberOfSegments);
            segment.lengths.reserve(numberOfSegments);
            // Calculate lengths and rotations for every shape
            for (int i = 0; i < numberOfSegments; i++) {
                segment.rotations.push_back(calculateRotation(segment.shape[i], segment.shape[i + 1]));
                segment.lengths.push_back(calculateLength(segment.shape[i], segment.shape[i + 1]));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GNEGeometry::SegmentGeometry::Segment - methods
// ---------------------------------------------------------------------------

GNEGeometry::SegmentGeometry::Segment::Segment(const GNELane* lane, const bool valid) :
    myLane(lane),
    myNextLane(nullptr),
    myValid(valid),
    myUseLaneShape(true) {
}


GNEGeometry::SegmentGeometry::Segment::Segment(const GNELane* lane, const Geometry& geometry, const bool valid) :
    myLane(lane),
    myNextLane(nullptr),
    myValid(valid),
    myUseLaneShape(false),
    mySegmentGeometry(geometry) {
}


GNEGeometry::SegmentGeometry::Segment::Segment(const GNELane* lane, const GNELane* nextLane, const bool valid) :
    myLane(lane),
    myNextLane(nextLane),
    myValid(valid),
    myUseLaneShape(false) {
}


void
GNEGeometry::SegmentGeometry::Segment::update(const GNEGeometry::Geometry& geometry) {
    // disable use lane shape
    myUseLaneShape = false;
    // update geometry
    mySegmentGeometry.updateGeometry(geometry);
}


void
GNEGeometry::SegmentGeometry::Segment::update(const GNELane* lane) {
    // enable use lane shape
    myUseLaneShape = true;
    // update geometry
    mySegmentGeometry.updateGeometry(lane);
}


const PositionVector&
GNEGeometry::SegmentGeometry::Segment::getShape() const {
    if (myNextLane) {
        return myLane->getLane2laneConnections().getLane2laneGeometry(myNextLane).getShape();
    } else if (myUseLaneShape) {
        return myLane->getLaneShape();
    } else {
        return mySegmentGeometry.getShape();
    }
}


const std::vector<double>&
GNEGeometry::SegmentGeometry::Segment::getShapeRotations() const {
    if (myNextLane) {
        return myLane->getLane2laneConnections().getLane2laneGeometry(myNextLane).getShapeRotations();
    } else if (myUseLaneShape) {
        return myLane->getShapeRotations();
    } else {
        return mySegmentGeometry.getShapeRotations();
    }
}


const std::vector<double>&
GNEGeometry::SegmentGeometry::Segment::getShapeLengths() const {
    if (myNextLane) {
        return myLane->getLane2laneConnections().getLane2laneGeometry(myNextLane).getShapeLengths();
    } else if (myUseLaneShape) {
        return myLane->getShapeLengths();
    } else {
        return mySegmentGeometry.getShapeLengths();
    }
}


const GNELane*
GNEGeometry::SegmentGeometry::Segment::getLane() const {
    return myLane;
}


const GNEJunction*
GNEGeometry::SegmentGeometry::Segment::getJunction() const {
    return myLane->getParentEdge()->getParentJunctions().back();
}


bool
GNEGeometry::SegmentGeometry::Segment::getValid() const {
    return myValid;
}


bool
GNEGeometry::SegmentGeometry::Segment::isLaneSegment() const {
    return (myNextLane == nullptr);
}

// ---------------------------------------------------------------------------
// GNEGeometry::SegmentGeometry::SegmentToUpdate - methods
// ---------------------------------------------------------------------------

GNEGeometry::SegmentGeometry::SegmentToUpdate::SegmentToUpdate(const int segmentIndex, const GNELane* lane, const GNELane* nextLane) :
    mySegmentIndex(segmentIndex),
    myLane(lane),
    myNextLane(nextLane) {
}


int
GNEGeometry::SegmentGeometry::SegmentToUpdate::getSegmentIndex() const {
    return mySegmentIndex;
}


const GNELane*
GNEGeometry::SegmentGeometry::SegmentToUpdate::getLane() const {
    return myLane;
}


const GNELane*
GNEGeometry::SegmentGeometry::SegmentToUpdate::getNextLane() const {
    return myNextLane;
}

// ---------------------------------------------------------------------------
// GNEGeometry::Geometry - methods
// ---------------------------------------------------------------------------

GNEGeometry::SegmentGeometry::SegmentGeometry() {}


void
GNEGeometry::SegmentGeometry::insertLaneSegment(const GNELane* lane, const bool valid) {
    // add segment in myShapeSegments
    myShapeSegments.push_back(Segment(lane, valid));
}


void
GNEGeometry::SegmentGeometry::insertCustomSegment(const GNELane* lane, const Geometry& geometry, const bool valid) {
    // add segment in myShapeSegments
    myShapeSegments.push_back(Segment(lane, geometry, valid));
}


void
GNEGeometry::SegmentGeometry::insertLane2LaneSegment(const GNELane* currentLane, const GNELane* nextLane, const bool valid) {
    // add segment in myShapeSegments
    myShapeSegments.push_back(Segment(currentLane, nextLane, valid));
}


void
GNEGeometry::SegmentGeometry::updateCustomSegment(const int segmentIndex, const Geometry& geometry) {
    myShapeSegments.at(segmentIndex).update(geometry);
}


void
GNEGeometry::SegmentGeometry::updateLane2LaneSegment(const int segmentIndex, const GNELane* lane, const GNELane* nextLane) {
    myShapeSegments.at(segmentIndex + 1).update(lane->getLane2laneConnections().getLane2laneGeometry(nextLane));
}


void
GNEGeometry::SegmentGeometry::clearSegmentGeometry() {
    // clear segments
    myShapeSegments.clear();
}


const Position&
GNEGeometry::SegmentGeometry::getFirstPosition() const {
    if ((myShapeSegments.size() > 0) && (myShapeSegments.front().getShape().size() > 0)) {
        return myShapeSegments.front().getShape().front();
    } else {
        return Position::INVALID;
    }
}

const Position&
GNEGeometry::SegmentGeometry::getLastPosition() const {
    if ((myShapeSegments.size() > 0) && (myShapeSegments.back().getShape().size() > 0)) {
        return myShapeSegments.back().getShape().back();
    } else {
        return Position::INVALID;
    }
}


double
GNEGeometry::SegmentGeometry::getFirstRotation() const {
    if ((myShapeSegments.size() > 0) && (myShapeSegments.front().getShapeRotations().size() > 0)) {
        return myShapeSegments.front().getShapeRotations().front();
    } else {
        return 0;
    }
}


Boundary
GNEGeometry::SegmentGeometry::getBoxBoundary() const {
    Boundary b;
    for (const auto& i : myShapeSegments) {
        b.add(i.getShape().getBoxBoundary());
    }
    return b;
}


std::vector<GNEGeometry::SegmentGeometry::Segment>::const_iterator
GNEGeometry::SegmentGeometry::begin() const {
    return myShapeSegments.cbegin();
}


std::vector<GNEGeometry::SegmentGeometry::Segment>::const_iterator
GNEGeometry::SegmentGeometry::end() const {
    return myShapeSegments.cend();
}


const GNEGeometry::SegmentGeometry::Segment&
GNEGeometry::SegmentGeometry::front() const {
    return myShapeSegments.front();
}


const GNEGeometry::SegmentGeometry::Segment&
GNEGeometry::SegmentGeometry::back() const {
    return myShapeSegments.back();
}


int
GNEGeometry::SegmentGeometry::size() const {
    return (int)myShapeSegments.size();
}

// ---------------------------------------------------------------------------
// GNEGeometry::Lane2laneConnection - methods
// ---------------------------------------------------------------------------

GNEGeometry::Lane2laneConnection::Lane2laneConnection(const GNELane* fromLane) :
    myFromLane(fromLane) {
}


void
GNEGeometry::Lane2laneConnection::updateLane2laneConnection() {
    // declare numPoints
    const int numPoints = 5;
    const int maximumLanes = 10;
    // clear connectionsMap
    myConnectionsMap.clear();
    // iterate over outgoingEdge's lanes
    for (const auto& outgoingEdge : myFromLane->getParentEdge()->getParentJunctions().back()->getGNEOutgoingEdges()) {
        for (const auto& outgoingLane : outgoingEdge->getLanes()) {
            // get NBEdges from and to
            const NBEdge* NBEdgeFrom = myFromLane->getParentEdge()->getNBEdge();
            const NBEdge* NBEdgeTo = outgoingLane->getParentEdge()->getNBEdge();
            // declare shape
            PositionVector shape;
            // only create smooth shapes if Edge From has as maximum 10 lanes
            if ((NBEdgeFrom->getNumLanes() <= maximumLanes) && (NBEdgeFrom->getToNode()->getShape().area() > 4)) {
                // calculate smoot shape
                shape = NBEdgeFrom->getToNode()->computeSmoothShape(
                            NBEdgeFrom->getLaneShape(myFromLane->getIndex()),
                            NBEdgeTo->getLaneShape(outgoingLane->getIndex()),
                            numPoints, NBEdgeFrom->getTurnDestination() == NBEdgeTo,
                            (double) numPoints * (double) NBEdgeFrom->getNumLanes(),
                            (double) numPoints * (double) NBEdgeTo->getNumLanes());
            } else {
                // create a shape using lane shape extremes
                shape = {myFromLane->getLaneShape().back(), outgoingLane->getLaneShape().front()};
            }
            // update connection map
            myConnectionsMap[outgoingLane].first.updateGeometry(shape);
            if (myFromLane->getNet()->getViewNet()) {
                myConnectionsMap[outgoingLane].second.updateDottedGeometry(myFromLane->getNet()->getViewNet()->getVisualisationSettings(), shape, false);
            }
        }
    }
}


bool
GNEGeometry::Lane2laneConnection::exist(const GNELane* toLane) const {
    return (myConnectionsMap.count(toLane) > 0);
}


const GNEGeometry::Geometry&
GNEGeometry::Lane2laneConnection::getLane2laneGeometry(const GNELane* toLane) const {
    return myConnectionsMap.at(toLane).first;
}


const GNEGeometry::DottedGeometry&
GNEGeometry::Lane2laneConnection::getLane2laneDottedGeometry(const GNELane* toLane) const {
    return myConnectionsMap.at(toLane).second;
}


GNEGeometry::Lane2laneConnection::Lane2laneConnection() :
    myFromLane(nullptr) {
}

// ---------------------------------------------------------------------------
// GNEGeometry::HierarchicalConnections - methods
// ---------------------------------------------------------------------------

GNEGeometry::HierarchicalConnections::ConnectionGeometry::ConnectionGeometry(GNELane* lane) :
    myLane(lane),
    myRotation(0) {
    // set position and length depending of shape's lengt
    if (lane->getLaneShape().length() - 6 > 0) {
        myPosition = lane->getLaneShape().positionAtOffset(lane->getLaneShape().length() - 6);
        myRotation = lane->getLaneShape().rotationDegreeAtOffset(lane->getLaneShape().length() - 6);
    } else {
        myPosition = lane->getLaneShape().positionAtOffset(lane->getLaneShape().length());
        myRotation = lane->getLaneShape().rotationDegreeAtOffset(lane->getLaneShape().length());
    }
}


const GNELane*
GNEGeometry::HierarchicalConnections::ConnectionGeometry::getLane() const {
    return myLane;
}


const Position&
GNEGeometry::HierarchicalConnections::ConnectionGeometry::getPosition() const {
    return myPosition;
}


double
GNEGeometry::HierarchicalConnections::ConnectionGeometry::getRotation() const {
    return myRotation;
}


GNEGeometry::HierarchicalConnections::ConnectionGeometry::ConnectionGeometry() :
    myLane(nullptr),
    myRotation(0) {
}


GNEGeometry::HierarchicalConnections::HierarchicalConnections(GNEHierarchicalElement* hierarchicalElement) :
    myHierarchicalElement(hierarchicalElement) {}


void
GNEGeometry::HierarchicalConnections::update() {
    // first clear containers
    connectionsGeometries.clear();
    symbolsPositionAndRotation.clear();
    // calculate position and rotation of every simbol for every edge
    for (const auto& edge : myHierarchicalElement->getChildEdges()) {
        for (const auto& lane : edge->getLanes()) {
            symbolsPositionAndRotation.push_back(ConnectionGeometry(lane));
        }
    }
    // calculate position and rotation of every symbol for every lane
    for (const auto& lane : myHierarchicalElement->getChildLanes()) {
        symbolsPositionAndRotation.push_back(ConnectionGeometry(lane));
    }
    // calculate position for every child additional
    for (const auto& additional : myHierarchicalElement->getChildAdditionals()) {
        // check that additional position is different of parent position
        if (additional->getPositionInView() != myHierarchicalElement->getPositionInView()) {
            // create connection shape
            std::vector<Position> connectionShape;
            const double A = std::abs(additional->getPositionInView().x() - myHierarchicalElement->getPositionInView().x());
            const double B = std::abs(additional->getPositionInView().y() - myHierarchicalElement->getPositionInView().y());
            // Set positions of connection's vertex. Connection is build from Entry to E3
            connectionShape.push_back(additional->getPositionInView());
            if (myHierarchicalElement->getPositionInView().x() > additional->getPositionInView().x()) {
                if (myHierarchicalElement->getPositionInView().y() > additional->getPositionInView().y()) {
                    connectionShape.push_back(Position(additional->getPositionInView().x() + A, additional->getPositionInView().y()));
                } else {
                    connectionShape.push_back(Position(additional->getPositionInView().x(), additional->getPositionInView().y() - B));
                }
            } else {
                if (myHierarchicalElement->getPositionInView().y() > additional->getPositionInView().y()) {
                    connectionShape.push_back(Position(additional->getPositionInView().x(), additional->getPositionInView().y() + B));
                } else {
                    connectionShape.push_back(Position(additional->getPositionInView().x() - A, additional->getPositionInView().y()));
                }
            }
            connectionShape.push_back(myHierarchicalElement->getPositionInView());
            // declare Geometry
            GNEGeometry::Geometry geometry;
            // update geometry with connectino shape
            geometry.updateGeometry(connectionShape);
            // add geometry in connectionsGeometry
            connectionsGeometries.push_back(geometry);
        }
    }
    // calculate geometry for connections between parent and children
    for (const auto& symbol : symbolsPositionAndRotation) {
        // create connection shape
        std::vector<Position> connectionShape;
        const double A = std::abs(symbol.getPosition().x() - myHierarchicalElement->getPositionInView().x());
        const double B = std::abs(symbol.getPosition().y() - myHierarchicalElement->getPositionInView().y());
        // Set positions of connection's vertex. Connection is build from Entry to E3
        connectionShape.push_back(symbol.getPosition());
        if (myHierarchicalElement->getPositionInView().x() > symbol.getPosition().x()) {
            if (myHierarchicalElement->getPositionInView().y() > symbol.getPosition().y()) {
                connectionShape.push_back(Position(symbol.getPosition().x() + A, symbol.getPosition().y()));
            } else {
                connectionShape.push_back(Position(symbol.getPosition().x(), symbol.getPosition().y() - B));
            }
        } else {
            if (myHierarchicalElement->getPositionInView().y() > symbol.getPosition().y()) {
                connectionShape.push_back(Position(symbol.getPosition().x(), symbol.getPosition().y() + B));
            } else {
                connectionShape.push_back(Position(symbol.getPosition().x() - A, symbol.getPosition().y()));
            }
        }
        connectionShape.push_back(myHierarchicalElement->getPositionInView());
        // declare Geometry
        GNEGeometry::Geometry geometry;
        // update geometry with connectino shape
        geometry.updateGeometry(connectionShape);
        // add geometry in connectionsGeometry
        connectionsGeometries.push_back(geometry);
    }
}


void
GNEGeometry::HierarchicalConnections::drawConnection(const GUIVisualizationSettings& s, const GNEAttributeCarrier* AC, const double exaggeration) const {
    // Iterate over myConnectionPositions
    for (const auto& connectionGeometry : connectionsGeometries) {
        // Add a draw matrix
        glPushMatrix();
        // Set color
        if (AC->isAttributeCarrierSelected()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor.changedBrightness(-32));
        } else {
            GLHelper::setColor(s.colorSettings.childConnections);
        }
        // Draw box lines
        GLHelper::drawBoxLines(connectionGeometry.getShape(), connectionGeometry.getShapeRotations(), connectionGeometry.getShapeLengths(), exaggeration * 0.1);
        // Pop draw matrix
        glPopMatrix();
    }
}


void
GNEGeometry::HierarchicalConnections::drawDottedConnection(const bool inspect, const GUIVisualizationSettings& s, const double exaggeration) const {
    // Iterate over myConnectionPositions
    for (const auto& connectionGeometry : connectionsGeometries) {
        // calculate dotted geometry
        GNEGeometry::DottedGeometry dottedGeometry(s, connectionGeometry.getShape(), false);
        // Add a draw matrix
        glPushMatrix();
        // traslate back
        if (inspect) {
            glTranslated(0, 0, (-1 * GLO_DOTTEDCONTOUR_INSPECTED) - 0.01);
        } else {
            glTranslated(0, 0, (-1 * GLO_DOTTEDCONTOUR_FRONT) - 0.01);
        }
        // change default width
        dottedGeometry.setWidth(0.1);
        // use drawDottedContourLane to draw it
        GNEGeometry::drawDottedContourLane(inspect, s, dottedGeometry, exaggeration * 0.1, false, false);
        // Pop draw matrix
        glPopMatrix();
    }
}

// ---------------------------------------------------------------------------
// GNEGeometry - methods
// ---------------------------------------------------------------------------

double
GNEGeometry::calculateRotation(const Position& first, const Position& second) {
    // return rotation (angle) of the vector constructed by points first and second
    return ((double)atan2((second.x() - first.x()), (first.y() - second.y())) * (double) 180.0 / (double)M_PI);
}


double
GNEGeometry::calculateLength(const Position& first, const Position& second) {
    // return 2D distance between two points
    return first.distanceTo2D(second);
}


void
GNEGeometry::adjustStartPosGeometricPath(double& startPos, const GNELane* startLane, double& endPos, const GNELane* endLane) {
    // adjust both, if start and end lane are the same
    if (startLane && endLane && (startLane == endLane) && (startPos != -1) && (endPos != -1)) {
        if (startPos >= endPos) {
            endPos = (startPos + POSITION_EPS);
        }
    }
    // adjust startPos
    if ((startPos != -1) && startLane) {
        if (startPos < POSITION_EPS) {
            startPos = POSITION_EPS;
        }
        if (startPos > (startLane->getLaneShape().length() - POSITION_EPS)) {
            startPos = (startLane->getLaneShape().length() - POSITION_EPS);
        }
    }
    // adjust endPos
    if ((endPos != -1) && endLane) {
        if (endPos < POSITION_EPS) {
            endPos = POSITION_EPS;
        }
        if (endPos > (endLane->getLaneShape().length() - POSITION_EPS)) {
            endPos = (endLane->getLaneShape().length() - POSITION_EPS);
        }
    }
}


void
GNEGeometry::calculateLaneGeometricPath(GNEGeometry::SegmentGeometry& segmentGeometry, const std::vector<GNEPathElements::PathElement>& path, GNEGeometry::ExtremeGeometry& extremeGeometry) {
    // clear geometry
    segmentGeometry.clearSegmentGeometry();
    // first check that there is parent edges
    if (path.size() > 0) {
        // calculate depending if both from and to edges are the same
        if (path.size() == 1) {
            // filter start and end pos
            adjustStartPosGeometricPath(extremeGeometry.laneStartPosition, path.front().getLane(), extremeGeometry.laneEndPosition, path.front().getLane());
            // check if we have to define a new custom Segment, or we can use the commonLane shape
            if ((extremeGeometry.laneStartPosition != -1) || (extremeGeometry.laneEndPosition != -1) || (extremeGeometry.viewStartPos != Position::INVALID) || (extremeGeometry.viewEndPos != Position::INVALID)) {
                // declare a lane to be trimmed
                Geometry trimmedLane;
                // update geometry
                trimmedLane.updateGeometry(path.front().getLane()->getLaneShape(), extremeGeometry.laneStartPosition, extremeGeometry.laneEndPosition, extremeGeometry.viewStartPos, extremeGeometry.viewEndPos);
                // add sublane geometry
                segmentGeometry.insertCustomSegment(path.front().getLane(), trimmedLane, true);
            } else {
                // add entire lane geometry geometry
                segmentGeometry.insertLaneSegment(path.front().getLane(), true);
            }
        } else {
            // iterate over path
            for (int i = 0; i < (int)path.size(); i++) {
                // get lane (only for code readability)
                const GNELane* lane = path.at(i).getLane();
                // first check that lane shape isn't empty
                if (lane->getLaneShape().size() > 0) {
                    // check if first or last lane must be splitted
                    if ((path.at(i).getLane() == path.front().getLane()) && (extremeGeometry.laneStartPosition != -1)) {
                        // filter start position
                        adjustStartPosGeometricPath(extremeGeometry.laneStartPosition, path.at(i).getLane(), extremeGeometry.laneEndPosition, nullptr);
                        // declare a lane to be trimmed
                        Geometry frontTrimmedLane;
                        // update geometry
                        frontTrimmedLane.updateGeometry(path.at(i).getLane()->getLaneShape(), extremeGeometry.laneStartPosition, -1, extremeGeometry.viewStartPos, Position::INVALID);
                        // add sublane geometry
                        segmentGeometry.insertCustomSegment(lane, frontTrimmedLane, true);
                    } else if ((lane == path.back().getLane()) && (extremeGeometry.laneEndPosition != -1)) {
                        // filter end position
                        adjustStartPosGeometricPath(extremeGeometry.laneStartPosition, nullptr, extremeGeometry.laneEndPosition, lane);
                        // declare a lane to be trimmed
                        Geometry backTrimmedLane;
                        // update geometry
                        backTrimmedLane.updateGeometry(path.at(i).getLane()->getLaneShape(), -1, extremeGeometry.laneEndPosition, Position::INVALID, extremeGeometry.viewEndPos);
                        // add sublane geometry
                        segmentGeometry.insertCustomSegment(lane, backTrimmedLane, true);
                    } else {
                        // add entire lane geometry
                        segmentGeometry.insertLaneSegment(path.at(i).getLane(), true);
                    }
                }
                // now continue with connection
                if ((i + 1) < (int)path.size()) {
                    // obtain next lane
                    const GNELane* nextLane = path.at(i + 1).getLane();
                    // check that next lane exist
                    if (lane->getLane2laneConnections().exist(nextLane)) {
                        // add lane2laneConnection segment geometry
                        segmentGeometry.insertLane2LaneSegment(lane, nextLane, true);
                    }
                }
            }
        }
    }
}


void
GNEGeometry::updateGeometricPath(GNEGeometry::SegmentGeometry& segmentGeometry, const GNELane* lane, GNEGeometry::ExtremeGeometry& extremeGeometry) {
    // calculate depending if both from and to edges are the same
    if ((segmentGeometry.size() == 1) && (segmentGeometry.front().getLane() == lane)) {
        // filter start and end pos
        adjustStartPosGeometricPath(extremeGeometry.laneStartPosition, segmentGeometry.front().getLane(), extremeGeometry.laneEndPosition, segmentGeometry.front().getLane());
        // check if we have to define a new custom Segment, or we can use the commonLane shape
        if ((extremeGeometry.laneStartPosition != -1) || (extremeGeometry.laneEndPosition != -1) || (extremeGeometry.viewStartPos != Position::INVALID) || (extremeGeometry.viewEndPos != Position::INVALID)) {
            // declare a lane to be trimmed
            Geometry trimmedLane;
            // update geometry
            trimmedLane.updateGeometry(segmentGeometry.front().getLane()->getLaneShape(), extremeGeometry.laneStartPosition, extremeGeometry.laneEndPosition, extremeGeometry.viewStartPos, extremeGeometry.viewEndPos);
            // add sublane geometry
            segmentGeometry.updateCustomSegment(0, trimmedLane);
        }
    } else {
        // declare a vector to save segments to update
        std::vector<GNEGeometry::SegmentGeometry::SegmentToUpdate> segmentsToUpdate;
        // iterate over all segments
        for (auto segment = segmentGeometry.begin(); segment != segmentGeometry.end(); segment++) {
            if (segment->getLane() == lane) {
                // obtain segment index
                const int index = (int)(segment - segmentGeometry.begin());
                // add SegmentToUpdate in vector
                segmentsToUpdate.push_back(GNEGeometry::SegmentGeometry::SegmentToUpdate(index, segment->getLane(), nullptr));
                // check if we have to add the next segment (it correspond to a lane2lane
                if (((segment + 1) != segmentGeometry.end()) && (segment + 1)->getJunction()) {
                    segmentsToUpdate.push_back(GNEGeometry::SegmentGeometry::SegmentToUpdate(index, segment->getLane(), (segment + 1)->getLane()));
                }
            }
        }
        // iterate over segments to update
        for (const auto& segmentToUpdate : segmentsToUpdate) {
            // first check that lane shape isn't empty
            if (segmentToUpdate.getLane()->getLaneShape().size() > 0) {
                // check if first or last lane must be splitted
                if ((segmentToUpdate.getSegmentIndex() == 0) && (extremeGeometry.laneStartPosition != -1)) {
                    // filter start position
                    adjustStartPosGeometricPath(extremeGeometry.laneStartPosition, segmentToUpdate.getLane(), extremeGeometry.laneEndPosition, nullptr);
                    // declare a lane to be trimmed
                    Geometry frontTrimmedLane;
                    // update geometry
                    frontTrimmedLane.updateGeometry(segmentToUpdate.getLane()->getLaneShape(), extremeGeometry.laneStartPosition, -1, extremeGeometry.viewStartPos, Position::INVALID);
                    // update segment
                    segmentGeometry.updateCustomSegment(segmentToUpdate.getSegmentIndex(), frontTrimmedLane);
                } else if ((segmentToUpdate.getSegmentIndex() == (segmentGeometry.size() - 1)) && (extremeGeometry.laneEndPosition != -1)) {
                    // filter end position
                    adjustStartPosGeometricPath(extremeGeometry.laneStartPosition, nullptr, extremeGeometry.laneEndPosition, segmentToUpdate.getLane());
                    // declare a lane to be trimmed
                    Geometry backTrimmedLane;
                    // update geometry
                    backTrimmedLane.updateGeometry(segmentToUpdate.getLane()->getLaneShape(), -1, extremeGeometry.laneEndPosition, Position::INVALID, extremeGeometry.viewEndPos);
                    // update segment
                    segmentGeometry.updateCustomSegment(segmentToUpdate.getSegmentIndex(), backTrimmedLane);
                }
            }
            // check that next lane exist
            if (segmentToUpdate.getLane()->getLane2laneConnections().exist(segmentToUpdate.getNextLane())) {
                // update lane2laneConnection shape
                segmentGeometry.updateLane2LaneSegment(segmentToUpdate.getSegmentIndex(), segmentToUpdate.getLane(), segmentToUpdate.getNextLane());
            }
        }
    }
}


void
GNEGeometry::drawGeometry(const GNEViewNet* viewNet, const Geometry& geometry, const double width) {
    // first check if we're in draw for selecting cliking mode
    if (viewNet->getVisualisationSettings().drawForPositionSelection) {
        // obtain mouse Position
        const Position mousePosition = viewNet->getPositionInformation();
        // obtain position over lane relative to mouse position
        const Position posOverLane = geometry.getShape().positionAtOffset2D(geometry.getShape().nearest_offset_to_point2D(mousePosition));
        // if mouse is over segment
        if (posOverLane.distanceSquaredTo2D(mousePosition) <= (width * width)) {
            // push matrix
            glPushMatrix();
            // translate to position over lane
            glTranslated(posOverLane.x(), posOverLane.y(), 0);
            // Draw circle
            GLHelper::drawFilledCircle(width, viewNet->getVisualisationSettings().getCircleResolution());
            // pop draw matrix
            glPopMatrix();
        }
    } else {
        GLHelper::drawBoxLines(geometry.getShape(), geometry.getShapeRotations(), geometry.getShapeLengths(), width);
    }
}


void
GNEGeometry::drawGeometryPoints(const GUIVisualizationSettings& s, const GNEViewNet* viewNet, const PositionVector& shape,
                                const RGBColor& geometryPointColor, const RGBColor& textColor, const double radius, const double exaggeration) {
    // get mouse position
    const Position mousePosition = viewNet->getPositionInformation();
    // get exaggeratedRadio
    const double exaggeratedRadio = (radius * exaggeration);
    // get radius squared
    const double exaggeratedRadioSquared = (exaggeratedRadio * exaggeratedRadio);
    // iterate over shape
    for (const auto& vertex : shape) {
        // if drawForPositionSelection is enabled, check distance between mouse and vertex
        if (!s.drawForPositionSelection || (mousePosition.distanceSquaredTo2D(vertex) <= exaggeratedRadioSquared)) {
            // push geometry point matrix
            glPushMatrix();
            // move to vertex
            glTranslated(vertex.x(), vertex.y(), 0.2);
            // set color
            GLHelper::setColor(geometryPointColor);
            // draw circle
            GLHelper::drawFilledCircle(exaggeratedRadio, s.getCircleResolution());
            // pop geometry point matrix
            glPopMatrix();
            // draw elevation or special symbols (Start, End and Block)
            if (!s.drawForRectangleSelection && !s.drawForPositionSelection) {
                // get draw detail
                const bool drawDetail = s.drawDetail(s.detailSettings.geometryPointsText, exaggeration);
                // draw text
                if (viewNet->getNetworkViewOptions().editingElevation()) {
                    // Push Z matrix
                    glPushMatrix();
                    // draw Z (elevation)
                    GLHelper::drawText(toString(vertex.z()), vertex, 0.3, 0.7, textColor);
                    // pop Z matrix
                    glPopMatrix();
                } else if ((vertex == shape.front()) && drawDetail) {
                    // push "S" matrix
                    glPushMatrix();
                    // draw a "s" over first point
                    GLHelper::drawText("S", vertex, 0.3, 2 * exaggeratedRadio, textColor);
                    // pop "S" matrix
                    glPopMatrix();
                } else if ((vertex == shape.back()) && (shape.isClosed() == false) && drawDetail) {
                    // push "E" matrix
                    glPushMatrix();
                    // draw a "e" over last point if polygon isn't closed
                    GLHelper::drawText("E", vertex, 0.3, 2 * exaggeratedRadio, textColor);
                    // pop "E" matrix
                    glPopMatrix();
                }
            }
        }
    }
}


void
GNEGeometry::drawMovingHint(const GUIVisualizationSettings& s, const GNEViewNet* viewNet, const PositionVector& shape,
                            const RGBColor& hintColor, const double radius, const double exaggeration) {
    // first NetworkEditMode
    if (viewNet->getEditModes().networkEditMode == NetworkEditMode::NETWORK_MOVE) {
        // get mouse position
        const Position mousePosition = viewNet->getPositionInformation();
        // get exaggeratedRadio
        const double exaggeratedRadio = (radius * exaggeration);
        // obtain distance to shape
        const double distanceToShape = shape.distance2D(mousePosition);
        // obtain squared radius
        const double squaredRadius = (radius * radius * exaggeration);
        // declare index
        int index = -1;
        // iterate over shape
        for (int i = 0; i < (int)shape.size(); i++) {
            // check distance
            if (shape[i].distanceSquaredTo2D(mousePosition) <= squaredRadius) {
                index = i;
            }
        }
        // continue depending of distance to shape
        if ((distanceToShape < exaggeratedRadio) && (index == -1)) {
            // obtain position over lane
            const Position positionOverLane = shape.positionAtOffset2D(shape.nearest_offset_to_point2D(mousePosition));
            // calculate hintPos
            const Position hintPos = shape.size() > 1 ? positionOverLane : shape[0];
            // push hintPos matrix
            glPushMatrix();
            // translate to hintPos
            glTranslated(hintPos.x(), hintPos.y(), 0.2);
            // set color
            GLHelper::setColor(hintColor);
            // draw filled circle
            GLHelper:: drawFilledCircle(exaggeratedRadio, s.getCircleResolution());
            // pop hintPos matrix
            glPopMatrix();
        }
    }
}


void
GNEGeometry::drawLaneGeometry(const GNEViewNet* viewNet, const PositionVector& shape, const std::vector<double>& rotations,
                              const std::vector<double>& lengths, const std::vector<RGBColor>& colors, double width) {
    // first check if we're in draw for selecting cliking mode
    if (viewNet->getVisualisationSettings().drawForPositionSelection) {
        // obtain mouse Position
        const Position mousePosition = viewNet->getPositionInformation();
        // obtain position over lane relative to mouse position
        const Position posOverLane = shape.positionAtOffset2D(shape.nearest_offset_to_point2D(mousePosition));
        // if mouse is over segment
        if (posOverLane.distanceSquaredTo2D(mousePosition) <= (width * width)) {
            // push matrix
            glPushMatrix();
            // translate to position over lane
            glTranslated(posOverLane.x(), posOverLane.y(), 0);
            // Draw circle
            GLHelper::drawFilledCircle(width, viewNet->getVisualisationSettings().getCircleResolution());
            // pop draw matrix
            glPopMatrix();
        }
    } else if (colors.size() > 0) {
        // draw box lines with own colors
        GLHelper::drawBoxLines(shape, rotations, lengths, colors, width);
    } else {
        // draw box lines with current color
        GLHelper::drawBoxLines(shape, rotations, lengths, width);
    }
}


void
GNEGeometry::drawSegmentGeometry(const GNEViewNet* viewNet, const SegmentGeometry::Segment& segment, const double width) {
    // first check if we're in draw for selecting mode
    if (viewNet->getVisualisationSettings().drawForPositionSelection) {
        // obtain mouse Position
        const Position mousePosition = viewNet->getPositionInformation();
        // obtain position over lane relative to mouse position
        const Position posOverLane = segment.getShape().positionAtOffset2D(segment.getShape().nearest_offset_to_point2D(mousePosition));
        // if mouse is over segment
        if (posOverLane.distanceSquaredTo2D(mousePosition) <= (width * width)) {
            // push matrix
            glPushMatrix();
            // translate to position over lane
            glTranslated(posOverLane.x(), posOverLane.y(), 0);
            // Draw circle
            GLHelper::drawFilledCircle(width, viewNet->getVisualisationSettings().getCircleResolution());
            // pop draw matrix
            glPopMatrix();
        }
    } else {
        // draw a boxline as usual
        GLHelper::drawBoxLines(segment.getShape(), segment.getShapeRotations(), segment.getShapeLengths(), width);
    }
}


void
GNEGeometry::drawDottedContourLane(const bool inspect, const GUIVisualizationSettings& s, const DottedGeometry& dottedGeometry, const double width, const bool drawFirstExtrem, const bool drawLastExtrem) {
    // declare DottedGeometryColor
    DottedGeometryColor dottedGeometryColor(s);
    // make a copy of dotted geometry
    DottedGeometry topDottedGeometry = dottedGeometry;
    DottedGeometry botDottedGeometry = dottedGeometry;
    // move geometries
    topDottedGeometry.moveShapeToSide(width);
    botDottedGeometry.moveShapeToSide(width * -1);
    // invert offset of top dotted geometry
    topDottedGeometry.invertOffset();
    // calculate extremes
    DottedGeometry extremes(s, topDottedGeometry, drawFirstExtrem, botDottedGeometry, drawLastExtrem);
    // Push draw matrix
    glPushMatrix();
    // draw inspect or front dotted contour
    if (inspect) {
        // translate to front
        glTranslated(0, 0, GLO_DOTTEDCONTOUR_INSPECTED);
        // draw top dotted geometry
        topDottedGeometry.drawInspectedDottedGeometry(dottedGeometryColor);
        // reset color
        dottedGeometryColor.reset();
        // draw top dotted geometry
        botDottedGeometry.drawInspectedDottedGeometry(dottedGeometryColor);
        // change color
        dottedGeometryColor.changeColor();
        // draw extrem dotted geometry
        extremes.drawInspectedDottedGeometry(dottedGeometryColor);
    } else {
        // translate to front
        glTranslated(0, 0, GLO_DOTTEDCONTOUR_FRONT);
        // draw top dotted geometry
        topDottedGeometry.drawFrontDottedGeometry(dottedGeometryColor);
        // reset color
        dottedGeometryColor.reset();
        // draw top dotted geometry
        botDottedGeometry.drawFrontDottedGeometry(dottedGeometryColor);
        // change color
        dottedGeometryColor.changeColor();
        // draw extrem dotted geometry
        extremes.drawFrontDottedGeometry(dottedGeometryColor);
    }
    // pop matrix
    glPopMatrix();
}


void
GNEGeometry::drawDottedContourEdge(const bool inspect, const GUIVisualizationSettings& s, const GNEEdge* edge, const bool drawFrontExtreme, const bool drawBackExtreme) {
    if (edge->getLanes().size() == 1) {
        GNELane::LaneDrawingConstants laneDrawingConstants(s, edge->getLanes().front());
        GNEGeometry::drawDottedContourLane(inspect, s, edge->getLanes().front()->getDottedLaneGeometry(), laneDrawingConstants.halfWidth, drawFrontExtreme, drawBackExtreme);
    } else {
        // obtain a copy of both geometries
        GNEGeometry::DottedGeometry dottedGeometryTop = edge->getLanes().front()->getDottedLaneGeometry();
        GNEGeometry::DottedGeometry dottedGeometryBot = edge->getLanes().back()->getDottedLaneGeometry();
        // obtain both LaneDrawingConstants
        GNELane::LaneDrawingConstants laneDrawingConstantsFront(s, edge->getLanes().front());
        GNELane::LaneDrawingConstants laneDrawingConstantsBack(s, edge->getLanes().back());
        // move shapes to side
        dottedGeometryTop.moveShapeToSide(laneDrawingConstantsFront.halfWidth);
        dottedGeometryBot.moveShapeToSide(laneDrawingConstantsBack.halfWidth * -1);
        // invert offset of top dotted geometry
        dottedGeometryTop.invertOffset();
        // declare DottedGeometryColor
        DottedGeometryColor dottedGeometryColor(s);
        // calculate extremes
        DottedGeometry extremes(s, dottedGeometryTop, drawFrontExtreme, dottedGeometryBot, drawBackExtreme);
        // Push draw matrix
        glPushMatrix();
        // draw inspect or front dotted contour
        if (inspect) {
            // translate to front
            glTranslated(0, 0, GLO_DOTTEDCONTOUR_INSPECTED);
            // draw top dotted geometry
            dottedGeometryTop.drawInspectedDottedGeometry(dottedGeometryColor);
            // reset color
            dottedGeometryColor.reset();
            // draw top dotted geometry
            dottedGeometryBot.drawInspectedDottedGeometry(dottedGeometryColor);
            // change color
            dottedGeometryColor.changeColor();
            // draw extrem dotted geometry
            extremes.drawInspectedDottedGeometry(dottedGeometryColor);
        } else {
            // translate to front
            glTranslated(0, 0, GLO_DOTTEDCONTOUR_FRONT);
            // draw top dotted geometry
            dottedGeometryTop.drawFrontDottedGeometry(dottedGeometryColor);
            // reset color
            dottedGeometryColor.reset();
            // draw top dotted geometry
            dottedGeometryBot.drawFrontDottedGeometry(dottedGeometryColor);
            // change color
            dottedGeometryColor.changeColor();
            // draw extrem dotted geometry
            extremes.drawFrontDottedGeometry(dottedGeometryColor);
        }
        // pop matrix
        glPopMatrix();
    }
}


void
GNEGeometry::drawDottedContourClosedShape(const bool inspect, const GUIVisualizationSettings& s, const PositionVector& shape, const double exaggeration) {
    if (exaggeration > 0) {
        // declare DottedGeometryColor
        DottedGeometryColor dottedGeometryColor(s);
        // scale shape using exaggeration and default dotted geometry width
        PositionVector scaledShape = shape;
        // scale exaggeration
        scaledShape.scaleRelative(exaggeration);
        // calculate dotted geometry
        GNEGeometry::DottedGeometry dottedGeometry(s, scaledShape, true);
        // Push draw matrix
        glPushMatrix();
        // draw inspect or front dotted contour
        if (inspect) {
            // translate to front
            glTranslated(0, 0, GLO_DOTTEDCONTOUR_INSPECTED);
            // draw dotted geometry
            dottedGeometry.drawInspectedDottedGeometry(dottedGeometryColor);
        } else {
            // translate to front
            glTranslated(0, 0, GLO_DOTTEDCONTOUR_FRONT);
            // draw dotted geometry
            dottedGeometry.drawFrontDottedGeometry(dottedGeometryColor);
        }
        // pop matrix
        glPopMatrix();
    }
}


void
GNEGeometry::drawDottedContourShape(const bool inspect, const GUIVisualizationSettings& s, const PositionVector& shape, const double width, const double exaggeration) {
    // calculate dotted geometry
    GNEGeometry::DottedGeometry dottedGeometry(s, shape, false);
    // use drawDottedContourLane to draw it
    drawDottedContourLane(inspect, s, dottedGeometry, width * exaggeration, true, true);
}


void
GNEGeometry::drawDottedContourCircle(const bool inspect, const GUIVisualizationSettings& s, const Position& pos, const double radius, const double exaggeration) {
    // continue depending of exaggeratedRadio
    if ((radius * exaggeration) < 2) {
        drawDottedContourClosedShape(inspect, s, getVertexCircleAroundPosition(pos, radius, 8), exaggeration);
    } else {
        drawDottedContourClosedShape(inspect, s, getVertexCircleAroundPosition(pos, radius, 16), exaggeration);
    }
}


void
GNEGeometry::drawDottedSquaredShape(const bool inspect, const GUIVisualizationSettings& s, const Position& pos,
                                    const double width, const double height, const double offsetX, const double offsetY, const double rot, const double exaggeration) {
    // create shape
    PositionVector shape;
    // make rectangle
    shape.push_back(Position(0 + width, 0 + height));
    shape.push_back(Position(0 + width, 0 - height));
    shape.push_back(Position(0 - width, 0 - height));
    shape.push_back(Position(0 - width, 0 + height));
    // move shape
    shape.add(offsetX, offsetY, 0);
    // rotate shape
    shape.rotate2D(DEG2RAD((rot * -1) + 90));
    // move to position
    shape.add(pos);
    // draw using drawDottedContourClosedShape
    drawDottedContourClosedShape(inspect, s, shape, exaggeration);
}


PositionVector
GNEGeometry::getVertexCircleAroundPosition(const Position& pos, const double width, const int steps) {
    // first check if we have to fill myCircleCoords (only once)
    if (myCircleCoords.size() == 0) {
        for (int i = 0; i <= (int)(360 * CIRCLE_RESOLUTION); ++i) {
            const double x = (double) sin(DEG2RAD(i / CIRCLE_RESOLUTION));
            const double y = (double) cos(DEG2RAD(i / CIRCLE_RESOLUTION));
            myCircleCoords.push_back(Position(x, y));
        }
    }
    PositionVector vertexCircle;
    const double inc = 360 / (double)steps;
    // obtain all vertices
    for (int i = 0; i <= steps; ++i) {
        const Position& vertex = myCircleCoords[GNEGeometry::angleLookup(i * inc)];
        vertexCircle.push_back(Position(vertex.x() * width, vertex.y() * width));
    }
    // move result using position
    vertexCircle.add(pos);
    return vertexCircle;
}


int
GNEGeometry::angleLookup(const double angleDeg) {
    const int numCoords = (int)myCircleCoords.size() - 1;
    int index = ((int)(floor(angleDeg * CIRCLE_RESOLUTION + 0.5))) % numCoords;
    if (index < 0) {
        index += numCoords;
    }
    assert(index >= 0);
    return (int)index;
}


/****************************************************************************/
