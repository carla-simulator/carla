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
/// @file    PositionVector.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    Sept 2002
///
// A list of positions
/****************************************************************************/
#include <config.h>

#include <queue>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <utils/common/StdDefs.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "AbstractPoly.h"
#include "Position.h"
#include "PositionVector.h"
#include "GeomHelper.h"
#include "Boundary.h"

// ===========================================================================
// static members
// ===========================================================================
const PositionVector PositionVector::EMPTY;

// ===========================================================================
// method definitions
// ===========================================================================

PositionVector::PositionVector() {}


PositionVector::PositionVector(const std::vector<Position>& v) {
    std::copy(v.begin(), v.end(), std::back_inserter(*this));
}


PositionVector::PositionVector(const std::vector<Position>::const_iterator beg, const std::vector<Position>::const_iterator end) {
    std::copy(beg, end, std::back_inserter(*this));
}


PositionVector::PositionVector(const Position& p1, const Position& p2) {
    push_back(p1);
    push_back(p2);
}


PositionVector::~PositionVector() {}


bool
PositionVector::around(const Position& p, double offset) const {
    if (size() < 2) {
        return false;
    }
    if (offset != 0) {
        PositionVector tmp(*this);
        tmp.scaleAbsolute(offset);
        return tmp.around(p);
    }
    double angle = 0;
    // iterate over all points, and obtain angle between current and next
    for (const_iterator i = begin(); i != (end() - 1); i++) {
        Position p1(
            i->x() - p.x(),
            i->y() - p.y());
        Position p2(
            (i + 1)->x() - p.x(),
            (i + 1)->y() - p.y());
        angle += GeomHelper::angle2D(p1, p2);
    }
    // add angle between last and first point
    Position p1(
        (end() - 1)->x() - p.x(),
        (end() - 1)->y() - p.y());
    Position p2(
        begin()->x() - p.x(),
        begin()->y() - p.y());
    angle += GeomHelper::angle2D(p1, p2);
    // if angle is less than PI, then point lying in Polygon
    return (!(fabs(angle) < M_PI));
}


bool
PositionVector::overlapsWith(const AbstractPoly& poly, double offset) const {
    if (
        // check whether one of my points lies within the given poly
        partialWithin(poly, offset) ||
        // check whether the polygon lies within me
        poly.partialWithin(*this, offset)) {
        return true;
    }
    if (size() >= 2) {
        for (const_iterator i = begin(); i != end() - 1; i++) {
            if (poly.crosses(*i, *(i + 1))) {
                return true;
            }
        }
        if (size() > 2 && poly.crosses(back(), front())) {
            return true;
        }
    }
    return false;
}


double
PositionVector::getOverlapWith(const PositionVector& poly, double zThreshold) const {
    double result = 0;
    if ((size() == 0) || (poly.size() == 0)) {
        return result;
    }
    // this points within poly
    for (const_iterator i = begin(); i != end() - 1; i++) {
        if (poly.around(*i)) {
            Position closest = poly.positionAtOffset2D(poly.nearest_offset_to_point2D(*i));
            if (fabs(closest.z() - (*i).z()) < zThreshold) {
                result = MAX2(result, poly.distance2D(*i));
            }
        }
    }
    // polys points within this
    for (const_iterator i = poly.begin(); i != poly.end() - 1; i++) {
        if (around(*i)) {
            Position closest = positionAtOffset2D(nearest_offset_to_point2D(*i));
            if (fabs(closest.z() - (*i).z()) < zThreshold) {
                result = MAX2(result, distance2D(*i));
            }
        }
    }
    return result;
}


bool
PositionVector::intersects(const Position& p1, const Position& p2) const {
    if (size() < 2) {
        return false;
    }
    for (const_iterator i = begin(); i != end() - 1; i++) {
        if (intersects(*i, *(i + 1), p1, p2)) {
            return true;
        }
    }
    return false;
}


bool
PositionVector::intersects(const PositionVector& v1) const {
    if (size() < 2) {
        return false;
    }
    for (const_iterator i = begin(); i != end() - 1; i++) {
        if (v1.intersects(*i, *(i + 1))) {
            return true;
        }
    }
    return false;
}


Position
PositionVector::intersectionPosition2D(const Position& p1, const Position& p2, const double withinDist) const {
    for (const_iterator i = begin(); i != end() - 1; i++) {
        double x, y, m;
        if (intersects(*i, *(i + 1), p1, p2, withinDist, &x, &y, &m)) {
            return Position(x, y);
        }
    }
    return Position::INVALID;
}


Position
PositionVector::intersectionPosition2D(const PositionVector& v1) const {
    for (const_iterator i = begin(); i != end() - 1; i++) {
        if (v1.intersects(*i, *(i + 1))) {
            return v1.intersectionPosition2D(*i, *(i + 1));
        }
    }
    return Position::INVALID;
}


const Position&
PositionVector::operator[](int index) const {
    /* bracket operators works as in Python. Examples:
        - A = {'a', 'b', 'c', 'd'} (size 4)
        - A [2] returns 'c' because 0 < 2 < 4
        - A [100] thrown an exception because 100 > 4
        - A [-1] returns 'd' because 4 - 1 = 3
        - A [-100] thrown an exception because (4-100) < 0
    */
    if (index >= 0 && index < (int)size()) {
        return at(index);
    } else if (index < 0 && -index <= (int)size()) {
        return at((int)size() + index);
    } else {
        throw ProcessError("Index out of range in bracket operator of PositionVector");
    }
}


Position&
PositionVector::operator[](int index) {
    /* bracket operators works as in Python. Examples:
        - A = {'a', 'b', 'c', 'd'} (size 4)
        - A [2] returns 'c' because 0 < 2 < 4
        - A [100] thrown an exception because 100 > 4
        - A [-1] returns 'd' because 4 - 1 = 3
        - A [-100] thrown an exception because (4-100) < 0
    */
    if (index >= 0 && index < (int)size()) {
        return at(index);
    } else if (index < 0 && -index <= (int)size()) {
        return at((int)size() + index);
    } else {
        throw ProcessError("Index out of range in bracket operator of PositionVector");
    }
}


Position
PositionVector::positionAtOffset(double pos, double lateralOffset) const {
    if (size() == 0) {
        return Position::INVALID;
    }
    if (size() == 1) {
        return front();
    }
    const_iterator i = begin();
    double seenLength = 0;
    do {
        const double nextLength = (*i).distanceTo(*(i + 1));
        if (seenLength + nextLength > pos) {
            return positionAtOffset(*i, *(i + 1), pos - seenLength, lateralOffset);
        }
        seenLength += nextLength;
    } while (++i != end() - 1);
    if (lateralOffset == 0 || size() < 2) {
        return back();
    } else {
        return positionAtOffset(*(end() - 2), *(end() - 1), (*(end() - 2)).distanceTo(*(end() - 1)), lateralOffset);
    }
}


Position
PositionVector::positionAtOffset2D(double pos, double lateralOffset) const {
    if (size() == 0) {
        return Position::INVALID;
    }
    if (size() == 1) {
        return front();
    }
    const_iterator i = begin();
    double seenLength = 0;
    do {
        const double nextLength = (*i).distanceTo2D(*(i + 1));
        if (seenLength + nextLength > pos) {
            return positionAtOffset2D(*i, *(i + 1), pos - seenLength, lateralOffset);
        }
        seenLength += nextLength;
    } while (++i != end() - 1);
    return back();
}


double
PositionVector::rotationAtOffset(double pos) const {
    if (size() == 0) {
        return INVALID_DOUBLE;
    }
    if (pos < 0) {
        pos += length();
    }
    const_iterator i = begin();
    double seenLength = 0;
    do {
        const Position& p1 = *i;
        const Position& p2 = *(i + 1);
        const double nextLength = p1.distanceTo(p2);
        if (seenLength + nextLength > pos) {
            return p1.angleTo2D(p2);
        }
        seenLength += nextLength;
    } while (++i != end() - 1);
    const Position& p1 = (*this)[-2];
    const Position& p2 = back();
    return p1.angleTo2D(p2);
}


double
PositionVector::rotationDegreeAtOffset(double pos) const {
    return GeomHelper::legacyDegree(rotationAtOffset(pos));
}


double
PositionVector::slopeDegreeAtOffset(double pos) const {
    if (size() == 0) {
        return INVALID_DOUBLE;
    }
    const_iterator i = begin();
    double seenLength = 0;
    do {
        const Position& p1 = *i;
        const Position& p2 = *(i + 1);
        const double nextLength = p1.distanceTo(p2);
        if (seenLength + nextLength > pos) {
            return RAD2DEG(atan2(p2.z() - p1.z(), p1.distanceTo2D(p2)));
        }
        seenLength += nextLength;
    } while (++i != end() - 1);
    const Position& p1 = (*this)[-2];
    const Position& p2 = back();
    return RAD2DEG(atan2(p2.z() - p1.z(), p1.distanceTo2D(p2)));
}


Position
PositionVector::positionAtOffset(const Position& p1, const Position& p2, double pos, double lateralOffset) {
    const double dist = p1.distanceTo(p2);
    if (pos < 0. || dist < pos) {
        return Position::INVALID;
    }
    if (lateralOffset != 0) {
        if (dist == 0.) {
            return Position::INVALID;
        }
        const Position offset = sideOffset(p1, p2, -lateralOffset); // move in the same direction as Position::move2side
        if (pos == 0.) {
            return p1 + offset;
        }
        return p1 + (p2 - p1) * (pos / dist) + offset;
    }
    if (pos == 0.) {
        return p1;
    }
    return p1 + (p2 - p1) * (pos / dist);
}


Position
PositionVector::positionAtOffset2D(const Position& p1, const Position& p2, double pos, double lateralOffset) {
    const double dist = p1.distanceTo2D(p2);
    if (pos < 0 || dist < pos) {
        return Position::INVALID;
    }
    if (lateralOffset != 0) {
        const Position offset = sideOffset(p1, p2, -lateralOffset); // move in the same direction as Position::move2side
        if (pos == 0.) {
            return p1 + offset;
        }
        return p1 + (p2 - p1) * (pos / dist) + offset;
    }
    if (pos == 0.) {
        return p1;
    }
    return p1 + (p2 - p1) * (pos / dist);
}


Boundary
PositionVector::getBoxBoundary() const {
    Boundary ret;
    for (const Position& i : *this) {
        ret.add(i);
    }
    return ret;
}


Position
PositionVector::getPolygonCenter() const {
    double x = 0;
    double y = 0;
    double z = 0;
    for (const Position& i : *this) {
        x += i.x();
        y += i.y();
        z += i.z();
    }
    return Position(x / (double) size(), y / (double) size(), z / (double)size());
}


Position
PositionVector::getCentroid() const {
    if (size() == 0) {
        return Position::INVALID;
    } else if (size() == 1) {
        return (*this)[0];
    } else if (size() == 2) {
        return ((*this)[0] + (*this)[1]) * 0.5;
    }
    PositionVector tmp = *this;
    if (!isClosed()) { // make sure its closed
        tmp.push_back(tmp[0]);
    }
    // shift to origin to increase numerical stability
    Position offset = tmp[0];
    Position result;
    tmp.sub(offset);
    const int endIndex = (int)tmp.size() - 1;
    double div = 0; // 6 * area including sign
    double x = 0;
    double y = 0;
    if (tmp.area() != 0) { // numerical instability ?
        // http://en.wikipedia.org/wiki/Polygon
        for (int i = 0; i < endIndex; i++) {
            const double z = tmp[i].x() * tmp[i + 1].y() - tmp[i + 1].x() * tmp[i].y();
            div += z; // area formula
            x += (tmp[i].x() + tmp[i + 1].x()) * z;
            y += (tmp[i].y() + tmp[i + 1].y()) * z;
        }
        div *= 3; //  6 / 2, the 2 comes from the area formula
        result = Position(x / div, y / div);
    } else {
        // compute by decomposing into line segments
        // http://en.wikipedia.org/wiki/Centroid#By_geometric_decomposition
        double lengthSum = 0;
        for (int i = 0; i < endIndex; i++) {
            double length = tmp[i].distanceTo(tmp[i + 1]);
            x += (tmp[i].x() + tmp[i + 1].x()) * length / 2;
            y += (tmp[i].y() + tmp[i + 1].y()) * length / 2;
            lengthSum += length;
        }
        if (lengthSum == 0) {
            // it is probably only one point
            result = tmp[0];
        }
        result = Position(x / lengthSum, y / lengthSum) + offset;
    }
    return result + offset;
}


void
PositionVector::scaleRelative(double factor) {
    Position centroid = getCentroid();
    for (int i = 0; i < static_cast<int>(size()); i++) {
        (*this)[i] = centroid + (((*this)[i] - centroid) * factor);
    }
}


void
PositionVector::scaleAbsolute(double offset) {
    Position centroid = getCentroid();
    for (int i = 0; i < static_cast<int>(size()); i++) {
        (*this)[i] = centroid + (((*this)[i] - centroid) + offset);
    }
}


Position
PositionVector::getLineCenter() const {
    if (size() == 1) {
        return (*this)[0];
    } else {
        return positionAtOffset(double((length() / 2.)));
    }
}


double
PositionVector::length() const {
    if (size() == 0) {
        return 0;
    }
    double len = 0;
    for (const_iterator i = begin(); i != end() - 1; i++) {
        len += (*i).distanceTo(*(i + 1));
    }
    return len;
}


double
PositionVector::length2D() const {
    if (size() == 0) {
        return 0;
    }
    double len = 0;
    for (const_iterator i = begin(); i != end() - 1; i++) {
        len += (*i).distanceTo2D(*(i + 1));
    }
    return len;
}


double
PositionVector::area() const {
    if (size() < 3) {
        return 0;
    }
    double area = 0;
    PositionVector tmp = *this;
    if (!isClosed()) { // make sure its closed
        tmp.push_back(tmp[0]);
    }
    const int endIndex = (int)tmp.size() - 1;
    // http://en.wikipedia.org/wiki/Polygon
    for (int i = 0; i < endIndex; i++) {
        area += tmp[i].x() * tmp[i + 1].y() - tmp[i + 1].x() * tmp[i].y();
    }
    if (area < 0) { // we whether we had cw or ccw order
        area *= -1;
    }
    return area / 2;
}


bool
PositionVector::partialWithin(const AbstractPoly& poly, double offset) const {
    if (size() < 2) {
        return false;
    }
    for (const_iterator i = begin(); i != end(); i++) {
        if (poly.around(*i, offset)) {
            return true;
        }
    }
    return false;
}


bool
PositionVector::crosses(const Position& p1, const Position& p2) const {
    return intersects(p1, p2);
}


std::pair<PositionVector, PositionVector>
PositionVector::splitAt(double where, bool use2D) const {
    const double len = use2D ? length2D() : length();
    if (size() < 2) {
        throw InvalidArgument("Vector to short for splitting");
    }
    if (where < 0 || where > len) {
        throw InvalidArgument("Invalid split position " + toString(where) + " for vector of length " + toString(len));
    }
    if (where <= POSITION_EPS || where >= len - POSITION_EPS) {
        WRITE_WARNING("Splitting vector close to end (pos: " + toString(where) + ", length: " + toString(len) + ")");
    }
    PositionVector first, second;
    first.push_back((*this)[0]);
    double seen = 0;
    const_iterator it = begin() + 1;
    double next = use2D ? first.back().distanceTo2D(*it) : first.back().distanceTo(*it);
    // see how many points we can add to first
    while (where >= seen + next + POSITION_EPS) {
        seen += next;
        first.push_back(*it);
        it++;
        next = use2D ? first.back().distanceTo2D(*it) : first.back().distanceTo(*it);
    }
    if (fabs(where - (seen + next)) > POSITION_EPS || it == end() - 1) {
        // we need to insert a new point because 'where' is not close to an
        // existing point or it is to close to the endpoint
        const Position p = (use2D
                            ? positionAtOffset2D(first.back(), *it, where - seen)
                            : positionAtOffset(first.back(), *it, where - seen));
        first.push_back(p);
        second.push_back(p);
    } else {
        first.push_back(*it);
    }
    // add the remaining points to second
    for (; it != end(); it++) {
        second.push_back(*it);
    }
    assert(first.size() >= 2);
    assert(second.size() >= 2);
    assert(first.back() == second.front());
    assert(fabs((use2D ? first.length2D() + second.length2D() : first.length() + second.length()) - len) < 2 * POSITION_EPS);
    return std::pair<PositionVector, PositionVector>(first, second);
}


std::ostream&
operator<<(std::ostream& os, const PositionVector& geom) {
    for (PositionVector::const_iterator i = geom.begin(); i != geom.end(); i++) {
        if (i != geom.begin()) {
            os << " ";
        }
        os << (*i);
    }
    return os;
}


void
PositionVector::sortAsPolyCWByAngle() {
    std::sort(begin(), end(), as_poly_cw_sorter());
}


void
PositionVector::add(double xoff, double yoff, double zoff) {
    for (int i = 0; i < (int)size(); i++) {
        (*this)[i].add(xoff, yoff, zoff);
    }
}


void
PositionVector::sub(const Position& offset) {
    sub(offset.x(), offset.y(), offset.z());
}


void
PositionVector::sub(double xoff, double yoff, double zoff) {
    for (int i = 0; i < (int)size(); i++) {
        (*this)[i].add(-xoff, -yoff, -zoff);
    }
}


void
PositionVector::add(const Position& offset) {
    add(offset.x(), offset.y(), offset.z());
}


PositionVector
PositionVector::added(const Position& offset) const {
    PositionVector pv;
    for (auto i1 = begin(); i1 != end(); ++i1) {
        pv.push_back(*i1 + offset);
    }
    return pv;
}


void
PositionVector::mirrorX() {
    for (int i = 0; i < (int)size(); i++) {
        (*this)[i].mul(1, -1);
    }
}


PositionVector::as_poly_cw_sorter::as_poly_cw_sorter() {}


int
PositionVector::as_poly_cw_sorter::operator()(const Position& p1, const Position& p2) const {
    return atan2(p1.x(), p1.y()) < atan2(p2.x(), p2.y());
}


void
PositionVector::sortByIncreasingXY() {
    std::sort(begin(), end(), increasing_x_y_sorter());
}


PositionVector::increasing_x_y_sorter::increasing_x_y_sorter() {}


int
PositionVector::increasing_x_y_sorter::operator()(const Position& p1, const Position& p2) const {
    if (p1.x() != p2.x()) {
        return p1.x() < p2.x();
    }
    return p1.y() < p2.y();
}


double
PositionVector::isLeft(const Position& P0, const Position& P1,  const Position& P2) const {
    return (P1.x() - P0.x()) * (P2.y() - P0.y()) - (P2.x() - P0.x()) * (P1.y() - P0.y());
}


void
PositionVector::append(const PositionVector& v, double sameThreshold) {
    if ((size() > 0) && (v.size() > 0) && (back().distanceTo(v[0]) < sameThreshold)) {
        copy(v.begin() + 1, v.end(), back_inserter(*this));
    } else {
        copy(v.begin(), v.end(), back_inserter(*this));
    }
}


PositionVector
PositionVector::getSubpart(double beginOffset, double endOffset) const {
    PositionVector ret;
    Position begPos = front();
    if (beginOffset > POSITION_EPS) {
        begPos = positionAtOffset(beginOffset);
    }
    Position endPos = back();
    if (endOffset < length() - POSITION_EPS) {
        endPos = positionAtOffset(endOffset);
    }
    ret.push_back(begPos);

    double seen = 0;
    const_iterator i = begin();
    // skip previous segments
    while ((i + 1) != end()
            &&
            seen + (*i).distanceTo(*(i + 1)) < beginOffset) {
        seen += (*i).distanceTo(*(i + 1));
        i++;
    }
    // append segments in between
    while ((i + 1) != end()
            &&
            seen + (*i).distanceTo(*(i + 1)) < endOffset) {

        ret.push_back_noDoublePos(*(i + 1));
        seen += (*i).distanceTo(*(i + 1));
        i++;
    }
    // append end
    ret.push_back_noDoublePos(endPos);
    if (ret.size() == 1) {
        ret.push_back(endPos);
    }
    return ret;
}


PositionVector
PositionVector::getSubpart2D(double beginOffset, double endOffset) const {
    if (size() == 0) {
        return PositionVector();
    }
    PositionVector ret;
    Position begPos = front();
    if (beginOffset > POSITION_EPS) {
        begPos = positionAtOffset2D(beginOffset);
    }
    Position endPos = back();
    if (endOffset < length2D() - POSITION_EPS) {
        endPos = positionAtOffset2D(endOffset);
    }
    ret.push_back(begPos);

    double seen = 0;
    const_iterator i = begin();
    // skip previous segments
    while ((i + 1) != end()
            &&
            seen + (*i).distanceTo2D(*(i + 1)) < beginOffset) {
        seen += (*i).distanceTo2D(*(i + 1));
        i++;
    }
    // append segments in between
    while ((i + 1) != end()
            &&
            seen + (*i).distanceTo2D(*(i + 1)) < endOffset) {

        ret.push_back_noDoublePos(*(i + 1));
        seen += (*i).distanceTo2D(*(i + 1));
        i++;
    }
    // append end
    ret.push_back_noDoublePos(endPos);
    if (ret.size() == 1) {
        ret.push_back(endPos);
    }
    return ret;
}


PositionVector
PositionVector::getSubpartByIndex(int beginIndex, int count) const {
    if (size() == 0) {
        return PositionVector();
    }
    if (beginIndex < 0) {
        beginIndex += (int)size();
    }
    assert(count >= 0);
    assert(beginIndex < (int)size());
    assert(beginIndex + count <= (int)size());
    PositionVector result;
    for (int i = beginIndex; i < beginIndex + count; ++i) {
        result.push_back((*this)[i]);
    }
    return result;
}


double
PositionVector::beginEndAngle() const {
    if (size() == 0) {
        return INVALID_DOUBLE;
    }
    return front().angleTo2D(back());
}


double
PositionVector::nearest_offset_to_point2D(const Position& p, bool perpendicular) const {
    if (size() == 0) {
        return INVALID_DOUBLE;
    }
    double minDist = std::numeric_limits<double>::max();
    double nearestPos = GeomHelper::INVALID_OFFSET;
    double seen = 0;
    for (const_iterator i = begin(); i != end() - 1; i++) {
        const double pos =
            GeomHelper::nearest_offset_on_line_to_point2D(*i, *(i + 1), p, perpendicular);
        const double dist = pos == GeomHelper::INVALID_OFFSET ? minDist : p.distanceTo2D(positionAtOffset2D(*i, *(i + 1), pos));
        if (dist < minDist) {
            nearestPos = pos + seen;
            minDist = dist;
        }
        if (perpendicular && i != begin() && pos == GeomHelper::INVALID_OFFSET) {
            // even if perpendicular is set we still need to check the distance to the inner points
            const double cornerDist = p.distanceTo2D(*i);
            if (cornerDist < minDist) {
                const double pos1 =
                    GeomHelper::nearest_offset_on_line_to_point2D(*(i - 1), *i, p, false);
                const double pos2 =
                    GeomHelper::nearest_offset_on_line_to_point2D(*i, *(i + 1), p, false);
                if (pos1 == (*(i - 1)).distanceTo2D(*i) && pos2 == 0.) {
                    nearestPos = seen;
                    minDist = cornerDist;
                }
            }
        }
        seen += (*i).distanceTo2D(*(i + 1));
    }
    return nearestPos;
}


double
PositionVector::nearest_offset_to_point25D(const Position& p, bool perpendicular) const {
    if (size() == 0) {
        return INVALID_DOUBLE;
    }
    double minDist = std::numeric_limits<double>::max();
    double nearestPos = GeomHelper::INVALID_OFFSET;
    double seen = 0;
    for (const_iterator i = begin(); i != end() - 1; i++) {
        const double pos =
            GeomHelper::nearest_offset_on_line_to_point2D(*i, *(i + 1), p, perpendicular);
        const double dist = pos == GeomHelper::INVALID_OFFSET ? minDist : p.distanceTo2D(positionAtOffset2D(*i, *(i + 1), pos));
        if (dist < minDist) {
            const double pos25D = pos * (*i).distanceTo(*(i + 1)) / (*i).distanceTo2D(*(i + 1));
            nearestPos = pos25D + seen;
            minDist = dist;
        }
        if (perpendicular && i != begin() && pos == GeomHelper::INVALID_OFFSET) {
            // even if perpendicular is set we still need to check the distance to the inner points
            const double cornerDist = p.distanceTo2D(*i);
            if (cornerDist < minDist) {
                const double pos1 =
                    GeomHelper::nearest_offset_on_line_to_point2D(*(i - 1), *i, p, false);
                const double pos2 =
                    GeomHelper::nearest_offset_on_line_to_point2D(*i, *(i + 1), p, false);
                if (pos1 == (*(i - 1)).distanceTo2D(*i) && pos2 == 0.) {
                    nearestPos = seen;
                    minDist = cornerDist;
                }
            }
        }
        seen += (*i).distanceTo(*(i + 1));
    }
    return nearestPos;
}


Position
PositionVector::transformToVectorCoordinates(const Position& p, bool extend) const {
    if (size() == 0) {
        return Position::INVALID;
    }
    // @toDo this duplicates most of the code in nearest_offset_to_point2D. It should be refactored
    if (extend) {
        PositionVector extended = *this;
        const double dist = 2 * distance2D(p);
        extended.extrapolate(dist);
        return extended.transformToVectorCoordinates(p) - Position(dist, 0);
    }
    double minDist = std::numeric_limits<double>::max();
    double nearestPos = -1;
    double seen = 0;
    int sign = 1;
    for (const_iterator i = begin(); i != end() - 1; i++) {
        const double pos =
            GeomHelper::nearest_offset_on_line_to_point2D(*i, *(i + 1), p, true);
        const double dist = pos < 0 ? minDist : p.distanceTo2D(positionAtOffset(*i, *(i + 1), pos));
        if (dist < minDist) {
            nearestPos = pos + seen;
            minDist = dist;
            sign = isLeft(*i, *(i + 1), p) >= 0 ? -1 : 1;
        }
        if (i != begin() && pos == GeomHelper::INVALID_OFFSET) {
            // even if perpendicular is set we still need to check the distance to the inner points
            const double cornerDist = p.distanceTo2D(*i);
            if (cornerDist < minDist) {
                const double pos1 =
                    GeomHelper::nearest_offset_on_line_to_point2D(*(i - 1), *i, p, false);
                const double pos2 =
                    GeomHelper::nearest_offset_on_line_to_point2D(*i, *(i + 1), p, false);
                if (pos1 == (*(i - 1)).distanceTo2D(*i) && pos2 == 0.) {
                    nearestPos = seen;
                    minDist = cornerDist;
                    sign = isLeft(*(i - 1), *i, p) >= 0 ? -1 : 1;
                }
            }
        }
        seen += (*i).distanceTo2D(*(i + 1));
    }
    if (nearestPos != -1) {
        return Position(nearestPos, sign * minDist);
    } else {
        return Position::INVALID;
    }
}


int
PositionVector::indexOfClosest(const Position& p) const {
    if (size() == 0) {
        return -1;
    }
    double minDist = std::numeric_limits<double>::max();
    double dist;
    int closest = 0;
    for (int i = 0; i < (int)size(); i++) {
        dist = p.distanceTo((*this)[i]);
        if (dist < minDist) {
            closest = i;
            minDist = dist;
        }
    }
    return closest;
}


int
PositionVector::insertAtClosest(const Position& p, bool interpolateZ) {
    if (size() == 0) {
        return -1;
    }
    double minDist = std::numeric_limits<double>::max();
    int insertionIndex = 1;
    for (int i = 0; i < (int)size() - 1; i++) {
        const double length = GeomHelper::nearest_offset_on_line_to_point2D((*this)[i], (*this)[i + 1], p, false);
        const Position& outIntersection = PositionVector::positionAtOffset2D((*this)[i], (*this)[i + 1], length);
        const double dist = p.distanceTo2D(outIntersection);
        if (dist < minDist) {
            insertionIndex = i + 1;
            minDist = dist;
        }
    }
    // check if we have to adjust Position Z
    if (interpolateZ) {
        // obtain previous and next Z
        const double previousZ = (begin() + (insertionIndex - 1))->z();
        const double nextZ = (begin() + insertionIndex)->z();
        // insert new position using x and y of p, and the new z
        insert(begin() + insertionIndex, Position(p.x(), p.y(), ((previousZ + nextZ) / 2.0)));
    } else {
        insert(begin() + insertionIndex, p);
    }
    return insertionIndex;
}


int
PositionVector::removeClosest(const Position& p) {
    if (size() == 0) {
        return -1;
    }
    double minDist = std::numeric_limits<double>::max();
    int removalIndex = 0;
    for (int i = 0; i < (int)size(); i++) {
        const double dist = p.distanceTo2D((*this)[i]);
        if (dist < minDist) {
            removalIndex = i;
            minDist = dist;
        }
    }
    erase(begin() + removalIndex);
    return removalIndex;
}


std::vector<double>
PositionVector::intersectsAtLengths2D(const PositionVector& other) const {
    std::vector<double> ret;
    if (other.size() == 0) {
        return ret;
    }
    for (const_iterator i = other.begin(); i != other.end() - 1; i++) {
        std::vector<double> atSegment = intersectsAtLengths2D(*i, *(i + 1));
        copy(atSegment.begin(), atSegment.end(), back_inserter(ret));
    }
    return ret;
}


std::vector<double>
PositionVector::intersectsAtLengths2D(const Position& lp1, const Position& lp2) const {
    std::vector<double> ret;
    if (size() == 0) {
        return ret;
    }
    double pos = 0;
    for (const_iterator i = begin(); i != end() - 1; i++) {
        const Position& p1 = *i;
        const Position& p2 = *(i + 1);
        double x, y, m;
        if (intersects(p1, p2, lp1, lp2, 0., &x, &y, &m)) {
            ret.push_back(Position(x, y).distanceTo2D(p1) + pos);
        }
        pos += p1.distanceTo2D(p2);
    }
    return ret;
}


void
PositionVector::extrapolate(const double val, const bool onlyFirst, const bool onlyLast) {
    if (size() > 0) {
        Position& p1 = (*this)[0];
        Position& p2 = (*this)[1];
        const Position offset = (p2 - p1) * (val / p1.distanceTo(p2));
        if (!onlyLast) {
            p1.sub(offset);
        }
        if (!onlyFirst) {
            if (size() == 2) {
                p2.add(offset);
            } else {
                const Position& e1 = (*this)[-2];
                Position& e2 = (*this)[-1];
                e2.sub((e1 - e2) * (val / e1.distanceTo(e2)));
            }
        }
    }
}


void
PositionVector::extrapolate2D(const double val, const bool onlyFirst) {
    if (size() > 0) {
        Position& p1 = (*this)[0];
        Position& p2 = (*this)[1];
        if (p1.distanceTo2D(p2) > 0) {
            const Position offset = (p2 - p1) * (val / p1.distanceTo2D(p2));
            p1.sub(offset);
            if (!onlyFirst) {
                if (size() == 2) {
                    p2.add(offset);
                } else {
                    const Position& e1 = (*this)[-2];
                    Position& e2 = (*this)[-1];
                    e2.sub((e1 - e2) * (val / e1.distanceTo2D(e2)));
                }
            }
        }
    }
}


PositionVector
PositionVector::reverse() const {
    PositionVector ret;
    for (const_reverse_iterator i = rbegin(); i != rend(); i++) {
        ret.push_back(*i);
    }
    return ret;
}


Position
PositionVector::sideOffset(const Position& beg, const Position& end, const double amount) {
    const double scale = amount / beg.distanceTo2D(end);
    return Position((beg.y() - end.y()) * scale, (end.x() - beg.x()) * scale);
}


void
PositionVector::move2side(double amount, double maxExtension) {
    if (size() < 2) {
        return;
    }
    removeDoublePoints(POSITION_EPS, true);
    if (length2D() == 0) {
        return;
    }
    PositionVector shape;
    for (int i = 0; i < static_cast<int>(size()); i++) {
        if (i == 0) {
            const Position& from = (*this)[i];
            const Position& to = (*this)[i + 1];
            if (from != to) {
                shape.push_back(from - sideOffset(from, to, amount));
            }
        } else if (i == static_cast<int>(size()) - 1) {
            const Position& from = (*this)[i - 1];
            const Position& to = (*this)[i];
            if (from != to) {
                shape.push_back(to - sideOffset(from, to, amount));
            }
        } else {
            const Position& from = (*this)[i - 1];
            const Position& me = (*this)[i];
            const Position& to = (*this)[i + 1];
            PositionVector fromMe(from, me);
            fromMe.extrapolate2D(me.distanceTo2D(to));
            const double extrapolateDev = fromMe[1].distanceTo2D(to);
            if (fabs(extrapolateDev) < POSITION_EPS) {
                // parallel case, just shift the middle point
                shape.push_back(me - sideOffset(from, to, amount));
            } else if (fabs(extrapolateDev - 2 * me.distanceTo2D(to)) < POSITION_EPS) {
                // counterparallel case, just shift the middle point
                PositionVector fromMe(from, me);
                fromMe.extrapolate2D(amount);
                shape.push_back(fromMe[1]);
            } else {
                Position offsets = sideOffset(from, me, amount);
                Position offsets2 = sideOffset(me, to, amount);
                PositionVector l1(from - offsets, me - offsets);
                PositionVector l2(me - offsets2, to - offsets2);
                Position meNew  = l1.intersectionPosition2D(l2[0], l2[1], maxExtension);
                if (meNew == Position::INVALID) {
                    throw InvalidArgument("no line intersection");
                }
                meNew = meNew + Position(0, 0, me.z());
                shape.push_back(meNew);
            }
            // copy original z value
            shape.back().set(shape.back().x(), shape.back().y(), me.z());
        }
    }
    *this = shape;
}


void
PositionVector::move2side(std::vector<double> amount, double maxExtension) {
    if (size() < 2) {
        return;
    }
    if (length2D() == 0) {
        return;
    }
    if (size() != amount.size()) {
        throw InvalidArgument("Numer of offsets (" + toString(amount.size())
                              + ") does not match number of points (" + toString(size()) + ")");
    }
    PositionVector shape;
    for (int i = 0; i < static_cast<int>(size()); i++) {
        if (i == 0) {
            const Position& from = (*this)[i];
            const Position& to = (*this)[i + 1];
            if (from != to) {
                shape.push_back(from - sideOffset(from, to, amount[i]));
            }
        } else if (i == static_cast<int>(size()) - 1) {
            const Position& from = (*this)[i - 1];
            const Position& to = (*this)[i];
            if (from != to) {
                shape.push_back(to - sideOffset(from, to, amount[i]));
            }
        } else {
            const Position& from = (*this)[i - 1];
            const Position& me = (*this)[i];
            const Position& to = (*this)[i + 1];
            PositionVector fromMe(from, me);
            fromMe.extrapolate2D(me.distanceTo2D(to));
            const double extrapolateDev = fromMe[1].distanceTo2D(to);
            if (fabs(extrapolateDev) < POSITION_EPS) {
                // parallel case, just shift the middle point
                shape.push_back(me - sideOffset(from, to, amount[i]));
            } else if (fabs(extrapolateDev - 2 * me.distanceTo2D(to)) < POSITION_EPS) {
                // counterparallel case, just shift the middle point
                PositionVector fromMe(from, me);
                fromMe.extrapolate2D(amount[i]);
                shape.push_back(fromMe[1]);
            } else {
                Position offsets = sideOffset(from, me, amount[i]);
                Position offsets2 = sideOffset(me, to, amount[i]);
                PositionVector l1(from - offsets, me - offsets);
                PositionVector l2(me - offsets2, to - offsets2);
                Position meNew  = l1.intersectionPosition2D(l2[0], l2[1], maxExtension);
                if (meNew == Position::INVALID) {
                    throw InvalidArgument("no line intersection");
                }
                meNew = meNew + Position(0, 0, me.z());
                shape.push_back(meNew);
            }
            // copy original z value
            shape.back().set(shape.back().x(), shape.back().y(), me.z());
        }
    }
    *this = shape;
}

double
PositionVector::angleAt2D(int pos) const {
    if ((pos + 1) < (int)size()) {
        return (*this)[pos].angleTo2D((*this)[pos + 1]);
    } else {
        return INVALID_DOUBLE;
    }
}


void
PositionVector::closePolygon() {
    if ((size() != 0) && ((*this)[0] != back())) {
        push_back((*this)[0]);
    }
}


std::vector<double>
PositionVector::distances(const PositionVector& s, bool perpendicular) const {
    std::vector<double> ret;
    const_iterator i;
    for (i = begin(); i != end(); i++) {
        const double dist = s.distance2D(*i, perpendicular);
        if (dist != GeomHelper::INVALID_OFFSET) {
            ret.push_back(dist);
        }
    }
    for (i = s.begin(); i != s.end(); i++) {
        const double dist = distance2D(*i, perpendicular);
        if (dist != GeomHelper::INVALID_OFFSET) {
            ret.push_back(dist);
        }
    }
    return ret;
}


double
PositionVector::distance2D(const Position& p, bool perpendicular) const {
    if (size() == 0) {
        return std::numeric_limits<double>::max();
    } else if (size() == 1) {
        return front().distanceTo(p);
    }
    const double nearestOffset = nearest_offset_to_point2D(p, perpendicular);
    if (nearestOffset == GeomHelper::INVALID_OFFSET) {
        return GeomHelper::INVALID_OFFSET;
    } else {
        return p.distanceTo2D(positionAtOffset2D(nearestOffset));
    }
}


void
PositionVector::push_front(const Position& p) {
    if (empty()) {
        push_back(p);
    } else {
        insert(begin(), p);
    }
}


void
PositionVector::pop_front() {
    if (empty()) {
        throw ProcessError("PositionVector is empty");
    } else {
        erase(begin());
    }
}


void
PositionVector::push_back_noDoublePos(const Position& p) {
    if (size() == 0 || !p.almostSame(back())) {
        push_back(p);
    }
}


void
PositionVector::push_front_noDoublePos(const Position& p) {
    if ((size() == 0) || !p.almostSame(front())) {
        push_front(p);
    }
}


void
PositionVector::insert_noDoublePos(const std::vector<Position>::iterator& at, const Position& p) {
    if (at == begin()) {
        push_front_noDoublePos(p);
    } else if (at == end()) {
        push_back_noDoublePos(p);
    } else {
        if (!p.almostSame(*at) && !p.almostSame(*(at - 1))) {
            insert(at, p);
        }
    }
}


bool
PositionVector::isClosed() const {
    return (size() >= 2) && ((*this)[0] == back());
}


bool
PositionVector::isNAN() const {
    // iterate over all positions and check if is NAN
    for (auto i = begin(); i != end(); i++) {
        if (i->isNAN()) {
            return true;
        }
    }
    // all ok, then return false
    return false;
}


void
PositionVector::removeDoublePoints(double minDist, bool assertLength) {
    if (size() > 1) {
        iterator last = begin();
        for (iterator i = begin() + 1; i != end() && (!assertLength || size() > 2);) {
            if (last->almostSame(*i, minDist)) {
                if (i + 1 == end()) {
                    // special case: keep the last point and remove the next-to-last
                    erase(last);
                    i = end();
                } else {
                    i = erase(i);
                }
            } else {
                last = i;
                ++i;
            }
        }
    }
}


bool
PositionVector::operator==(const PositionVector& v2) const {
    return static_cast<vp>(*this) == static_cast<vp>(v2);
}


bool
PositionVector::operator!=(const PositionVector& v2) const {
    return static_cast<vp>(*this) != static_cast<vp>(v2);
}

PositionVector
PositionVector::operator-(const PositionVector& v2) const {
    if (length() != v2.length()) {
        WRITE_ERROR("Trying to substract PositionVectors of different lengths.");
    }
    PositionVector pv;
    auto i1 = begin();
    auto i2 = v2.begin();
    while (i1 != end()) {
        pv.add(*i1 - *i2);
    }
    return pv;
}

PositionVector
PositionVector::operator+(const PositionVector& v2) const {
    if (length() != v2.length()) {
        WRITE_ERROR("Trying to substract PositionVectors of different lengths.");
    }
    PositionVector pv;
    auto i1 = begin();
    auto i2 = v2.begin();
    while (i1 != end()) {
        pv.add(*i1 + *i2);
    }
    return pv;
}

bool
PositionVector::hasElevation() const {
    if (size() < 2) {
        return false;
    }
    for (const_iterator i = begin(); i != end() - 1; i++) {
        if ((*i).z() != (*(i + 1)).z()) {
            return true;
        }
    }
    return false;
}


bool
PositionVector::intersects(const Position& p11, const Position& p12, const Position& p21, const Position& p22, const double withinDist, double* x, double* y, double* mu) {
    const double eps = std::numeric_limits<double>::epsilon();
    const double denominator = (p22.y() - p21.y()) * (p12.x() - p11.x()) - (p22.x() - p21.x()) * (p12.y() - p11.y());
    const double numera = (p22.x() - p21.x()) * (p11.y() - p21.y()) - (p22.y() - p21.y()) * (p11.x() - p21.x());
    const double numerb = (p12.x() - p11.x()) * (p11.y() - p21.y()) - (p12.y() - p11.y()) * (p11.x() - p21.x());
    /* Are the lines coincident? */
    if (fabs(numera) < eps && fabs(numerb) < eps && fabs(denominator) < eps) {
        double a1;
        double a2;
        double a3;
        double a4;
        double a = -1e12;
        if (p11.x() != p12.x()) {
            a1 = p11.x() < p12.x() ? p11.x() : p12.x();
            a2 = p11.x() < p12.x() ? p12.x() : p11.x();
            a3 = p21.x() < p22.x() ? p21.x() : p22.x();
            a4 = p21.x() < p22.x() ? p22.x() : p21.x();
        } else {
            a1 = p11.y() < p12.y() ? p11.y() : p12.y();
            a2 = p11.y() < p12.y() ? p12.y() : p11.y();
            a3 = p21.y() < p22.y() ? p21.y() : p22.y();
            a4 = p21.y() < p22.y() ? p22.y() : p21.y();
        }
        if (a1 <= a3 && a3 <= a2) {
            if (a4 < a2) {
                a = (a3 + a4) / 2;
            } else {
                a = (a2 + a3) / 2;
            }
        }
        if (a3 <= a1 && a1 <= a4) {
            if (a2 < a4) {
                a = (a1 + a2) / 2;
            } else {
                a = (a1 + a4) / 2;
            }
        }
        if (a != -1e12) {
            if (x != nullptr) {
                if (p11.x() != p12.x()) {
                    *mu = (a - p11.x()) / (p12.x() - p11.x());
                    *x = a;
                    *y = p11.y() + (*mu) * (p12.y() - p11.y());
                } else {
                    *x = p11.x();
                    *y = a;
                    if (p12.y() == p11.y()) {
                        *mu = 0;
                    } else {
                        *mu = (a - p11.y()) / (p12.y() - p11.y());
                    }
                }
            }
            return true;
        }
        return false;
    }
    /* Are the lines parallel */
    if (fabs(denominator) < eps) {
        return false;
    }
    /* Is the intersection along the segments */
    double mua = numera / denominator;
    /* reduce rounding errors for lines ending in the same point */
    if (fabs(p12.x() - p22.x()) < eps && fabs(p12.y() - p22.y()) < eps) {
        mua = 1.;
    } else {
        const double offseta = withinDist / p11.distanceTo2D(p12);
        const double offsetb = withinDist / p21.distanceTo2D(p22);
        const double mub = numerb / denominator;
        if (mua < -offseta || mua > 1 + offseta || mub < -offsetb || mub > 1 + offsetb) {
            return false;
        }
    }
    if (x != nullptr) {
        *x = p11.x() + mua * (p12.x() - p11.x());
        *y = p11.y() + mua * (p12.y() - p11.y());
        *mu = mua;
    }
    return true;
}


void
PositionVector::rotate2D(double angle) {
    const double s = sin(angle);
    const double c = cos(angle);
    for (int i = 0; i < (int)size(); i++) {
        const double x = (*this)[i].x();
        const double y = (*this)[i].y();
        const double z = (*this)[i].z();
        const double xnew = x * c - y * s;
        const double ynew = x * s + y * c;
        (*this)[i].set(xnew, ynew, z);
    }
}


PositionVector
PositionVector::simplified() const {
    PositionVector result = *this;
    bool changed = true;
    while (changed && result.size() > 3) {
        changed = false;
        for (int i = 0; i < (int)result.size(); i++) {
            const Position& p1 = result[i];
            const Position& p2 = result[(i + 2) % result.size()];
            const int middleIndex = (i + 1) % result.size();
            const Position& p0 = result[middleIndex];
            // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
            const double triangleArea2 = fabs((p2.y() - p1.y()) * p0.x() - (p2.x() - p1.x()) * p0.y() + p2.x() * p1.y()  - p2.y() * p1.x());
            const double distIK = p1.distanceTo2D(p2);
            if (distIK > NUMERICAL_EPS && triangleArea2 / distIK < NUMERICAL_EPS) {
                changed = true;
                result.erase(result.begin() + middleIndex);
                break;
            }
        }
    }
    return result;
}


PositionVector
PositionVector::getOrthogonal(const Position& p, double extend, bool before, double length, double deg) const {
    PositionVector result;
    PositionVector tmp = *this;
    tmp.extrapolate2D(extend);
    const double baseOffset = tmp.nearest_offset_to_point2D(p);
    if (baseOffset == GeomHelper::INVALID_OFFSET || size() < 2) {
        // fail
        return result;
    }
    Position base = tmp.positionAtOffset2D(baseOffset);
    const int closestIndex = tmp.indexOfClosest(base);
    const double closestOffset = tmp.offsetAtIndex2D(closestIndex);
    result.push_back(base);
    if (fabs(baseOffset - closestOffset) > NUMERICAL_EPS) {
        result.push_back(tmp[closestIndex]);
        if ((closestOffset < baseOffset) != before) {
            deg *= -1;
        }
    } else if (before) {
        // take the segment before closestIndex if possible
        if (closestIndex > 0) {
            result.push_back(tmp[closestIndex - 1]);
        } else {
            result.push_back(tmp[1]);
            deg *= -1;
        }
    } else {
        // take the segment after closestIndex if possible
        if (closestIndex < (int)size() - 1) {
            result.push_back(tmp[closestIndex + 1]);
        } else {
            result.push_back(tmp[-1]);
            deg *= -1;
        }
    }
    result = result.getSubpart2D(0, length);
    // rotate around base
    result.add(base * -1);
    result.rotate2D(DEG2RAD(deg));
    result.add(base);
    return result;
}


PositionVector
PositionVector::smoothedZFront(double dist) const {
    PositionVector result = *this;
    if (size() == 0) {
        return result;
    }
    const double z0 = (*this)[0].z();
    // the z-delta of the first segment
    const double dz = (*this)[1].z() - z0;
    // if the shape only has 2 points it is as smooth as possible already
    if (size() > 2 && dz != 0) {
        dist = MIN2(dist, length2D());
        // check wether we need to insert a new point at dist
        Position pDist = positionAtOffset2D(dist);
        int iLast = indexOfClosest(pDist);
        // prevent close spacing to reduce impact of rounding errors in z-axis
        if (pDist.distanceTo2D((*this)[iLast]) > POSITION_EPS * 20) {
            iLast = result.insertAtClosest(pDist, false);
        }
        double dist2 = result.offsetAtIndex2D(iLast);
        const double dz2 = result[iLast].z() - z0;
        double seen = 0;
        for (int i = 1; i < iLast; ++i) {
            seen += result[i].distanceTo2D(result[i - 1]);
            result[i].set(result[i].x(), result[i].y(), z0 + dz2 * seen / dist2);
        }
    }
    return result;

}


PositionVector
PositionVector::interpolateZ(double zStart, double zEnd) const {
    PositionVector result = *this;
    if (size() == 0) {
        return result;
    }
    result[0].setz(zStart);
    result[-1].setz(zEnd);
    const double dz = zEnd - zStart;
    const double length = length2D();
    double seen = 0;
    for (int i = 1; i < (int)size() - 1; ++i) {
        seen += result[i].distanceTo2D(result[i - 1]);
        result[i].setz(zStart + dz * seen / length);
    }
    return result;
}


PositionVector
PositionVector::resample(double maxLength) const {
    PositionVector result;
    if (maxLength == 0) {
        return result;
    }
    const double length = length2D();
    if (length < POSITION_EPS) {
        return result;
    }
    maxLength = length / ceil(length / maxLength);
    for (double pos = 0; pos <= length; pos += maxLength) {
        result.push_back(positionAtOffset2D(pos));
    }
    return result;
}


double
PositionVector::offsetAtIndex2D(int index) const {
    if (index < 0 || index >= (int)size()) {
        return GeomHelper::INVALID_OFFSET;
    }
    double seen = 0;
    for (int i = 1; i <= index; ++i) {
        seen += (*this)[i].distanceTo2D((*this)[i - 1]);
    }
    return seen;
}


double
PositionVector::getMaxGrade(double& maxJump) const {
    double result = 0;
    for (int i = 1; i < (int)size(); ++i) {
        const Position& p1 = (*this)[i - 1];
        const Position& p2 = (*this)[i];
        const double distZ = fabs(p1.z() - p2.z());
        const double dist2D = p1.distanceTo2D(p2);
        if (dist2D == 0) {
            maxJump = MAX2(maxJump, distZ);
        } else {
            result = MAX2(result, distZ / dist2D);
        }
    }
    return result;
}


PositionVector
PositionVector::bezier(int numPoints) {
    // inspired by David F. Rogers
    assert(size() < 33);
    static const double fac[33] = {
        1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 720.0, 5040.0, 40320.0, 362880.0, 3628800.0, 39916800.0, 479001600.0,
        6227020800.0, 87178291200.0, 1307674368000.0, 20922789888000.0, 355687428096000.0, 6402373705728000.0,
        121645100408832000.0, 2432902008176640000.0, 51090942171709440000.0, 1124000727777607680000.0,
        25852016738884976640000.0, 620448401733239439360000.0, 15511210043330985984000000.0,
        403291461126605635584000000.0, 10888869450418352160768000000.0, 304888344611713860501504000000.0,
        8841761993739701954543616000000.0, 265252859812191058636308480000000.0,
        8222838654177922817725562880000000.0, 263130836933693530167218012160000000.0
    };
    PositionVector ret;
    const int npts = (int)size();
    // calculate the points on the Bezier curve
    const double step = (double) 1.0 / (numPoints - 1);
    double t = 0.;
    Position prev;
    for (int i1 = 0; i1 < numPoints; i1++) {
        if ((1.0 - t) < 5e-6) {
            t = 1.0;
        }
        double x = 0., y = 0., z = 0.;
        for (int i = 0; i < npts; i++) {
            const double ti = (i == 0) ? 1.0 : pow(t, i);
            const double tni = (npts == i + 1) ? 1.0 : pow(1 - t, npts - i - 1);
            const double basis = fac[npts - 1] / (fac[i] * fac[npts - 1 - i]) * ti * tni;
            x += basis * at(i).x();
            y += basis * at(i).y();
            z += basis * at(i).z();
        }
        t += step;
        Position current(x, y, z);
        if (prev != current && !ISNAN(x) && !ISNAN(y) && !ISNAN(z)) {
            ret.push_back(current);
        }
        prev = current;
    }
    return ret;
}


/****************************************************************************/
