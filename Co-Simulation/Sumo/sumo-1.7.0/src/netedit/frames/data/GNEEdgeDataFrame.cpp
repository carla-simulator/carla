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
/// @file    GNEEdgeDataFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// The Widget for add edgeData elements
/****************************************************************************/
#include <config.h>

#include <netedit/elements/data/GNEDataInterval.h>
#include <netedit/elements/data/GNEEdgeData.h>
#include <netedit/elements/data/GNEDataHandler.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/GNEViewNet.h>

#include "GNEEdgeDataFrame.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEEdgeDataFrame::GNEEdgeDataFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEGenericDataFrame(horizontalFrameParent, viewNet, SUMO_TAG_MEANDATA_EDGE, false) {
}


GNEEdgeDataFrame::~GNEEdgeDataFrame() {}


bool
GNEEdgeDataFrame::addEdgeData(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const GNEViewNetHelper::KeyPressed& /*keyPressed*/) {
    // first check if we clicked over an edge
    if (objectsUnderCursor.getEdgeFront() && myDataSetSelector->getDataSet() && myIntervalSelector->getDataInterval()) {
        // first check if the given interval there is already a EdgeData for the given ID
        for (const auto& genericData : myIntervalSelector->getDataInterval()->getGenericDataChildren()) {
            if ((genericData->getTagProperty().getTag() == SUMO_TAG_MEANDATA_EDGE) && (genericData->getParentEdges().front() == objectsUnderCursor.getEdgeFront())) {
                // write warning
                WRITE_WARNING("There is already a " + genericData->getTagStr() + " in edge '" + objectsUnderCursor.getEdgeFront()->getID() + "'");
                // abort edge data creation
                return false;
            }
        }
        // finally create edgeData
        GNEDataHandler::buildEdgeData(myViewNet->getNet(), true, myIntervalSelector->getDataInterval(), objectsUnderCursor.getEdgeFront(), myParametersEditor->getParametersMap());
        // edgeData created, then return true
        return true;
    } else {
        // invalid parent parameters
        return false;
    }
}


/****************************************************************************/
