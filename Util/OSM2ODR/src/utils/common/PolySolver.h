/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    PolySolver.h
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    Fri, 06.12.2019
///
//
/****************************************************************************/
#pragma once
#include <tuple>

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PolySolver
 * @brief Utility functions for solving quadratic and cubic equations in real domain.
 */
class PolySolver {
public:

    /** @brief Solver of quadratic equation ax^2 + bx + c = 0
     *
     * Returns only real-valued roots.
     *
     * @param[in] a The coefficient of the quadratic term x^2
     * @param[in] b The coefficient of the linear term x
     * @param[in] c The coefficient of the constant term
     * @return The number of real roots and these real roots
     */
    static std::tuple<int, double, double> quadraticSolve(double a, double b, double c);

    /** @brief Solver of cubic equation ax^3 + bx^2 + cx + d = 0
     *
     * Returns only real-valued roots.
     *
     * @param[in] a The coefficient of the cubic term x^3
     * @param[in] b The coefficient of the quadratic term x^2
     * @param[in] c The coefficient of the linear term x
     * @param[in] d The coefficient of the constant term
     * @return The number of real roots and these real roots
     */
    static std::tuple<int, double, double, double> cubicSolve(double a, double b, double c, double d);

};
