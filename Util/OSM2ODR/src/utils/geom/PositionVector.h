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
/// @file    PositionVector.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A list of positions
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <limits>
#include "AbstractPoly.h"


// ===========================================================================
// class declarations
// ===========================================================================

class Boundary;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PositionVector
 * @brief A list of positions
 */
class PositionVector : public AbstractPoly, private std::vector<Position> {

private:
    /// @brief  vector of position
    typedef std::vector<Position> vp;

public:
    /// @brief Constructor. Creates an empty position vector
    PositionVector();

    /// @brief Copy Constructor. Create a positionVector with the same elements as other positionVector
    /// @param[in] v The vector to copy
    PositionVector(const std::vector<Position>& v);

    /// @brief Parameter Constructor. Create a positionVector using a part of other positionVector
    /// @param[in] beg The begin iterator for copy
    /// @param[in] end The end iterator to copy
    PositionVector(const std::vector<Position>::const_iterator beg, const std::vector<Position>::const_iterator end);

    /// @brief Parameter Constructor used for lines
    /// @param[in] p1 the first position
    /// @param[in] p2 the second position
    PositionVector(const Position& p1, const Position& p2);

    /// @brief Destructor
    ~PositionVector();

    /// @brief empty Vector
    static const PositionVector EMPTY;

    /// @name methode for iterate over PositionVector
    /// @{
    /// @brief iterator
    using vp::iterator;

    /// @brief constant iterator
    using vp::const_iterator;

    /// @brief contant reference
    using vp::const_reference;

    /// @brief value type
    using vp::value_type;

    /// @brief begin of position vector
    using vp::begin;

    /// @brief end of position vector
    using vp::end;

    /// @brief push a position in the back of position vector
    using vp::push_back;

    /// @brief push a position in the front of position vector
    using vp::pop_back;

    /// @brief clear all elements of position vector
    using vp::clear;
    /// @brief returns size of position vector
    using vp::size;

    /// @brief get the front element of position vector
    using vp::front;

    /// @brief get back element of position vector
    using vp::back;

    /// @brief get a reference of position vector
    using vp::reference;

    /// @brief erase a position of position vector gived by iterator
    using vp::erase;

    /// @brief insert a position in position vector gived by iterator
    using vp::insert;
    /// @}

    /// @brief Returns the information whether the position vector describes a polygon lying around the given point
    /// @note The optional offset is added to the polygon's boundaries
    bool around(const Position& p, double offset = 0) const;

    /// @brief Returns the information whether the given polygon overlaps with this
    /// @note Again a boundary may be specified
    bool overlapsWith(const AbstractPoly& poly, double offset = 0) const;

    /// @brief Returns the maximum overlaps between this and the given polygon (when not separated by at least zThreshold)
    double getOverlapWith(const PositionVector& poly, double zThreshold) const;

    /// @brief Returns the information whether this list of points interesects the given line
    bool intersects(const Position& p1, const Position& p2) const;

    /// @brief Returns the information whether this list of points interesects one the given lines
    bool intersects(const PositionVector& v1) const;

    /// @brief Returns the position of the intersection
    Position intersectionPosition2D(const Position& p1, const Position& p2, const double withinDist = 0.) const;

    /// @brief For all intersections between this vector and other, return the 2D-length of the subvector from this vectors start to the intersection
    std::vector<double> intersectsAtLengths2D(const PositionVector& other) const;

    /// @brief For all intersections between this vector and line, return the 2D-length of the subvector from this vectors start to the intersection
    std::vector<double> intersectsAtLengths2D(const Position& lp1, const Position& lp2) const;

    /// @brief Returns the position of the intersection
    Position intersectionPosition2D(const PositionVector& v1) const;

    /// @brief ensures that the last position equals the first
    void closePolygon();

    /// @brief returns the constat position at the given index
    /// @ToDo !!! exceptions?
    const Position& operator[](int index) const;

    /// @brief returns the position at the given index
    Position& operator[](int index);

    /// @brief Returns the position at the given length
    Position positionAtOffset(double pos, double lateralOffset = 0) const;

    /// @brief Returns the position at the given length
    Position positionAtOffset2D(double pos, double lateralOffset = 0) const;

    /// @brief Returns the rotation at the given length
    double rotationAtOffset(double pos) const;

    /// @brief Returns the rotation at the given length
    double rotationDegreeAtOffset(double pos) const;

    /// @brief Returns the slope at the given length
    double slopeDegreeAtOffset(double pos) const;

    /// @brief Returns the position between the two given point at the specified position
    static Position positionAtOffset(const Position& p1, const Position& p2, double pos, double lateralOffset = 0.);

    /// Returns the position between the two given point at the specified position
    static Position positionAtOffset2D(const Position& p1, const Position& p2, double pos, double lateralOffset = 0.);

    /// @brief Returns a boundary enclosing this list of lines
    Boundary getBoxBoundary() const;

    /// @brief Returns the arithmetic of all corner points
    /// @note: this is different from the centroid!
    Position getPolygonCenter() const;

    /// @brief Returns the centroid (closes the polygon if unclosed)
    Position getCentroid() const;

    /// @brief enlarges/shrinks the polygon by a factor based at the centroid
    void scaleRelative(double factor);

    /// @brief enlarges/shrinks the polygon by an absolute offset based at the centroid
    void scaleAbsolute(double offset);

    /// @brief get line center
    Position getLineCenter() const;

    /// @brief Returns the length
    double length() const;

    /// @brief Returns the length
    double length2D() const;

    /// @brief Returns the area (0 for non-closed)
    double area() const;

    /// @brief Returns the information whether this polygon lies partially within the given polygon
    bool partialWithin(const AbstractPoly& poly, double offset = 0) const;

    /// @brief Returns the two lists made when this list vector is splitted at the given point
    std::pair<PositionVector, PositionVector> splitAt(double where, bool use2D = false) const;

    //// @brief Output operator
    friend std::ostream& operator<<(std::ostream& os, const PositionVector& geom);

    //// @brief check if two positions crosses
    bool crosses(const Position& p1, const Position& p2) const;

    //// @brief add a position
    void add(double xoff, double yoff, double zoff);

    //// @brief substract a position
    void add(const Position& offset);

    //// @brief add a position
    void sub(double xoff, double yoff, double zoff);

    //// @brief substract a position
    void sub(const Position& offset);

    //// @brief adds a position without modifying the vector itself but returning the result
    PositionVector added(const Position& offset) const;

    //// @brief mirror coordinates along the x-axis
    void mirrorX();

    //// @brief rotate all points around (0,0) in the plane by the given angle
    void rotate2D(double angle);

    //// @brief append the given vector to this one
    void append(const PositionVector& v, double sameThreshold = 2.0);

    /// @brief get subpart of a position vector
    PositionVector getSubpart(double beginOffset, double endOffset) const;

    /// @brief get subpart of a position vector in two dimensions (Z is ignored)
    PositionVector getSubpart2D(double beginOffset, double endOffset) const;

    /// @brief get subpart of a position vector using index and a cout
    PositionVector getSubpartByIndex(int beginIndex, int count) const;

    /// @brief sort as polygon CW by angle
    void sortAsPolyCWByAngle();

    /// @brief sort by increasing X-Y Positions
    void sortByIncreasingXY();

    /// @brief extrapolate position vector
    void extrapolate(const double val, const bool onlyFirst = false, const bool onlyLast = false);

    /// @brief extrapolate position vector in two dimensions (Z is ignored)
    void extrapolate2D(const double val, const bool onlyFirst = false);

    /// @brief reverse position vector
    PositionVector reverse() const;

    /// @brief get a side position of position vector using a offset
    static Position sideOffset(const Position& beg, const Position& end, const double amount);

    /// @brief move position vector to side using certain ammount
    void move2side(double amount, double maxExtension = 100);

    /// @brief move position vector to side using a custom offset for each geometry point
    void move2side(std::vector<double> amount, double maxExtension = 100);

    /// @brief get angle in certain position of position vector
    double angleAt2D(int pos) const;

    /**@brief inserts p between the two closest positions
     * @param p position to be inserted
     * @param interpolateZ flag to enable/disable interpolation of Z Value between the two closest positions
     * @return the insertion index
     */
    int insertAtClosest(const Position& p, bool interpolateZ);

    /// @brief removes the point closest to p and return the removal index
    int removeClosest(const Position& p);

    /// @brief comparing operation
    bool operator==(const PositionVector& v2) const;

    /// @brief comparing operation
    bool operator!=(const PositionVector& v2) const;

    /// @brief substracts two vectors (requires vectors of the same length)
    PositionVector operator-(const PositionVector& v2) const;

    /// @brief adds two vectors (requires vectors of the same length)
    PositionVector operator+(const PositionVector& v2) const;

    /// @brief clase for CW Sorter
    class as_poly_cw_sorter {
    public:
        /// @brief constructor
        as_poly_cw_sorter();

        /// @brief comparing operation for sort
        int operator()(const Position& p1, const Position& p2) const;
    };

    /// @brief clase for increasing Sorter
    class increasing_x_y_sorter {
    public:
        /// constructor
        explicit increasing_x_y_sorter();

        /// comparing operation
        int operator()(const Position& p1, const Position& p2) const;
    };

    /// @brief get left
    /// @note previously marked with "!!!"
    double isLeft(const Position& P0, const Position& P1, const Position& P2) const;

    /// @brief returns the angle in radians of the line connecting the first and the last position
    double beginEndAngle() const;

    /// @brief return the nearest offest to point 2D
    double nearest_offset_to_point2D(const Position& p, bool perpendicular = true) const;

    /// @brief return the nearest offest to point 2D projected onto the 3D geometry
    double nearest_offset_to_point25D(const Position& p, bool perpendicular = true) const;

    /** @brief return position p within the length-wise coordinate system
     * defined by this position vector. The x value is the same as that returned
     * by nearest_offset_to_point2D(p) and the y value is the perpendicular distance to this
     * vector with the sign indicating the side (right is postive).
     * if extend is true, the vector is extended on both sides and the
     * x-coordinate of the result may be below 0 or above the length of the original vector
     */
    Position transformToVectorCoordinates(const Position& p, bool extend = false) const;

    /// @brief index of the closest position to p
    /// @note: may only be called for a non-empty vector
    int indexOfClosest(const Position& p) const;

    /// @brief distances of all my points to s and all of s points to myself
    /// @note if perpendicular is set to true, only the perpendicular distances are returned
    std::vector<double> distances(const PositionVector& s, bool perpendicular = false) const;

    /// @brief closest 2D-distance to point p (or -1 if perpendicular is true and the point is beyond this vector)
    double distance2D(const Position& p, bool perpendicular = false) const;

    /// @brief insert in front a Position
    void push_front(const Position& p);

    /// @brief pop first Position
    void pop_front();

    /// @brief insert in back a non double position
    void push_back_noDoublePos(const Position& p);

    /// @brief insert in front a non double position
    void push_front_noDoublePos(const Position& p);

    /// @brief insert in front a non double position
    void insert_noDoublePos(const std::vector<Position>::iterator& at, const Position& p);

    /// @brief check if PositionVector is closed
    bool isClosed() const;

    /// @brief check if PositionVector is NAN
    bool isNAN() const;

    /** @brief Removes positions if too near
     * @param[in] minDist The minimum accepted distance; default: POSITION_EPS
     * @param[in] assertLength Whether the result must at least contain two points (be a line); default: false, to ensure original behaviour
     */
    void removeDoublePoints(double minDist = POSITION_EPS, bool assertLength = false);

    /// @brief return whether two positions differ in z-coordinate
    bool hasElevation() const;

    /// @brief return the same shape with intermediate colinear points removed
    PositionVector simplified() const;

    /** @brief return orthogonal through p (extending this vector if necessary)
     * @param[in] p The point through which to draw the orthogonal
     * @param[in] extend how long to extend this vector for finding an orthogonal
     * @param[in] front Whether to take the segment before or after the base point in case of ambiguity
     * @param[in] length the length of the orthogonal
     * @param[in] deg the rotation angle relative to the shape direction
     */
    PositionVector getOrthogonal(const Position& p, double extend, bool before, double length = 1.0, double deg = 90) const;

    /// @brief returned vector that is smoothed at the front (within dist)
    PositionVector smoothedZFront(double dist = std::numeric_limits<double>::max()) const;

    /// @brief returned vector that varies z smoothly over its length
    PositionVector interpolateZ(double zStart, double zEnd) const;

    /// @brief resample shape with the given number of points (equal spacing)
    PositionVector resample(double maxLength) const;

    /// @brief return the offset at the given index
    double offsetAtIndex2D(int index) const;

    /* @brief return the maximum grade of all segments as a fraction of zRange/length2D
     * @param[out] maxJump The maximum vertical jump (with grade infinity)
     */
    double getMaxGrade(double& maxJump) const;

    /// @brief return a bezier interpolation
    PositionVector bezier(int numPoints);

private:
    /// @brief return whether the line segments defined by Line p11,p12 and Line p21,p22 intersect
    static bool intersects(const Position& p11, const Position& p12, const Position& p21, const Position& p22, const double withinDist = 0., double* x = 0, double* y = 0, double* mu = 0);
};
