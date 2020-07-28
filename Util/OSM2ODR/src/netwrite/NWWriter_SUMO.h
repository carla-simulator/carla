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
/// @file    NWWriter_SUMO.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Leonhard Luecken
/// @date    Tue, 04.05.2011
///
// Exporter writing networks using the SUMO format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <netbuild/NBEdge.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>
#include <netbuild/NBConnectionDefs.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class OptionsCont;
class NBNetBuilder;
class NBTrafficLightLogic;
class NBTrafficLightLogicCont;
class NBNode;
class NBDistrict;
class NBEdgeControl;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NWWriter_SUMO
 * @brief Exporter writing networks using the SUMO format
 *
 */
class NWWriter_SUMO {
public:

    enum ConnectionStyle {
        SUMONET, // all connection information
        PLAIN,   // only edges and link indices
        TLL      // like plain but include tl information
    };

    /** @brief Writes the network into a SUMO-file
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void writeNetwork(const OptionsCont& oc, NBNetBuilder& nb);


    /** @brief Writes connections outgoing from the given edge (also used in NWWriter_XML)
     * @param[in] into The device to write the edge into
     * @param[in] from The edge to write connections for
     * @param[in] c The connection to write
     * @param[in] includeInternal Whether information about inner-lanes used to cross the intersection shall be written
     * @param[in] plain Whether only plain-xml output should be written (omit some attributes)
     */
    static void writeConnection(OutputDevice& into, const NBEdge& from, const NBEdge::Connection& c,
                                bool includeInternal, ConnectionStyle style = SUMONET, bool geoAccuracy = false);

    /// @brief writes the given prohibitions
    static void writeProhibitions(OutputDevice& into, const NBConnectionProhibits& prohibitions);

    /// @brief writes the traffic light logics to the given device
    static void writeTrafficLights(OutputDevice& into, const NBTrafficLightLogicCont& tllCont);

    /// @brief writes a single traffic light logic to the given device
    static void writeTrafficLight(OutputDevice& into, const NBTrafficLightLogic* logic);

    /** @brief Writes roundabouts
     * @param[in] into The device to write the edge into
     * @param[in] roundaboutes The roundabouts to write
     * @param[in] ec The edge control to retrieve named edges from
     */
    static void writeRoundabouts(OutputDevice& into, const std::set<EdgeSet>& roundabouts,
                                 const NBEdgeCont& ec);


    /** @brief Write a stopOffset element into output device
     */
    static void writeStopOffsets(OutputDevice& into, const std::map<SVCPermissions, double>& stopOffsets);

    /** @brief Writes a district
     * @param[in] into The device to write the edge into
     * @param[in] d The district
     */
    static void writeDistrict(OutputDevice& into, const NBDistrict& d);


private:
    /// @name Methods for writing network parts
    /// @{

    /** @brief Writes internal edges (<edge ... with id[0]==':') of the given node
     * @param[in] into The device to write the edges into
     * @param[in] n The node to write the edges of
     * @return Whether an internal edge was written
     */
    static bool writeInternalEdges(OutputDevice& into, const NBEdgeCont& ec, const NBNode& n);


    /** @brief Writes an edge (<edge ...)
     * @param[in] into The device to write the edge into
     * @param[in] e The edge to write
     * @param[in] noNames Whether names shall be ignored
     * @see writeLane()
     */
    static void writeEdge(OutputDevice& into, const NBEdge& e, bool noNames);


    /** @brief Writes a lane (<lane ...) of an edge
     * @param[in] into The device to write the edge into
     * @param[in] lID The ID of the lane
     * @param[in] origID The original ID of the edge in the input
     * @param[in] length Lane's length
     * @param[in] index The index of the lane within the edge
     * @param[in] oppositeID The ID of the opposite lane for overtaking
     * @param[in] accelRamp whether this lane is an acceleration lane
     * @param[in] customShape whether this lane has a custom shape
     */
    static void writeLane(OutputDevice& into, const std::string& lID,
                          double speed, SVCPermissions permissions, SVCPermissions preferred,
                          double startOffset, double endOffset,
                          std::map<SVCPermissions, double> stopOffsets, double width, PositionVector shape,
                          const Parameterised* params, double length, int index,
                          const std::string& oppositeID, const std::string& type,
                          bool accelRamp = false,
                          bool customShape = false);


    /** @brief Writes a junction (<junction ...)
     * @param[in] into The device to write the edge into
     * @param[in] n The junction/node to write
     */
    static void writeJunction(OutputDevice& into, const NBNode& n);


    /** @brief Writes internal junctions (<junction with id[0]==':' ...) of the given node
     * @param[in] into The device to write the edge into
     * @param[in] n The junction/node to write internal nodes for
     */
    static bool writeInternalNodes(OutputDevice& into, const NBNode& n);


    /** @brief Writes inner connections within the node
     * @param[in] into The device to write the edge into
     * @param[in] n The node to write inner links for
     */
    static bool writeInternalConnections(OutputDevice& into, const NBNode& n);


    /** @brief Writes a single internal connection
     * @param[in] from The id of the from-edge
     * @param[in] to The id of the to-edge
     * @param[in] toLane The indexd of the to-lane
     * @param[in] via The (optional) via edge
     */
    static void writeInternalConnection(OutputDevice& into,
                                        const std::string& from, const std::string& to,
                                        int fromLane, int toLane, const std::string& via,
                                        LinkDirection dir = LinkDirection::STRAIGHT,
                                        const std::string& tlID = "",
                                        int linkIndex = NBConnection::InvalidTlIndex);

    /// @brief writes a SUMOTime as int if possible, otherwise as a float
    static std::string writeSUMOTime(SUMOTime time);


    /// @brief the attribute value for a prohibition
    static std::string prohibitionConnection(const NBConnection& c);

    /** @brief Writes a roundabout
     * @param[in] into The device to write the edge into
     * @param[in] r The roundabout to write
     * @param[in] ec The edge control to retrieve named edges from
     */
    static void writeRoundabout(OutputDevice& into, const std::vector<std::string>& r,
                                const NBEdgeCont& ec);

    /// @brief retrieve the id of the opposite direction internal lane if it exists
    static std::string getOppositeInternalID(const NBEdgeCont& ec, const NBEdge* from, const NBEdge::Connection& con, double& oppositeLength);

};
