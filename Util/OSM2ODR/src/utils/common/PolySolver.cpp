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
/// @file    PolySolver.cpp
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    Fri, 06.12.2019
///
//
/****************************************************************************/
#include <math.h>
#include <cmath>
#include <limits>
#include <utils/geom/GeomHelper.h>  // defines M_PI
#include "PolySolver.h"


// ===========================================================================
// static member variables
// ===========================================================================
// (none)

// ===========================================================================
// member method definitions
// ===========================================================================
std::tuple<int, double, double>
PolySolver::quadraticSolve(double a, double b, double c) {
    // ax^2 + bx + c = 0
    // return only real roots
    if (a == 0) {
        //WRITE_WARNING("The coefficient of the quadrat of x is 0. Please use the utility for a FIRST degree linear. No further action taken.");
        if (b == 0) {
            if (c == 0) {
                return std::make_tuple(2, std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
            } else {
                return std::make_tuple(0, NAN, NAN);
            }
        } else {
            return std::make_tuple(1, -c / b, NAN);
        }
    }

    if (c == 0) {
        //WRITE_WARNING("One root is 0. Now divide through by x and use the utility for a FIRST degree linear to solve the resulting equation for the other one root. No further action taken.");
        return std::make_tuple(2, 0, -b / a);
    }

    double disc, x1_real, x2_real ;
    disc = b * b - 4 * a * c;

    if (disc > 0) {
        // two different real roots
        x1_real = (-b + sqrt(disc)) / (2 * a);
        x2_real = (-b - sqrt(disc)) / (2 * a);
        return std::make_tuple(2, x1_real, x2_real);
    } else if (disc == 0) {
        // a real root with multiplicity 2
        x1_real = (-b + sqrt(disc)) / (2 * a);
        return std::make_tuple(1, x1_real, NAN);
    } else {
        // all roots are complex
        return std::make_tuple(0, NAN, NAN);
    }
}

std::tuple<int, double, double, double>
PolySolver::cubicSolve(double a, double b, double c, double d) {
    // ax^3 + bx^2 + cx + d = 0
    // return only real roots
    if (a == 0) {
        //WRITE_WARNING("The coefficient of the cube of x is 0. Please use the utility for a SECOND degree quadratic. No further action taken.");
        int numX;
        double x1, x2;
        std::tie(numX, x1, x2) = quadraticSolve(b, c, d);
        return std::make_tuple(numX, x1, x2, NAN);
    }
    if (d == 0)	{
        //WRITE_WARNING("One root is 0. Now divide through by x and use the utility for a SECOND degree quadratic to solve the resulting equation for the other two roots. No further action taken.");
        int numX;
        double x1, x2;
        std::tie(numX, x1, x2) = quadraticSolve(a, b, c);
        return std::make_tuple(numX + 1, 0, x1, x2);
    }

    b /= a;
    c /= a;
    d /= a;

    double disc, q, r, dum1, s, t, term1, r13;
    q = (3.0 * c - (b * b)) / 9.0;
    r = -(27.0 * d) + b * (9.0 * c - 2.0 * (b * b));
    r /= 54.0;
    disc = q * q * q + r * r;
    term1 = (b / 3.0);

    double x1_real, x2_real, x3_real;
    if (disc > 0) { // One root real, two are complex
        s = r + sqrt(disc);
        s = s < 0 ? -cbrt(-s) : cbrt(s);
        t = r - sqrt(disc);
        t = t < 0 ? -cbrt(-t) : cbrt(t);
        x1_real = -term1 + s + t;
        term1 += (s + t) / 2.0;
        x3_real = x2_real = -term1;
        return std::make_tuple(1, x1_real, NAN, NAN);
    }
    // The remaining options are all real
    else if (disc == 0) { // All roots real, at least two are equal.
        r13 = r < 0 ? -cbrt(-r) : cbrt(r);
        x1_real = -term1 + 2.0 * r13;
        x3_real = x2_real = -(r13 + term1);
        return std::make_tuple(2, x1_real, x2_real, NAN);
    }
    // Only option left is that all roots are real and unequal (to get here, q < 0)
    else {
        q = -q;
        dum1 = q * q * q;
        dum1 = acos(r / sqrt(dum1));
        r13 = 2.0 * sqrt(q);
        x1_real = -term1 + r13 * cos(dum1 / 3.0);
        x2_real = -term1 + r13 * cos((dum1 + 2.0 * M_PI) / 3.0);
        x3_real = -term1 + r13 * cos((dum1 + 4.0 * M_PI) / 3.0);
        return std::make_tuple(3, x1_real, x2_real, x3_real);
    }
}


/****************************************************************************/
