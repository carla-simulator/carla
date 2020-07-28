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
/// @file    Boundary.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A class that stores a 2D geometrical boundary
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <utility>

#include "AbstractPoly.h"
#include "Position.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Boundary
 * @brief A class that stores a 2D geometrical boundary
 */
class Boundary : public AbstractPoly {
public:
    /// @brief Constructor - the boundary is unset
    Boundary();

    /// @brief Constructor - the boundary will be build using the given values
    Boundary(double x1, double y1, double x2, double y2);

    /// @brief Constructor - the boundary will be build using the given values including Z
    Boundary(double x1, double y1, double z1, double x2, double y2, double z2);

    /// @brief Destructor
    ~Boundary();

    /// @brief Resets the boundary
    void reset();

    /// @brief Makes the boundary include the given coordinate
    void add(double x, double y, double z = 0);

    /// @brief Makes the boundary include the given coordinate
    void add(const Position& p);

    /// @brief Makes the boundary include the given boundary
    void add(const Boundary& p);

    /// @brief Returns the center of the boundary
    Position getCenter() const;

    /// @brief Returns minimum x-coordinate
    double xmin() const;

    /// @brief Returns maximum x-coordinate
    double xmax() const;

    /// @brief Returns minimum y-coordinate
    double ymin() const;

    /// @brief Returns maximum y-coordinate
    double ymax() const;

    /// @brief Returns minimum z-coordinate
    double zmin() const;

    /// @brief Returns maximum z-coordinate
    double zmax() const;

    /// @brief Returns the width of the boudary (x-axis)
    double getWidth() const;

    /// @brief Returns the height of the boundary (y-axis)
    double getHeight() const;

    /// @brief Returns the elevation range of the boundary (z-axis)
    double getZRange() const;

    /// @name inherited from AbstractPoly
    /// @{
    /// @brief Returns whether the boundary contains the given coordinate
    bool around(const Position& p, double offset = 0) const;

    /// @brief Returns whether the boundary overlaps with the given polygon
    bool overlapsWith(const AbstractPoly& poly, double offset = 0) const;

    /// @brief Returns whether the boundary is partially within the given polygon
    bool partialWithin(const AbstractPoly& poly, double offset = 0) const;

    /// @brief Returns whether the boundary crosses the given line
    bool crosses(const Position& p1, const Position& p2) const;
    /// @}

    /// @brief check if Boundary is Initialised
    bool isInitialised() const;

    /// @brief returns the euclidean distance in the x-y-plane
    double distanceTo2D(const Position& p) const;

    /// @brief returns the euclidean distance in the x-y-plane
    double distanceTo2D(const Boundary& b) const;

    /**@brief extends the boundary by the given amount
     * @return a reference to the instance for further use
     */
    Boundary& grow(double by);

    /// @brief Increases the width of the boundary (x-axis)
    void growWidth(double by);

    /// @brief Increases the height of the boundary (y-axis)
    void growHeight(double by);

    /// @brief flips ymin and ymax
    void flipY();

    /// @brief Sets the boundary to the given values
    void set(double xmin, double ymin, double xmax, double ymax);

    /// @brief Moves the boundary by the given amount
    void moveby(double x, double y, double z = 0);

    /// @brief Output operator
    friend std::ostream& operator<<(std::ostream& os, const Boundary& b);

    /// @brief Comparison operator equal
    bool operator==(const Boundary& b) const;

    /// @brief Comparison operator not equal
    bool operator!=(const Boundary& b) const;

private:
    /// @brief The boundaries
    double myXmin, myXmax, myYmin, myYmax, myZmin, myZmax;

    /// @brief Information whether the boundary was initialised
    bool myWasInitialised;
};
