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
/// @file    GNEEdgeRelDataFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Mar 2020
///
// The Widget for add EdgeRelationData elements
/****************************************************************************/
#include <config.h>

#include <netedit/elements/data/GNEDataHandler.h>
#include <netedit/GNEViewNet.h>

#include "GNEEdgeRelDataFrame.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEEdgeRelDataFrame::GNEEdgeRelDataFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEGenericDataFrame(horizontalFrameParent, viewNet, SUMO_TAG_EDGEREL, true) {
}


GNEEdgeRelDataFrame::~GNEEdgeRelDataFrame() {}


bool
GNEEdgeRelDataFrame::addEdgeRelationData(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const GNEViewNetHelper::KeyPressed& keyPressed) {
    // first check if we clicked over an edge
    if (objectsUnderCursor.getEdgeFront() && myDataSetSelector->getDataSet() && myIntervalSelector->getDataInterval()) {
        return myPathCreator->addEdge(objectsUnderCursor.getEdgeFront(), keyPressed.shiftKeyPressed(), keyPressed.controlKeyPressed());
    } else {
        // invalid parent parameters
        return false;
    }
}


void
GNEEdgeRelDataFrame::createPath() {
    // first check that we have at least two edges
    if (myPathCreator->getSelectedEdges().size() > 1) {
        // just create EdgeRelationData
        GNEDataHandler::buildEdgeRelationData(myViewNet->getNet(), true, myIntervalSelector->getDataInterval(), myPathCreator->getSelectedEdges().front(),
                                              myPathCreator->getSelectedEdges().back(), myParametersEditor->getParametersMap());
        // abort path creation
        myPathCreator->abortPathCreation();
    }
}

/****************************************************************************/
