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
/// @file    GNEMoveFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2020
///
// The Widget for move elements
/****************************************************************************/
#pragma once

#include <netedit/frames/GNEFrame.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEMoveFrame
 * The Widget for create edges
 */
class GNEMoveFrame : public GNEFrame {

public:
    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEMoveFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEMoveFrame();

    /**@brief handle processClick and set the relative colouring
     * @param[in] clickedPosition clicked position over ViewNet
     * @param objectsUnderCursor collection of objects under cursor after click over view
     * @param objectsUnderGrippedCursor collection of objects under gripped cursor after click over view
     */
    void processClick(const Position& clickedPosition,
                      const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor,
                      const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderGrippedCursor);

    /// @brief show prohibition frame
    void show();

    /// @brief hide prohibition frame
    void hide();
};
