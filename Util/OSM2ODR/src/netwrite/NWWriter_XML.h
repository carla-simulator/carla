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
/// @file    NWWriter_XML.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 11.05.2011
///
// Exporter writing networks using XML (native input) format
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
class NBParkingCont;
class NBPTStopCont;
class NBPTLineCont;
class NBTrafficLightLogicCont;
class NBTypeCont;
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NWWriter_XML
 * @brief Exporter writing networks using XML (native input) format
 *
 */
class NWWriter_XML {
public:
    /** @brief Writes the network into XML-files (nodes, edges, connections,
     *   traffic lights)
     * @param[in] oc The options to use
     * @param[in] nb The network builder from which to read data
     */
    static void writeNetwork(const OptionsCont& oc, NBNetBuilder& nb);

    /** @brief Writes the joined-juncionts to file
     * @param[in] oc The options to use
     * @param[in] nc The node-container from which to read data
     */
    static void writeJoinedJunctions(const OptionsCont& oc, NBNodeCont& nc);

    /** @brief Writes street signs as POIs to file
     * @param[in] oc The options to use
     * @param[in] ec The edge-container from which to read data
     */
    static void writeStreetSigns(const OptionsCont& oc, NBEdgeCont& ec);

private:
    /** @brief Writes the nodes file
     * @param[in] oc The options to use
     * @param[in] nc The node-container from which to read data
     */
    static void writeNodes(const OptionsCont& oc, NBNodeCont& nc);

    /** @brief Writes the types file
     * @param[in] oc The options to use
     * @param[in] nc The type-container from which to read data
     */
    static void writeTypes(const OptionsCont& oc, NBTypeCont& tc);

    /** @brief Writes the edges and connections files
     * @param[in] oc The options to use
     * @param[in] nb The network build from which to read data
     */
    static void writeEdgesAndConnections(const OptionsCont& oc, NBNodeCont& nc, NBEdgeCont& ec);


    /** @brief Writes the traffic lights file
     * @param[in] oc The options to use
     * @param[in] tc The tll-container from which to read data
     * @param[in] ec The edge-container from which to read data
     */
    static void writeTrafficLights(const OptionsCont& oc, NBTrafficLightLogicCont& tc, NBEdgeCont& ec);

    /** @brief Writes the pt stops file
     * @param[in] oc The options to use
     * @param[in] nc The pt stop container from which to read data
     */
    static void writePTStops(const OptionsCont& oc, NBPTStopCont& ec);
    static void writePTLines(const OptionsCont& cont, NBPTLineCont& lc, NBEdgeCont& ec);

    /// @brief writes imported parking areas to file
    static void writeParkingAreas(const OptionsCont& cont, NBParkingCont& pc, NBEdgeCont& ec);

    /// @brief writes imported districts (TAZ) to file
    static void writeDistricts(const OptionsCont& oc, NBDistrictCont& dc);

    static void writeShape(OutputDevice& out, const GeoConvHelper& gch, PositionVector shape, SumoXMLAttr attr, bool useGeo, bool geoAccuracy);
};
