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
/// @file    GNEPathElements.h
/// @author  Pablo Alvarez Lopez
/// @date    May 2020
///
// A abstract class for representation of element paths
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/gui/globjects/GUIGlObjectTypes.h>

// ===========================================================================
// class declaration
// ===========================================================================

class GNEEdge;
class GNELane;
class GNEJunction;
class GNEAdditional;
class GNEDemandElement;
class GNEGenericData;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEPathElements
 * @brief An special type of Attribute carrier that owns hierarchical elements
 */
class GNEPathElements {

public:
    /// @brief path element
    class PathElement {

    public:
        /// @brief constructor for lanes
        PathElement(GNELane* lane);

        /// @brief update nextLane
        void updateNextLane(GNELane* lane);

        /// @brief get junction
        GNEJunction* getJunction() const;

        /// @brief get lane
        GNELane* getLane() const;

        /// @brief get next lane
        GNELane* getNextLane() const;

    protected:
        /// @brief lane
        GNELane* myLane;

        /// @brief nextLane
        GNELane* myNextLane;

    private:
        /// @brief default constructor
        PathElement();
    };

    /// @brief Constructor for additional elements
    GNEPathElements(GNEAdditional* additionalElement);

    /// @brief Constructor for demand elements
    GNEPathElements(GNEDemandElement* demandElement);

    /// @brief Constructor for generic datas
    GNEPathElements(GNEGenericData* genericData);

    /// @brief Destructor
    ~GNEPathElements();

    /// @brief get path edges
    const std::vector<GNEPathElements::PathElement>& getPath() const;

    /// @brief draw lane path child
    void drawLanePathChildren(const GUIVisualizationSettings& s, const GNELane* lane, const double offset) const;

    /// @brief draw junction path child
    void drawJunctionPathChildren(const GUIVisualizationSettings& s, const GNEJunction* junction, const double offset) const;

protected:
    /// @brief calculate path lanes (Dijkstra)
    void calculatePathLanes(SUMOVehicleClass vClass, const bool allowedVClass, GNELane* fromLane, GNELane* toLane, const std::vector<GNEEdge*>& viaEdges);

    /// @brief calculate consecutive path lanes (used by routes)
    void calculateConsecutivePathLanes(SUMOVehicleClass vClass, const bool allowedVClass, const std::vector<GNEEdge*>& edges);

    /// @brief calculate consecutive path lanes (used by E2Detectors)
    void calculateConsecutivePathLanes(const std::vector<GNELane*>& lanes);

    /// @brief reset path lanes
    void resetPathLanes(SUMOVehicleClass vClass, const bool allowedVClass, GNELane* fromLane, GNELane* toLane, const std::vector<GNEEdge*>& viaEdges);

    /// @brief calculate consecutive path lanes (used by genericdatas)
    void calculateGenericDataLanePath(const std::vector<GNEEdge*>& edges);

private:
    /// @brief pointer to additional element
    GNEAdditional* myAdditionalElement;

    /// @brief pointer to demand element
    GNEDemandElement* myDemandElement;

    /// @brief pointer to generic data
    GNEGenericData* myGenericData;

    /// @brief vector of edges used in paths
    std::vector<PathElement> myPathElements;

    /// @brief add elements
    void addElements();

    /// @brief remove elements
    void removeElements();

    /// @brief update path element
    void updatePathElements();

    /// @brief calculate from-via-to edges
    const std::vector<GNEEdge*> calculateFromViaToEdges(GNELane* fromLane, GNELane* toLane, const std::vector<GNEEdge*>& viaEdges);

    /// @brief default constructor
    GNEPathElements();

    /// @brief Invalidated copy constructor.
    GNEPathElements(const GNEPathElements&) = delete;

    /// @brief Invalidated assignment operator.
    GNEPathElements& operator=(const GNEPathElements&) = delete;
};

