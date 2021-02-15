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
/// @file    GNEHierarchicalContainer.h
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2020
///
// Container for GNEHierarchicalElements
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class declarations
// ===========================================================================

class GNENetworkElement;
class GNEJunction;
class GNEEdge;
class GNELane;
class GNEAdditional;
class GNEShape;
class GNETAZElement;
class GNEDemandElement;
class GNEGenericData;
class GNEHierarchicalElement;

// ===========================================================================
// class definitions
// ===========================================================================

/// @brief Hierarchical container (used for keep myParent and myChildren
class GNEHierarchicalContainer {

public:
    /// @brief default constructor
    GNEHierarchicalContainer();

    /// @brief parameter constructor (only for parents)
    GNEHierarchicalContainer(
        const std::vector<GNEJunction*>& parentJunctions,
        const std::vector<GNEEdge*>& parentEdges,
        const std::vector<GNELane*>& parentLanes,
        const std::vector<GNEAdditional*>& parentAdditionals,
        const std::vector<GNEShape*>& parentShapes,
        const std::vector<GNETAZElement*>& parentTAZElements,
        const std::vector<GNEDemandElement*>& ParentDemandElements,
        const std::vector<GNEGenericData*>& parentGenericDatas);

    /// @brief get container size
    size_t getContainerSize() const;

    /// @brief add parent element
    template<typename T>
    void addParentElement(const GNEHierarchicalElement* hierarchicalElement, T* element);

    /// @brief remove parent element
    template<typename T>
    void removeParentElement(const GNEHierarchicalElement* hierarchicalElement, T* element);

    /// @brief add child element
    template<typename T>
    void addChildElement(const GNEHierarchicalElement* hierarchicalElement, T* element);

    /// @brief remove child element
    template<typename T>
    void removeChildElement(const GNEHierarchicalElement* hierarchicalElement, T* element);

    /// @brief get parents
    template<typename T>
    const T& getParents() const;

    /// @brief set parents
    template<typename T>
    void setParents(const T& newParents);

    /// @brief get children
    template<typename T>
    const T& getChildren() const;

    /// @brief set children
    template<typename T>
    void setChildren(const T& newChildren);

private:
    /// @brief vector of parent junctions
    std::vector<GNEJunction*> myParentJunctions;

    /// @brief vector of parent edges
    std::vector<GNEEdge*> myParentEdges;

    /// @brief vector of parent lanes
    std::vector<GNELane*> myParentLanes;

    /// @brief vector of parent additionals
    std::vector<GNEAdditional*> myParentAdditionals;

    /// @brief vector of parent shapes
    std::vector<GNEShape*> myParentShapes;

    /// @brief vector of parent TAZElements
    std::vector<GNETAZElement*> myParentTAZElements;

    /// @brief vector of parent demand elements
    std::vector<GNEDemandElement*> myParentDemandElements;

    /// @brief vector of parent generic datas
    std::vector<GNEGenericData*> myParentGenericDatas;

    /// @brief vector with the child junctions
    std::vector<GNEJunction*> myChildJunctions;

    /// @brief vector with the child edges
    std::vector<GNEEdge*> myChildEdges;

    /// @brief vector with the child lanes
    std::vector<GNELane*> myChildLanes;

    /// @brief vector with the child additional
    std::vector<GNEAdditional*> myChildAdditionals;

    /// @brief vector with the child lanes
    std::vector<GNEShape*> myChildShapes;

    /// @brief vector with the child TAZ Elements
    std::vector<GNETAZElement*> myChildTAZElements;

    /// @brief vector with the child demand elements
    std::vector<GNEDemandElement*> myChildDemandElements;

    /// @brief vector with the generic child data elements
    std::vector<GNEGenericData*> myChildGenericDatas;
};
