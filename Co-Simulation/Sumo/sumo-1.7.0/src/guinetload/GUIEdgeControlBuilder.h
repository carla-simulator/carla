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
/// @file    GUIEdgeControlBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Derivation of NLEdgeControlBuilder which builds gui-edges
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <netload/NLEdgeControlBuilder.h>
#include <utils/geom/PositionVector.h>
#include <guisim/GUIEdge.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSJunction;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIEdgeControlBuilder
 * @brief Derivation of NLEdgeControlBuilder which builds gui-edges
 *
 * Instead of building pure microsim-objects (MSEdge and MSLane), this class
 *  builds GUIEdges and GUILanes.
 * @see NLEdgeControlBuilder
 */
class GUIEdgeControlBuilder : public NLEdgeControlBuilder {
public:
    /** @brief Constructor
     *
     * @param[in] glObjectIDStorage Storage of gl-ids used to assign new ids to built edges
     */
    GUIEdgeControlBuilder();


    /// @brief Destructor
    ~GUIEdgeControlBuilder();


    /** @brief Builds and adds a lane
     * @param[in] id The lane's id
     * @param[in] maxSpeed The speed allowed on this lane
     * @param[in] length The lane's length
     * @param[in] shape The shape of the lane
     * @param[in] width The width of the lane
     * @param[in] permissions Encoding of vehicle classes that may drive on this lane
     * @param[in] index The index of this lane within its parent edge
     * @see SUMOVehicleClass
     * @see MSLane
     */
    virtual MSLane* addLane(const std::string& id,
                            double maxSpeed, double length,
                            const PositionVector& shape, double width,
                            SVCPermissions permissions,
                            int index, bool isRampAccel,
                            const std::string& type);



    /** @brief Builds an edge instance (GUIEdge in this case)
     *
     * Builds an GUIEdge-instance using the given name and the current index
     *  "myCurrentNumericalEdgeID"
     *  Post-increments the index, returns the built edge.
     *
     * @param[in] id The id of the edge to build
     */
    MSEdge* buildEdge(const std::string& id, const SumoXMLEdgeFunc function,
                      const std::string& streetName, const std::string& edgeType, const int priority, const double distance);


private:
    /// @brief invalidated copy constructor
    GUIEdgeControlBuilder(const GUIEdgeControlBuilder& s);

    /// @brief invalidated assignment operator
    GUIEdgeControlBuilder& operator=(const GUIEdgeControlBuilder& s);

};
