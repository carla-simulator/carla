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
/// @file    GNEMoveFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2020
///
// The Widget for move elements
/****************************************************************************/
#include <config.h>

#include <netedit/frames/common/GNEMoveFrame.h>


// ===========================================================================
// method definitions
// ===========================================================================

GNEMoveFrame::GNEMoveFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Move") {
}


GNEMoveFrame::~GNEMoveFrame() {}


void
GNEMoveFrame::processClick(const Position& /*clickedPosition*/,
                           const GNEViewNetHelper::ObjectsUnderCursor& /*objectsUnderCursor*/,
                           const GNEViewNetHelper::ObjectsUnderCursor& /*objectsUnderGrippedCursor*/) {
    // currently unused
}


void
GNEMoveFrame::show() {
    GNEFrame::show();
}


void
GNEMoveFrame::hide() {
    GNEFrame::hide();
}

/****************************************************************************/
