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

//
// Support functions for mathematical operations
//

#include <cmath>
#include <limits>
#include <type_traits>

#include "veins/veins.h"

namespace veins {

/* Windows math.h doesn't define the the following variables: */
#ifndef M_E
#define M_E 2.7182818284590452354
#endif

#ifndef M_LOG2E
#define M_LOG2E 1.4426950408889634074
#endif

#ifndef M_LOG10E
#define M_LOG10E 0.43429448190325182765
#endif

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#ifndef M_LN10
#define M_LN10 2.30258509299404568402
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962
#endif

#ifndef M_1_PI
#define M_1_PI 0.31830988618379067154
#endif

#ifndef M_2_PI
#define M_2_PI 0.63661977236758134308
#endif

#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.12837916709551257390
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440
#endif

/* Constant for comparing doubles. Two doubles at most epsilon apart
   are declared equal.*/
#ifndef EPSILON
#define EPSILON 0.001
#endif

namespace math {

/**
 * Compare two floats for approximate equality
 *
 * Floating point numbers are prone to inequalities.
 * Expressions that logically result in the same values therefore often are not exactly equal, because two representations of very similar values are computed.
 * This function allows comparisons for such cases by allowing for a small error (epsilon), without considering numbers to be unequal.
 *
 * @param x First comparand
 * @param y Second comparand
 * @param ulp
 *      How many ULP's (Units in the Last Place) we want to tolerate when comparing two numbers.
 *      The larger the value, the more error we allow.
 *      The maximum floating point error on most platforms is below 0.5 ULP.
 *      A 0 value means that two numbers must be exactly the same to be considered equal.
 * @return Indicator whether the lhs and rhs are virtually equal
 */
template <class T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
almost_equal(T x, T y, int ulp = 1)
{
    const T epsilon = std::numeric_limits<T>::epsilon();
    const T delta = std::abs(x - y);
    return (delta <= epsilon * std::abs(x + y) * ulp) || (delta < std::numeric_limits<T>::min());
}

} // namespace math

/**
 * @brief Support functions for mathematical operations.
 *
 * This class contains all kind of mathematical support functions
 *
 * @ingroup basicUtils
 * @ingroup utils
 * @author Christian Frank
 */
class VEINS_API FWMath {

public:
    /**
     * Returns the rest of a whole-numbered division.
     */
    static double mod(double dividend, double divisor)
    {
        double i;
        return modf(dividend / divisor, &i) * divisor;
    }

    /**
     * Returns the result of a whole-numbered division.
     */
    static double div(double dividend, double divisor)
    {
        double i;
        modf(dividend / divisor, &i);
        return i;
    }

    /**
     * Returns the remainder r on division of dividend a by divisor n,
     * using floored division. The remainder r has the same sign as the divisor n.
     */
    static double modulo(double a, double n)
    {
        return (a - n * floor(a / n));
    }

    /**
     * Tests whether two doubles are close enough to be declared equal.
     * Returns true if parameters are at most epsilon apart, false
     * otherwise
     *
     * @deprecated
     * @see veins::math::almost_equal
     */
    VEINS_DEPRECATED static bool close(double one, double two)
    {
        return fabs(one - two) < EPSILON;
    }

    /**
     * Returns 0 if i is close to 0, 1 if i is positive and greater than epsilon,
     * or -1 if it is negative and less than epsilon.
     */
    static int stepfunction(double i)
    {
        if (math::almost_equal<double>(i, 0)) {
            return 0;
        }

        if (std::signbit(i) == 1) {
            return -1;
        }
        else {
            return 1;
        }
    };

    /**
     * Returns 1 if the parameter is greater or equal to zero, -1 otherwise
     */
    static int sign(double i)
    {
        return (i >= 0) ? 1 : -1;
    };

    /**
     * Returns an integer that corresponds to rounded double parameter
     */
    static int round(double d)
    {
        return (int) (ceil(d - 0.5));
    }

    /**
     * Discards the fractional part of the parameter, e.g. -3.8 becomes -3
     */
    static double floorToZero(double d)
    {
        return (d >= 0.0) ? floor(d) : ceil(d);
    }

    /**
     * Returns the greater of the given parameters
     */
    static double max(double a, double b)
    {
        return (a < b) ? b : a;
    }

    /**
     * Converts a dBm value into milliwatts
     */
    static double dBm2mW(double dBm)
    {
        return pow(10.0, dBm / 10.0);
    }

    /**
     * Convert a mW value to dBm.
     */
    static double mW2dBm(double mW)
    {
        return (10 * log10(mW));
    }
};

} // namespace veins
