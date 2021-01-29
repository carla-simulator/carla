//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// author:      Christian Frank
// part of:     framework implementation developed by tkn

#pragma once

#include "veins/veins.h"

namespace veins {
class Coord;
}

#include "veins/base/utils/FWMath.h"

namespace veins {

/**
 * @brief Class for storing 3D coordinates.
 *
 * Some comparison and basic arithmetic operators are implemented.
 *
 * @ingroup utils
 * @author Christian Frank
 */
class VEINS_API Coord : public cObject {
public:
    /** @brief Constant with all values set to 0. */
    static const Coord ZERO;

public:
    /** @name x, y and z coordinate of the position. */
    /*@{*/
    double x;
    double y;
    double z;
    /*@}*/

private:
    void copy(const Coord& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
    }

public:
    /** @brief Default constructor. */
    Coord()
        : x(0.0)
        , y(0.0)
        , z(0.0)
    {
    }

    /** @brief Initializes a coordinate. */
    Coord(double x, double y, double z = 0.0)
        : x(x)
        , y(y)
        , z(z)
    {
    }

    /** @brief Initializes coordinate from other coordinate. */
    Coord(const Coord& other)
        : cObject(other)
    {
        copy(other);
    }

    /** @brief Returns a string with the value of the coordinate. */
    std::string info() const override;

    /** @brief Adds two coordinate vectors. */
    friend Coord operator+(const Coord& a, const Coord& b)
    {
        Coord tmp(a);
        tmp += b;
        return tmp;
    }

    /** @brief Subtracts two coordinate vectors. */
    friend Coord operator-(const Coord& a, const Coord& b)
    {
        Coord tmp(a);
        tmp -= b;
        return tmp;
    }

    /** @brief Multiplies a coordinate vector by a real number. */
    friend Coord operator*(const Coord& a, double f)
    {
        Coord tmp(a);
        tmp *= f;
        return tmp;
    }

    /**
     * @brief Multiplies a coordinate vector by another coordinate vector;
     * @return Scalar value
     * in Algebra: referred to as "dot product" or "scalar product";
     * Takes two equal-length sequences of numbers (usually coordinate vectors) and returns a single number.
     * in Euclidean geometry: the dot product of the Cartesian coordinates of two vectors (i.e. inner product).
     */
    friend double operator*(const Coord& a, const Coord& b)
    {
        return (a.x * b.x + a.y * b.y);
    }

    /**
     * @brief Returns the magnitude of the vector that would result from a regular 3D cross product
     * of the input vectors; The values on the z-plane are assumed to be 0 (i.e. treating the 2D space as a plane in the 3D space).
     * The 3D cross product will be perpendicular to that plane, and thus have 0 X & Y components
     * (thus the scalar returned is the Z value of the 3D cross product vector).
     */
    double twoDimensionalCrossProduct(const Coord& a) const
    {
        return (x * a.y - y * a.x);
    }

    /** @brief Divides a coordinate vector by a real number. */
    friend Coord operator/(const Coord& a, double f)
    {
        Coord tmp(a);
        tmp /= f;
        return tmp;
    }

    /**
     * @brief Multiplies this coordinate vector by a real number.
     */
    Coord& operator*=(double f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    /**
     * @brief Divides this coordinate vector by a real number.
     */
    Coord& operator/=(double f)
    {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    /**
     * @brief Adds coordinate vector 'a' to this.
     */
    Coord& operator+=(const Coord& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }

    /**
     * @brief Assigns coordinate vector 'other' to this.
     *
     * This operator can change the dimension of the coordinate.
     */
    Coord& operator=(const Coord& other)
    {
        if (this == &other) return *this;
        cObject::operator=(other);
        copy(other);
        return *this;
    }

    /**
     * @brief Subtracts coordinate vector 'a' from this.
     */
    Coord& operator-=(const Coord& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        return *this;
    }

    /**
     * @brief Tests whether two coordinate vectors are equal.
     *
     * Because coordinates are floating point values, this is done using an epsilon comparison.
     * @see math::almost_equal
     *
     */
    friend bool operator==(const Coord& a, const Coord& b)
    {
        // FIXME: this implementation is not transitive
        return math::almost_equal(a.x, b.x) && math::almost_equal(a.y, b.y) && math::almost_equal(a.z, b.z);
    }

    /**
     * @brief Tests whether two coordinate vectors are not equal.
     *
     * Negation of the operator==.
     */
    friend bool operator!=(const Coord& a, const Coord& b)
    {
        return !(a == b);
    }

    /**
     * @brief Returns the distance to Coord 'a'.
     */
    double distance(const Coord& a) const
    {
        Coord dist(*this - a);
        return dist.length();
    }

    /**
     * @brief Returns distance^2 to Coord 'a' (omits calling square root).
     */
    double sqrdist(const Coord& a) const
    {
        Coord dist(*this - a);
        return dist.squareLength();
    }

    /**
     * @brief Returns the squared distance on a torus of this to Coord 'b' (omits calling square root).
     */
    double sqrTorusDist(const Coord& b, const Coord& size) const;

    /**
     * @brief Returns the square of the length of this Coords position vector.
     */
    double squareLength() const
    {
        return x * x + y * y + z * z;
    }

    /**
     * @brief Returns the length of this Coords position vector.
     */
    double length() const
    {
        return sqrt(squareLength());
    }

    /**
     * @brief Checks if this coordinate is inside a specified rectangle.
     *
     * @param lowerBound The upper bound of the rectangle.
     * @param upperBound The lower bound of the rectangle.
     */
    bool isInBoundary(const Coord& lowerBound, const Coord& upperBound) const
    {
        return lowerBound.x <= x && x <= upperBound.x && lowerBound.y <= y && y <= upperBound.y && lowerBound.z <= z && z <= upperBound.z;
    }

    /**
     * @brief Returns the minimal coordinates.
     */
    Coord min(const Coord& a)
    {
        return Coord(this->x < a.x ? this->x : a.x, this->y < a.y ? this->y : a.y, this->z < a.z ? this->z : a.z);
    }

    /**
     * @brief Returns the maximal coordinates.
     */
    Coord max(const Coord& a)
    {
        return Coord(this->x > a.x ? this->x : a.x, this->y > a.y ? this->y : a.y, this->z > a.z ? this->z : a.z);
    }

    /**
     * Returns this coord when rotated around z axis (i.e., yaw).
     *
     * Coord(0,1,0) rotated by -90 degrees is Coord(1,0,0)
     *
     * @param rad: angle to rotate by (in rad)
     */
    Coord rotatedYaw(double rad) const
    {
        return Coord(x * cos(rad) - y * sin(rad), x * sin(rad) + y * cos(rad), z);
    }

    /**
     * @brief Returns this coord when after inverting y axis
     */
    Coord flippedY() const
    {
        return Coord(x, -y, z);
    }

    /**
     * @brief Return a new coord with the z coordinate set to newZ
     */
    Coord atZ(double newZ) const
    {
        return Coord(x, y, newZ);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Coord& coord)
{
    return os << "(" << coord.x << "," << coord.y << "," << coord.z << ")";
}

inline std::string Coord::info() const
{
    std::stringstream os;
    os << *this;
    return os.str();
}

} // namespace veins
