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
/// @file    GNEPersonPlanFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// The Widget for add PersonPlan elements
/****************************************************************************/
#pragma once

#include <netedit/frames/GNEFrame.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEPersonPlanFrame
 */
class GNEPersonPlanFrame : public GNEFrame {

public:

    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEPersonPlanFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEPersonPlanFrame();

    /// @brief show Frame
    void show();

    /// @brief hide Frame
    void hide();

    /**@brief add person plan element
     * @param objectsUnderCursor collection of objects under cursor after click over view
     * @param keyPressed key pressed during click
     * @return true if element was sucesfully added
     */
    bool addPersonPlanElement(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const GNEViewNetHelper::KeyPressed& keyPressed);

    /// @brief get path creator modul
    GNEFrameModuls::PathCreator* getPathCreator() const;

protected:
    /// @brief Tag selected in TagSelector
    void tagSelected();

    /// @brief selected demand element in DemandElementSelector
    void demandElementSelected();

    /// @brief create path
    void createPath();

private:
    /// @brief Person selectors
    GNEFrameModuls::DemandElementSelector* myPersonSelector;

    /// @brief personPlan selector
    GNEFrameModuls::TagSelector* myPersonPlanTagSelector;

    /// @brief internal vehicle attributes
    GNEFrameAttributesModuls::AttributesCreator* myPersonPlanAttributes;

    /// @brief Path Creator
    GNEFrameModuls::PathCreator* myPathCreator;

    /// @brief Person Hierarchy
    GNEFrameModuls::HierarchicalElementTree* myPersonHierarchy;
};
