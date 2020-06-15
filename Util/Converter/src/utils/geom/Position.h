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
/// @file    Position.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Axel Wegener
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A position in the 2D- or 3D-world
/****************************************************************************/
#pragma once
#include <iostream>
#include <cmath>

#include <config.h>

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Position
 * @brief A point in 2D or 3D with translation and scaling methods.
 */
class Position {
public:
    /// @brief default constructor
    Position() :
        myX(0.0), myY(0.0), myZ(0.0) { }

    /// @brief Parametrised constructor (only for x-y)
    Position(double x, double y) :
        myX(x), myY(y), myZ(0) { }

    /// @brief Parametrised constructor
    Position(double x, double y, double z) :
        myX(x), myY(y), myZ(z) { }

    /// @brief Destructor
    ~Position() { }

    /// @brief Returns the x-position
    inline double x() const {
        return myX;
    }

    /// @brief Returns the y-position
    inline double y() const {
        return myY;
    }

    /// @brief Returns the z-position
    inline double z() const {
        return myZ;
    }

    /// @brief set position x
    void setx(double x) {
        myX = x;
    }

    /// @brief set position y
    void sety(double y) {
        myY = y;
    }

    /// @brief set position z
    void setz(double z) {
        myZ = z;
    }

    /// @brief set positions x and y
    void set(double x, double y) {
        myX = x;
        myY = y;
    }

    /// @brief set positions x, y and z
    void set(double x, double y, double z) {
        myX = x;
        myY = y;
        myZ = z;
    }

    /// @brief set position with another position
    void set(const Position& pos) {
        myX = pos.myX;
        myY = pos.myY;
        myZ = pos.myZ;
    }

    /// @brief Multiplies both positions with the given value
    void mul(double val) {
        myX *= val;
        myY *= val;
        myZ *= val;
    }

    /// @brief Multiplies position with the given values
    void mul(double mx, double my) {
        myX *= mx;
        myY *= my;
    }

    /// @brief Multiplies position with the given values
    void mul(double mx, double my, double mz) {
        myX *= mx;
        myY *= my;
        myZ *= mz;
    }

    /// @brief Adds the given position to this one
    void add(const Position& pos) {
        myX += pos.myX;
        myY += pos.myY;
        myZ += pos.myZ;
    }

    /// @brief Adds the given position to this one
    void add(double dx, double dy) {
        myX += dx;
        myY += dy;
    }

    /// @brief Adds the given position to this one
    void add(double dx, double dy, double dz) {
        myX += dx;
        myY += dy;
        myZ += dz;
    }

    /// @brief Substracts the given position from this one
    void sub(double dx, double dy) {
        myX -= dx;
        myY -= dy;
    }

    /// @brief Substracts the given position from this one
    void sub(double dx, double dy, double dz) {
        myX -= dx;
        myY -= dy;
        myZ -= dz;
    }

    /// @brief Substracts the given position from this one
    void sub(const Position& pos) {
        myX -= pos.myX;
        myY -= pos.myY;
        myZ -= pos.myZ;
    }

    /// @brief
    void norm2d() {
        double val = sqrt(myX * myX + myY * myY);
        myX = myX / val;
        myY = myY / val;
    }

    /// @brief output operator
    friend std::ostream& operator<<(std::ostream& os, const Position& p) {
        os << p.x() << "," << p.y();
        if (p.z() != double(0.0)) {
            os << "," << p.z();
        }
        return os;
    }

    /// @brief add operator
    Position operator+(const Position& p2) const {
        return Position(myX + p2.myX,  myY + p2.myY, myZ + p2.myZ);
    }

    /// @brief sub operator
    Position operator-(const Position& p2) const {
        return Position(myX - p2.myX,  myY - p2.myY, myZ - p2.myZ);
    }

    /// @brief keep the direction but modify the length of the (location) vector to length * scalar
    Position operator*(double scalar) const {
        return Position(myX * scalar, myY * scalar, myZ * scalar);
    }

    /// @brief keep the direction but modify the length of the (location) vector to length + scalar
    Position operator+(double offset) const {
        const double length = distanceTo(Position(0, 0, 0));
        if (length == 0) {
            return *this;
        }
        const double scalar = (length + offset) / length;
        return Position(myX * scalar, myY * scalar, myZ * scalar);
    }

    /// @brief comparation operator
    bool operator==(const Position& p2) const {
        return myX == p2.myX && myY == p2.myY && myZ == p2.myZ;
    }

    /// @brief difference  operator
    bool operator!=(const Position& p2) const {
        return myX != p2.myX || myY != p2.myY || myZ != p2.myZ;
    }

    /// @brief lexicographical sorting for use in maps and sets
    bool operator<(const Position& p2) const {
        if (myX < p2.myX) {
            return true;
        } else if (myY < p2.myY) {
            return true;
        } else {
            return myZ < p2.myZ;
        }
    }

    /// @brief check if two position is almost the sme as other
    bool almostSame(const Position& p2, double maxDiv = POSITION_EPS) const {
        return distanceTo(p2) < maxDiv;
    }

    /// @brief returns the euclidean distance in 3 dimension
    inline double distanceTo(const Position& p2) const {
        return sqrt(distanceSquaredTo(p2));
    }

    /// @brief returns the square of the distance to another position
    inline double distanceSquaredTo(const Position& p2) const {
        return (myX - p2.myX) * (myX - p2.myX) + (myY - p2.myY) * (myY - p2.myY) + (myZ - p2.myZ) * (myZ - p2.myZ);
    }

    /// @brief returns the euclidean distance in the x-y-plane
    inline double distanceTo2D(const Position& p2) const {
        return sqrt(distanceSquaredTo2D(p2));
    }

    /// @brief returns the square of the distance to another position (Only using x and y positions)
    inline double distanceSquaredTo2D(const Position& p2) const {
        return (myX - p2.myX) * (myX - p2.myX) + (myY - p2.myY) * (myY - p2.myY);
    }

    /// @brief returns the angle in the plane of the vector pointing from here to the other position
    inline double angleTo2D(const Position& other) const {
        return atan2(other.myY - myY, other.myX - myX);
    }

    /// @brief returns the cross product between this point and the second one
    Position crossProduct(const Position& pos) {
        return Position(
                   myY * pos.myZ - myZ * pos.myY,
                   myZ * pos.myX - myX * pos.myZ,
                   myX * pos.myY - myY * pos.myX);
    }

    /// @brief returns the dot product (scalar product) between this point and the second one
    inline double dotProduct(const Position& pos) {
        return myX * pos.myX + myY * pos.myY + myZ * pos.myZ;
    }

    /// @brief rotate this position by rad around origin and return the result
    Position rotateAround2D(double rad, const Position& origin);

    /// @brief swap position X and Y
    void swapXY() {
        std::swap(myX, myY);
    }

    /// @brief check if position is NAN
    bool isNAN() const {
        return (std::isnan(myX) || std::isnan(myY) || std::isnan(myZ));
    }

    /// @brief used to indicate that a position is valid
    static const Position INVALID;

private:
    /// @brief  The x-position
    double myX;

    /// @brief  The y-position
    double myY;

    /// @brief  The z-position
    double myZ;
};
