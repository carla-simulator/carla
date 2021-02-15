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
/// @file    LayeredRTree.h
/// @author  Jakob Erdmann
/// @date    16.10.2012
///
// A wrapper around RT-trees for for efficient storing of SUMO's GL-objects and
// accessing them ordered by their layer
// Note that we only need two layers at this time: 
// 1 (GLO_LANE, GLO_VEHICLE, GLO_POI)
// 2 all the rest
// The search order returns layer 2 first because it must be drawn before layer
// 1 for alpha blending to work
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/geom/Boundary.h>

#include "SUMORTree.h"


// ===========================================================================
// class definitions
// ===========================================================================
/** @class LayeredRTree
 * @brief A RT-tree for efficient storing of SUMO's GL-objects in layers
 * 
 * This class maintains SUMORTrees for each layer (only 2 at the moment) but
 * provides the same interface as SUMORTree
 */
class LayeredRTree : public SUMORTree {
public:
    /// @brief Constructor
    LayeredRTree() { 
        myLayers.push_back(new SUMORTree());
        myLayers.push_back(new SUMORTree());
    }


    /// @brief Destructor
    ~LayeredRTree() {
        for (std::vector<SUMORTree*>::iterator it = myLayers.begin(); it != myLayers.end(); ++it) {
            delete *it;
        }
        myLayers.clear();
    }


    /** @brief Insert entry (delegate to appropriate layer)
     * @param a_min Min of bounding rect
     * @param a_max Max of bounding rect
     * @param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
     */
    void Insert(const float a_min[2], const float a_max[2], GUIGlObject* const & a_dataId) {
        myLayers[selectLayer(a_dataId)]->Insert(a_min, a_max, a_dataId);
    }


    /** @brief Remove entry (delegate to appropriate layer)
     * @param a_min Min of bounding rect
     * @param a_max Max of bounding rect
     * @param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
     */
    void Remove(const float a_min[2], const float a_max[2], GUIGlObject* const & a_dataId) {
        myLayers[selectLayer(a_dataId)]->Remove(a_min, a_max, a_dataId);
    }

    /** @brief Find all within search rectangle (searches all layers in order)
     * @param a_min Min of search bounding rect
     * @param a_max Max of search bounding rect
     * @param a_searchResult Search result array.  Caller should set grow size. Function will reset, not append to array.
     * @param a_resultCallback Callback function to return result.  Callback should return 'true' to continue searching
     * @param a_context User context to pass as parameter to a_resultCallback
     * @return Returns the number of entries found
     */
    int Search(const float a_min[2], const float a_max[2], const GUIVisualizationSettings& c) const {
        int result = 0;
        for (std::vector<SUMORTree*>::const_iterator it = myLayers.begin(); it != myLayers.end(); ++it) {
            result += (*it)->Search(a_min, a_max, c);
        }
        return result;
    }


protected:
    /// @brief the layers for drawing
    std::vector<SUMORTree*> myLayers;

private:

    /// @brief select the appropriate layer for each object
    inline size_t selectLayer(GUIGlObject* o) {
        switch (o->getType()) {
            case GLO_EDGE:
            case GLO_LANE:
            case GLO_POI:
            case GLO_VEHICLE:
            case GLO_PERSON:
                return 1;
                break;
            default:
                return 0;
        }
    }

};
