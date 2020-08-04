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
/// @file    NIXMLNodesHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Importer for network nodes stored in XML
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/xml/SUMOSAXHandler.h>
#include <utils/geom/Position.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class GeoConvHelper;
class NBNode;
class NBNodeCont;
class NBTrafficLightLogicCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIXMLNodesHandler
 * @brief Importer for network nodes stored in XML
 *
 * This SAX-handler parses node information and stores it in the given node
 *  container. Additionally, the given tls-container may be filled with
 *  additional information.
 */
class NIXMLNodesHandler : public SUMOSAXHandler {

public:
    /** @brief Constructor
     *
     * @param[in, filled] nc The node container to fill
     * @param[in, filled] tlc The traffic lights container to fill
     * @param[in] options The options to use
     * @todo Options are only given to determine whether "flip-y" is set; maybe this should be done by giving a bool
     * @todo Why are options not const?
     */
    NIXMLNodesHandler(NBNodeCont& nc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc,
                      OptionsCont& options);


    /// @brief Destructor
    ~NIXMLNodesHandler();

    /** @brief parses node attributes (not related to positioning)
     */
    static NBNode* processNodeType(const SUMOSAXAttributes& attrs, NBNode* node, const std::string& nodeID, const Position& position,
                                   bool updateEdgeGeometries,
                                   NBNodeCont& nc, NBEdgeCont& ec,
                                   NBTrafficLightLogicCont& tlc);

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * In dependence to the obtained type, an appropriate parsing method is called.
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails (not used herein)
     * @note policy is to throw no exception in order to allow further processing
     * @todo ProcessErrors are thrown when parsing traffic lights!?
     */
    void myStartElement(int element,
                        const SUMOSAXAttributes& attrs);
    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(int element);
    //@}


private:
    /*
     * @brief Parses node information
     * Tries to parse a node. If the node can be parsed, it is stored within
     *  "myNodeCont". Otherwise an error is generated. Then, if given
     *  the tls information is parsed and inserted into "myTLLogicCont".
     */
    void addNode(const SUMOSAXAttributes& attrs);

    /*
     * @brief Parses node deletion information
     */
    void deleteNode(const SUMOSAXAttributes& attrs);

    /*
     * @brief Parses a cluster of nodes to be joined
     */
    void addJoinCluster(const SUMOSAXAttributes& attrs);

    /*
     * @brief Parses a list of nodes to be excluded from joining
     */
    void addJoinExclusion(const SUMOSAXAttributes& attrs);


    /** @brief Builds the defined traffic light or adds a node to it
     *
     * @param[in] attrs Attributes within the currently opened node
     * @param[in] currentNode The built node to add the tls information to
     */
    static void processTrafficLightDefinitions(const SUMOSAXAttributes& attrs,
            NBNode* currentNode, NBTrafficLightLogicCont& tlc);


private:
    /// @brief A reference to the program's options
    OptionsCont& myOptions;

    /// @brief The id of the currently parsed node
    std::string myID;

    /// @brief The position of the currently parsed node
    Position myPosition;

    /// @brief The node container to add built nodes to
    NBNodeCont& myNodeCont;

    /// @brief The node container to add built nodes to
    NBEdgeCont& myEdgeCont;

    /// @brief The traffic lights container to add built tls to
    NBTrafficLightLogicCont& myTLLogicCont;

    /// @brief The coordinate transformation which was used compute the node coordinates
    GeoConvHelper* myLocation;

    /// @brief last item the could receive parameters
    Parameterised* myLastParameterised;

private:
    /** @brief invalid copy constructor */
    NIXMLNodesHandler(const NIXMLNodesHandler& s);

    /** @brief invalid assignment operator */
    NIXMLNodesHandler& operator=(const NIXMLNodesHandler& s);

};
