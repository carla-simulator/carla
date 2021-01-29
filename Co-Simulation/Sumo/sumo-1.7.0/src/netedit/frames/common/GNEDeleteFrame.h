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
/// @file    GNEDeleteFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Dec 2016
///
// The Widget for remove network-elements
/****************************************************************************/
#pragma once

#include <netedit/frames/GNEFrame.h>

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEDeleteFrame
 * The Widget for deleting elements
 */
class GNEDeleteFrame : public GNEFrame {

public:

    // ===========================================================================
    // class DeleteOptions
    // ===========================================================================

    class DeleteOptions : protected FXGroupBox {

    public:
        /// @brief constructor
        DeleteOptions(GNEDeleteFrame* deleteFrameParent);

        /// @brief destructor
        ~DeleteOptions();

        /// @brief check if only delete geometry points checkbox is enabled
        bool deleteOnlyGeometryPoints() const;

        /// @brief check if protect additional elements checkbox is enabled
        bool protectAdditionals() const;

        /// @brief check if protect TAZ elements checkbox is enabled
        bool protectTAZs() const;

        /// @brief check if protect shapes elements checkbox is enabled
        bool protectShapes() const;

        /// @brief check if protect demand elements checkbox is enabled
        bool protectDemandElements() const;

        /// @brief check if protect generic datas checkbox is enabled
        bool protectGenericDatas() const;

    private:
        /// @brief checkbox for enable/disable delete only geometry points
        FXCheckButton* myDeleteOnlyGeometryPoints;

        /// @brief checkbox for enable/disable protect additionals
        FXCheckButton* myProtectAdditionals;

        /// @brief checkbox for enable/disable protect TAZs
        FXCheckButton* myProtectTAZs;

        /// @brief checkbox for enable/disable protect shapes
        FXCheckButton* myProtectShapes;

        /// @brief checkbox for enable/disable protect demand elements
        FXCheckButton* myProtectDemandElements;

        /// @brief checkbox for enable/disable protect generic datas
        FXCheckButton* myProtectGenericDatas;
    };

    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEDeleteFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEDeleteFrame();

    /// @brief show delete frame
    void show();

    /// @brief hide delete frame
    void hide();

    /// @brief remove selected attribute carriers (element)
    void removeSelectedAttributeCarriers();

    /**@brief remove attribute carrier (element)
     * @param objectsUnderCursor objects under cursors
     * @param ignoreOptions ignore delete options and ALWAYS remove AC
     */
    void removeAttributeCarrier(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, bool ignoreOptions = false);

    /// @brief get delete options
    DeleteOptions* getDeleteOptions() const;

protected:

    /// @brief struct for saving subordinated elements (Junction->Edge->Lane->(Additional | DemandElement)
    class SubordinatedElements {

    public:
        /// @brief constructor (for junctions)
        SubordinatedElements(const GNEJunction* junction);

        /// @brief constructor (for edges)
        SubordinatedElements(const GNEEdge* edge);

        /// @brief constructor (for lanes)
        SubordinatedElements(const GNELane* lane);

        /// @brief constructor (for additionals)
        SubordinatedElements(const GNEAdditional* additional);

        /// @brief constructor (for shapes)
        SubordinatedElements(const GNEShape* shape);

        /// @brief constructor (for demandElements)
        SubordinatedElements(const GNEDemandElement* demandElement);

        /// @brief constructor (for shapes)
        SubordinatedElements(const GNEGenericData* genericData);

        /// @brief destructor
        ~SubordinatedElements();

        /// @brief if element can be removed
        bool checkElements(const DeleteOptions* deleteOptions);

    protected:
        /// @brief parent of SubordinatedElements
        const GNEAttributeCarrier* myAttributeCarrier;

        /// @brief pointer to view net
        GNEViewNet* myViewNet;

        /// @brief parent additionals (except TAZs)
        size_t myAdditionalParents;

        /// @brief child additional (except TAZs)
        size_t myAdditionalChilds;

        /// @brief parent TAZs
        size_t myTAZParents;

        /// @brief child TAZ
        size_t myTAZChilds;

        /// @brief parent shapes
        size_t myShapeParents;

        /// @brief child shape
        size_t myShapeChilds;

        /// @brief parent demand elements
        size_t myDemandElementParents;

        /// @brief child demand elements
        size_t myDemandElementChilds;

        /// @brief parent demand elements
        size_t myGenericDataParents;

        /// @brief child demand elements
        size_t myGenericDataChilds;

    private:
        // default constructor for non-net elements
        SubordinatedElements(const GNEAttributeCarrier* attributeCarrier, GNEViewNet* viewNet);

        // default constructor for Net Elements
        SubordinatedElements(const GNEAttributeCarrier* attributeCarrier, GNEViewNet* viewNet, const GNEHierarchicalElement* hierarchicalElement);

        /// @brief add in originalSE the values of newSE
        void addValuesFromSubordinatedElements(SubordinatedElements* originalSE, const SubordinatedElements& newSE);

        // @brief open warning dialog
        void openWarningDialog(const std::string& elementType, const size_t number, const bool isChild);

        /// @brief Invalidated copy constructor.
        SubordinatedElements(const SubordinatedElements&) = delete;

        /// @brief Invalidated assignment operator.
        SubordinatedElements& operator=(const SubordinatedElements&) = delete;
    };

    /// @brief check if there is selected ACs to delete
    bool selectedACsToDelete() const;

private:
    /// @brief modul for delete options
    DeleteOptions* myDeleteOptions;
};
