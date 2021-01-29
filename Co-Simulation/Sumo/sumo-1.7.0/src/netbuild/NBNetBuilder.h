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
/// @file    NBNetBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Gregor Laemmel
/// @date    Fri, 29.04.2005
///
// Instance responsible for building networks
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <utils/shapes/ShapeContainer.h>
#include "NBEdgeCont.h"
#include "NBTypeCont.h"
#include "NBNodeCont.h"
#include "NBNode.h"
#include "NBParking.h"
#include "NBTrafficLightLogicCont.h"
#include "NBDistrictCont.h"
#include "NBPTStopCont.h"
#include "NBPTLineCont.h"
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class OutputDevice;
class GeoConvHelper;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBNetBuilder
 * @brief Instance responsible for building networks
 *
 * The class' - and the netbuild module's - functionality is embedded within the
 *  compute() method.
 *
 * @addtogroup netbuild
 * @{
 *
 * -# Removing self loops
 *  \n Removes edges which end at the node they start at using NBNodeCont::removeSelfLoops().
 * -# Joining double connections
 *  \n Joins edges between same nodes using NBNodeCont::recheckEdges().
 * -# Finding isolated roads (optional)
 * -# Removing empty nodes and geometry nodes (optional)
 *  \n Removed nodes with no incoming/outgoing edges and nodes which can be transformed into
 *   geometry point using NBNodeCont::removeUnwishedNodes().
 * -# Removing unwished edges (optional)
 *  \n If "keep-edges.postload" and "keep-edges.explicit" are set, the edges not within "keep-edges.explicit" are
 *   removed from the network using NBEdgeCont::removeUnwishedEdges().
 * -# Rechecking nodes after edge removal (optional)
 *  \n If any of the edge removing options was set ("keep-edges.explicit", "remove-edges.explicit", "keep-edges.postload",
 *   "keep-edges.by-vclass", "keep-edges.input-file"), the now orphaned nodes are removed using
 *   NBNodeCont::removeUnwishedNodes().
 * -# Splitting geometry edges (optional)
 *  \n If "geometry.split" is set, edge geometries are converted to nodes using
 *   NBEdgeCont::splitGeometry().
 * -# Normalising/transposing node positions
 *  \n If "offset.disable-normalization", "offset.x", and "offset.y" are not
 *   set, the road graph's instances are moved to the origin.
 * -# Guessing and setting on-/off-ramps
 * -# Guessing and setting TLs
 * -# Computing turning directions
 * -# Sorting nodes' edges
 * -# Guessing and setting roundabouts
 * -# Computing Approached Edges
 * -# Computing Approaching Lanes
 * -# Dividing of Lanes on Approached Lanes
 * -# Appending Turnarounds (optional)
 * -# Rechecking of lane endings
 * -# Computing node shapes
 * -# Computing edge shapes
 * -# Computing tls logics
 * -# Computing node logics
 * -# Computing traffic light logics
 *
 *  @todo Removing unwished edges: Recheck whether this can be done during loading - whether this option/step is really needed.
 *  @todo Finding isolated roads: Describe
 *  @bug Removing empty nodes and geometry nodes: Ok, empty nodes should be removed, uh? But this is only done if "geometry.remove" is set.
 * @}
 */
class NBNetBuilder {
    friend class GNENet; // for triggering intermediate build steps

public:
    /// @brief Constructor
    NBNetBuilder();

    /// @brief Destructor
    ~NBNetBuilder();

    /** @brief Initialises the storage by applying given options
     *
     * Options, mainly steering the acceptance of edges, are parsed
     *  and the according internal variables are set.
     *
     * @param[in] oc The options container to read options from
     * @exception ProcessError If something fails (message is included)
     */
    void applyOptions(OptionsCont& oc);

    /** @brief Performs the network building steps
     *
     * @param[in] oc Container that contains options for building
     * @param[in] explicitTurnarounds List of edge ids for which turn-arounds should be added (used by NETEDIT)
     * @param[in] mayAddOrRemove whether processing steps which cause nodes and edges to be added or removed shall be triggered (used by netedit)
     * @exception ProcessError (recheck)
     */
    void compute(OptionsCont& oc, const std::set<std::string>& explicitTurnarounds = std::set<std::string>(), bool mayAddOrRemove = true);

    /** @brief Updates the shape for a single Node
     *
     * @param[in] oc Container that contains options for building
     * @param[in] explicitTurnarounds List of edge ids for which turn-arounds should be added (used by NETEDIT)
     * @param[in] mayAddOrRemove whether processing steps which cause nodes and edges to be added or removed shall be triggered (used by netedit)
     * @exception ProcessError (recheck)
     */
    //void computeSingleNode(NBNode* node, OptionsCont& oc, const std::set<std::string>& explicitTurnarounds = std::set<std::string>(), bool mayAddOrRemove = true);

    /// @name Retrieval of subcontainers
    /// @{
    /// @brief Returns a reference to edge container
    NBEdgeCont& getEdgeCont() {
        return myEdgeCont;
    }

    /// @brief Returns a reference to the node container
    NBNodeCont& getNodeCont() {
        return myNodeCont;
    }

    /// @brief Returns a reference to the type container
    NBTypeCont& getTypeCont() {
        return myTypeCont;
    }

    /// @brief Returns a reference to the traffic light logics container
    NBTrafficLightLogicCont& getTLLogicCont() {
        return myTLLCont;
    }

    /// @brief Returns a reference the districts container
    NBDistrictCont& getDistrictCont() {
        return myDistrictCont;
    }


    /// @brief Returns a reference to the pt stop container
    NBPTStopCont& getPTStopCont() {
        return myPTStopCont;
    }

    /// @brief Returns a reference to the pt line container
    NBPTLineCont& getPTLineCont() {
        return myPTLineCont;
    }
    /// @}

    NBParkingCont& getParkingCont() {
        return myParkingCont;
    }

    ShapeContainer& getShapeCont() {
        return myShapeCont;
    }

    /// @brief notify about style of loaded network (Without Crossings)
    bool haveNetworkCrossings() {
        return myNetworkHaveCrossings;
    }

    /// @brief enable crossing in networks
    void setHaveNetworkCrossings(bool value) {
        myNetworkHaveCrossings = value;
    }

    /**
     * @brief transforms loaded coordinates
     * handles projections, offsets (using GeoConvHelper) and import of height data (using NBHeightMapper)
     * @param[in,out] from The coordinate to be transformed
     * @param[in] includeInBoundary Whether to patch the convex boundary of the GeoConvHelper default instance
     * @param[in] from_srs The spatial reference system of the input coordinate
     * @notde These methods are located outside of GeoConvHelper to avoid linker-dependencies on GDAL for libgeom
     */
    static bool transformCoordinate(Position& from, bool includeInBoundary = true, GeoConvHelper* from_srs = 0);
    static bool transformCoordinates(PositionVector& from, bool includeInBoundary = true, GeoConvHelper* from_srs = 0);

    /// @brief insertion geometry points to ensure maximum segment length between points
    static int addGeometrySegments(PositionVector& from, const PositionVector& cartesian, const double maxLength);

    /// @brief whether netbuilding takes place in the context of NETEDIT
    static bool runningNetedit();


protected:
    /**
     * @class by_id_sorter
     * @brief Sorts nodes by their ids
     */
    class by_id_sorter {
    public:
        /// @brief constructor
        explicit by_id_sorter() {}

        /// @brief selection operator
        int operator()(const NBNode* n1, const NBNode* n2) const {
            return n1->getID() < n2->getID();
        }
    };

protected:
    /// @brief The used container for nodes
    NBNodeCont myNodeCont;

    /// @brief The used container for street types
    NBTypeCont myTypeCont;

    /// @brief The used container for edges
    NBEdgeCont myEdgeCont;

    /// @brief The used container for traffic light logics
    NBTrafficLightLogicCont myTLLCont;

    /// @brief The used container for districts
    NBDistrictCont myDistrictCont;

    /// @brief The used container for pt stops
    NBPTStopCont myPTStopCont;

    /// @brief The used container for pt stops
    NBPTLineCont myPTLineCont;

    NBParkingCont myParkingCont;

    /// @brief container for loaded polygon data
    ShapeContainer myShapeCont;

    /// @brief flag to indicate that network has crossings
    bool myNetworkHaveCrossings;

private:
    /// @brief shift network so its lower left corner is at 0,0
    void moveToOrigin(GeoConvHelper& geoConvHelper, bool lefthand);

    /// @brief mirror the network along the X-axis
    void mirrorX();

private:
    /// @brief invalidated copy constructor
    NBNetBuilder(const NBNetBuilder& s);

    /// @brief invalidated assignment operator
    NBNetBuilder& operator=(const NBNetBuilder& s);
};
