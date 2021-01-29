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
/// @file    RONetHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The handler that parses a SUMO-network for its usage in a router
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class RONet;
class OptionsCont;
class ROEdge;
class ROAbstractEdgeBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RONetHandler
 * @brief The handler that parses a SUMO-network for its usage in a router
 *
 * SAX2-Handler for SUMO-network loading. As this class is used for both
 *  the dua- and the jtrrouter, a reference to the edge builder is given.
 */
class RONetHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     *
     * @param[in] net The network instance to fill
     * @param[in] eb The abstract edge builder to use
     */
    RONetHandler(RONet& net, ROAbstractEdgeBuilder& eb, const bool ignoreInternal, const double minorPenalty);


    /// @brief Destructor
    virtual ~RONetHandler();

    /// @brief retrieve mapping of edges to bidi edges (must be resolved after loading network)
    const std::map<ROEdge*, std::string>& getBidiMap() const {
        return myBidiEdges;
    }

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    virtual void myStartElement(int element, const SUMOSAXAttributes& attrs);

    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    virtual void myEndElement(int element);
    //@}

protected:
    /// @name called from myStartElement
    //@{

    /// @brief assign arbitrary vehicle parameters
    void addParam(const SUMOSAXAttributes& attrs);

    /** @brief Parses and builds an edge
     *
     * Parses attributes from an "edge"-element (id, from/to-nodes, function, etc.).
     *  If the given nodes are not yet known, they are added to the network.
     *  Uses the internal edge builder to build the edge and adds the edge
     *  to the network.
     *
     * @param[in] attrs The attributes (of the "edge"-element) to parse
     * @todo The edge is "built" first, then the nodes are added; should be done while constructing, probably a legacy issue
     * @todo No exception?
     */
    void parseEdge(const SUMOSAXAttributes& attrs);


    /** @brief Parses and builds a lane
     *
     * Parses attributes from an "lane"-element (speed, length, vehicle classes, etc.).
     *  Builds a ROLane using these attributes (if they are valid) and adds it to the edge.
     *
     * @param[in] attrs The attributes (of the "lane"-element) to parse
     * @todo No exception?
     */
    virtual void parseLane(const SUMOSAXAttributes& attrs);


    /** @brief Parses a junction's position
     *
     * Parses the position of the junction. Sets it to the junction.
     *
     * @param[in] attrs The attributes (of the "lane"-element) to parse
     * @todo In fact, the junction should be built given its position.
     * @todo No exception?
     */
    void parseJunction(const SUMOSAXAttributes& attrs);


    /** @begin Parses a connection
     * Called on the occurence of a "connection" element
     * @param[in] attrs The attributes (of the "connection"-element) to parse
     */
    void parseConnection(const SUMOSAXAttributes& attrs);


    /** @begin Parses a stopping place
     * Called on the occurence of a "busStop", "trainStop" or "containerStop" element
     * @param[in] attrs The attributes to parse
     * @param[in] element which kind of stop is to be parsed
     */
    void parseStoppingPlace(const SUMOSAXAttributes& attrs, const SumoXMLTag element);


    /** @begin Parses an access point to a train stop
     * Called on the occurence of an "access" element
     * @param[in] attrs The attributes to parse
     */
    void parseAccess(const SUMOSAXAttributes& attrs);


    /** @begin Parses a district and creates a pseudo edge for it
     *
     * Called on the occurence of a "district" element, this method
     *  retrieves the id of the district and creates a district type
     *  edge with this id.
     *
     * @param[in] attrs The attributes (of the "district"-element) to parse
     * @exception ProcessError If an edge given in district@edges is not known
     */
    void parseDistrict(const SUMOSAXAttributes& attrs);


    /** @begin Parses a district edge and connects it to the district
     *
     * Called on the occurence of a "dsource" or "dsink" element, this method
     *  retrieves the id of the approachable edge. If this edge is known
     *  and valid, the approaching edge is informed about it (by calling
     *  "ROEdge::addFollower").
     *
     * @param[in] attrs The attributes to parse
     * @param[in] isSource whether a "dsource or a "dsink" was given
     * @todo No exception?
     */
    void parseDistrictEdge(const SUMOSAXAttributes& attrs, bool isSource);

    //@}

    /// Parses network location description
    void setLocation(const SUMOSAXAttributes& attrs);

protected:
    /// @brief The net to store the information into
    RONet& myNet;

    /// @brief the loaded network version
    double myNetworkVersion;

    /// @brief The object used to build of edges of the desired type
    ROAbstractEdgeBuilder& myEdgeBuilder;

    /// @brief whether to ignore junction internal edges
    const bool myIgnoreInternal;

    /// @brief The name of the edge/node that is currently processed
    std::string myCurrentName;

    /// The id of the currently processed edge type
    std::string myCurrentTypeID;

    /// @brief The currently built edge
    ROEdge* myCurrentEdge;

    /// @brief The currently built stopping place
    SUMOVehicleParameter::Stop* myCurrentStoppingPlace;

    /// @brief temporary data for checking node initialisation after network parsing is finished
    std::set<std::string> myUnseenNodeIDs;

    /// @brief time penalty for passing a minor link
    const double myMinorPenalty;

    /// @brief temporary storage for bidi attributes (to be resolved after loading all edges)
    std::map<ROEdge*, std::string> myBidiEdges;

private:
    /// @brief Invalidated copy constructor
    RONetHandler(const RONetHandler& src);

    /// @brief Invalidated assignment operator
    RONetHandler& operator=(const RONetHandler& src);

};
