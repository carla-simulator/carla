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
/// @file    GNEHierarchicalElement.h
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2020
///
// A abstract class for representation of hierarchical elements
/****************************************************************************/
#pragma once

#include <netedit/GNEGeometry.h>
#include <netedit/elements/GNEAttributeCarrier.h>

#include "GNEHierarchicalContainer.h"


// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEHierarchicalElements
 * @brief An special type of Attribute carrier that owns hierarchical elements
 */
class GNEHierarchicalElement : public GNEAttributeCarrier {

public:
    /// @brief declare GNEChange_Children as friend class
    friend class GNEChange_Children;
    friend class GNEDemandElement;

    /**@brief Constructor
     * @param[in] tag SUMO Tag assigned to this type of object
     * @param[in] net GNENet in which this AttributeCarrier is stored
     * @param[in] parentJunctions vector of parent junctions
     * @param[in] parentEdges vector of parent edges
     * @param[in] parentLanes vector of parent lanes
     * @param[in] parentAdditionals vector of parent additionals
     * @param[in] parentShapes vector of parent shapes
     * @param[in] parentTAZElements vector of parent TAZs
     * @param[in] parentDemandElements vector of parent demand elements
     * @param[in] parentGenericData vector of parent generic data elements
     */
    GNEHierarchicalElement(GNENet* net, SumoXMLTag tag,
                           const std::vector<GNEJunction*>& parentJunctions,
                           const std::vector<GNEEdge*>& parentEdges,
                           const std::vector<GNELane*>& parentLanes,
                           const std::vector<GNEAdditional*>& parentAdditionals,
                           const std::vector<GNEShape*>& parentShapes,
                           const std::vector<GNETAZElement*>& parentTAZElements,
                           const std::vector<GNEDemandElement*>& parentDemandElements,
                           const std::vector<GNEGenericData*>& parentGenericDatas);

    /// @brief Destructor
    ~GNEHierarchicalElement();

    /// @name Functions related with geometry of element
    /// @{
    /// @brief update pre-computed geometry information
    virtual void updateGeometry() = 0;

    /// @brief Returns position of hierarchical element in view
    virtual Position getPositionInView() const = 0;
    /// @}

    /// @brief get hierarchicalcontainer with parents and children
    const GNEHierarchicalContainer& getHierarchicalContainer() const;

    /// @brief restore hierarchical container
    void restoreHierarchicalContainer(const GNEHierarchicalContainer& container);

    /// @name common get functions
    /// @{

    /// @brief get all parents and children
    std::vector<GNEHierarchicalElement*> getAllHierarchicalElements() const;

    /// @brief get parent junctions
    const std::vector<GNEJunction*>& getParentJunctions() const;

    /// @brief get parent edges
    const std::vector<GNEEdge*>& getParentEdges() const;

    /// @brief get parent lanes
    const std::vector<GNELane*>& getParentLanes() const;

    /// @brief get parent additionals
    const std::vector<GNEAdditional*>& getParentAdditionals() const;

    /// @brief get parent shapes
    const std::vector<GNEShape*>& getParentShapes() const;

    /// @brief get parent TAZElements
    const std::vector<GNETAZElement*>& getParentTAZElements() const;

    /// @brief get parent demand elements
    const std::vector<GNEDemandElement*>& getParentDemandElements() const;

    /// @brief get parent demand elements
    const std::vector<GNEGenericData*>& getParentGenericDatas() const;

    /// @brief get child junctions
    const std::vector<GNEJunction*>& getChildJunctions() const;

    /// @brief get child edges
    const std::vector<GNEEdge*>& getChildEdges() const;

    /// @brief get child lanes
    const std::vector<GNELane*>& getChildLanes() const;

    /// @brief return child additionals
    const std::vector<GNEAdditional*>& getChildAdditionals() const;

    /// @brief get child shapes
    const std::vector<GNEShape*>& getChildShapes() const;

    /// @brief get child TAZElements
    const std::vector<GNETAZElement*>& getChildTAZElements() const;

    /// @brief return child demand elements
    const std::vector<GNEDemandElement*>& getChildDemandElements() const;

    /// @brief return child generic data elements
    const std::vector<GNEGenericData*>& getChildGenericDatas() const;
    /// @}

    /// @name common generic add/remove functions
    /// @{

    /// @brief add parent element
    template<typename T>
    void addParentElement(T* element);

    /// @brief remove parent element
    template<typename T>
    void removeParentElement(T* element);

    /// @brief add child element
    template<typename T>
    void addChildElement(T* element);

    /// @brief remove child element
    template<typename T>
    void removeChildElement(T* element);
    /// @}

    /// @name specific get functions
    /// @{

    /// @brief if use edge/parent lanes as a list of consecutive elements, obtain a list of IDs of elements after insert a new element
    std::string getNewListOfParents(const GNENetworkElement* currentElement, const GNENetworkElement* newNextElement) const;

    /// @}

    /// @brief update child connections
    void updateHierarchicalConnections();

    /// @brief Draw hierarchical connections between parent and children
    void drawHierarchicalConnections(const GUIVisualizationSettings& s, const GNEAttributeCarrier* AC, const double exaggeration) const;

    /// @brief check if children are overlapped (Used by Rerouters)
    bool checkChildAdditionalsOverlapping() const;

    /// @brief check if childs demand elements are overlapped
    bool checkChildDemandElementsOverlapping() const;

    /// @brief update parent after add or remove a child (can be reimplemented, for example used for statistics)
    virtual void updateParentAdditional();

protected:
    /// @brief replace parent elements
    template<typename T, typename U>
    void replaceParentElements(T* elementChild, const U& newParents) {
        // remove elementChild from parents
        for (const auto& parent : myHierarchicalContainer.getParents<U>()) {
            parent->removeChildElement(elementChild);
        }
        // set new parents junctions
        myHierarchicalContainer.setParents(newParents);
        // add elementChild into new parents
        for (const auto& parent : myHierarchicalContainer.getParents<U>()) {
            parent->addChildElement(elementChild);
        }
    }

    /// @brief replace child elements
    template<typename T, typename U>
    void replaceChildElements(T* elementChild, const U& newChildren) {
        // remove elementChild from childs
        for (const auto& child : myHierarchicalContainer.getChildren<U>()) {
            child->removeChildElement(elementChild);
        }
        // set new childs junctions
        myHierarchicalContainer.setChildren(newChildren);
        // add elementChild into new childs
        for (const auto& child : myHierarchicalContainer.getChildren<U>()) {
            child->addChildElement(elementChild);
        }
    }

    /// @brief hierarchical connections
    GNEGeometry::HierarchicalConnections myHierarchicalConnections;

private:
    /// @brief hierarchical container with parents and children
    GNEHierarchicalContainer myHierarchicalContainer;

    /// @brief Invalidated copy constructor.
    GNEHierarchicalElement(const GNEHierarchicalElement&) = delete;

    /// @brief Invalidated assignment operator.
    GNEHierarchicalElement& operator=(const GNEHierarchicalElement&) = delete;
};

