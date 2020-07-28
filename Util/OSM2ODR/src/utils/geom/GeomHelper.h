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
/// @file    GeomHelper.h
/// @author  Daniel Krajzewicz
/// @author  Friedemann Wesner
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Some static methods performing geometrical operations
/****************************************************************************/
#pragma once
#include <config.h>

#include <cmath>
#include "Position.h"
#include "PositionVector.h"
#include <utils/common/UtilExceptions.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#define DEG2RAD(x) static_cast<double>((x) * M_PI / 180.)
#define RAD2DEG(x) static_cast<double>((x) * 180. / M_PI)


// ===========================================================================
// class definitions
// ===========================================================================
/** @class GeomHelper
 * @brief Some static methods performing geometrical operations
 */
class GeomHelper {

public:
    /// @brief a value to signify offsets outside the range of [0, Line.length()]
    static const double INVALID_OFFSET;

    /** @brief Returns the positions the given circle is crossed by the given line
     * @param[in] c The center position of the circle
     * @param[in] radius The radius of the circle
     * @param[in] p1 The begin of the line
     * @param[in] p2 The end of the line
     * @param[filled] into The list of crossing positions (0-1 along the line's length)
     * @see http://blog.csharphelper.com/2010/03/28/determine-where-a-line-intersects-a-circle-in-c.aspx
     * @see http://gamedev.stackexchange.com/questions/18333/circle-line-collision-detection-problem (jazzdawg)
     */
    static void findLineCircleIntersections(const Position& c, double radius, const Position& p1, const Position& p2,
                                            std::vector<double>& into);


    /** @brief Returns the angle between two vectors on a plane
       The angle is from vector 1 to vector 2, positive anticlockwise
       The result is between -pi and pi
    */
    static double angle2D(const Position& p1, const Position& p2);

    static double nearest_offset_on_line_to_point2D(
        const Position& lineStart, const Position& lineEnd,
        const Position& p, bool perpendicular = true);

    static double nearest_offset_on_line_to_point25D(
        const Position& lineStart, const Position& lineEnd,
        const Position& p, bool perpendicular = true);

    static Position crossPoint(const Boundary& b,
                               const PositionVector& v);

    /** @brief Returns the distance of second angle from first angle counter-clockwise
     * @param[in] angle1 The first angle
     * @param[in] angle2 The second angle
     * @return Angle (counter-clockwise) starting from first to second angle
     */
    static double getCCWAngleDiff(double angle1, double angle2);


    /** @brief Returns the distance of second angle from first angle clockwise
     * @param[in] angle1 The first angle
     * @param[in] angle2 The second angle
     * @return Angle (clockwise) starting from first to second angle
     */
    static double getCWAngleDiff(double angle1, double angle2);


    /** @brief Returns the minimum distance (clockwise/counter-clockwise) between both angles
     * @param[in] angle1 The first angle
     * @param[in] angle2 The second angle
     * @return The minimum distance between both angles
     */
    static double getMinAngleDiff(double angle1, double angle2);


    /** @brief Returns the difference of the second angle to the first angle in radiants
     *
     * The results are always between -pi and pi.
     * Positive values denote that the second angle is counter clockwise closer, negative values mean
     * it is clockwise closer.
     * @param[in] angle1 The first angle
     * @param[in] angle2 The second angle
     * @return angle starting from first to second angle
     */
    static double angleDiff(const double angle1, const double angle2);


    /** Converts an angle from mathematical radians where 0 is to the right and positive angles
     *  are counterclockwise to navigational degrees where 0 is up and positive means clockwise.
     *  The result is always in the range [0, 360).
     * @param[in] angle The angle in radians to convert
     * @return the angle in degrees
     */
    static double naviDegree(const double angle);

    /** Converts an angle from navigational degrees to mathematical radians.
     * @see naviDegree
     * @param[in] angle The angle in degree to convert
     * @return the angle in radians
     */
    static double fromNaviDegree(const double angle);

    /** Converts an angle from mathematical radians where 0 is to the right and positive angles
     *  are counterclockwise to the legacy degrees used in sumo where 0 is down and positive means clockwise.
     *  If positive is true the result is in the range [0, 360), otherwise in the range [-180, 180).
     * @param[in] angle The angle in radians to convert
     * @return the angle in degrees
     */
    static double legacyDegree(const double angle, const bool positive = false);

    /** Creates a circular polygon
     * @param[in] radius Radius of the circle
     * @param[in] center Position of the circle's center
     * @param[in] nPoints Number of points of the circle (Polygon's shape will have noPoints+1 points), must be >=3
     * @return the polygon approximating the circle
     */
    static PositionVector makeCircle(const double radius, const Position& center, unsigned int nPoints);

    /** Creates a circular polygon
     * @param[in] radius1 Inner radius of the ring
     * @param[in] radius2 Outer radius of the ring
     * @param[in] center Position of the circle's center
     * @param[in] nPoints Number of points of the circle (Polygon's shape will have noPoints+1 points), must be >=3
     * @return the polygon approximating the circle
     */
    static PositionVector makeRing(const double radius1, const double radius2, const Position& center, unsigned int nPoints);

};
