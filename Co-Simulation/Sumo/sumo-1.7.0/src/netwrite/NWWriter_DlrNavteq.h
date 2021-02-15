/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    NWWriter_DlrNavteq.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    26.10.2012
///
// Exporter writing networks using DlrNavteq (Elmar) format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class NBEdgeCont;
class NBNetBuilder;
class NBNode;
class NBNodeCont;
class NBTrafficLightLogicCont;
class NBTypeCont;
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NWWriter_DlrNavteq
 * @brief Exporter writing networks using XML (native input) format
 *
 */
class NWWriter_DlrNavteq {
public:
    /** @brief Writes the network into XML-files (nodes, edges, connections,
     *   traffic lights)
     * @param[in] oc The options to use
     * @param[in] nb The network builder from which to read data
     */
    static void writeNetwork(const OptionsCont& oc, NBNetBuilder& nb);

    /// @brief get the navteq road class
    static int getRoadClass(NBEdge* edge);

    /// @brief get the navteq brunnel type
    static int getBrunnelType(NBEdge* edge);

    /// @brief get the form of way
    static int getFormOfWay(NBEdge* edge);

private:
    /** @brief Writes the nodes_unsplitted file
     * @param[in] oc The options to use
     * @param[in] nc The node-container from which to read data
     * @param[in] ec The edge-container from which to read data
     * @param[out] internalNodes The internal node ids, generated for edges with complex geometry
     */
    static void writeNodesUnsplitted(const OptionsCont& oc, NBNodeCont& nc, NBEdgeCont& ec, std::map<NBEdge*, std::string>& internalNodes);

    /** @brief Writes the links_unsplitted file
     * @param[in] oc The options to use
     * @param[in] ec The edge-container from which to read data
     * @param[int] internalNodes The internal node ids, generated for edges with complex geometry
     */
    static void writeLinksUnsplitted(const OptionsCont& oc, NBEdgeCont& ec, std::map<NBEdge*, std::string>& internalNodes);

    /** @brief Writes the traffic_signals file
     * @param[in] oc The options to use
     * @param[in] nc The node-container from which to read data
     */
    static void writeTrafficSignals(const OptionsCont& oc, NBNodeCont& nc);


    /** @brief Writes the prohibited_manoeuvres file
     * @param[in] oc The options to use
     * @param[in] nc The node-container from which to read data
     */
    static void writeProhibitedManoeuvres(const OptionsCont& oc, const NBNodeCont& nc, const NBEdgeCont& ec);

    /** @brief Writes the connected_lanes file
     * @param[in] oc The options to use
     * @param[in] nc The node-container from which to read data
     */
    static void writeConnectedLanes(const OptionsCont& oc, NBNodeCont& nc);

    /// @brief write header comments (input paramters, date, etc...)
    static void writeHeader(OutputDevice& device, const OptionsCont& oc);

    /// @brief build the ascii-bit-vector for column vehicle_type
    static std::string getAllowedTypes(SVCPermissions permissions);

    /// @brief get the navteq speed class based on the speed in km/h
    static int getSpeedCategory(int kph);

    /// @brief get the SPEED_LIMIT as defined by elmar (upper bound of speed category)
    static int getSpeedCategoryUpperBound(int kph);

    /// @brief get the lane number encoding
    static int getNavteqLaneCode(const int numLanes);

    /// @brief get the length of the edge when measured up to the junction center
    static double getGraphLength(NBEdge* edge);

    static std::string getSinglePostalCode(const std::string& zipCode, const std::string edgeID);

    /// @brief magic value for undefined stuff
    static const std::string UNDEFINED;

    /// @brief get edge speed rounded to kmh
    static inline int speedInKph(double metersPerSecond) {
        return (int)std::floor(metersPerSecond * 3.6 + 0.5);
    }
};
