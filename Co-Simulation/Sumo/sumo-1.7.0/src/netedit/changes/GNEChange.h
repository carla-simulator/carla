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
/// @file    GNEChange.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// The reification of a NETEDIT editing operation (see command pattern)
// inherits from FXCommand and is used to for undo/redo
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBNode.h>
#include <netedit/elements/GNEHierarchicalContainer.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNELane.h>
#include <netedit/elements/additional/GNEAdditional.h>
#include <netedit/elements/additional/GNEShape.h>
#include <netedit/elements/additional/GNETAZElement.h>
#include <netedit/elements/demand/GNEDemandElement.h>
#include <netedit/elements/data/GNEGenericData.h>
#include <utils/foxtools/fxexdefs.h>
#include <utils/geom/PositionVector.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GNEHierarchicalElement;
class GNEAttributeCarrier;
class GNEAdditional;
class GNEDataSet;
class GNEDemandElement;
class GNEGenericData;
class GNEDataInterval;
class GNEEdge;
class GNELane;
class GNEShape;
class GNETAZElement;
class GNENet;
class GNEViewNet;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange
 * @brief the function-object for an editing operation (abstract base)
 */
class GNEChange : public FXCommand {
    FXDECLARE_ABSTRACT(GNEChange)

public:
    /**@brief Constructor
     * @param[in] forward The direction of this change
     * @param[in] selectedElement flag to mark if element is selected
     */
    GNEChange(bool forward, const bool selectedElement);

    /**@brief Constructor
     * @param[in] element hierarchical element
     * @param[in] forward The direction of this change
     * @param[in] selectedElement flag to mark if element is selected
     */
    GNEChange(GNEHierarchicalElement* element, bool forward, const bool selectedElement);

    /// @brief Destructor
    ~GNEChange();

    /// @brief return actual size
    virtual FXuint size() const;

    /// @brief return undoName
    virtual FXString undoName() const;

    /// @brief return rendoName
    virtual FXString redoName() const;

    /// @brief undo action/operation
    virtual void undo();

    /// @brief redo action/operation
    virtual void redo();

protected:
    /// @brief restore container (only use in undo() function)
    void restoreHierarchicalContainers();

    /// @brief add given element into parents and children (only use in redo() function)
    template<typename T>
    void addElementInParentsAndChildren(T* element) {
        // add element in parents
        for (const auto& junction : myOriginalHierarchicalContainer.getParents<std::vector<GNEJunction*> >()) {
            junction->addChildElement(element);
        }
        for (const auto& edge : myOriginalHierarchicalContainer.getParents<std::vector<GNEEdge*> >()) {
            edge->addChildElement(element);
        }
        for (const auto& lane : myOriginalHierarchicalContainer.getParents<std::vector<GNELane*> >()) {
            lane->addChildElement(element);
        }
        for (const auto& additional : myOriginalHierarchicalContainer.getParents<std::vector<GNEAdditional*> >()) {
            additional->addChildElement(element);
        }
        for (const auto& shape : myOriginalHierarchicalContainer.getParents<std::vector<GNEShape*> >()) {
            shape->addChildElement(element);
        }
        for (const auto& TAZElement : myOriginalHierarchicalContainer.getParents<std::vector<GNETAZElement*> >()) {
            TAZElement->addChildElement(element);
        }
        for (const auto& demandElement : myOriginalHierarchicalContainer.getParents<std::vector<GNEDemandElement*> >()) {
            demandElement->addChildElement(element);
        }
        for (const auto& genericData : myOriginalHierarchicalContainer.getParents<std::vector<GNEGenericData*> >()) {
            genericData->addChildElement(element);
        }
        // add element in children
        for (const auto& junction : myOriginalHierarchicalContainer.getChildren<std::vector<GNEJunction*> >()) {
            junction->addParentElement(element);
        }
        for (const auto& edge : myOriginalHierarchicalContainer.getChildren<std::vector<GNEEdge*> >()) {
            edge->addParentElement(element);
        }
        for (const auto& lane : myOriginalHierarchicalContainer.getChildren<std::vector<GNELane*> >()) {
            lane->addParentElement(element);
        }
        for (const auto& additional : myOriginalHierarchicalContainer.getChildren<std::vector<GNEAdditional*> >()) {
            additional->addParentElement(element);
        }
        for (const auto& shape : myOriginalHierarchicalContainer.getChildren<std::vector<GNEShape*> >()) {
            shape->addParentElement(element);
        }
        for (const auto& TAZElement : myOriginalHierarchicalContainer.getChildren<std::vector<GNETAZElement*> >()) {
            TAZElement->addParentElement(element);
        }
        for (const auto& demandElement : myOriginalHierarchicalContainer.getChildren<std::vector<GNEDemandElement*> >()) {
            demandElement->addParentElement(element);
        }
        for (const auto& genericData : myOriginalHierarchicalContainer.getChildren<std::vector<GNEGenericData*> >()) {
            genericData->addParentElement(element);
        }
    }

    /// @brief remove given element from parents and children (only use in redo() function)
    template<typename T>
    void removeElementFromParentsAndChildren(T* element) {
        // Remove element from parents
        for (const auto& junction : myOriginalHierarchicalContainer.getParents<std::vector<GNEJunction*> >()) {
            junction->removeChildElement(element);
        }
        for (const auto& edge : myOriginalHierarchicalContainer.getParents<std::vector<GNEEdge*> >()) {
            edge->removeChildElement(element);
        }
        for (const auto& lane : myOriginalHierarchicalContainer.getParents<std::vector<GNELane*> >()) {
            lane->removeChildElement(element);
        }
        for (const auto& additional : myOriginalHierarchicalContainer.getParents<std::vector<GNEAdditional*> >()) {
            additional->removeChildElement(element);
        }
        for (const auto& shape : myOriginalHierarchicalContainer.getParents<std::vector<GNEShape*> >()) {
            shape->removeChildElement(element);
        }
        for (const auto& TAZElement : myOriginalHierarchicalContainer.getParents<std::vector<GNETAZElement*> >()) {
            TAZElement->removeChildElement(element);
        }
        for (const auto& demandElement : myOriginalHierarchicalContainer.getParents<std::vector<GNEDemandElement*> >()) {
            demandElement->removeChildElement(element);
        }
        for (const auto& genericData : myOriginalHierarchicalContainer.getParents<std::vector<GNEGenericData*> >()) {
            genericData->removeChildElement(element);
        }
        // Remove element from children
        for (const auto& junction : myOriginalHierarchicalContainer.getChildren<std::vector<GNEJunction*> >()) {
            junction->removeParentElement(element);
        }
        for (const auto& edge : myOriginalHierarchicalContainer.getChildren<std::vector<GNEEdge*> >()) {
            edge->removeParentElement(element);
        }
        for (const auto& lane : myOriginalHierarchicalContainer.getChildren<std::vector<GNELane*> >()) {
            lane->removeParentElement(element);
        }
        for (const auto& additional : myOriginalHierarchicalContainer.getChildren<std::vector<GNEAdditional*> >()) {
            additional->removeParentElement(element);
        }
        for (const auto& shape : myOriginalHierarchicalContainer.getChildren<std::vector<GNEShape*> >()) {
            shape->removeParentElement(element);
        }
        for (const auto& TAZElement : myOriginalHierarchicalContainer.getChildren<std::vector<GNETAZElement*> >()) {
            TAZElement->removeParentElement(element);
        }
        for (const auto& demandElement : myOriginalHierarchicalContainer.getChildren<std::vector<GNEDemandElement*> >()) {
            demandElement->removeParentElement(element);
        }
        for (const auto& genericData : myOriginalHierarchicalContainer.getChildren<std::vector<GNEGenericData*> >()) {
            genericData->removeParentElement(element);
        }
    }

    /// @brief we group antagonistic commands (create junction/delete junction) and keep them apart by this flag
    bool myForward;

    /// @brief flag for check if element is selected
    const bool mySelectedElement;

    /// @brief Hierarchical container with parent and children
    const GNEHierarchicalContainer myOriginalHierarchicalContainer;

    /// @brief map with hierarchical container of all parent and children elements
    std::map<GNEHierarchicalElement*, GNEHierarchicalContainer> myHierarchicalContainers;
};
