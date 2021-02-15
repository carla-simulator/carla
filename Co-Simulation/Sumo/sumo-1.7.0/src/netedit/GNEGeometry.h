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
/// @file    GNEGeometry.h
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2019
///
// File for geometry classes and functions
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/common/Parameterised.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/geom/PositionVector.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include <utils/gui/images/GUIIcons.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include <utils/xml/SUMOXMLDefinitions.h>

#include <netedit/elements/GNEPathElements.h>

#include "GNEReferenceCounter.h"


// ===========================================================================
// class declarations
// ===========================================================================
class GNEAttributeCarrier;
class GNELane;
class GNEEdge;
class GNEAdditional;
class GNEDemandElement;
class GNEJunction;
class GNEViewNet;
class GNEHierarchicalElement;

// ===========================================================================
// class definitions
// ===========================================================================

struct GNEGeometry {

    /// @brief struct for variables used in Geometry extremes
    struct ExtremeGeometry {
        /// @brief constructor
        ExtremeGeometry();

        /// @brief depart position over lane
        double laneStartPosition;

        /// @brief arrival position over lane
        double laneEndPosition;

        /// @brief start position over view
        Position viewStartPos;

        /// @brief end position over view
        Position viewEndPos;
    };

    /// @brief class for NETEDIT geometries over lanes
    class Geometry {

    public:
        /// @brief constructor
        Geometry();

        /// @brief parameter constructor
        Geometry(const PositionVector& shape, const std::vector<double>& shapeRotations, const std::vector<double>& shapeLengths);

        /**@brief update geometry shape
         * @param shape Shape to be updated
         * @param extremeGeometry ExtremeGeometry used to cut/adjust shape
         */
        void updateGeometry(const PositionVector& shape, double startPos = -1, double endPos = -1,
                            const Position& extraFirstPosition = Position::INVALID,
                            const Position& extraLastPosition = Position::INVALID);

        /// @brief update position and rotation
        void updateGeometry(const Position& position, const double rotation);

        /// @brief update position and rotation (using a lane and a position over lane)
        void updateGeometry(const GNELane* lane, const double posOverLane);

        /// @brief update geometry (using a lane)
        void updateGeometry(const GNELane* lane);

        /// @brief update geometry (using geometry of another additional)
        void updateGeometry(const GNEAdditional* additional);

        /// @brief update geometry (using a new shape, rotations and lenghts)
        void updateGeometry(const Geometry& geometry);

        /// @brief scale geometry
        void scaleGeometry(const double scale);

        /// @brief get Position
        const Position& getPosition() const;

        /// @brief get rotation
        double getRotation() const;

        /// @brief The shape of the additional element
        const PositionVector& getShape() const;

        /// @brief The rotations of the single shape parts
        const std::vector<double>& getShapeRotations() const;

        /// @brief The lengths of the single shape parts
        const std::vector<double>& getShapeLengths() const;

    private:
        /// @brief clear geometry
        void clearGeometry();

        /// @brief calculate shape rotations and lengths
        void calculateShapeRotationsAndLengths();

        /// @brief get single position
        Position myPosition;

        /// @brief get single rotation
        double myRotation;

        /// @brief element shape
        PositionVector myShape;

        /// @brief The rotations of the shape (note: Always size = myShape.size()-1)
        std::vector<double> myShapeRotations;

        /// @brief The lengths of the shape (note: Always size = myShape.size()-1)
        std::vector<double> myShapeLengths;

        /// @brief lane (to use lane geometry)
        const GNELane* myLane;

        /// @brief additional (to use additional geometry)
        const GNEAdditional* myAdditional;

        /// @brief Invalidated assignment operator
        Geometry& operator=(const Geometry& other) = delete;
    };

    /// @brief class for pack all variables related with DottedGeometry color
    class DottedGeometryColor {

    public:
        /// @brief constructor
        DottedGeometryColor(const GUIVisualizationSettings& settings);

        /// @brief get inspected color (and change flag)
        const RGBColor& getInspectedColor();

        /// @brief get front color (and change flag)
        const RGBColor& getFrontColor();

        /// @brief change color
        void changeColor();

        /// @brief rest Dotted Geometry Color
        void reset();

    private:
        /// @brief pointer to GUIVisualizationSettings
        const GUIVisualizationSettings& mySettings;

        /// @brief flag to get color
        bool myColorFlag;

        /// @brief Invalidated assignment operator
        DottedGeometryColor& operator=(const DottedGeometryColor& other) = delete;
    };

    /// @brief class for pack all variables related with DottedGeometry
    class DottedGeometry {

    public:
        /// @brief dotted geometry segment
        struct Segment {
            /// @brief default constructor
            Segment();

            /// @brief constructor for a given shape
            Segment(PositionVector newShape);

            /// @brief shape
            PositionVector shape;

            /// @brief rotations
            std::vector<double> rotations;

            /// @brief lengths
            std::vector<double> lengths;

            /// @brief drawing offset (-1 or 1 only)
            double offset;
        };

        /// @brief constructor
        DottedGeometry();

        /// @brief constructor for shapes
        DottedGeometry(const GUIVisualizationSettings& s, PositionVector shape, const bool closeShape);

        /// @brief constructor for extremes
        DottedGeometry(const GUIVisualizationSettings& s,
                       const DottedGeometry& topDottedGeometry, const bool drawFirstExtrem,
                       const DottedGeometry& botDottedGeometry, const bool drawLastExtrem);

        /// @brief update DottedGeometry (using lane shape)
        void updateDottedGeometry(const GUIVisualizationSettings& s, const GNELane* lane);

        /// @brief update DottedGeometry (using shape)
        void updateDottedGeometry(const GUIVisualizationSettings& s, PositionVector shape, const bool closeShape);

        /// @brief draw inspected dottedShape
        void drawInspectedDottedGeometry(DottedGeometryColor& dottedGeometryColor) const;

        /// @brief draw front inspected dottedShape
        void drawFrontDottedGeometry(DottedGeometryColor& dottedGeometryColor) const;

        /// @brief move shape to side
        void moveShapeToSide(const double value);

        /// @brief get width
        double getWidth() const;

        /// @brief change default width
        void setWidth(const double width);

        /// @brief invert offset of all segments
        void invertOffset();

    private:
        /// @brief calculate shape rotations and lengths
        void calculateShapeRotationsAndLengths();

        /// @brief geometry width
        double myWidth;

        /// @brief dotted element shape (note: It's centered in 0,0 due scaling)
        std::vector<DottedGeometry::Segment> myDottedGeometrySegments;

        /// @brief Invalidated assignment operator
        DottedGeometry& operator=(const DottedGeometry& other) = delete;
    };

    /// @brief struct for pack all variables related with geometry of elemements divided in segments
    struct SegmentGeometry {

        /// @brief struct used for represent segments of element geometry
        class Segment {

        public:
            /// @brief parameter constructor for lanes (geometry will be taked from lane)
            Segment(const GNELane* lane, const bool valid);

            /// @brief parameter constructor for segments which geometry will be storaged in segment
            Segment(const GNELane* lane, const Geometry& geometry, const bool valid);

            /// @brief parameter constructor for lane2lane connections
            Segment(const GNELane* lane, const GNELane* nextLane, const bool valid);

            /// @brief update segment using geometry
            void update(const Geometry& geometry);

            /// @brief update segment using lane
            void update(const GNELane* lane);

            /// @brief get lane/lane2lane shape
            const PositionVector& getShape() const;

            /// @brief get lane/lane2lane shape rotations
            const std::vector<double>& getShapeRotations() const;

            /// @brief get lane/lane2lane shape lengths
            const std::vector<double>& getShapeLengths() const;

            /// @brief lane (only for lane segments)
            const GNELane* getLane() const;

            /// @brief junction
            const GNEJunction* getJunction() const;

            /// @brief valid
            bool getValid() const;

            /// @brief return true if this is a lane segment (i.e. myNextLane is nullptr)
            bool isLaneSegment() const;

        protected:
            /// @brief lane
            const GNELane* myLane;

            /// @brief nextLane
            const GNELane* myNextLane;

            /// @brief valid
            const bool myValid;

            /// @brief flag to use lane shape
            bool myUseLaneShape;

        private:
            /// @brief geometry used in segment
            Geometry mySegmentGeometry;

            /// @brief Invalidated assignment operator
            Segment& operator=(const Segment& other) = delete;
        };

        /// @brief struct used for represent segments that must be updated
        class SegmentToUpdate {

        public:
            /// @brief constructor
            SegmentToUpdate(const int segmentIndex, const GNELane* lane, const GNELane* nextLane);

            /// @brief get segment index
            int getSegmentIndex() const;

            // @brief get lane segment
            const GNELane* getLane() const;

            /// @brief get lane segment (used for updating lane2lane segments)
            const GNELane* getNextLane() const;

        private:
            /// @brief segment index
            const int mySegmentIndex;

            // @brief lane segment
            const GNELane* myLane;

            /// @brief lane segment (used for updating lane2lane segments)
            const GNELane* myNextLane;

        private:
            /// @brief Invalidated assignment operator
            SegmentToUpdate& operator=(const SegmentToUpdate& other) = delete;
        };

        /// @brief constructor
        SegmentGeometry();

        /// @brief insert entire lane segment (used to avoid unnecessary calculation in calculatePartialShapeRotationsAndLengths)
        void insertLaneSegment(const GNELane* lane, const bool valid);

        /// @brief insert custom segment
        void insertCustomSegment(const GNELane* lane, const Geometry& geometry, const bool valid);

        /// @brief insert entire lane2lane segment (used to avoid unnecessary calculation in calculatePartialShapeRotationsAndLengths)
        void insertLane2LaneSegment(const GNELane* currentLane, const GNELane* nextLane, const bool valid);

        /// @brief update custom segment
        void updateCustomSegment(const int segmentIndex, const Geometry& geometry);

        /// @brief update lane2Lane segment (used to avoid unnecessary calculation in calculatePartialShapeRotationsAndLengths)
        void updateLane2LaneSegment(const int segmentIndex, const GNELane* lane, const GNELane* nextLane);

        /// @brief clear element geometry
        void clearSegmentGeometry();

        /// @brief get first position (or Invalid position if segments are empty)
        const Position& getFirstPosition() const;

        /// @brief get first position (or Invalid position if segments are empty)
        const Position& getLastPosition() const;

        /// @brief get first rotation (or Invalid position if segments are empty)
        double getFirstRotation() const;

        /// @brief Returns a boundary enclosing all segments
        Boundary getBoxBoundary() const;

        /// @brief begin iterator
        std::vector<Segment>::const_iterator begin() const;

        /// @brief end iterator
        std::vector<Segment>::const_iterator end() const;

        /// @brief front segment
        const Segment& front() const;

        /// @brief back segment
        const Segment& back() const;

        /// @brief number of segments
        int size() const;

    private:
        /// @brief vector of segments that constitutes the shape
        std::vector<Segment> myShapeSegments;

        /// @brief Invalidated assignment operator
        SegmentGeometry& operator=(const SegmentGeometry& other) = delete;
    };

    /// @brief class lane2lane connection geometry
    class Lane2laneConnection {

    public:
        /// @brief constructor
        Lane2laneConnection(const GNELane* fromLane);

        /// @brief update
        void updateLane2laneConnection();

        /// @brief check if exist a lane2lane geometry for the given tolane
        bool exist(const GNELane* toLane) const;

        /// @brief get lane2lane geometry
        const GNEGeometry::Geometry& getLane2laneGeometry(const GNELane* toLane) const;

        /// @brief get lane2lane dotted geometry
        const GNEGeometry::DottedGeometry& getLane2laneDottedGeometry(const GNELane* toLane) const;

    protected:
        /// @brief from lane
        const GNELane* myFromLane;

        /// @brief connection shape
        std::map<const GNELane*, std::pair<GNEGeometry::Geometry, GNEGeometry::DottedGeometry> > myConnectionsMap;

    private:
        /// @brief constructor
        Lane2laneConnection();

        /// @brief Invalidated assignment operator
        Lane2laneConnection& operator=(const Lane2laneConnection& other) = delete;
    };

    /// @brief class to pack all variables and functions relative to connections between hierarchical element and their children
    class HierarchicalConnections {

    private:
        /// @brief connection geometry
        class ConnectionGeometry {

        public:
            /// @brief parameter constructor
            ConnectionGeometry(GNELane* lane);

            /// @brief get lane
            const GNELane* getLane() const;

            /// @brief get position
            const Position& getPosition() const;

            /// @brief get rotation
            double getRotation() const;

        private:
            /// @brief lane
            GNELane* myLane;

            /// @brief position
            Position myPosition;

            /// @brief rotation
            double myRotation;

            /// @brief default constructor
            ConnectionGeometry();
        };

    public:
        /// @brief constructor
        HierarchicalConnections(GNEHierarchicalElement* hierarchicalElement);

        /// @brief update Connection's geometry
        void update();

        /// @brief draw connections between Parent and childrens
        void drawConnection(const GUIVisualizationSettings& s, const GNEAttributeCarrier* AC, const double exaggeration) const;

        /// @brief draw dotted connections between Parent and childrens
        void drawDottedConnection(const bool inspect, const GUIVisualizationSettings& s, const double exaggeration) const;

        /// @brief position and rotation of every symbol over lane
        std::vector<ConnectionGeometry> symbolsPositionAndRotation;

        /// @brief geometry connections between parents an their children
        std::vector<GNEGeometry::Geometry> connectionsGeometries;

    private:
        /// @brief pointer to hierarchical element parent
        GNEHierarchicalElement* myHierarchicalElement;
    };

    /// @brief return angle between two points (used in geometric calculations)
    static double calculateRotation(const Position& first, const Position& second);

    /// @brief return length between two points (used in geometric calculations)
    static double calculateLength(const Position& first, const Position& second);

    /// @brief adjust start and end positions in geometric path
    static void adjustStartPosGeometricPath(double& startPos, const GNELane* startLane, double& endPos, const GNELane* endLane);

    /**@brief calculate route between lanes
     * @brief segmentGeometry segment geometry to be updated
     * @brief path list of pathElements (lanes)
     * @param extremeGeometry ExtremeGeometry used to cut/adjust shape
     */
    static void calculateLaneGeometricPath(GNEGeometry::SegmentGeometry& segmentGeometry,
                                           const std::vector<GNEPathElements::PathElement>& path,
                                           GNEGeometry::ExtremeGeometry& extremeGeometry);

    /**@brief calculate route between edges
     * @brief segmentGeometry segment geometry to be updated
     * @brief lane GNELane that called this function
     * @param extremeGeometry ExtremeGeometry used to cut/adjust shape
     */
    static void updateGeometricPath(GNEGeometry::SegmentGeometry& segmentGeometry, const GNELane* lane,
                                    GNEGeometry::ExtremeGeometry& extremeGeometry);

    /// @brief draw lane geometry (use their own function due colors)
    static void drawLaneGeometry(const GNEViewNet* viewNet, const PositionVector& shape, const std::vector<double>& rotations,
                                 const std::vector<double>& lengths, const std::vector<RGBColor>& colors, double width);

    /// @brief draw geometry
    static void drawGeometry(const GNEViewNet* viewNet, const Geometry& geometry, const double width);

    /// @brief draw geometry points
    static void drawGeometryPoints(const GUIVisualizationSettings& s, const GNEViewNet* viewNet, const PositionVector& shape,
                                   const RGBColor& geometryPointColor, const RGBColor& textColor, const double radius, const double exaggeration);

    /// @brief draw moving hint
    static void drawMovingHint(const GUIVisualizationSettings& s, const GNEViewNet* viewNet, const PositionVector& shape,
                               const RGBColor& hintColor, const double radius, const double exaggeration);

    /// @brief draw geometry segment
    static void drawSegmentGeometry(const GNEViewNet* viewNet, const SegmentGeometry::Segment& segment, const double width);

    /// @brief draw dotted contour for the given dottedGeometry (used by lanes, routes, etc.)
    static void drawDottedContourLane(const bool inspect, const GUIVisualizationSettings& s, const DottedGeometry& dottedGeometry, const double width, const bool drawFirstExtrem, const bool drawLastExtrem);

    /// @brief draw dotted contour for the given dottedGeometries (used by edges)
    static void drawDottedContourEdge(const bool inspect, const GUIVisualizationSettings& s, const GNEEdge* edge, const bool drawFrontExtreme, const bool drawBackExtreme);

    /// @brief draw dotted contour for the given closed shape (used by Juctions, shapes and TAZs)
    static void drawDottedContourClosedShape(const bool inspect, const GUIVisualizationSettings& s, const PositionVector& shape, const double exaggeration);

    /// @brief draw dotted contour for the given shape (used by additionals)
    static void drawDottedContourShape(const bool inspect, const GUIVisualizationSettings& s, const PositionVector& shape, const double width, const double exaggeration);

    /// @brief draw dotted contour for the given Position and radius (used by Juctions and POIs)
    static void drawDottedContourCircle(const bool inspect, const GUIVisualizationSettings& s, const Position& pos, const double radius, const double exaggeration);

    /// @brief draw dotted squared contour (used by additionals and demand elements)
    static void drawDottedSquaredShape(const bool inspect, const GUIVisualizationSettings& s, const Position& pos,
                                       const double width, const double height, const double offsetX, const double offsetY, const double rot, const double exaggeration);

    /// @brief get a circle around the given position
    static PositionVector getVertexCircleAroundPosition(const Position& pos, const double width, const int steps = 8);

private:
    /// @brief Storage for precomputed sin/cos-values describing a circle
    static PositionVector myCircleCoords;

    /// @brief normalize angle for lookup in myCircleCoords
    static int angleLookup(const double angleDeg);
};
